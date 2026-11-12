#include "global.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "battle_setup.h"
#include "event_data.h"
#include "follower_npc.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "wild_encounter.h"
#include "constants/flags.h"
#include "constants/map_types.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/vars.h"

struct HfcRaidOpponent
{
    enum Species species;
    enum Move moves[MAX_MON_MOVES];
};

static const struct HfcRaidOpponent sRaidOpponents[] =
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

struct HfcWatchRaid HfcWatch_GetDailyRaid(u16 day, u32 trainerId, u8 badges)
{
    // The daily offer is stable across maps, menu opens and saving/reloading.
    u32 seed = (day ^ trainerId) * 0x9E3779B9u;
    seed ^= seed >> 16;
    u8 extraBadges = max(4, min(badges, 8)) - 4;
    const struct HfcRaidOpponent *opponent = &sRaidOpponents[seed % ARRAY_COUNT(sRaidOpponents)];
    u16 mutation = 0;

    if ((seed >> 8) % 100 < 20)
        mutation = sRaidMutations[(seed >> 16) % (4 + extraBadges)];

    return (struct HfcWatchRaid) {
        .species = opponent->species,
        .level = 35 + 6 * extraBadges,
        .mutation = mutation,
        .moves = opponent->moves,
    };
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
    if (CountPartyAliveNonEggMonsExcept(PARTY_SIZE) == 0)
        return HFC_RAID_NEEDS_PARTY;
    return HFC_RAID_READY;
}

bool32 HfcWatch_StartDailyRaid(void)
{
    if (HfcWatch_GetRaidStatus() != HFC_RAID_READY)
        return FALSE;

    struct HfcWatchRaid raid = HfcWatch_GetDailyRaid(VarGet(VAR_DAYS), GetTrainerId(gSaveBlock2Ptr->playerTrainerId), HfcWatch_CountBadges());
    struct Pokemon *opponent = &gParties[B_TRAINER_OPPONENT_A][0];

    ZeroEnemyPartyMons();
    CreateRandomMonWithIVs(opponent, raid.species, raid.level, 25);
    for (u8 i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(opponent, raid.moves[i], i);
    SetMonData(opponent, MON_DATA_SPECIAL_BADGES, &raid.mutation);
    CalculateMonStats(opponent);
    FlagSet(FLAG_DAILY_HFC_RAID);
    BattleSetup_StartHfcRaid();
    return TRUE;
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
