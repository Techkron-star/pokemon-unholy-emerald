#include "global.h"
#include "unholy_genesis.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "constants/game_stat.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/region_map_sections.h"

struct GenesisEvent
{
    u16 mapSection;
    u16 prerequisiteFlag;
    u16 storyFlag;
    u16 prerequisiteVar;
    u16 prerequisiteValue;
    u16 completionFlag;
    u16 previousCaptureFlag;
    bool8 requiresAllBadges;
    u8 minBadges;
    bool8 requiresKantoBirds;
};

// Registry of persistent world discoveries. Map scripts own the encounters;
// the regional map reads the same story state without mutating the save.
static const struct GenesisEvent sEvents[GENESIS_EVENT_COUNT] =
{
    [GENESIS_EVENT_ZAPDOS] = {
        .mapSection = MAPSEC_MAUVILLE_CITY,
        .prerequisiteFlag = FLAG_BADGE06_GET,
        .prerequisiteVar = VAR_NEW_MAUVILLE_STATE,
        .prerequisiteValue = 2,
        .completionFlag = FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT,
    },
    [GENESIS_EVENT_ZAPDOS_GALAR] = {
        .mapSection = MAPSEC_MAUVILLE_CITY,
        .prerequisiteFlag = FLAG_BADGE06_GET,
        .prerequisiteVar = VAR_NEW_MAUVILLE_STATE,
        .prerequisiteValue = 2,
        .completionFlag = FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT,
        .previousCaptureFlag = FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT,
        .requiresAllBadges = TRUE,
    },
    [GENESIS_EVENT_MOLTRES] = {
        .mapSection = MAPSEC_MT_CHIMNEY,
        .prerequisiteFlag = FLAG_BADGE04_GET,
        .storyFlag = FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY,
        .completionFlag = FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT,
    },
    [GENESIS_EVENT_MOLTRES_GALAR] = {
        .mapSection = MAPSEC_MT_CHIMNEY,
        .prerequisiteFlag = FLAG_BADGE04_GET,
        .storyFlag = FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY,
        .completionFlag = FLAG_UNHOLY_GENESIS_MOLTRES_GALAR_CAUGHT,
        .previousCaptureFlag = FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT,
        .requiresAllBadges = TRUE,
    },
    [GENESIS_EVENT_ARTICUNO] = {
        .mapSection = MAPSEC_SHOAL_CAVE,
        .prerequisiteFlag = FLAG_BADGE07_GET,
        .completionFlag = FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT,
    },
    [GENESIS_EVENT_ARTICUNO_GALAR] = {
        .mapSection = MAPSEC_SHOAL_CAVE,
        .prerequisiteFlag = FLAG_BADGE07_GET,
        .completionFlag = FLAG_UNHOLY_GENESIS_ARTICUNO_GALAR_CAUGHT,
        .previousCaptureFlag = FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT,
        .requiresAllBadges = TRUE,
    },
    [GENESIS_EVENT_AURORA_TICKET] = {
        .mapSection = MAPSEC_MOSSDEEP_CITY,
        .prerequisiteFlag = FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE,
        .completionFlag = FLAG_ENABLE_SHIP_BIRTH_ISLAND,
        .requiresAllBadges = TRUE,
    },
    [GENESIS_EVENT_DEOXYS] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_BIRTH_ISLAND,
        .completionFlag = FLAG_BATTLED_DEOXYS,
    },
    [GENESIS_EVENT_OLD_SEA_MAP] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_SYS_GAME_CLEAR,
        .completionFlag = FLAG_ENABLE_SHIP_FARAWAY_ISLAND,
    },
    [GENESIS_EVENT_MYSTIC_TICKET] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_SYS_GAME_CLEAR,
        .completionFlag = FLAG_ENABLE_SHIP_NAVEL_ROCK,
        .requiresKantoBirds = TRUE,
    },
    [GENESIS_EVENT_EON_TICKET] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_SYS_GAME_CLEAR,
        .completionFlag = FLAG_ENABLE_SHIP_SOUTHERN_ISLAND,
    },
    [GENESIS_EVENT_BIRTH_FAUNA] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_BIRTH_ISLAND,
        .completionFlag = FLAG_UNHOLY_GENESIS_BIRTH_FAUNA_DISCOVERED,
    },
    [GENESIS_EVENT_FARAWAY_FAUNA] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_FARAWAY_ISLAND,
        .completionFlag = FLAG_UNHOLY_GENESIS_FARAWAY_FAUNA_DISCOVERED,
    },
    [GENESIS_EVENT_NAVEL_FAUNA] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_NAVEL_ROCK,
        .completionFlag = FLAG_UNHOLY_GENESIS_NAVEL_FAUNA_DISCOVERED,
    },
    [GENESIS_EVENT_SOUTHERN_FAUNA] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_SOUTHERN_ISLAND,
        .completionFlag = FLAG_UNHOLY_GENESIS_SOUTHERN_FAUNA_DISCOVERED,
    },
    [GENESIS_EVENT_BEHEEYEM_DISCOVERY] = {
        .mapSection = MAPSEC_LILYCOVE_CITY,
        .prerequisiteFlag = FLAG_ENABLE_SHIP_BIRTH_ISLAND,
        .storyFlag = FLAG_BATTLED_DEOXYS,
        .completionFlag = FLAG_UNHOLY_GENESIS_BEHEEYEM_DISCOVERED,
    },
    [GENESIS_EVENT_TADBULB_DISCOVERY] = {
        .mapSection = MAPSEC_MAUVILLE_CITY,
        .prerequisiteVar = VAR_NEW_MAUVILLE_STATE,
        .prerequisiteValue = 2,
        .completionFlag = FLAG_UNHOLY_GENESIS_TADBULB_DISCOVERED,
    },
    [GENESIS_EVENT_VAROOM_DISCOVERY] = {
        .mapSection = MAPSEC_MAUVILLE_CITY,
        .prerequisiteVar = VAR_NEW_MAUVILLE_STATE,
        .prerequisiteValue = 2,
        .completionFlag = FLAG_UNHOLY_GENESIS_VAROOM_DISCOVERED,
    },
    [GENESIS_EVENT_CRAMORANT_DISCOVERY] = {
        .mapSection = MAPSEC_ROUTE_124,
        .prerequisiteFlag = FLAG_BADGE06_GET,
        .completionFlag = FLAG_UNHOLY_GENESIS_CRAMORANT_DISCOVERED,
    },
    [GENESIS_EVENT_CRYSTAL_GATE] = {
        .mapSection = MAPSEC_ROUTE_115,
        .minBadges = 5,
        .completionFlag = FLAG_GENESIS_CRYSTAL_OPEN,
    },
    [GENESIS_EVENT_MEWTWO] = {
        .mapSection = MAPSEC_ROUTE_115,
        .storyFlag = FLAG_GENESIS_CRYSTAL_OPEN,
        .requiresAllBadges = TRUE,
        .completionFlag = FLAG_GENESIS_MEWTWO_CAUGHT,
    },
};

bool32 GenesisEvent_IsAvailable(enum GenesisEventId id)
{
    if ((u32)id >= GENESIS_EVENT_COUNT)
        return FALSE;
    const struct GenesisEvent *event = &sEvents[id];
    if (event->minBadges)
    {
        u8 count = 0;
        for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
            if (FlagGet(flag))
                count++;
        if (count < event->minBadges)
            return FALSE;
    }
    if (event->requiresAllBadges)
    {
        for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
            if (!FlagGet(flag))
                return FALSE;
    }
    if (event->requiresKantoBirds
     && (!FlagGet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT)
      || !FlagGet(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT)
      || !FlagGet(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT)))
        return FALSE;
    if (event->previousCaptureFlag && !FlagGet(event->previousCaptureFlag))
        return FALSE;
    return !FlagGet(event->completionFlag)
        && (!event->prerequisiteFlag || FlagGet(event->prerequisiteFlag))
        && (!event->storyFlag || FlagGet(event->storyFlag))
        && (!event->prerequisiteVar || VarGet(event->prerequisiteVar) == event->prerequisiteValue);
}

u8 GenesisEvent_GetPendingSections(u16 *sections, u8 capacity)
{
    u8 count = 0;
    if (sections == NULL)
        return 0;
    for (u32 i = 0; i < GENESIS_EVENT_COUNT && count < capacity; i++)
    {
        if (!GenesisEvent_IsAvailable(i))
            continue;
        u16 section = sEvents[i].mapSection;
        bool32 duplicate = FALSE;
        for (u8 j = 0; j < count; j++)
            if (sections[j] == section)
                duplicate = TRUE;
        if (!duplicate)
            sections[count++] = section;
    }
    return count;
}

// Script bridge: no duplicate prerequisite logic in map scripts.
bool32 GenesisEvent_ZapdosAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS);
}

bool32 GenesisEvent_ZapdosGalarAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS_GALAR);
}

bool32 GenesisEvent_MoltresAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_MOLTRES);
}

bool32 GenesisEvent_MoltresGalarAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_MOLTRES_GALAR);
}

bool32 GenesisEvent_ArticunoAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_ARTICUNO);
}

bool32 GenesisEvent_ArticunoGalarAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_ARTICUNO_GALAR);
}

bool32 GenesisEvent_AuroraTicketAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_AURORA_TICKET);
}

bool32 GenesisEvent_DeoxysAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_DEOXYS);
}

bool32 GenesisEvent_OldSeaMapAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_OLD_SEA_MAP);
}

bool32 GenesisEvent_MysticTicketAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_MYSTIC_TICKET);
}

bool32 GenesisEvent_EonTicketAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_EON_TICKET);
}

static const u16 sGenesisStarters[GENESIS_STARTER_GENERATIONS][3] =
{
    {SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE},
    {SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE},
    {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP},
    {SPECIES_TURTWIG, SPECIES_CHIMCHAR, SPECIES_PIPLUP},
    {SPECIES_SNIVY, SPECIES_TEPIG, SPECIES_OSHAWOTT},
    {SPECIES_CHESPIN, SPECIES_FENNEKIN, SPECIES_FROAKIE},
    {SPECIES_ROWLET, SPECIES_LITTEN, SPECIES_POPPLIO},
    {SPECIES_GROOKEY, SPECIES_SCORBUNNY, SPECIES_SOBBLE},
    {SPECIES_SPRIGATITO, SPECIES_FUECOCO, SPECIES_QUAXLY},
};

u16 GenesisStarter_GetSpecies(u8 generation, u8 slot)
{
    if (generation >= GENESIS_STARTER_GENERATIONS || slot >= 3)
        return SPECIES_NONE;
    return sGenesisStarters[generation][slot];
}

// Pure selector, uniform among generations not yet completed.
u8 GenesisStarter_SelectGeneration(u16 completedMask, u32 roll)
{
    u8 count = 0;
    for (u8 i = 0; i < GENESIS_STARTER_GENERATIONS; i++)
        if (!(completedMask & (1u << i)))
            count++;
    if (!count)
        return GENESIS_STARTER_NONE;
    roll %= count;
    for (u8 i = 0; i < GENESIS_STARTER_GENERATIONS; i++)
        if (!(completedMask & (1u << i)))
        {
            if (!roll)
                return i;
            roll--;
        }
    return GENESIS_STARTER_NONE;
}

static u8 StarterReadBits(u16 first, u8 count)
{
    u8 value = 0;
    for (u8 i = 0; i < count; i++)
        if (FlagGet(first + i))
            value |= 1u << i;
    return value;
}

static void StarterWriteBits(u16 first, u8 count, u8 value)
{
    for (u8 i = 0; i < count; i++)
        if (value & (1u << i))
            FlagSet(first + i);
        else
            FlagClear(first + i);
}

static u16 StarterCompletedMask(void)
{
    u16 mask = 0;
    for (u8 i = 0; i < GENESIS_STARTER_GENERATIONS; i++)
        if (FlagGet(FLAG_GENESIS_STARTER_GEN1 + i))
            mask |= 1u << i;
    return mask;
}

bool32 GenesisStarter_LeagueAvailable(void)
{
    if (!FlagGet(FLAG_SYS_GAME_CLEAR))
        return FALSE;
    u8 pending = StarterReadBits(FLAG_GENESIS_STARTER_PENDING_GEN, 4);
    if (pending > 0 && pending <= GENESIS_STARTER_GENERATIONS)
        return TRUE;
    u16 mask = StarterCompletedMask();
    u8 completed = 0;
    for (u8 i = 0; i < GENESIS_STARTER_GENERATIONS; i++)
        if (mask & (1u << i))
            completed++;
    return completed < GENESIS_STARTER_GENERATIONS
        && completed < max(1u, GetGameStat(GAME_STAT_ENTERED_HOF));
}

static bool32 StarterDeliver(u16 species)
{
    struct Pokemon mon;
    if (species == SPECIES_NONE)
        return FALSE;
    CreateMon(&mon, species, 5, Random32(), OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&mon);
    return GiveScriptedMonToPlayer(&mon, PARTY_SIZE) != MON_CANT_GIVE;
}

// 0 = nothing due, 1 = complete trio, 2 = storage full (progress preserved).
u16 GenesisStarter_ClaimLeague(void)
{
    if (!GenesisStarter_LeagueAvailable())
        return 0;
    u8 pending = StarterReadBits(FLAG_GENESIS_STARTER_PENDING_GEN, 4);
    if (!pending || pending > GENESIS_STARTER_GENERATIONS)
    {
        u8 generation = GenesisStarter_SelectGeneration(StarterCompletedMask(), Random32());
        if (generation == GENESIS_STARTER_NONE)
            return 0;
        pending = generation + 1;
        StarterWriteBits(FLAG_GENESIS_STARTER_PENDING_GEN, 4, pending);
        StarterWriteBits(FLAG_GENESIS_STARTER_PROGRESS, 2, 0);
    }
    u8 progress = StarterReadBits(FLAG_GENESIS_STARTER_PROGRESS, 2);
    while (progress < 3)
    {
        if (!StarterDeliver(GenesisStarter_GetSpecies(pending - 1, progress)))
            return 2;
        StarterWriteBits(FLAG_GENESIS_STARTER_PROGRESS, 2, ++progress);
    }
    FlagSet(FLAG_GENESIS_STARTER_GEN1 + pending - 1);
    StarterWriteBits(FLAG_GENESIS_STARTER_PROGRESS, 2, 0);
    StarterWriteBits(FLAG_GENESIS_STARTER_PENDING_GEN, 4, 0);
    return 1;
}

bool32 GenesisStarter_HasCofradiaPrize(void)
{
    u8 prize = StarterReadBits(FLAG_GENESIS_COFRADIA_PRIZE, 5);
    return prize > 0 && prize <= GENESIS_STARTER_GENERATIONS * 3;
}

void GenesisStarter_QueueCofradiaPrize(void)
{
    if (!GenesisStarter_HasCofradiaPrize())
        StarterWriteBits(FLAG_GENESIS_COFRADIA_PRIZE, 5,
                         1 + Random32() % (GENESIS_STARTER_GENERATIONS * 3));
}

u16 GenesisStarter_ClaimCofradia(void)
{
    if (!GenesisStarter_HasCofradiaPrize())
        return SPECIES_NONE;
    u8 index = StarterReadBits(FLAG_GENESIS_COFRADIA_PRIZE, 5) - 1;
    u16 species = GenesisStarter_GetSpecies(index / 3, index % 3);
    if (!StarterDeliver(species))
        return SPECIES_NONE;
    StarterWriteBits(FLAG_GENESIS_COFRADIA_PRIZE, 5, 0);
    return species;
}

bool32 GenesisEvent_CrystalGateAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_CRYSTAL_GATE);
}

bool32 GenesisEvent_MewtwoAvailable(void)
{
    return GenesisEvent_IsAvailable(GENESIS_EVENT_MEWTWO);
}
