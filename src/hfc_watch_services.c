#include "global.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "battle_setup.h"
#include "event_data.h"
#include "follower_npc.h"
#include "item.h"
#include "new_game.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "wild_encounter.h"
#include "constants/flags.h"
#include "constants/battle.h"
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
        .loot = {{ITEM_ULTRA_BALL, 3}, {ITEM_MAX_POTION, 2}, {ITEM_RARE_CANDY, 1}},
    },
    [HFC_RAID_EXPERT] = {
        .name = COMPOUND_STRING("EXPERT"), .minBadges = 7, .level = 47, .bossBoost = 2,
        .minCaptureChance = 5001, .maxCaptureChance = 6500,
        .loot = {{ITEM_ULTRA_BALL, 4}, {ITEM_FULL_RESTORE, 2}, {ITEM_RARE_CANDY, 2}},
    },
    [HFC_RAID_MASTER] = {
        .name = COMPOUND_STRING("MASTER"), .minBadges = 8, .level = 54, .bossBoost = 2,
        .minCaptureChance = 6501, .maxCaptureChance = 8000,
        .loot = {{ITEM_ULTRA_BALL, 5}, {ITEM_FULL_RESTORE, 3}, {ITEM_RARE_CANDY, 3}},
    },
};

static const struct
{
    enum Species species;
    enum Move moves[MAX_MON_MOVES];
} sRaidOpponents[] =
{
    {SPECIES_FLYGON,    {MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_U_TURN}},
    {SPECIES_GARDEVOIR, {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_SHADOW_BALL, MOVE_CALM_MIND}},
    {SPECIES_AGGRON,    {MOVE_IRON_HEAD, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_THUNDER_WAVE}},
    {SPECIES_MILOTIC,   {MOVE_SURF, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_HAZE}},
    {SPECIES_METAGROSS, {MOVE_METEOR_MASH, MOVE_ZEN_HEADBUTT, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE}},
    {SPECIES_SALAMENCE, {MOVE_DRAGON_CLAW, MOVE_FLAMETHROWER, MOVE_CRUNCH, MOVE_DRAGON_DANCE}},
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

struct HfcRaidOpponent HfcWatch_GetRaidOpponent(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty,
                                               enum HfcRaidStage stage, u8 slot)
{
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(difficulty);
    if (info == NULL || (u32)stage > HFC_RAID_BOSS || slot >= (stage == HFC_RAID_BOSS ? 1 : 2))
        return (struct HfcRaidOpponent){0};

    u32 index = (DailyRaidSeed(day, trainerId) % ARRAY_COUNT(sRaidOpponents) + difficulty + 2 * stage + slot)
              % ARRAY_COUNT(sRaidOpponents);
    return (struct HfcRaidOpponent) {
        .species = sRaidOpponents[index].species,
        .level = info->level + (stage == HFC_RAID_BOSS ? 6 : 2 * stage),
        .moves = sRaidOpponents[index].moves,
    };
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
            sRaid.boss = *opponent;
            bossBoost = sRaidDifficulties[sRaid.difficulty].bossBoost;
        }
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
