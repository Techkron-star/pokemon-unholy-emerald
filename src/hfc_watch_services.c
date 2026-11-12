#include "global.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "battle_setup.h"
#include "event_data.h"
#include "follower_npc.h"
#include "gear.h"
#include "item.h"
#include "new_game.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "wild_encounter.h"
#include "constants/flags.h"
#include "constants/battle.h"
#include "constants/form_change_types.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/vars.h"

static const struct HfcRaidDifficultyInfo sRaidDifficulties[HFC_RAID_DIFFICULTY_COUNT] =
{
    [HFC_RAID_STANDARD] = {
        .name = COMPOUND_STRING("STANDARD"), .minBadges = 5, .level = 34, .bossBoost = 1,
        .minCaptureChance = 2000, .maxCaptureChance = 3500,
        .loot = {{ITEM_GREAT_BALL, 3}, {ITEM_HYPER_POTION, 2}, {ITEM_RARE_CANDY, 1}},
    },
    [HFC_RAID_ADVANCED] = {
        .name = COMPOUND_STRING("ADVANCED"), .minBadges = 6, .level = 40, .bossBoost = 1,
        .minCaptureChance = 3501, .maxCaptureChance = 5000,
        .doubleItemChance = 50,
        .loot = {{ITEM_ULTRA_BALL, 3}, {ITEM_MAX_POTION, 2}, {ITEM_RARE_CANDY, 1}},
    },
    [HFC_RAID_EXPERT] = {
        .name = COMPOUND_STRING("EXPERT"), .minBadges = 7, .level = 47, .bossBoost = 2,
        .minCaptureChance = 5001, .maxCaptureChance = 6500,
        .doubleItemChance = 100, .doubleGearChance = 50, .doubleMegaChance = 30, .bossMegaChance = 55,
        .loot = {{ITEM_ULTRA_BALL, 4}, {ITEM_FULL_RESTORE, 2}, {ITEM_RARE_CANDY, 2}},
    },
    [HFC_RAID_MASTER] = {
        .name = COMPOUND_STRING("MASTER"), .minBadges = 8, .level = 54, .bossBoost = 2,
        .minCaptureChance = 6501, .maxCaptureChance = 8000,
        .doubleItemChance = 100, .doubleGearChance = 80, .doubleMegaChance = 65, .bossMegaChance = 100,
        .loot = {{ITEM_ULTRA_BALL, 5}, {ITEM_FULL_RESTORE, 3}, {ITEM_RARE_CANDY, 3}},
    },
};

struct HfcRaidSpecies
{
    enum Species species;
    enum Move moves[MAX_MON_MOVES];
    enum Item heldItem;
    enum Item megaStone;
    enum Gear gear;
};

static const struct HfcRaidSpecies sRaidOpponents[] =
{
    {SPECIES_FLYGON,    {MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_U_TURN},         ITEM_SOFT_SAND,       ITEM_NONE,           GEAR_CORAZON_TITAN},
    {SPECIES_GARDEVOIR, {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_SHADOW_BALL, MOVE_CALM_MIND},          ITEM_SITRUS_BERRY,    ITEM_GARDEVOIRITE,    GEAR_VISOR_COSMICO},
    {SPECIES_AGGRON,    {MOVE_IRON_HEAD, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_THUNDER_WAVE},      ITEM_SHUCA_BERRY,     ITEM_AGGRONITE,       GEAR_CORAZON_TITAN},
    {SPECIES_MILOTIC,   {MOVE_SURF, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_HAZE},                       ITEM_LEFTOVERS,       ITEM_NONE,           GEAR_PERMAFROST_FRAGMENT},
    {SPECIES_METAGROSS, {MOVE_METEOR_MASH, MOVE_ZEN_HEADBUTT, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE}, ITEM_OCCA_BERRY,      ITEM_METAGROSSITE,    GEAR_CORAZON_TITAN},
    {SPECIES_SALAMENCE, {MOVE_DRAGON_CLAW, MOVE_FLAMETHROWER, MOVE_CRUNCH, MOVE_DRAGON_DANCE},     ITEM_YACHE_BERRY,     ITEM_SALAMENCITE,     GEAR_CORAZON_TITAN},
    {SPECIES_MANECTRIC, {MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_SNARL, MOVE_VOLT_SWITCH},       ITEM_MAGNET,          ITEM_MANECTITE,       GEAR_VISOR_COSMICO},
    {SPECIES_CAMERUPT,  {MOVE_HEAT_WAVE, MOVE_EARTH_POWER, MOVE_ROCK_SLIDE, MOVE_YAWN},            ITEM_PASSHO_BERRY,    ITEM_CAMERUPTITE,     GEAR_VISOR_COSMICO},
    {SPECIES_ABSOL,     {MOVE_NIGHT_SLASH, MOVE_SUCKER_PUNCH, MOVE_PSYCHO_CUT, MOVE_SWORDS_DANCE}, ITEM_BLACK_GLASSES,   ITEM_ABSOLITE,        GEAR_SKI_MASK},
    {SPECIES_ALTARIA,   {MOVE_DRAGON_PULSE, MOVE_HYPER_VOICE, MOVE_FLAMETHROWER, MOVE_ROOST},      ITEM_YACHE_BERRY,     ITEM_ALTARIANITE,     GEAR_VISOR_COSMICO},
    {SPECIES_SHARPEDO,  {MOVE_CRUNCH, MOVE_LIQUIDATION, MOVE_ICE_FANG, MOVE_PROTECT},              ITEM_MYSTIC_WATER,    ITEM_SHARPEDONITE,    GEAR_CETRO_SINIESTRO},
    {SPECIES_GLALIE,    {MOVE_ICE_BEAM, MOVE_FREEZE_DRY, MOVE_SHADOW_BALL, MOVE_PROTECT},          ITEM_NEVER_MELT_ICE,  ITEM_GLALITITE,       GEAR_PERMAFROST_FRAGMENT},
    {SPECIES_HERACROSS, {MOVE_MEGAHORN, MOVE_CLOSE_COMBAT, MOVE_ROCK_BLAST, MOVE_PIN_MISSILE},     ITEM_SITRUS_BERRY,    ITEM_HERACRONITE,     GEAR_CORAZON_TITAN},
    {SPECIES_SKARMORY,  {MOVE_DRILL_PECK, MOVE_IRON_HEAD, MOVE_ROOST, MOVE_TAILWIND},              ITEM_ROCKY_HELMET,    ITEM_NONE,           GEAR_CORAZON_TITAN},
    {SPECIES_SCIZOR,    {MOVE_BULLET_PUNCH, MOVE_X_SCISSOR, MOVE_CLOSE_COMBAT, MOVE_SWORDS_DANCE},  ITEM_METAL_COAT,      ITEM_SCIZORITE,       GEAR_CORAZON_TITAN},
    {SPECIES_HOUNDOOM,  {MOVE_HEAT_WAVE, MOVE_DARK_PULSE, MOVE_SLUDGE_BOMB, MOVE_NASTY_PLOT},      ITEM_CHARCOAL,        ITEM_HOUNDOOMINITE,   GEAR_SKI_MASK},
    {SPECIES_BRELOOM,   {MOVE_MACH_PUNCH, MOVE_SEED_BOMB, MOVE_SPORE, MOVE_ROCK_TOMB},             ITEM_SITRUS_BERRY,    ITEM_NONE,           GEAR_CORAZON_TITAN},
    {SPECIES_WALREIN,   {MOVE_ICE_BEAM, MOVE_SURF, MOVE_ICY_WIND, MOVE_PROTECT},                   ITEM_LEFTOVERS,       ITEM_NONE,           GEAR_PERMAFROST_FRAGMENT},
    {SPECIES_LUDICOLO,  {MOVE_GIGA_DRAIN, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_RAIN_DANCE},        ITEM_MIRACLE_SEED,    ITEM_NONE,           GEAR_VISOR_COSMICO},
    {SPECIES_CLAYDOL,   {MOVE_EARTH_POWER, MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_REFLECT},          ITEM_LIGHT_CLAY,      ITEM_NONE,           GEAR_IMPULSO_COSMICO},
};

static const u16 sRaidMutations[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER, POKEMON_BADGE_OVERLOADED,
    POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL, POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED,
};

static EWRAM_DATA struct
{
    enum HfcRaidState state;
    enum HfcRaidStage stage;
    enum HfcRaidDifficulty difficulty;
    u16 day;
    u16 captureChance;
    struct Pokemon boss;
} sRaid;

const struct HfcRaidDifficultyInfo *HfcWatch_GetRaidDifficultyInfo(enum HfcRaidDifficulty difficulty)
{
    if ((u32)difficulty >= HFC_RAID_DIFFICULTY_COUNT)
        return NULL;
    return &sRaidDifficulties[difficulty];
}

bool32 HfcWatch_IsRaidDifficultyUnlocked(enum HfcRaidDifficulty difficulty, u8 badges)
{
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(difficulty);
    return info != NULL && badges >= info->minBadges;
}

static u32 DailyRaidSeed(u16 day, u32 trainerId)
{
    u32 seed = (day ^ trainerId) * 0x9E3779B9u;
    return seed ^ (seed >> 16);
}

u16 HfcWatch_GetRaidCaptureChance(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty)
{
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(difficulty);
    if (info == NULL)
        return 0;

    // Each daily offer keeps the displayed odds through all three rounds.
    u32 seed = DailyRaidSeed(day, trainerId) ^ (0x85EBCA6Bu * (difficulty + 1));
    return info->minCaptureChance + seed % (info->maxCaptureChance - info->minCaptureChance + 1);
}

static u32 NextRaidRoll(u32 *seed, u32 limit)
{
    *seed = *seed * 1664525u + 1013904223u;
    return (*seed >> 8) % limit;
}

static bool32 BuildRaidRoster(u32 seed, u8 *roster, u8 slots)
{
    u8 eligible[ARRAY_COUNT(sRaidOpponents)];
    u8 count = 0;

    for (u8 i = 0; i < ARRAY_COUNT(sRaidOpponents); i++)
        if (IsSpeciesEnabled(sRaidOpponents[i].species))
            eligible[count++] = i;
    if (count < slots)
        return FALSE;

    for (u8 i = 0; i < slots; i++)
    {
        u8 chosen = NextRaidRoll(&seed, count);
        roster[i] = eligible[chosen];
        eligible[chosen] = eligible[--count];
    }
    return TRUE;
}

static bool32 HasCompatibleMegaStone(const struct HfcRaidSpecies *entry)
{
    const struct FormChange *form = GetSpeciesFormChanges(entry->species);

    if (entry->megaStone == ITEM_NONE || form == NULL)
        return FALSE;

    for (; form->method != FORM_CHANGE_TERMINATOR; form++)
        if (form->method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM
         && form->param1 == entry->megaStone && IsSpeciesEnabled(form->targetSpecies))
            return TRUE;
    return FALSE;
}

struct HfcRaidOpponent HfcWatch_GetRaidOpponent(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty,
                                               enum HfcRaidStage stage, u8 slot)
{
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(difficulty);
    bool32 isBoss = stage == HFC_RAID_BOSS;
    u8 slots = isBoss ? 1 : 2;
    u8 roster[2 * HFC_RAID_BOSS + 1];

    if (info == NULL || (u32)stage > HFC_RAID_BOSS || slot >= slots)
        return (struct HfcRaidOpponent){0};

    u32 seed = DailyRaidSeed(day, trainerId) ^ ((difficulty + 1) * 0x27D4EB2Du);
    if (!BuildRaidRoster(seed, roster, ARRAY_COUNT(roster)))
        return (struct HfcRaidOpponent){0};

    const u8 *stageRoster = &roster[2 * stage];
    const struct HfcRaidSpecies *entry = &sRaidOpponents[stageRoster[slot]];
    u32 stageRoll = seed ^ ((stage + 1) * 0x165667B1u);
    u32 slotRoll = stageRoll ^ ((slot + 1) * 0x85EBCA6Bu);
    u8 itemChance = isBoss ? (difficulty >= HFC_RAID_ADVANCED ? 100 : 0) : info->doubleItemChance;
    u8 gearChance = isBoss ? (difficulty >= HFC_RAID_EXPERT ? 100 : 0) : info->doubleGearChance;
    u8 megaChance = isBoss ? info->bossMegaChance : info->doubleMegaChance;
    bool32 hasItem = NextRaidRoll(&slotRoll, 100) < itemChance;
    bool32 hasGear = NextRaidRoll(&slotRoll, 100) < gearChance;

    struct HfcRaidOpponent opponent =
    {
        .species = entry->species,
        .level = info->level + (isBoss ? 6 : 2 * stage),
        .moves = entry->moves,
        .heldItem = hasItem ? entry->heldItem : ITEM_NONE,
        .gear = hasGear ? entry->gear : GEAR_NONE,
    };

    // Both slots compute the same Mega candidate. Each double can have one Mega.
    if (NextRaidRoll(&stageRoll, 100) < megaChance)
    {
        u8 candidate = NextRaidRoll(&stageRoll, slots);
        if (!isBoss && !HasCompatibleMegaStone(&sRaidOpponents[stageRoster[candidate]]))
            candidate ^= 1;
        if (candidate == slot && HasCompatibleMegaStone(entry))
        {
            opponent.heldItem = entry->megaStone;
            opponent.mega = TRUE;
        }
    }
    return opponent;
}

u16 HfcWatch_GetRaidMutation(enum HfcRaidDifficulty difficulty, u32 roll)
{
    static const u8 sPoolSizes[HFC_RAID_DIFFICULTY_COUNT] = {4, 6, 7, 8};
    if ((u32)difficulty >= HFC_RAID_DIFFICULTY_COUNT)
        return 0;
    return sRaidMutations[roll % sPoolSizes[difficulty]];
}

enum HfcWatchRaidStatus HfcWatch_GetRaidStatus(void)
{
    enum MapType mapType = GetCurrentMapType();
    struct HfcWatchProgress progress = { .badges = HfcWatch_CountBadges() };

    if (HfcWatch_GetAccess(HfcWatch_GetTier(), HFC_WATCH_FEATURE_RAID, progress) != HFC_WATCH_READY
     || !FlagGet(FLAG_SYS_CLOCK_SET) || IsNPCFollowerWildBattle())
        return HFC_RAID_UNAVAILABLE;
    if (mapType != MAP_TYPE_ROUTE && mapType != MAP_TYPE_OCEAN_ROUTE)
        return HFC_RAID_NOT_ON_ROUTE;
    if (FlagGet(FLAG_DAILY_HFC_RAID))
        return HFC_RAID_USED_TODAY;
    if (CountPartyAliveNonEggMonsExcept(PARTY_SIZE) < 2)
        return HFC_RAID_NEEDS_PARTY;
    if (HfcWatch_GetRaidOpponent(VarGet(VAR_DAYS), GetTrainerId(gSaveBlock2Ptr->playerTrainerId),
                               HFC_RAID_STANDARD, HFC_RAID_BOSS, 0).species == SPECIES_NONE)
        return HFC_RAID_UNAVAILABLE;
    return HFC_RAID_READY;
}

enum HfcRaidState HfcWatch_GetRaidState(void)
{
    return sRaid.state;
}

enum HfcRaidStage HfcWatch_GetRaidStage(void)
{
    return sRaid.stage;
}

enum HfcRaidDifficulty HfcWatch_GetActiveRaidDifficulty(void)
{
    return sRaid.difficulty;
}

u16 HfcWatch_GetActiveRaidCaptureChance(void)
{
    return sRaid.captureChance;
}

static void StartRaidStage(void)
{
    bool32 isBoss = sRaid.stage == HFC_RAID_BOSS;
    u8 bossBoost = 0;
    u32 trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);

    ZeroEnemyPartyMons();
    for (u8 slot = 0; slot < (isBoss ? 1 : 2); slot++)
    {
        struct HfcRaidOpponent entry = HfcWatch_GetRaidOpponent(sRaid.day, trainerId, sRaid.difficulty, sRaid.stage, slot);
        struct Pokemon *opponent = &gParties[B_TRAINER_OPPONENT_A][slot];

        CreateRandomMonWithIVs(opponent, entry.species, entry.level, isBoss ? 25 : 18 + 2 * sRaid.difficulty);
        for (u8 move = 0; move < MAX_MON_MOVES; move++)
            SetMonMoveSlot(opponent, entry.moves[move], move);
        if (isBoss)
        {
            u16 mutation = HfcWatch_GetRaidMutation(sRaid.difficulty, Random32());
            SetMonData(opponent, MON_DATA_SPECIAL_BADGES, &mutation);
            CalculateMonStats(opponent);
            // Save the permanent Pokémon before lending the encounter equipment.
            sRaid.boss = *opponent;
            bossBoost = sRaidDifficulties[sRaid.difficulty].bossBoost;
        }
        SetMonData(opponent, MON_DATA_HELD_ITEM, &entry.heldItem);
        Gear_SetEquipped(opponent, entry.gear);
    }

    sRaid.state = HFC_RAID_FIGHTING;
    BattleSetup_StartHfcRaid(!isBoss, bossBoost);
}

bool32 HfcWatch_StartDailyRaid(enum HfcRaidDifficulty difficulty)
{
    if (sRaid.state != HFC_RAID_IDLE || HfcWatch_GetRaidStatus() != HFC_RAID_READY
     || !HfcWatch_IsRaidDifficultyUnlocked(difficulty, HfcWatch_CountBadges()))
        return FALSE;

    sRaid.difficulty = difficulty;
    sRaid.stage = HFC_RAID_FIRST_DOUBLE;
    sRaid.day = VarGet(VAR_DAYS);
    sRaid.captureChance = HfcWatch_GetRaidCaptureChance(sRaid.day, GetTrainerId(gSaveBlock2Ptr->playerTrainerId), difficulty);
    FlagSet(FLAG_DAILY_HFC_RAID);
    StartRaidStage();
    return TRUE;
}

bool32 HfcWatch_ContinueRaid(void)
{
    if (sRaid.state != HFC_RAID_NEXT_BATTLE)
        return FALSE;

    u8 required = sRaid.stage == HFC_RAID_BOSS ? 1 : 2;
    if (CountPartyAliveNonEggMonsExcept(PARTY_SIZE) < required)
    {
        sRaid.state = HFC_RAID_ENDED;
        return FALSE;
    }

    StartRaidStage();
    return TRUE;
}

void HfcWatch_OnRaidBattleEnd(u8 outcome)
{
    if (sRaid.state != HFC_RAID_FIGHTING)
        return;

    if (outcome != B_OUTCOME_WON)
        sRaid.state = HFC_RAID_ENDED;
    else if (sRaid.stage == HFC_RAID_BOSS)
        sRaid.state = HFC_RAID_CHOOSE_REWARD;
    else
    {
        sRaid.stage++;
        sRaid.state = HFC_RAID_NEXT_BATTLE;
    }
}

void HfcWatch_ResetRaid(void)
{
    sRaid.state = HFC_RAID_IDLE;
}

enum HfcRaidRewardError HfcWatch_ClaimRaidReward(bool32 capture)
{
    if (sRaid.state != HFC_RAID_CHOOSE_REWARD)
        return HFC_RAID_REWARD_OK;

    if (capture)
    {
        if (IsPokemonStorageFull())
            return HFC_RAID_REWARD_STORAGE_FULL;

        if (Random32() % HFC_RAID_CHANCE_SCALE >= sRaid.captureChance)
            sRaid.state = HFC_RAID_CAPTURE_FAILED;
        else
        {
            if (GiveCapturedMonToPlayer(&sRaid.boss) == MON_CANT_GIVE)
                return HFC_RAID_REWARD_STORAGE_FULL;
            HandleSetPokedexFlagFromMon(&sRaid.boss, FLAG_SET_SEEN);
            HandleSetPokedexFlagFromMon(&sRaid.boss, FLAG_SET_CAUGHT);
            sRaid.state = HFC_RAID_CAPTURED;
        }
    }
    else
    {
        const struct HfcRaidLoot *loot = sRaidDifficulties[sRaid.difficulty].loot;

        for (u8 i = 0; i < HFC_RAID_LOOT_COUNT; i++)
        {
            if (!AddBagItem(loot[i].item, loot[i].quantity))
            {
                while (i > 0)
                {
                    i--;
                    RemoveBagItem(loot[i].item, loot[i].quantity);
                }
                return HFC_RAID_REWARD_BAG_FULL;
            }
        }
        sRaid.state = HFC_RAID_LOOT_RECEIVED;
    }
    return HFC_RAID_REWARD_OK;
}

static u16 CurrentFrequencyMap(void)
{
    return ((u8)gSaveBlock1Ptr->location.mapGroup << 8) | (u8)gSaveBlock1Ptr->location.mapNum;
}

u8 HfcWatch_GetFrequencyBoost(u8 badges)
{
    if (badges < 3)
        return 0;
    return 25 + 5 * (min(badges, 8) - 3);
}

bool32 HfcWatch_CanActivateFrequency(void)
{
    struct HfcWatchProgress progress = { .badges = HfcWatch_CountBadges() };

    return HfcWatch_GetAccess(HfcWatch_GetTier(), HFC_WATCH_FEATURE_FREQUENCY, progress) == HFC_WATCH_READY
        && FlagGet(FLAG_SYS_CLOCK_SET)
        && !FlagGet(FLAG_DAILY_HFC_FREQUENCY)
        && !MapHasNoEncounterData();
}

bool32 HfcWatch_ActivateFrequency(void)
{
    if (!HfcWatch_CanActivateFrequency())
        return FALSE;

    u8 boost = HfcWatch_GetFrequencyBoost(HfcWatch_CountBadges());
    VarSet(VAR_HFC_FREQUENCY_MAP, CurrentFrequencyMap());
    VarSet(VAR_HFC_FREQUENCY_CHARGE, (boost << 8) | HFC_FREQUENCY_ENCOUNTERS);
    FlagSet(FLAG_DAILY_HFC_FREQUENCY);
    return TRUE;
}

u8 HfcWatch_GetFrequencyRemaining(void)
{
    if (!FlagGet(FLAG_DAILY_HFC_FREQUENCY) || VarGet(VAR_HFC_FREQUENCY_MAP) != CurrentFrequencyMap())
        return 0;
    return (u8)VarGet(VAR_HFC_FREQUENCY_CHARGE);
}

u8 HfcWatch_ConsumeFrequencyEncounter(void)
{
    u8 remaining = HfcWatch_GetFrequencyRemaining();
    if (remaining == 0)
        return 0;

    u16 charge = VarGet(VAR_HFC_FREQUENCY_CHARGE);
    VarSet(VAR_HFC_FREQUENCY_CHARGE, charge - 1);
    return charge >> 8;
}

static u16 ReadMonMutations(struct BoxPokemon *mon)
{
    if (!GetBoxMonData(mon, MON_DATA_SANITY_HAS_SPECIES)
     || GetBoxMonData(mon, MON_DATA_SANITY_IS_BAD_EGG)
     || GetBoxMonData(mon, MON_DATA_IS_EGG))
        return 0;
    return GetBoxMonData(mon, MON_DATA_SPECIAL_BADGES) & POKEMON_BADGES_ALL;
}

void HfcWatch_RegisterMutation(struct BoxPokemon *mon)
{
    u16 mutations = ReadMonMutations(mon);

    if (mutations != 0)
        VarSet(VAR_HFC_ARCHIVE_MUTATIONS, VarGet(VAR_HFC_ARCHIVE_MUTATIONS) | mutations);
}

u16 HfcWatch_ReadArchive(void)
{
    u16 registered = VarGet(VAR_HFC_ARCHIVE_MUTATIONS) & POKEMON_BADGES_ALL;

    for (u8 i = 0; i < PARTY_SIZE && registered != POKEMON_BADGES_ALL; i++)
        registered |= ReadMonMutations(&gParties[B_TRAINER_PLAYER][i].box);

    // Import existing collections so the archive also works on older saves.
    for (u8 box = 0; box < TOTAL_BOXES_COUNT && registered != POKEMON_BADGES_ALL; box++)
        for (u8 slot = 0; slot < IN_BOX_COUNT && registered != POKEMON_BADGES_ALL; slot++)
            registered |= ReadMonMutations(GetBoxedMonPtr(box, slot));

    VarSet(VAR_HFC_ARCHIVE_MUTATIONS, registered);
    return registered;
}
