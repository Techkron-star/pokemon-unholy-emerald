#include "global.h"
#include "unholy_genesis.h"
#include "event_data.h"
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
};

bool32 GenesisEvent_IsAvailable(enum GenesisEventId id)
{
    if ((u32)id >= GENESIS_EVENT_COUNT)
        return FALSE;
    const struct GenesisEvent *event = &sEvents[id];
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
