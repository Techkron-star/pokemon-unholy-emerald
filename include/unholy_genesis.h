#ifndef GUARD_UNHOLY_GENESIS_H
#define GUARD_UNHOLY_GENESIS_H

#define GENESIS_MAP_MARKER_LIMIT 8

enum GenesisEventId
{
    GENESIS_EVENT_ZAPDOS,
    GENESIS_EVENT_ZAPDOS_GALAR,
    GENESIS_EVENT_COUNT
};

bool32 GenesisEvent_IsAvailable(enum GenesisEventId id);
bool32 GenesisEvent_ZapdosAvailable(void);
bool32 GenesisEvent_ZapdosGalarAvailable(void);
u8 GenesisEvent_GetPendingSections(u16 *sections, u8 capacity);

#endif
