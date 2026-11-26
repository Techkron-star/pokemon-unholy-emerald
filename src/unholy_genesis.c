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
    u16 prerequisiteVar;
    u16 prerequisiteValue;
    u16 completionFlag;
    u16 previousCaptureFlag;
    bool8 requiresAllBadges;
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
    if (event->previousCaptureFlag && !FlagGet(event->previousCaptureFlag))
        return FALSE;
    return !FlagGet(event->completionFlag)
        && (!event->prerequisiteFlag || FlagGet(event->prerequisiteFlag))
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
