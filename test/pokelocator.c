#include "global.h"
#include "pokelocator.h"
#include "wild_encounter.h"
#include "random.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/species.h"

static u8 CatalogDexState(enum Species species)
{
    return species == SPECIES_MAGIKARP ? POKELOCATOR_DEX_CAUGHT : POKELOCATOR_DEX_SEEN;
}

TEST("Pokelocator catalog counts a species once even across grass surf and several rods")
{
    const struct PokelocatorList list = {.count = 5, .rows = {
        {.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_WATER},
        {.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_OLD_ROD},
        {.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_GOOD_ROD},
        {.species = SPECIES_RALTS, .method = POKELOCATOR_METHOD_LAND},
        {.species = SPECIES_RALTS, .method = POKELOCATOR_METHOD_HIDDEN},
    }};
    u8 index[POKELOCATOR_MAX_ROWS];
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_ALL, POKELOCATOR_COLLECTION_ALL, CatalogDexState, index), 2);
    EXPECT_EQ(index[0], 0);
    EXPECT_EQ(index[1], 3);
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_RODS, POKELOCATOR_COLLECTION_ALL, CatalogDexState, index), 1);
    EXPECT_EQ(index[0], 1);
    EXPECT_EQ(Pokelocator_GetSpeciesMethods(&list, SPECIES_MAGIKARP, index), 3);
    EXPECT_EQ(index[2], 2);
}

TEST("Pokelocator missing and caught filters combine with habitat without leaking another method")
{
    const struct PokelocatorList list = {.count = 3, .rows = {
        {.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_OLD_ROD},
        {.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_WATER},
        {.species = SPECIES_RALTS, .method = POKELOCATOR_METHOD_LAND},
    }};
    u8 index[POKELOCATOR_MAX_ROWS];
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_ALL, POKELOCATOR_COLLECTION_MISSING, CatalogDexState, index), 1);
    EXPECT_EQ(index[0], 2);
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_RODS, POKELOCATOR_COLLECTION_MISSING, CatalogDexState, index), 0);
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_RODS, POKELOCATOR_COLLECTION_CAUGHT, CatalogDexState, index), 1);
    EXPECT_EQ(index[0], 0);
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_COUNT, POKELOCATOR_COLLECTION_ALL, CatalogDexState, index), 0);
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_ALL, POKELOCATOR_COLLECTION_COUNT, CatalogDexState, index), 0);
}

TEST("Pokelocator grid wraps safely on odd final pages and on an empty collection")
{
    u8 cursor = 3, scroll = 0;
    Pokelocator_MoveGrid(&cursor, &scroll, 5, POKELOCATOR_GRID_DOWN);
    EXPECT_EQ(cursor, 1);
    EXPECT_EQ(scroll, 0);
    cursor = 2;
    Pokelocator_MoveGrid(&cursor, &scroll, 5, POKELOCATOR_GRID_DOWN);
    EXPECT_EQ(cursor, 4);
    EXPECT_EQ(scroll, 4);
    Pokelocator_MoveGrid(&cursor, &scroll, 5, POKELOCATOR_GRID_RIGHT);
    EXPECT_EQ(cursor, 0);
    EXPECT_EQ(scroll, 0);
    Pokelocator_MoveGrid(&cursor, &scroll, 5, POKELOCATOR_GRID_UP);
    EXPECT_EQ(cursor, 4);
    cursor = 99;
    Pokelocator_MoveGrid(&cursor, &scroll, 1, POKELOCATOR_GRID_DOWN);
    EXPECT_EQ(cursor, 0);
    Pokelocator_MoveGrid(&cursor, &scroll, 0, POKELOCATOR_GRID_LEFT);
    EXPECT_EQ(cursor, 0);
    EXPECT_EQ(scroll, 0);
}

TEST("Pokelocator collection filtering is read only and does not alter encounter slots")
{
    struct PokelocatorList list = {.count = 1, .rows = {{.species = SPECIES_MAGIKARP, .method = POKELOCATOR_METHOD_OLD_ROD, .minLevel = 5, .maxLevel = 10, .weight = 70}}};
    struct PokelocatorList before = list;
    rng_value_t rng = gRngValue;
    u8 index[POKELOCATOR_MAX_ROWS];
    EXPECT_EQ(Pokelocator_BuildCatalogIndex(&list, POKELOCATOR_GROUP_ALL, POKELOCATOR_COLLECTION_CAUGHT, CatalogDexState, index), 1);
    EXPECT_EQ(memcmp(&list, &before, sizeof(list)), 0);
    EXPECT_EQ(memcmp(&rng, &gRngValue, sizeof(rng)), 0);
}

TEST("Pokelocator search preserves weighted slots instead of inventing levels between them")
{
    struct WildPokemon mons[NUM_LAND_MONS_ENCOUNTER_SLOTS] = {0};
    const struct WildPokemonInfo info = {.encounterRate = 20, .wildPokemon = mons};
    mons[1] = (struct WildPokemon){.species = SPECIES_WURMPLE, .minLevel = 3, .maxLevel = 3};
    mons[4] = (struct WildPokemon){.species = SPECIES_WURMPLE, .minLevel = 17, .maxLevel = 17};
    u16 first = GetWildEncounterSlotWeight(WILD_AREA_LAND, 1);
    u16 total = first + GetWildEncounterSlotWeight(WILD_AREA_LAND, 4);
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND), total);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND, 0), 1);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND, first - 1), 1);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND, first), 4);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND, total - 1), 4);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_WURMPLE, POKELOCATOR_METHOD_LAND, total), 1);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_RALTS, POKELOCATOR_METHOD_LAND, 0), 0xFF);
}

TEST("Pokelocator search never borrows a matching species from another rod")
{
    struct WildPokemon mons[NUM_FISHING_MONS_ENCOUNTER_SLOTS] = {0};
    const struct WildPokemonInfo info = {.encounterRate = 20, .wildPokemon = mons};
    u8 good = NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS;
    u8 super = good + NUM_FISHING_MONS_GOOD_ROD_ENCOUNTER_SLOTS;
    mons[0] = mons[good] = mons[super] = (struct WildPokemon){.species = SPECIES_MAGIKARP, .minLevel = 5, .maxLevel = 5};
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_OLD_ROD), GetWildEncounterSlotWeight(WILD_AREA_FISHING, 0));
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_GOOD_ROD), GetWildEncounterSlotWeight(WILD_AREA_FISHING, good));
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_SUPER_ROD), GetWildEncounterSlotWeight(WILD_AREA_FISHING, super));
    for (u16 roll = 0; roll < 100; roll++)
    {
        EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_OLD_ROD, roll), 0);
        EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_GOOD_ROD, roll), good);
        EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_MAGIKARP, POKELOCATOR_METHOD_SUPER_ROD, roll), super);
    }
}

TEST("Pokelocator search rejects unsafe entries and supports hidden encounter slots")
{
    struct WildPokemon mons[NUM_HIDDEN_MONS_ENCOUNTER_SLOTS] = {0};
    const struct WildPokemonInfo info = {.wildPokemon = mons};
    mons[0] = mons[2] = (struct WildPokemon){.species = SPECIES_RALTS, .minLevel = 5, .maxLevel = 5};
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_RALTS, POKELOCATOR_METHOD_HIDDEN), 2);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_RALTS, POKELOCATOR_METHOD_HIDDEN, 0), 0);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_RALTS, POKELOCATOR_METHOD_HIDDEN, 1), 2);
    mons[0].minLevel = 0;
    mons[2].maxLevel = MAX_LEVEL + 1;
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_RALTS, POKELOCATOR_METHOD_HIDDEN), 0);
    EXPECT_EQ(Pokelocator_GetSearchWeight(NULL, SPECIES_RALTS, POKELOCATOR_METHOD_HIDDEN), 0);
    EXPECT_EQ(Pokelocator_GetSearchWeight(&info, SPECIES_NONE, POKELOCATOR_METHOD_HIDDEN), 0);
    EXPECT_EQ(Pokelocator_SelectSearchSlot(&info, SPECIES_RALTS, POKELOCATOR_METHOD_COUNT, 0), 0xFF);
}

TEST("Pokelocator rejected search changes neither RNG nor the enemy party")
{
    struct Pokemon before = gParties[B_TRAINER_OPPONENT_A][0];
    rng_value_t rng = gRngValue;
    EXPECT_NE(Pokelocator_GetSearchStatus(HEADER_NONE, SPECIES_RALTS, POKELOCATOR_METHOD_LAND), POKELOCATOR_SEARCH_READY);
    EXPECT_EQ(Pokelocator_CreateSearchMon(HEADER_NONE, SPECIES_RALTS, POKELOCATOR_METHOD_LAND), FALSE);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(memcmp(&before, &gParties[B_TRAINER_OPPONENT_A][0], sizeof(before)), 0);
}

// Tests for the Pokélocator data core: reading encounter tables, merging slots,
// chances, filters and cursor navigation. The screen itself needs mGBA (see DISENO.md).

static u32 SumGroupWeights(enum WildPokemonArea area, u8 firstSlot, u8 numSlots)
{
    u32 sum = 0;
    u8 i;

    for (i = firstSlot; i < firstSlot + numSlots; i++)
        sum += GetWildEncounterSlotWeight(area, i);
    return sum;
}

static bool8 RevealNothing(enum Species species)
{
    return FALSE;
}

static bool8 RevealWurmple(enum Species species)
{
    return species == SPECIES_WURMPLE;
}

TEST("Pokelocator slot weights add up to the group total the encounter code rolls against")
{
    EXPECT_EQ(SumGroupWeights(WILD_AREA_LAND, 0, NUM_LAND_MONS_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_LAND, 0));
    EXPECT_EQ(SumGroupWeights(WILD_AREA_WATER, 0, NUM_WATER_MONS_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_WATER, 0));
    EXPECT_EQ(SumGroupWeights(WILD_AREA_ROCKS, 0, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_ROCKS, 0));
}

TEST("Pokelocator fishing weights add up per rod")
{
    u8 good = NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS;
    u8 super = good + NUM_FISHING_MONS_GOOD_ROD_ENCOUNTER_SLOTS;

    EXPECT_EQ(SumGroupWeights(WILD_AREA_FISHING, 0, NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_FISHING, 0));
    EXPECT_EQ(SumGroupWeights(WILD_AREA_FISHING, good, NUM_FISHING_MONS_GOOD_ROD_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_FISHING, good));
    EXPECT_EQ(SumGroupWeights(WILD_AREA_FISHING, super, NUM_FISHING_MONS_SUPER_ROD_ENCOUNTER_SLOTS),
              GetWildEncounterSlotGroupTotal(WILD_AREA_FISHING, super));
}

TEST("Pokelocator reports no chance for invalid slots and hidden encounters")
{
    EXPECT_EQ(GetWildEncounterSlotWeight(WILD_AREA_LAND, NUM_LAND_MONS_ENCOUNTER_SLOTS), 0);
    EXPECT_EQ(GetWildEncounterSlotGroupTotal(WILD_AREA_LAND, NUM_LAND_MONS_ENCOUNTER_SLOTS), 0);
    EXPECT_EQ(GetWildEncounterSlotWeight(WILD_AREA_HIDDEN, 0), 0);
    EXPECT_EQ(GetWildEncounterSlotGroupTotal(WILD_AREA_HIDDEN, 0), 0);
}

TEST("Pokelocator merges slots of the same species and joins their level ranges")
{
    struct WildPokemon mons[NUM_LAND_MONS_ENCOUNTER_SLOTS] = {0};
    struct WildPokemonInfo info = { .encounterRate = 20, .wildPokemon = mons };
    struct PokelocatorList list;
    u8 i;

    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        mons[i].species = SPECIES_ZIGZAGOON;
        mons[i].minLevel = 3;
        mons[i].maxLevel = 3;
    }
    mons[1].species = SPECIES_WURMPLE;
    mons[1].minLevel = 2;
    mons[1].maxLevel = 4;
    mons[2].species = SPECIES_WURMPLE;
    mons[2].minLevel = 3;
    mons[2].maxLevel = 6;

    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_LAND, &info, NULL);

    EXPECT_EQ(list.count, 2);
    EXPECT_EQ(list.methodMask, 1 << POKELOCATOR_METHOD_LAND);
    EXPECT_EQ(list.rows[0].species, SPECIES_ZIGZAGOON);
    EXPECT_EQ(list.rows[1].species, SPECIES_WURMPLE);
    EXPECT_EQ(list.rows[1].minLevel, 2);
    EXPECT_EQ(list.rows[1].maxLevel, 6);
    EXPECT_EQ(list.rows[1].weight,
              GetWildEncounterSlotWeight(WILD_AREA_LAND, 1) + GetWildEncounterSlotWeight(WILD_AREA_LAND, 2));
    EXPECT_EQ(list.rows[0].weight + list.rows[1].weight, GetWildEncounterSlotGroupTotal(WILD_AREA_LAND, 0));
}

TEST("Pokelocator chance is a tenth-of-percent value and the full table adds up to 100 percent")
{
    struct WildPokemon mons[NUM_LAND_MONS_ENCOUNTER_SLOTS];
    struct WildPokemonInfo info = { .encounterRate = 20, .wildPokemon = mons };
    struct PokelocatorList list;
    u32 sum = 0;
    u8 i;

    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        mons[i].species = SPECIES_ZIGZAGOON;
        mons[i].minLevel = 5;
        mons[i].maxLevel = 5;
    }
    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_LAND, &info, NULL);

    EXPECT_EQ(list.count, 1);
    EXPECT(Pokelocator_RowHasChance(&list.rows[0]));
    EXPECT_EQ(Pokelocator_GetChanceTenths(&list.rows[0]), 1000);

    // A species with a single slot must be below 100 percent.
    mons[0].species = SPECIES_WURMPLE;
    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_LAND, &info, NULL);
    for (i = 0; i < list.count; i++)
        sum += Pokelocator_GetChanceTenths(&list.rows[i]);
    EXPECT_EQ(list.count, 2);
    EXPECT_LE(sum, 1000);
    EXPECT_GE(sum, 998); // per-row rounding to a tenth can lose at most 1 each
    EXPECT_LT(Pokelocator_GetChanceTenths(&list.rows[0]), 1000);
}

TEST("Pokelocator skips empty slots and tolerates missing tables")
{
    struct WildPokemon mons[NUM_WATER_MONS_ENCOUNTER_SLOTS] = {0};
    struct WildPokemonInfo info = { .encounterRate = 4, .wildPokemon = mons };
    struct PokelocatorList list;

    mons[0].species = SPECIES_TENTACOOL;
    mons[0].minLevel = 5;
    mons[0].maxLevel = 10;

    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_WATER, NULL, NULL);
    EXPECT_EQ(list.count, 0);

    Pokelocator_AddArea(&list, WILD_AREA_WATER, &info, NULL);
    EXPECT_EQ(list.count, 1);
    EXPECT_EQ(list.rows[0].species, SPECIES_TENTACOOL);
    EXPECT_EQ(list.rows[0].method, POKELOCATOR_METHOD_WATER);
}

TEST("Pokelocator assigns Old, Good and Super Rod by slot position")
{
    struct WildPokemon mons[NUM_FISHING_MONS_ENCOUNTER_SLOTS];
    struct WildPokemonInfo info = { .encounterRate = 30, .wildPokemon = mons };
    struct PokelocatorList list;
    u8 i;

    for (i = 0; i < NUM_FISHING_MONS_ENCOUNTER_SLOTS; i++)
    {
        mons[i].species = SPECIES_MAGIKARP;
        mons[i].minLevel = 5;
        mons[i].maxLevel = 5;
    }
    mons[NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS].species = SPECIES_TENTACOOL;
    mons[NUM_FISHING_MONS_ENCOUNTER_SLOTS - 1].species = SPECIES_WAILMER;

    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_FISHING, &info, NULL);

    // Magikarp appears in all three rods, so it gets one row per rod.
    EXPECT_EQ(list.methodMask, (1 << POKELOCATOR_METHOD_OLD_ROD) | (1 << POKELOCATOR_METHOD_GOOD_ROD)
                               | (1 << POKELOCATOR_METHOD_SUPER_ROD));
    for (i = 0; i < list.count; i++)
    {
        if (list.rows[i].species == SPECIES_TENTACOOL)
            EXPECT_EQ(list.rows[i].method, POKELOCATOR_METHOD_GOOD_ROD);
        if (list.rows[i].species == SPECIES_WAILMER)
            EXPECT_EQ(list.rows[i].method, POKELOCATOR_METHOD_SUPER_ROD);
    }
    // Weight is relative to its own rod: an Old Rod row can reach 100 percent.
    for (i = 0; i < list.count; i++)
    {
        if (list.rows[i].method == POKELOCATOR_METHOD_OLD_ROD)
            EXPECT_EQ(Pokelocator_GetChanceTenths(&list.rows[i]), 1000);
    }
}

TEST("Pokelocator hides secret methods until the reveal rule allows the species")
{
    struct WildPokemon mons[NUM_HIDDEN_MONS_ENCOUNTER_SLOTS] = {0};
    struct WildPokemonInfo info = { .encounterRate = 0, .wildPokemon = mons };
    struct PokelocatorList list;
    u8 i;

    for (i = 0; i < NUM_HIDDEN_MONS_ENCOUNTER_SLOTS; i++)
    {
        mons[i].species = SPECIES_ABRA;
        mons[i].minLevel = 8;
        mons[i].maxLevel = 8;
    }
    mons[0].species = SPECIES_WURMPLE;

    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_HIDDEN, &info, RevealNothing);
    EXPECT_EQ(list.count, 0);

    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_HIDDEN, &info, RevealWurmple);
    EXPECT_EQ(list.count, 1);
    EXPECT_EQ(list.rows[0].species, SPECIES_WURMPLE);
    EXPECT(!Pokelocator_RowHasChance(&list.rows[0])); // hidden slots have no defined odds
    EXPECT_EQ(Pokelocator_GetChanceTenths(&list.rows[0]), 0);
}

TEST("Pokelocator reveal rule only gates the hidden method")
{
    struct WildPokemon land[NUM_LAND_MONS_ENCOUNTER_SLOTS];
    struct WildPokemonInfo landInfo = { .encounterRate = 20, .wildPokemon = land };
    struct PokelocatorList list;
    u8 i;

    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        land[i].species = SPECIES_ZIGZAGOON;
        land[i].minLevel = 3;
        land[i].maxLevel = 3;
    }
    // A reveal rule only gates the hidden method: ordinary methods are always listed.
    Pokelocator_ClearList(&list);
    Pokelocator_AddArea(&list, WILD_AREA_LAND, &landInfo, RevealNothing);
    EXPECT_EQ(list.count, 1);
}

TEST("Pokelocator filters keep only the selected method")
{
    struct PokelocatorList list;
    u8 index[POKELOCATOR_MAX_ROWS];

    Pokelocator_ClearList(&list);
    list.count = 3;
    list.rows[0].method = POKELOCATOR_METHOD_LAND;
    list.rows[1].method = POKELOCATOR_METHOD_WATER;
    list.rows[2].method = POKELOCATOR_METHOD_LAND;

    EXPECT_EQ(Pokelocator_BuildFilteredIndex(&list, POKELOCATOR_FILTER_ALL, index), 3);
    EXPECT_EQ(Pokelocator_BuildFilteredIndex(&list, POKELOCATOR_METHOD_LAND + 1, index), 2);
    EXPECT_EQ(index[0], 0);
    EXPECT_EQ(index[1], 2);
    EXPECT_EQ(Pokelocator_BuildFilteredIndex(&list, POKELOCATOR_METHOD_WATER + 1, index), 1);
    EXPECT_EQ(index[0], 1);
    EXPECT_EQ(Pokelocator_BuildFilteredIndex(&list, POKELOCATOR_METHOD_ROCKS + 1, index), 0);
}

TEST("Pokelocator L and R cycle only through methods present in the zone")
{
    u8 mask = (1 << POKELOCATOR_METHOD_LAND) | (1 << POKELOCATOR_METHOD_SUPER_ROD);
    u8 filter = POKELOCATOR_FILTER_ALL;

    filter = Pokelocator_NextFilter(mask, filter, 1);
    EXPECT_EQ(filter, POKELOCATOR_METHOD_LAND + 1);
    filter = Pokelocator_NextFilter(mask, filter, 1);
    EXPECT_EQ(filter, POKELOCATOR_METHOD_SUPER_ROD + 1);
    filter = Pokelocator_NextFilter(mask, filter, 1);
    EXPECT_EQ(filter, POKELOCATOR_FILTER_ALL);

    filter = Pokelocator_NextFilter(mask, filter, -1);
    EXPECT_EQ(filter, POKELOCATOR_METHOD_SUPER_ROD + 1);
    filter = Pokelocator_NextFilter(mask, filter, -1);
    EXPECT_EQ(filter, POKELOCATOR_METHOD_LAND + 1);
    filter = Pokelocator_NextFilter(mask, filter, -1);
    EXPECT_EQ(filter, POKELOCATOR_FILTER_ALL);

    // A zone without any encounters has nothing to cycle to.
    EXPECT_EQ(Pokelocator_NextFilter(0, POKELOCATOR_FILTER_ALL, 1), POKELOCATOR_FILTER_ALL);
}

TEST("Pokelocator cursor wraps around and keeps itself inside the visible window")
{
    u8 cursor = 0, scroll = 0;

    // 5 rows, 3 visible.
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, 1);
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, 1);
    EXPECT_EQ(cursor, 2);
    EXPECT_EQ(scroll, 0);
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, 1);
    EXPECT_EQ(cursor, 3);
    EXPECT_EQ(scroll, 1);
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, 1);
    EXPECT_EQ(cursor, 4);
    EXPECT_EQ(scroll, 2);

    // Down from the last row wraps to the first one.
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, 1);
    EXPECT_EQ(cursor, 0);
    EXPECT_EQ(scroll, 0);

    // Up from the first row wraps to the last one, and the window follows.
    Pokelocator_MoveCursor(&cursor, &scroll, 5, 3, -1);
    EXPECT_EQ(cursor, 4);
    EXPECT_EQ(scroll, 2);
}

TEST("Pokelocator cursor handles short and empty lists")
{
    u8 cursor = 7, scroll = 7;

    Pokelocator_MoveCursor(&cursor, &scroll, 0, 3, 1);
    EXPECT_EQ(cursor, 0);
    EXPECT_EQ(scroll, 0);

    Pokelocator_MoveCursor(&cursor, &scroll, 2, 3, 1);
    EXPECT_EQ(cursor, 1);
    EXPECT_EQ(scroll, 0);
    Pokelocator_MoveCursor(&cursor, &scroll, 2, 3, 1);
    EXPECT_EQ(cursor, 0);
    EXPECT_EQ(scroll, 0);
}

TEST("Pokelocator mutation modifier stays undefined until its rules exist")
{
    EXPECT_EQ(Pokelocator_GetModifierStatus(POKELOCATOR_MODIFIER_MUTATION, 0), POKELOCATOR_MODIFIER_UNDEFINED);
    EXPECT_EQ(Pokelocator_GetModifierStatus(POKELOCATOR_MODIFIER_COUNT, 0), POKELOCATOR_MODIFIER_UNDEFINED);
}

TEST("Pokelocator can hold the worst-case number of rows of one map")
{
    EXPECT_GE(POKELOCATOR_MAX_ROWS,
              NUM_LAND_MONS_ENCOUNTER_SLOTS + NUM_WATER_MONS_ENCOUNTER_SLOTS
              + NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS + NUM_FISHING_MONS_ENCOUNTER_SLOTS
              + NUM_HIDDEN_MONS_ENCOUNTER_SLOTS);
}
