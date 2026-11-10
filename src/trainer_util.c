#include "global.h"
#include "main.h"
#include "data.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "pokemon.h"
#include "config/special_badges.h"
#include "constants/flags.h"
#include "constants/trainers.h"
#include "move.h"
#include "random.h"
#include "string_util.h"
#include "trainer_util.h"
#include "text.h"

#include "constants/battle_ai.h"
#include "constants/pokeball.h"

rng_value_t GeneratePartySeed(const struct Trainer *trainer)
{
    u32 seed = Crc32B((const u8 *)trainer, sizeof(struct Trainer)) ^ READ_OTID_FROM_SAVE;
    return LocalRandomSeed(seed);
}


// Stable trainer-mutation RNG: it NEVER consumes the global battle or encounter RNG.
// Derive every member's roll from the saved player OTID, the trainer's stable name,
// its evolution family and the occurrence within that family, not its party slot.
static u32 MixTrainerMutationSeed(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    value ^= value >> 16;
    return value;
}

static bool8 IsImportantMutationTrainer(u32 trainerClass)
{
    switch (trainerClass)
    {
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_LEADER_FRLG:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_ELITE_FOUR_FRLG:
    case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_CHAMPION_FRLG:
    case TRAINER_CLASS_RIVAL:
    case TRAINER_CLASS_RIVAL_EARLY_FRLG:
    case TRAINER_CLASS_RIVAL_LATE_FRLG:
    case TRAINER_CLASS_AQUA_LEADER:
    case TRAINER_CLASS_MAGMA_LEADER:
    case TRAINER_CLASS_AQUA_ADMIN:
    case TRAINER_CLASS_MAGMA_ADMIN:
        return TRUE;
    default:
        return FALSE;
    }
}

static u32 GetPersistentMutationTrainerSeed(const struct Trainer *trainer)
{
    u32 seed = 2166136261u ^ READ_OTID_FROM_SAVE;
    u32 i;

    seed = MixTrainerMutationSeed(seed ^ trainer->trainerClass);
    for (i = 0; i < TRAINER_NAME_LENGTH && trainer->trainerName[i] != EOS && trainer->trainerName[i] != 0; i++)
        seed = (seed ^ trainer->trainerName[i]) * 16777619u;

    return MixTrainerMutationSeed(seed);
}

static enum Species GetMutationFamily(enum Species species)
{
    u32 depth;

    species = GET_BASE_SPECIES_ID(species);
    // A bounded walk also protects us against malformed evolution tables.
    for (depth = 0; depth < 5; depth++)
    {
        enum Species previous = GetSpeciesPreEvolution(species);
        if (previous == SPECIES_NONE || previous == species)
            break;
        species = previous;
    }
    return species;
}

static u8 MutationCount(u8 mask)
{
    u8 n = 0;

    while (mask != 0)
    {
        n += mask & 1;
        mask >>= 1;
    }
    return n;
}

static u8 GetGymRematchMutationTier(const struct TrainerGenerator *trainer)
{
    u8 i, count = 0;

    if (trainer->trainerClass != TRAINER_CLASS_LEADER
     && trainer->trainerClass != TRAINER_CLASS_LEADER_FRLG)
        return 0;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || !TRAINER_BATTLE_PARAM.isRematch)
        return 0;

    for (i = 0; i < NUM_BADGES; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            count++;

    return count;
}

// A deterministic rematch tier can be replayed without obtaining new rolls.
// Earlier tiers are replayed as a prefix, so existing badges never disappear
// when the player earns the next badge. This uses no new save layout fields.
static u8 RollStableTrainerMutations(u32 seed, bool8 important, u8 rematchTier)
{
    static const u8 sMutationFlags[] =
    {
        POKEMON_BADGE_ALPHA,
        POKEMON_BADGE_SUPER,
        POKEMON_BADGE_ENRAGED,
        POKEMON_BADGE_OMEGA,
    };
    u8 result = 0;
    u8 tier;

    for (tier = 0; tier <= rematchTier; tier++)
    {
        rng_value_t rng = LocalRandomSeed(MixTrainerMutationSeed(seed ^ (0x9E3779B9u * (tier + 1))));
        u8 rates[] =
        {
            important ? SPECIAL_BADGE_TRAINER_ALPHA_CHANCE : SPECIAL_BADGE_WILD_ALPHA_CHANCE,
            important ? SPECIAL_BADGE_TRAINER_SUPER_CHANCE : SPECIAL_BADGE_WILD_SUPER_CHANCE,
            important ? SPECIAL_BADGE_TRAINER_ENRAGED_CHANCE : SPECIAL_BADGE_WILD_ENRAGED_CHANCE,
            important ? SPECIAL_BADGE_TRAINER_OMEGA_CHANCE : 0,
        };
        u8 alreadyHad = MutationCount(result);
        u8 candidates = 0;
        u8 i;

        // A gym rematch doubles the chance of a FIRST mutation if none
        // was present; with one existing mutation it doubles the gate
        // for a SECOND instead. Three or more are never boosted.
        if (tier != 0 && alreadyHad == 0)
        {
            for (i = 0; i < ARRAY_COUNT(rates); i++)
                rates[i] = min(100, rates[i] * 2);
        }

        for (i = 0; i < ARRAY_COUNT(sMutationFlags); i++)
        {
            if (rates[i] != 0 && LocalRandom32(&rng) % 100 < rates[i]
             && !(result & sMutationFlags[i]))
                candidates |= sMutationFlags[i];
        }

        while (candidates != 0)
        {
            u8 choice = LocalRandom32(&rng) % MutationCount(candidates);
            u8 flag = 0;
            u32 gate;
            u8 accepted = MutationCount(result);

            for (i = 0; i < ARRAY_COUNT(sMutationFlags); i++)
            {
                if (candidates & sMutationFlags[i])
                {
                    if (choice-- == 0)
                    {
                        flag = sMutationFlags[i];
                        break;
                    }
                }
            }

            candidates &= ~flag;
            // Multi-mutants stay exceptionally rare in natural encounters.
            gate = accepted == 0 ? 1 : accepted == 1 ? 12 : accepted == 2 ? 40 : 100;
            if (tier != 0 && alreadyHad == 1 && accepted == 1)
                gate = 6; // 2x chance for a second mutation on rematches.

            if (LocalRandom32(&rng) % gate == 0)
                result |= flag;
        }
    }

    return result;
}

static void AssignStableTrainerMutations(struct Pokemon *mon, const struct TrainerMon *entry, struct TrainerGenerator *trainer)
{
    u32 occurrence = 0;
    enum Species family = GetMutationFamily(entry->species);
    u8 i, mask;
    u32 seed;

    for (i = 0; i < trainer->generatedMutationCount; i++)
        if (trainer->mutationLineages[i] == family)
            occurrence++;

    if (trainer->generatedMutationCount < ARRAY_COUNT(trainer->mutationLineages))
        trainer->mutationLineages[trainer->generatedMutationCount++] = family;

    seed = MixTrainerMutationSeed(trainer->mutationSeed
                               ^ (u32)family * 0x85EBCA6Bu
                               ^ (occurrence + 1) * 0xC2B2AE35u);
    mask = RollStableTrainerMutations(seed, trainer->isImportantTrainer, GetGymRematchMutationTier(trainer));
    for (i = 0; i < 4; i++)
        if (mask & (1 << i))
            Pokemon_SetBadge(mon, 1 << i);
}

static void InitTrainerMutationIdentity(struct TrainerGenerator *trainerGen, const struct Trainer *trainer)
{
    trainerGen->mutationSeed = GetPersistentMutationTrainerSeed(trainer);
    trainerGen->generatedMutationCount = 0;
    memset(trainerGen->mutationLineages, 0, sizeof(trainerGen->mutationLineages));
    trainerGen->isImportantTrainer = IsImportantMutationTrainer(trainer->trainerClass);
}

static void CustomTrainerPartyAssignMoves(struct Pokemon *mon, const struct TrainerMon *partyEntry)
{
    bool32 noMoveSet = TRUE;
    u32 j;

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        if (partyEntry->moves[j] != MOVE_NONE)
            noMoveSet = FALSE;
    }
    if (noMoveSet)
    {
        GiveMonInitialMoveset(mon);
        // TODO: Figure out a default strategy when moves are not set, to generate a good moveset
        return;
    }

    for (j = 0; j < MAX_MON_MOVES; ++j)
    {
        u32 pp = GetMovePP(partyEntry->moves[j]);
        SetMonData(mon, MON_DATA_MOVE1 + j, &partyEntry->moves[j]);
        SetMonData(mon, MON_DATA_PP1 + j, &pp);
    }
}

u32 GeneratePersonalityForGender(u32 gender, u32 species, u32 genderRng)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    if (gender == MON_MALE)
    {
        assertf(speciesInfo->genderRatio < MON_FEMALE, "species %d cannot be male", species);
        return speciesInfo->genderRatio + (genderRng % (256 - speciesInfo->genderRatio));
    }
    if (gender == MON_FEMALE)
    {
        assertf(speciesInfo->genderRatio != MON_MALE && speciesInfo->genderRatio != MON_GENDERLESS, "species %d cannot be female", species);
        return genderRng % speciesInfo->genderRatio;
    }
    if (gender == MON_GENDERLESS)
        assertf(speciesInfo->genderRatio == MON_GENDERLESS, "species %d cannot be genderless", species);
    else
        errorf("GeneratePersonalityForGender called with invalid gender value %d", gender);
    return 0;
}

const u8 sModuloLUT[25] = {0, 21, 17, 13, 9, 5, 1, 22, 18, 14, 10, 6, 2, 23, 19, 15, 11, 7, 3, 24, 20, 16, 12, 8, 4};

void ModifyPersonalityForNature(u32 *personality, s32 newNature)
{
    s32 nature = GetNatureFromPersonality(*personality);
    s32 diff = abs(newNature - nature);
    s32 sign = (newNature > nature) ? 1 : -1;
    if (diff > NUM_NATURES / 2)
    {
        diff = NUM_NATURES - diff;
        sign *= -1;
    }
    *personality += (sModuloLUT[diff] * 0x100 * sign);
}

bool32 SetCorrectAbilityNum(struct Pokemon *mon, enum Species species, enum Ability ability)
{
    const struct SpeciesInfo *speciesInfo = &gSpeciesInfo[species];
    u32 abilityNum;
    u32 maxAbilityNum = ARRAY_COUNT(speciesInfo->abilities);
    for (abilityNum = 0; abilityNum < maxAbilityNum; ++abilityNum)
    {
        if (speciesInfo->abilities[abilityNum] == ability)
            break;
    }
    assertf(abilityNum < maxAbilityNum, "illegal ability %S for %S", gAbilitiesInfo[ability].name, speciesInfo->speciesName)
    {
        return FALSE;
    }
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
    return TRUE;
}

void MakeTrainerGenerator(struct TrainerGenerator *trainerGen, const struct Trainer *trainer)
{
    trainerGen->gender = trainer->gender;
    if (trainer->aiFlags & AI_FLAG_SMART_TERA)
        trainerGen->smartTera = TRUE;
    trainerGen->isFrontier = FALSE;
    StringCopyN(trainerGen->name, trainer->trainerName, TRAINER_NAME_LENGTH + 1);
    trainerGen->trainerClass = trainer->trainerClass;
    trainerGen->otID = OTID_STRUCT_RANDOM_NO_SHINY;
    trainerGen->localRngState = GeneratePartySeed(trainer);
    InitTrainerMutationIdentity(trainerGen, trainer);
}

void MakePartnerGenerator(struct TrainerGenerator *trainerGen, const struct Trainer *partner)
{
    u32 otID;
    trainerGen->gender = partner->gender;
    if (partner->aiFlags & AI_FLAG_SMART_TERA)
        trainerGen->smartTera = TRUE;
    trainerGen->isFrontier = FALSE;
    StringCopyN(trainerGen->name, partner->trainerName, TRAINER_NAME_LENGTH + 1);
    trainerGen->trainerClass = partner->trainerClass;
    otID = Crc32B((const u8 *)partner, sizeof(struct Trainer));
    trainerGen->otID = OTID_STRUCT_PRESET(otID);
    trainerGen->localRngState = LocalRandomSeed(otID);
    InitTrainerMutationIdentity(trainerGen, partner);
}

void GenerateMonFromTrainerMon(struct Pokemon *mon, const struct TrainerMon *trainerMon, struct TrainerGenerator *trainer)
{
    u32 data;
    u32 personality = (LocalRandom32(&trainer->localRngState) & 0xFFFFDF00) + 0x1000;
    u32 genderValue = LocalRandom32(&trainer->localRngState);
    if (trainerMon->gender == TRAINER_MON_RANDOM_GENDER)
        genderValue = genderValue & 0x000000FF;
    else if (trainerMon->gender == TRAINER_MON_MALE)
        genderValue = GeneratePersonalityForGender(MON_MALE, trainerMon->species, genderValue);
    else if (trainerMon->gender == TRAINER_MON_FEMALE)
        genderValue = GeneratePersonalityForGender(MON_FEMALE, trainerMon->species, genderValue);
    else
        errorf("Unkwown trainer mon gender value %d", trainerMon->gender);
    personality |= genderValue;
    ModifyPersonalityForNature(&personality, trainerMon->nature);
    CreateMon(mon, trainerMon->species, trainerMon->lvl, personality, trainer->otID);
    AssignStableTrainerMutations(mon, trainerMon, trainer);
    if (trainerMon->nickname != NULL)
        SetMonData(mon, MON_DATA_NICKNAME, trainerMon->nickname);
    if (trainerMon->ev) //ev in struct TrainerMon are stored in Showdown order not vanilla Emerald order
    {
        SetMonData(mon, MON_DATA_HP_EV, &trainerMon->ev[0]);
        SetMonData(mon, MON_DATA_ATK_EV, &trainerMon->ev[1]);
        SetMonData(mon, MON_DATA_DEF_EV, &trainerMon->ev[2]);
        SetMonData(mon, MON_DATA_SPATK_EV, &trainerMon->ev[3]);
        SetMonData(mon, MON_DATA_SPDEF_EV, &trainerMon->ev[4]);
        SetMonData(mon, MON_DATA_SPEED_EV, &trainerMon->ev[5]);
    }

    SetMonData(mon, MON_DATA_IVS, &trainerMon->iv);
    CustomTrainerPartyAssignMoves(mon, trainerMon);
    SetMonData(mon, MON_DATA_HELD_ITEM, &trainerMon->heldItem);

    bool32 abilitySet = FALSE;
    if (trainerMon->ability)
    {
        abilitySet = SetCorrectAbilityNum(mon, trainerMon->species, trainerMon->ability);
    }

    if (!abilitySet)
    {
        if (B_TRAINER_MON_RANDOM_ABILITY == 2)
        {
            do {
                data = Random() % NUM_ABILITY_SLOTS; // includes hidden abilities
            } while (GetAbilityBySpecies(trainerMon->species, data) == ABILITY_NONE);
            SetMonData(mon, MON_DATA_ABILITY_NUM, &data);
        }
        else if (B_TRAINER_MON_RANDOM_ABILITY == 0)
        {
            data = 0;
            SetMonData(mon, MON_DATA_ABILITY_NUM, &data);
        }
        // else B_TRAINER_MON_RANDOM_ABILITY == 1
        // this is the default from CreateMon (random non-hidden ability based on personality)
    }

    if (trainerMon->ball < POKEBALL_COUNT)
    {
        data = trainerMon->ball;
        SetMonData(mon, MON_DATA_POKEBALL, &data);
    }
    else if (B_TRAINER_CLASS_POKE_BALLS >= GEN_7 && trainer->trainerClass && trainerMon->ball == POKEBALL_COUNT)
    {
        data = gTrainerClasses[trainer->trainerClass].ball ?: BALL_POKE;
        SetMonData(mon, MON_DATA_POKEBALL, &data);
    }
    else if (trainerMon->ball > POKEBALL_COUNT)
    {
        errorf("Invalid ball for %S in %S's party", GetMonData(mon, MON_DATA_NICKNAME), trainer->name);
    }

    SetMonData(mon, MON_DATA_FRIENDSHIP, &trainerMon->friendship);

    data = trainerMon->isShiny;
    SetMonData(mon, MON_DATA_IS_SHINY, &data);
    if (trainerMon->shouldUseDynamax)
    {
        data = trainerMon->dynamaxLevel;
    }
    else
    {
        data = BLOCK_AI_DYNAMAX;
    }
    SetMonData(mon, MON_DATA_DYNAMAX_LEVEL, &data);
    if (trainerMon->gigantamaxFactor)
    {
        data = trainerMon->gigantamaxFactor;
        SetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR, &data);
    }
    if (trainerMon->teraType)
    {
        data = trainerMon->teraType;
        SetMonData(mon, MON_DATA_TERA_TYPE, &data);
    }
    else if (!trainer->smartTera)
    {
        data = TYPE_MYSTERY;
        SetMonData(mon, MON_DATA_TERA_TYPE, &data);
    }

    CalculateMonStats(mon);
    SetMonData(mon, MON_DATA_OT_NAME, trainer->name);
    data = trainer->gender;
    SetMonData(mon, MON_DATA_OT_GENDER, &data);
}
