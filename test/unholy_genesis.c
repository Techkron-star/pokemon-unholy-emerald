#include "global.h"
#include "event_data.h"
#include "unholy_genesis.h"
#include "wild_encounter.h"
#include "constants/maps.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/region_map_sections.h"
#include "test/test.h"

static bool32 HasPendingSection(u16 section)
{
    u16 sections[GENESIS_MAP_MARKER_LIMIT];
    u8 count = GenesisEvent_GetPendingSections(sections, ARRAY_COUNT(sections));
    for (u8 i = 0; i < count; i++)
        if (sections[i] == section)
            return TRUE;
    return FALSE;
}

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
    EXPECT_EQ(HasPendingSection(MAPSEC_MAUVILLE_CITY), GenesisEvent_IsAvailable(GENESIS_EVENT_TADBULB_DISCOVERY) || GenesisEvent_IsAvailable(GENESIS_EVENT_VAROOM_DISCOVERY));
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
    EXPECT(HasPendingSection(MAPSEC_MAUVILLE_CITY));
    FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
    EXPECT(!GenesisEvent_ZapdosGalarAvailable());
    EXPECT_EQ(HasPendingSection(MAPSEC_MAUVILLE_CITY), GenesisEvent_IsAvailable(GENESIS_EVENT_TADBULB_DISCOVERY) || GenesisEvent_IsAvailable(GENESIS_EVENT_VAROOM_DISCOVERY));
    VarSet(VAR_NEW_MAUVILLE_STATE, oldState);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
    if (oldOriginal) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT);
    if (oldRegional) FlagSet(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ZAPDOS_GALAR_CAUGHT);
}

TEST("Genesis Moltres requires fourth badge and the volcano story")
{
    bool8 badge = FlagGet(FLAG_BADGE04_GET);
    bool8 story = FlagGet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    bool8 caught = FlagGet(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT);
    FlagClear(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT);
    FlagClear(FLAG_BADGE04_GET);
    FlagSet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    EXPECT(!GenesisEvent_MoltresAvailable());
    FlagSet(FLAG_BADGE04_GET);
    FlagClear(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    EXPECT(!GenesisEvent_MoltresAvailable());
    FlagSet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    EXPECT(GenesisEvent_MoltresAvailable());
    EXPECT(HasPendingSection(MAPSEC_MT_CHIMNEY));
    if (badge) FlagSet(FLAG_BADGE04_GET); else FlagClear(FLAG_BADGE04_GET);
    if (story) FlagSet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY); else FlagClear(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    if (caught) FlagSet(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT);
}

TEST("Genesis Articuno requires seventh badge")
{
    bool8 badge = FlagGet(FLAG_BADGE07_GET);
    bool8 caught = FlagGet(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT);
    FlagClear(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT);
    FlagClear(FLAG_BADGE07_GET);
    EXPECT(!GenesisEvent_ArticunoAvailable());
    FlagSet(FLAG_BADGE07_GET);
    EXPECT(GenesisEvent_ArticunoAvailable());
    EXPECT(HasPendingSection(MAPSEC_SHOAL_CAVE));
    if (badge) FlagSet(FLAG_BADGE07_GET); else FlagClear(FLAG_BADGE07_GET);
    if (caught) FlagSet(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT); else FlagClear(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT);
}

TEST("Genesis regional Articuno and Moltres require original captures and all badges")
{
    bool8 oldBadges[NUM_BADGES];
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        oldBadges[i] = FlagGet(FLAG_BADGE01_GET + i);
        FlagSet(FLAG_BADGE01_GET + i);
    }
    const u16 flags[] = {FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY, FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT, FLAG_UNHOLY_GENESIS_MOLTRES_GALAR_CAUGHT, FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT, FLAG_UNHOLY_GENESIS_ARTICUNO_GALAR_CAUGHT};
    bool8 oldFlags[ARRAY_COUNT(flags)];
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
    {
        oldFlags[i] = FlagGet(flags[i]);
        FlagClear(flags[i]);
    }
    FlagSet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
    EXPECT(!GenesisEvent_MoltresGalarAvailable());
    EXPECT(!GenesisEvent_ArticunoGalarAvailable());
    FlagSet(FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT);
    FlagSet(FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT);
    FlagClear(FLAG_BADGE08_GET);
    EXPECT(!GenesisEvent_MoltresGalarAvailable());
    EXPECT(!GenesisEvent_ArticunoGalarAvailable());
    FlagSet(FLAG_BADGE08_GET);
    EXPECT(GenesisEvent_MoltresGalarAvailable());
    EXPECT(GenesisEvent_ArticunoGalarAvailable());
    EXPECT(HasPendingSection(MAPSEC_MT_CHIMNEY));
    EXPECT(HasPendingSection(MAPSEC_SHOAL_CAVE));
    FlagSet(FLAG_UNHOLY_GENESIS_MOLTRES_GALAR_CAUGHT);
    FlagSet(FLAG_UNHOLY_GENESIS_ARTICUNO_GALAR_CAUGHT);
    EXPECT(!HasPendingSection(MAPSEC_MT_CHIMNEY));
    EXPECT(!HasPendingSection(MAPSEC_SHOAL_CAVE));
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        if (oldFlags[i]) FlagSet(flags[i]); else FlagClear(flags[i]);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
}

TEST("Genesis Aurora ticket requires resolved climate crisis and every badge")
{
    bool8 oldBadges[NUM_BADGES];
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        oldBadges[i] = FlagGet(FLAG_BADGE01_GET + i);
        FlagSet(FLAG_BADGE01_GET + i);
    }
    const u16 flags[] = {FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE, FLAG_ENABLE_SHIP_BIRTH_ISLAND, FLAG_BATTLED_DEOXYS};
    bool8 oldFlags[ARRAY_COUNT(flags)];
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
    {
        oldFlags[i] = FlagGet(flags[i]);
        FlagClear(flags[i]);
    }
    EXPECT(!GenesisEvent_AuroraTicketAvailable());
    EXPECT(!GenesisEvent_DeoxysAvailable());
    FlagSet(FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE);
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        FlagClear(FLAG_BADGE01_GET + i);
        EXPECT(!GenesisEvent_AuroraTicketAvailable());
        FlagSet(FLAG_BADGE01_GET + i);
    }
    EXPECT(GenesisEvent_AuroraTicketAvailable());
    EXPECT(HasPendingSection(MAPSEC_MOSSDEEP_CITY));
    FlagSet(FLAG_ENABLE_SHIP_BIRTH_ISLAND);
    EXPECT(!GenesisEvent_AuroraTicketAvailable());
    EXPECT(GenesisEvent_DeoxysAvailable());
    EXPECT(HasPendingSection(MAPSEC_LILYCOVE_CITY));
    FlagSet(FLAG_BATTLED_DEOXYS);
    EXPECT(!GenesisEvent_DeoxysAvailable());
    EXPECT(FlagGet(FLAG_ENABLE_SHIP_BIRTH_ISLAND));
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        if (oldFlags[i]) FlagSet(flags[i]); else FlagClear(flags[i]);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
}

TEST("Genesis postgame tickets retain individual progression")
{
    const u16 flags[] = {FLAG_SYS_GAME_CLEAR, FLAG_ENABLE_SHIP_FARAWAY_ISLAND, FLAG_ENABLE_SHIP_SOUTHERN_ISLAND, FLAG_ENABLE_SHIP_NAVEL_ROCK, FLAG_UNHOLY_GENESIS_ZAPDOS_CAUGHT, FLAG_UNHOLY_GENESIS_MOLTRES_CAUGHT, FLAG_UNHOLY_GENESIS_ARTICUNO_CAUGHT};
    bool8 oldFlags[ARRAY_COUNT(flags)];
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
    {
        oldFlags[i] = FlagGet(flags[i]);
        FlagClear(flags[i]);
    }
    EXPECT(!GenesisEvent_OldSeaMapAvailable());
    EXPECT(!GenesisEvent_EonTicketAvailable());
    EXPECT(!GenesisEvent_MysticTicketAvailable());
    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(GenesisEvent_OldSeaMapAvailable());
    EXPECT(GenesisEvent_EonTicketAvailable());
    EXPECT(!GenesisEvent_MysticTicketAvailable());
    for (u32 i = 4; i < ARRAY_COUNT(flags); i++)
        FlagSet(flags[i]);
    EXPECT(GenesisEvent_MysticTicketAvailable());
    for (u32 i = 4; i < ARRAY_COUNT(flags); i++)
    {
        FlagClear(flags[i]);
        EXPECT(!GenesisEvent_MysticTicketAvailable());
        FlagSet(flags[i]);
    }
    FlagSet(FLAG_ENABLE_SHIP_FARAWAY_ISLAND);
    FlagSet(FLAG_ENABLE_SHIP_SOUTHERN_ISLAND);
    FlagSet(FLAG_ENABLE_SHIP_NAVEL_ROCK);
    EXPECT(!GenesisEvent_OldSeaMapAvailable());
    EXPECT(!GenesisEvent_EonTicketAvailable());
    EXPECT(!GenesisEvent_MysticTicketAvailable());
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        if (oldFlags[i]) FlagSet(flags[i]); else FlagClear(flags[i]);
}

TEST("Genesis island fauna discovery is gated by travel and persists independently")
{
    const u16 travel[] = {FLAG_ENABLE_SHIP_BIRTH_ISLAND, FLAG_ENABLE_SHIP_FARAWAY_ISLAND, FLAG_ENABLE_SHIP_NAVEL_ROCK, FLAG_ENABLE_SHIP_SOUTHERN_ISLAND};
    const u16 discovered[] = {FLAG_UNHOLY_GENESIS_BIRTH_FAUNA_DISCOVERED, FLAG_UNHOLY_GENESIS_FARAWAY_FAUNA_DISCOVERED, FLAG_UNHOLY_GENESIS_NAVEL_FAUNA_DISCOVERED, FLAG_UNHOLY_GENESIS_SOUTHERN_FAUNA_DISCOVERED};
    const enum GenesisEventId events[] = {GENESIS_EVENT_BIRTH_FAUNA, GENESIS_EVENT_FARAWAY_FAUNA, GENESIS_EVENT_NAVEL_FAUNA, GENESIS_EVENT_SOUTHERN_FAUNA};
    for (u32 i = 0; i < ARRAY_COUNT(events); i++)
    {
        bool8 oldTravel = FlagGet(travel[i]);
        bool8 oldDiscovered = FlagGet(discovered[i]);
        FlagClear(travel[i]);
        FlagClear(discovered[i]);
        EXPECT(!GenesisEvent_IsAvailable(events[i]));
        FlagSet(travel[i]);
        EXPECT(GenesisEvent_IsAvailable(events[i]));
        EXPECT(HasPendingSection(MAPSEC_LILYCOVE_CITY));
        FlagSet(discovered[i]);
        EXPECT(!GenesisEvent_IsAvailable(events[i]));
        EXPECT(FlagGet(travel[i]));
        if (oldTravel) FlagSet(travel[i]); else FlagClear(travel[i]);
        if (oldDiscovered) FlagSet(discovered[i]); else FlagClear(discovered[i]);
    }
}

TEST("Genesis environmental discoveries depend on progression and capture")
{
    u16 oldGenerator = VarGet(VAR_NEW_MAUVILLE_STATE);
    bool8 oldBadge = FlagGet(FLAG_BADGE06_GET);
    const u16 flags[] = {FLAG_UNHOLY_GENESIS_TADBULB_DISCOVERED, FLAG_UNHOLY_GENESIS_VAROOM_DISCOVERED, FLAG_UNHOLY_GENESIS_CRAMORANT_DISCOVERED};
    const enum GenesisEventId events[] = {GENESIS_EVENT_TADBULB_DISCOVERY, GENESIS_EVENT_VAROOM_DISCOVERY, GENESIS_EVENT_CRAMORANT_DISCOVERY};
    bool8 oldFlags[ARRAY_COUNT(flags)];
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
    {
        oldFlags[i] = FlagGet(flags[i]);
        FlagClear(flags[i]);
    }
    VarSet(VAR_NEW_MAUVILLE_STATE, 1);
    FlagClear(FLAG_BADGE06_GET);
    for (u32 i = 0; i < ARRAY_COUNT(events); i++)
        EXPECT(!GenesisEvent_IsAvailable(events[i]));
    VarSet(VAR_NEW_MAUVILLE_STATE, 2);
    FlagSet(FLAG_BADGE06_GET);
    for (u32 i = 0; i < ARRAY_COUNT(events); i++)
    {
        EXPECT(GenesisEvent_IsAvailable(events[i]));
        FlagSet(flags[i]);
        EXPECT(!GenesisEvent_IsAvailable(events[i]));
    }
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        if (oldFlags[i]) FlagSet(flags[i]); else FlagClear(flags[i]);
    VarSet(VAR_NEW_MAUVILLE_STATE, oldGenerator);
    if (oldBadge) FlagSet(FLAG_BADGE06_GET); else FlagClear(FLAG_BADGE06_GET);
}

TEST("Genesis wild pools unlock cumulatively and preserve habitat separation")
{
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE101, WILD_AREA_LAND, 0), 0);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE101, WILD_AREA_LAND, 1), 3);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE101, WILD_AREA_LAND, 8), 3);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE101, WILD_AREA_WATER, 8), 0);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_MIRAGE_TOWER_1F, WILD_AREA_LAND, 2), 0);
    const struct WildPokemon *mon = GenesisWild_GetEligibleMon(MAP_MIRAGE_TOWER_1F, WILD_AREA_LAND, 3, 0);
    EXPECT(mon != NULL);
    if (mon != NULL)
    {
        EXPECT_EQ(mon->species, SPECIES_SILICOBRA);
        EXPECT(mon->minLevel <= mon->maxLevel);
    }
    EXPECT(GenesisWild_GetEligibleMon(MAP_MIRAGE_TOWER_1F, WILD_AREA_LAND, 3, 1) == NULL);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_DESERT_UNDERPASS, WILD_AREA_LAND, 7), 0);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_DESERT_UNDERPASS, WILD_AREA_LAND, 8), 4);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE105, WILD_AREA_WATER, 4), 0);
    EXPECT_EQ(GenesisWild_GetEligibleCount(MAP_ROUTE105, WILD_AREA_WATER, 5), 1);
}

TEST("Genesis starter generation selector excludes previously awarded trios")
{
    EXPECT_EQ(GenesisStarter_SelectGeneration(0, 0), 0);
    EXPECT_EQ(GenesisStarter_SelectGeneration(0, 8), 8);
    EXPECT_EQ(GenesisStarter_SelectGeneration(0x1FE, 99), 0);
    EXPECT_EQ(GenesisStarter_SelectGeneration(0x0FF, 99), 8);
    EXPECT_EQ(GenesisStarter_SelectGeneration(0x1FF, 0), GENESIS_STARTER_NONE);
    EXPECT_EQ(GenesisStarter_GetSpecies(0, 0), SPECIES_BULBASAUR);
    EXPECT_EQ(GenesisStarter_GetSpecies(8, 2), SPECIES_QUAXLY);
    EXPECT_EQ(GenesisStarter_GetSpecies(9, 0), SPECIES_NONE);
    EXPECT_EQ(GenesisStarter_GetSpecies(0, 3), SPECIES_NONE);
}

TEST("Genesis Crystal Cave gate needs five badges and Mewtwo needs all eight")
{
    bool8 oldBadges[NUM_BADGES];
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        oldBadges[i] = FlagGet(FLAG_BADGE01_GET + i);
        FlagClear(FLAG_BADGE01_GET + i);
    }
    bool8 oldOpen = FlagGet(FLAG_GENESIS_CRYSTAL_OPEN);
    bool8 oldCaught = FlagGet(FLAG_GENESIS_MEWTWO_CAUGHT);
    FlagClear(FLAG_GENESIS_CRYSTAL_OPEN);
    FlagClear(FLAG_GENESIS_MEWTWO_CAUGHT);
    for (u16 i = 0; i < 4; i++)
        FlagSet(FLAG_BADGE01_GET + i);
    EXPECT(!GenesisEvent_CrystalGateAvailable());
    FlagSet(FLAG_BADGE05_GET);
    EXPECT(GenesisEvent_CrystalGateAvailable());
    EXPECT(!GenesisEvent_MewtwoAvailable());
    FlagSet(FLAG_GENESIS_CRYSTAL_OPEN);
    EXPECT(!GenesisEvent_CrystalGateAvailable());
    for (u16 i = 0; i < NUM_BADGES; i++)
        FlagSet(FLAG_BADGE01_GET + i);
    EXPECT(GenesisEvent_MewtwoAvailable());
    FlagSet(FLAG_GENESIS_MEWTWO_CAUGHT);
    EXPECT(!GenesisEvent_MewtwoAvailable());
    if (oldOpen) FlagSet(FLAG_GENESIS_CRYSTAL_OPEN); else FlagClear(FLAG_GENESIS_CRYSTAL_OPEN);
    if (oldCaught) FlagSet(FLAG_GENESIS_MEWTWO_CAUGHT); else FlagClear(FLAG_GENESIS_MEWTWO_CAUGHT);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
}

TEST("Genesis Telluric Cave requires all badges and League clear, stays open after capture")
{
    bool8 oldBadges[NUM_BADGES];
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        oldBadges[i] = FlagGet(FLAG_BADGE01_GET + i);
        FlagSet(FLAG_BADGE01_GET + i);
    }
    bool8 oldClear = FlagGet(FLAG_SYS_GAME_CLEAR);
    bool8 oldVisited = FlagGet(FLAG_GENESIS_TELLURIC_VISITED);
    bool8 oldCaught = FlagGet(FLAG_GENESIS_ZYGARDE_CAUGHT);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    FlagClear(FLAG_GENESIS_TELLURIC_VISITED);
    FlagClear(FLAG_GENESIS_ZYGARDE_CAUGHT);
    EXPECT(!GenesisEvent_TelluricGateAvailable());
    FlagSet(FLAG_SYS_GAME_CLEAR);
    for (u16 i = 0; i < NUM_BADGES; i++)
    {
        FlagClear(FLAG_BADGE01_GET + i);
        EXPECT(!GenesisEvent_TelluricGateAvailable());
        EXPECT(!GenesisEvent_ZygardeAvailable());
        FlagSet(FLAG_BADGE01_GET + i);
    }
    EXPECT(GenesisEvent_TelluricGateAvailable());
    EXPECT(GenesisEvent_IsAvailable(GENESIS_EVENT_TELLURIC_GATE));
    EXPECT(!GenesisEvent_ZygardeAvailable());
    FlagSet(FLAG_GENESIS_TELLURIC_VISITED);
    EXPECT(GenesisEvent_TelluricGateAvailable());
    EXPECT(!GenesisEvent_IsAvailable(GENESIS_EVENT_TELLURIC_GATE));
    EXPECT(GenesisEvent_ZygardeAvailable());
    FlagSet(FLAG_GENESIS_ZYGARDE_CAUGHT);
    EXPECT(!GenesisEvent_ZygardeAvailable());
    EXPECT(GenesisEvent_TelluricGateAvailable());
    if (oldClear) FlagSet(FLAG_SYS_GAME_CLEAR); else FlagClear(FLAG_SYS_GAME_CLEAR);
    if (oldVisited) FlagSet(FLAG_GENESIS_TELLURIC_VISITED); else FlagClear(FLAG_GENESIS_TELLURIC_VISITED);
    if (oldCaught) FlagSet(FLAG_GENESIS_ZYGARDE_CAUGHT); else FlagClear(FLAG_GENESIS_ZYGARDE_CAUGHT);
    for (u16 i = 0; i < NUM_BADGES; i++)
        if (oldBadges[i]) FlagSet(FLAG_BADGE01_GET + i); else FlagClear(FLAG_BADGE01_GET + i);
}
