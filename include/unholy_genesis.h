#ifndef GUARD_UNHOLY_GENESIS_H
#define GUARD_UNHOLY_GENESIS_H

#define GENESIS_MAP_MARKER_LIMIT 8

enum GenesisEventId
{
    GENESIS_EVENT_ZAPDOS,
    GENESIS_EVENT_ZAPDOS_GALAR,
    GENESIS_EVENT_MOLTRES,
    GENESIS_EVENT_MOLTRES_GALAR,
    GENESIS_EVENT_ARTICUNO,
    GENESIS_EVENT_ARTICUNO_GALAR,
    GENESIS_EVENT_AURORA_TICKET,
    GENESIS_EVENT_DEOXYS,
    GENESIS_EVENT_OLD_SEA_MAP,
    GENESIS_EVENT_MYSTIC_TICKET,
    GENESIS_EVENT_EON_TICKET,
    GENESIS_EVENT_COUNT
};

bool32 GenesisEvent_IsAvailable(enum GenesisEventId id);
bool32 GenesisEvent_ZapdosAvailable(void);
bool32 GenesisEvent_ZapdosGalarAvailable(void);
bool32 GenesisEvent_MoltresAvailable(void);
bool32 GenesisEvent_MoltresGalarAvailable(void);
bool32 GenesisEvent_ArticunoAvailable(void);
bool32 GenesisEvent_ArticunoGalarAvailable(void);
bool32 GenesisEvent_AuroraTicketAvailable(void);
bool32 GenesisEvent_DeoxysAvailable(void);
bool32 GenesisEvent_OldSeaMapAvailable(void);
bool32 GenesisEvent_MysticTicketAvailable(void);
bool32 GenesisEvent_EonTicketAvailable(void);
u8 GenesisEvent_GetPendingSections(u16 *sections, u8 capacity);

#endif
