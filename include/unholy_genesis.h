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
    GENESIS_EVENT_BIRTH_FAUNA,
    GENESIS_EVENT_FARAWAY_FAUNA,
    GENESIS_EVENT_NAVEL_FAUNA,
    GENESIS_EVENT_SOUTHERN_FAUNA,
    GENESIS_EVENT_BEHEEYEM_DISCOVERY,
    GENESIS_EVENT_TADBULB_DISCOVERY,
    GENESIS_EVENT_VAROOM_DISCOVERY,
    GENESIS_EVENT_CRAMORANT_DISCOVERY,
    GENESIS_EVENT_CRYSTAL_GATE,
    GENESIS_EVENT_MEWTWO,
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
bool32 GenesisEvent_CrystalGateAvailable(void);
bool32 GenesisEvent_MewtwoAvailable(void);
u8 GenesisEvent_GetPendingSections(u16 *sections, u8 capacity);

#define GENESIS_STARTER_GENERATIONS 9
#define GENESIS_STARTER_NONE 0xFF
u8 GenesisStarter_SelectGeneration(u16 completedMask, u32 roll);
u16 GenesisStarter_GetSpecies(u8 generation, u8 slot);
bool32 GenesisStarter_LeagueAvailable(void);
u16 GenesisStarter_ClaimLeague(void);
bool32 GenesisStarter_HasCofradiaPrize(void);
void GenesisStarter_QueueCofradiaPrize(void);
u16 GenesisStarter_ClaimCofradia(void);

#endif
