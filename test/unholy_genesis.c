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
    if (oldBadge) FlagSet(FLAG_BADGE06_GET); else FlagClear(FLAG_BADGE06_GET);
    if (oldCaught) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
}
