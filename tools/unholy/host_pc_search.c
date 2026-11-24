#include "global.h"
#include "pc_search.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void TestFilters(void)
{
    // Arbitrary species/type IDs: the core must not depend on game tables.
    struct PcSearchMon mon = {.species = 42, .types = {4, 9}, .mutations = 0x101,
        .flags = PC_SEARCH_FAVORITE | PC_SEARCH_SHINY | PC_SEARCH_GEAR};
    struct PcSearchQuery query = {0};
    assert(PcSearch_Matches(&mon, &query));
    query.species = 43;
    assert(!PcSearch_Matches(&mon, &query));
    query.species = 42;
    query.type = 9; // Secondary type matches.
    query.mutationMode = PC_SEARCH_MUTATED;
    query.mutations = 0x101; // Combined Alpha/Omega style mask.
    query.flags = PC_SEARCH_FAVORITE | PC_SEARCH_SHINY | PC_SEARCH_GEAR;
    struct PcSearchMon beforeMon = mon;
    struct PcSearchQuery beforeQuery = query;
    assert(PcSearch_Matches(&mon, &query));
    assert(memcmp(&mon, &beforeMon, sizeof(mon)) == 0);
    assert(memcmp(&query, &beforeQuery, sizeof(query)) == 0);
    for (u8 flag = 1; flag <= PC_SEARCH_GEAR; flag <<= 1)
    {
        mon.flags ^= flag;
        assert(!PcSearch_Matches(&mon, &query));
        mon.flags ^= flag;
    }
    query.type = 8;
    assert(!PcSearch_Matches(&mon, &query));
    query.type = 4;
    for (u16 bit = 1; bit <= 0x100; bit <<= 1)
    {
        query.mutations = bit;
        mon.mutations = bit;
        assert(PcSearch_Matches(&mon, &query));
        mon.mutations = 0x1FF ^ bit;
        assert(!PcSearch_Matches(&mon, &query));
    }
    query.mutations = 0;
    query.mutationMode = PC_SEARCH_NORMAL;
    mon.mutations = 0;
    assert(PcSearch_Matches(&mon, &query));
    mon.mutations = 1;
    assert(!PcSearch_Matches(&mon, &query));
    query.mutationMode = PC_SEARCH_MUTATED;
    assert(PcSearch_Matches(&mon, &query));
    mon.mutations = 0;
    assert(!PcSearch_Matches(&mon, &query));
    query.mutationMode = 255;
    assert(!PcSearch_Matches(&mon, &query));
}

static void TestEggsAndEmptySlots(void)
{
    struct PcSearchMon mon = {.species = 42, .types = {4, 9}, .egg = TRUE,
        .flags = PC_SEARCH_FAVORITE | PC_SEARCH_SHINY | PC_SEARCH_GEAR, .mutations = 1};
    struct PcSearchQuery query = {.flags = PC_SEARCH_FAVORITE};
    assert(PcSearch_Matches(&mon, &query));
    query.species = 42;
    assert(!PcSearch_Matches(&mon, &query));
    query.species = 0;
    query.type = 9;
    assert(!PcSearch_Matches(&mon, &query));
    query.type = 0;
    query.mutations = 1;
    assert(!PcSearch_Matches(&mon, &query));
    query.mutations = 0;
    for (u8 mode = PC_SEARCH_NORMAL; mode <= PC_SEARCH_MUTATED; mode++)
    {
        query.mutationMode = mode;
        assert(!PcSearch_Matches(&mon, &query));
    }
    query.mutationMode = PC_SEARCH_ANY;
    query.flags = PC_SEARCH_SHINY;
    assert(!PcSearch_Matches(&mon, &query));
    query.flags = PC_SEARCH_GEAR;
    assert(!PcSearch_Matches(&mon, &query));
    query.flags = 0;
    mon.invalid = TRUE;
    assert(!PcSearch_Matches(&mon, &query));
    mon.invalid = FALSE;
    mon.species = 0;
    assert(!PcSearch_Matches(&mon, &query));
    assert(!PcSearch_Matches(NULL, &query));
    assert(!PcSearch_Matches(&mon, NULL));
}

static void TestNextMatch(void)
{
    enum { SLOTS = 997 }; // Final byte contains unused bits.
    u8 mask[PC_SEARCH_MASK_BYTES(SLOTS)] = {0};
    assert(PcSearch_FindNext(mask, SLOTS, 0) == PC_SEARCH_NONE);
    assert(PcSearch_FindNext(NULL, SLOTS, 0) == PC_SEARCH_NONE);
    assert(PcSearch_FindNext(mask, 0, 0) == PC_SEARCH_NONE);
    mask[(SLOTS - 1) / 8] = 0xE0; // Only out-of-range bits: no phantom result.
    assert(PcSearch_FindNext(mask, SLOTS, 0) == PC_SEARCH_NONE);
    mask[0] = 1;
    assert(PcSearch_FindNext(mask, SLOTS, 0) == 0); // Wrap to sole result.
    assert(PcSearch_FindNext(mask, SLOTS, PC_SEARCH_NONE) == 0);
    assert(PcSearch_FindNext(mask, 1, 0) == 0);
    mask[1] = 1; // First slot of the second byte.
    mask[(SLOTS - 1) / 8] |= 1 << ((SLOTS - 1) % 8);
    u8 before[sizeof(mask)];
    memcpy(before, mask, sizeof(mask));
    assert(PcSearch_FindNext(mask, SLOTS, 0) == 8);
    assert(PcSearch_FindNext(mask, SLOTS, 7) == 8);
    assert(PcSearch_FindNext(mask, SLOTS, 8) == SLOTS - 1);
    assert(PcSearch_FindNext(mask, SLOTS, SLOTS - 1) == 0);
    assert(memcmp(mask, before, sizeof(mask)) == 0);
}

int main(void)
{
    TestFilters();
    TestEggsAndEmptySlots();
    TestNextMatch();
    puts("PC search: combined filters, Egg privacy and circular navigation passed");
    return 0;
}
