#include "global.h"
#include "event_data.h"
#include "unholy_genesis.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/region_map_sections.h"
#include "test/test.h"

TEST("Genesis Zapdos requires both generator completion and the sixth badge")
{
    u16 oldState = VarGet(VAR_NEW_MAUVILLE_STATE);
    bool8 oldBadge = FlagGet(FLAG_BADGE06_GET);
    bool8 oldCaught = FlagGet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    bool8 oldBadge8 = FlagGet(FLAG_BADGE08_GET);
    FlagClear(FLAG_BADGE08_GET);
    u16 sections[GENESIS_MAP_MARKER_LIMIT];
    FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    FlagClear(FLAG_BADGE06_GET);
    VarSet(VAR_NEW_MAUVILLE_STATE, 1);
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS));
    VarSet(VAR_NEW_MAUVILLE_STATE, 2);
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS));
    FlagSet(FLAG_BADGE06_GET);
    VarSet(VAR_NEW_MAUVILLE_STATE, 1);
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS));
    VarSet(VAR_NEW_MAUVILLE_STATE, 2);
    EXPECT(GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS));
    EXPECT_EQ(GenesisEvent_GetPendingSections(sections, 0), 0);
    EXPECT_EQ(GenesisEvent_GetPendingSections(NULL, 1), 0);
    EXPECT_EQ(GenesisEvent_GetPendingSections(sections, 1), 1);
    EXPECT_EQ(sections[0], MAPSEC_MAUVILLE_CITY);
    FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_ZAPDOS));
    EXPECT_EQ(GenesisEvent_GetPendingSections(sections, GENESIS_MAP_MARKER_LIMIT), 0);
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_COUNT));
    VarSet(VAR_NEW_MAUVILLE_STATE, oldState);
    if (oldBadge8) FlagSet(FLAG_BADGE08_GET); else FlagClear(FLAG_BADGE08_GET);
    if (oldBadge) FlagSet(FLAG_BADGE06_GET); else FlagClear(FLAG_BADGE06_GET);
    if (oldCaught) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
}

TEST("Genesis regional encounter requires original capture and every badge")
{
    bool8 oldBadges[NUM_BADGES];
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        oldBadges[i] = FlagGet(FLAG_BADGE01_GET + i);
        FlagSet(FLAG_BADGE01_GET + i);
    }
    u16 oldState = VarGet(VAR_NEW_MAUVILLE_STATE);
    bool8 oldOriginal = FlagGet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    bool8 oldRegional = FlagGet(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
    VarSet(VAR_NEW_MAUVILLE_STATE, 2);
    FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
    EXPECT(!GenesisEvent_ZapdosGalarAvailable());
    FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        FlagClear(FLAG_BADGE01_GET + i);
        EXPECT(!GenesisEvent_ZapdosGalarAvailable());
        FlagSet(FLAG_BADGE01_GET + i);
    }
    EXPECT(!GenesisEvent_ZapdosAvailable());
    EXPECT(GenesisEvent_ZapdosGalarAvailable());
    u16 sections[GENESIS_MAP_MARKER_LIMIT];
    EXPECT_EQ(GenesisEvent_GetPendingSections(sections, ARRAY_COUNT(sections)), 1);
    EXPECT_EQ(sections[0], MAPSEC_MAUVILLE_CITY);
    FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
    EXPECT(!GenesisEvent_ZapdosGalarAvailable());
    EXPECT_EQ(GenesisEvent_GetPendingSections(sections, ARRAY_COUNT(sections)), 0);
    VarSet(VAR_NEW_MAUVILLE_STATE, oldState);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
    if (oldOriginal) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    if (oldRegional) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
}
