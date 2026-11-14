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

enum
{
    HFC_RAID_POOL_SIZE = 20,
    HFC_RAID_OPPONENT_COUNT = 2 * HFC_RAID_BOSS + 1,
};

#include "data/hfc_raid_opponents.h"

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

static u32 RaidRosterSeed(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty)
{
    return DailyRaidSeed(day, trainerId) ^ ((difficulty + 1) * 0x27D4EB2Du);
}

static bool32 BuildRaidRoster(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty,
                             u8 roster[HFC_RAID_OPPONENT_COUNT])
{
    const struct HfcRaidSpecies *pool = sRaidOpponents[difficulty];
    u8 eligible[HFC_RAID_POOL_SIZE];
    u8 count = 0;

    for (u8 i = 0; i < ARRAY_COUNT(eligible); i++)
        if (pool[i].species != SPECIES_NONE && IsSpeciesEnabled(pool[i].species))
            eligible[count++] = i;
    if (count < HFC_RAID_OPPONENT_COUNT)
        return FALSE;

    // A fixed shuffle per trainer and difficulty gives each boss one day per cycle.
    u32 rotationSeed = RaidRosterSeed(0, trainerId, difficulty);
    for (u8 i = count - 1; i > 0; i--)
    {
        u8 chosen = NextRaidRoll(&rotationSeed, i + 1);
        u8 swap;
        SWAP(eligible[i], eligible[chosen], swap);
    }

    u8 bossIndex = day % count;
    roster[HFC_RAID_OPPONENT_COUNT - 1] = eligible[bossIndex];
    eligible[bossIndex] = eligible[--count];

    u32 daySeed = RaidRosterSeed(day, trainerId, difficulty);
    for (u8 i = 0; i < HFC_RAID_OPPONENT_COUNT - 1; i++)
    {
        u8 chosen = NextRaidRoll(&daySeed, count);
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
    u8 roster[HFC_RAID_OPPONENT_COUNT];

    if (info == NULL || (u32)stage > HFC_RAID_BOSS || slot >= slots)
        return (struct HfcRaidOpponent){0};

    if (!BuildRaidRoster(day, trainerId, difficulty, roster))
        return (struct HfcRaidOpponent){0};

    const struct HfcRaidSpecies *pool = sRaidOpponents[difficulty];
    const u8 *stageRoster = &roster[2 * stage];
    const struct HfcRaidSpecies *entry = &pool[stageRoster[slot]];
    u32 stageRoll = RaidRosterSeed(day, trainerId, difficulty) ^ ((stage + 1) * 0x165667B1u);
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
        if (!isBoss && !HasCompatibleMegaStone(&pool[stageRoster[candidate]]))
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
    u16 day = VarGet(VAR_DAYS);
    u32 trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    for (enum HfcRaidDifficulty tier = HFC_RAID_STANDARD; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
    {
        if (HfcWatch_IsRaidDifficultyUnlocked(tier, progress.badges)
         && HfcWatch_GetRaidOpponent(day, trainerId, tier, HFC_RAID_BOSS, 0).species != SPECIES_NONE)
            return HFC_RAID_READY;
    }
    return HFC_RAID_UNAVAILABLE;
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

    u16 day = VarGet(VAR_DAYS);
    u32 trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    if (HfcWatch_GetRaidOpponent(day, trainerId, difficulty, HFC_RAID_BOSS, 0).species == SPECIES_NONE)
        return FALSE;

    sRaid.difficulty = difficulty;
    sRaid.stage = HFC_RAID_FIRST_DOUBLE;
    sRaid.day = day;
    sRaid.captureChance = HfcWatch_GetRaidCaptureChance(day, trainerId, difficulty);
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

u8 HfcWatch_GetActiveFrequencyBoost(void)
{
    return HfcWatch_GetFrequencyRemaining() != 0 ? VarGet(VAR_HFC_FREQUENCY_CHARGE) >> 8 : 0;
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
