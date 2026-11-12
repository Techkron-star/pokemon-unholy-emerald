#include "global.h"
#include "pokelocator.h"
#include "config/pokelocator.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "constants/maps.h"

// Data side of the Pokélocator. No hardware, no UI, no RNG: it only reads the game's
// own encounter tables (gWildMonHeaders) and Pokédex flags, so it can be tested alone.

#if POKELOCATOR_MUTATION_MODIFIER_ENABLED
#error "Mutation modifier rules (cost, duration, odds, unlocks) are undefined in DISENO.md. Add a read-only provider to sModifierProviders before enabling it."
#endif

// A table that is too small would silently drop species, so fail the build instead.
STATIC_ASSERT(NUM_LAND_MONS_ENCOUNTER_SLOTS + NUM_WATER_MONS_ENCOUNTER_SLOTS
              + NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS + NUM_FISHING_MONS_ENCOUNTER_SLOTS
              + NUM_HIDDEN_MONS_ENCOUNTER_SLOTS <= POKELOCATOR_MAX_ROWS, PokelocatorRowsHoldEveryEncounterSlot);

struct PokelocatorModifierProvider
{
    // Must be read-only: no Random(), no writes to encounter or save state.
    u8 (*getStatus)(u8 mapSec);
};

// Future providers plug in here. The mutation modifier has none on purpose.
static const struct PokelocatorModifierProvider sModifierProviders[POKELOCATOR_MODIFIER_COUNT] =
{
    [POKELOCATOR_MODIFIER_MUTATION] = { NULL },
};

// Fishing slots are grouped by rod: Old Rod first, then Good Rod, then Super Rod.
static u8 GetSlotMethod(enum WildPokemonArea area, u8 slot)
{
    switch (area)
    {
    case WILD_AREA_LAND:
        return POKELOCATOR_METHOD_LAND;
    case WILD_AREA_WATER:
        return POKELOCATOR_METHOD_WATER;
    case WILD_AREA_ROCKS:
        return POKELOCATOR_METHOD_ROCKS;
    case WILD_AREA_FISHING:
        if (slot < NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS)
            return POKELOCATOR_METHOD_OLD_ROD;
        if (slot < NUM_FISHING_MONS_OLD_ROD_ENCOUNTER_SLOTS + NUM_FISHING_MONS_GOOD_ROD_ENCOUNTER_SLOTS)
            return POKELOCATOR_METHOD_GOOD_ROD;
        return POKELOCATOR_METHOD_SUPER_ROD;
    case WILD_AREA_HIDDEN:
        break;
    }
    return POKELOCATOR_METHOD_HIDDEN;
}

static struct PokelocatorRow *FindRow(struct PokelocatorList *list, enum Species species, u8 method)
{
    u8 i;

    for (i = 0; i < list->count; i++)
    {
        if (list->rows[i].species == species && list->rows[i].method == method)
            return &list->rows[i];
    }
    return NULL;
}

void Pokelocator_ClearList(struct PokelocatorList *list)
{
    list->count = 0;
    list->methodMask = 0;
}

// Adds the species of one encounter table. Slots that lead to the same species with the
// same method are merged: their weights add up and their level ranges are joined.
void Pokelocator_AddArea(struct PokelocatorList *list, enum WildPokemonArea area,
                         const struct WildPokemonInfo *info, PokelocatorRevealFn reveal)
{
    u8 slotCount = GetWildEncounterSlotCount(area);
    u8 slot;

    if (info == NULL || info->wildPokemon == NULL)
        return;

    for (slot = 0; slot < slotCount; slot++)
    {
        const struct WildPokemon *mon = &info->wildPokemon[slot];
        u8 method = GetSlotMethod(area, slot);
        u8 weight = GetWildEncounterSlotWeight(area, slot);
        u8 total = GetWildEncounterSlotGroupTotal(area, slot);
        struct PokelocatorRow *row;

        if (mon->species == SPECIES_NONE)
            continue;
        if (method == POKELOCATOR_METHOD_HIDDEN && reveal != NULL && !reveal(mon->species))
            continue;

        row = FindRow(list, mon->species, method);
        if (row == NULL)
        {
            if (list->count >= POKELOCATOR_MAX_ROWS)
                continue;
            row = &list->rows[list->count++];
            row->species = mon->species;
            row->method = method;
            row->minLevel = mon->minLevel;
            row->maxLevel = mon->maxLevel;
            row->weight = 0;
            row->total = total;
            row->padding = 0;
            list->methodMask |= 1 << method;
        }
        else
        {
            if (mon->minLevel < row->minLevel)
                row->minLevel = mon->minLevel;
            if (mon->maxLevel > row->maxLevel)
                row->maxLevel = mon->maxLevel;
        }

        // A weight of 0 means "unknown", so the whole row has no computable chance.
        if (weight == 0 || total == 0)
            row->total = 0;
        else if (row->total != 0)
            row->weight += weight;
    }
}

// Reads every table of one map header. The table of the active time of day is used,
// exactly as the encounter code itself selects it.
void Pokelocator_BuildListForHeader(struct PokelocatorList *list, u32 headerId, PokelocatorRevealFn reveal)
{
    static const u8 areas[] =
    {
        WILD_AREA_LAND, WILD_AREA_WATER, WILD_AREA_ROCKS, WILD_AREA_FISHING, WILD_AREA_HIDDEN
    };
    const struct WildPokemonHeader *header = &gWildMonHeaders[headerId];
    u8 i;

    Pokelocator_ClearList(list);

    for (i = 0; i < ARRAY_COUNT(areas); i++)
    {
        enum WildPokemonArea area = areas[i];
        const struct WildEncounterTypes *types = &header->encounterTypes[GetTimeOfDayForEncounters(headerId, area)];
        const struct WildPokemonInfo *info;

        switch (area)
        {
        case WILD_AREA_LAND:
            info = types->landMonsInfo;
            break;
        case WILD_AREA_WATER:
            info = types->waterMonsInfo;
            break;
        case WILD_AREA_ROCKS:
            info = types->rockSmashMonsInfo;
            break;
        case WILD_AREA_FISHING:
            info = types->fishingMonsInfo;
            break;
        default:
            info = types->hiddenMonsInfo;
            break;
        }
        Pokelocator_AddArea(list, area, info, area == WILD_AREA_HIDDEN ? reveal : NULL);
    }
}

bool8 Pokelocator_RowHasChance(const struct PokelocatorRow *row)
{
    return row->total != 0;
}

// Chance in tenths of a percent (200 = 20.0%), rounded to the nearest tenth.
u16 Pokelocator_GetChanceTenths(const struct PokelocatorRow *row)
{
    if (row->total == 0)
        return 0;
    return (row->weight * 1000 + row->total / 2) / row->total;
}

// Writes the indices of the rows that pass the filter and returns how many there are.
u8 Pokelocator_BuildFilteredIndex(const struct PokelocatorList *list, u8 filter, u8 *indexOut)
{
    u8 i, count = 0;

    for (i = 0; i < list->count; i++)
    {
        if (filter == POKELOCATOR_FILTER_ALL || list->rows[i].method == filter - 1)
            indexOut[count++] = i;
    }
    return count;
}

// Steps to the next/previous filter, skipping methods the zone does not have:
// ALL -> each method present -> ALL.
u8 Pokelocator_NextFilter(u8 methodMask, u8 filter, s8 direction)
{
    u8 steps = POKELOCATOR_METHOD_COUNT + 1;
    u8 candidate = filter < steps ? filter : POKELOCATOR_FILTER_ALL;
    u8 i;

    for (i = 0; i < steps; i++)
    {
        candidate = (candidate + (direction > 0 ? 1 : steps - 1)) % steps;
        if (candidate == POKELOCATOR_FILTER_ALL || (methodMask & (1 << (candidate - 1))))
            return candidate;
    }
    return POKELOCATOR_FILTER_ALL;
}

// Moves the cursor one row (wrapping around) and keeps it inside the visible window.
void Pokelocator_MoveCursor(u8 *cursor, u8 *scroll, u8 count, u8 visibleRows, s8 delta)
{
    s16 pos;

    if (count == 0 || visibleRows == 0)
    {
        *cursor = 0;
        *scroll = 0;
        return;
    }

    pos = (*cursor < count ? *cursor : 0) + delta;
    if (pos < 0)
        pos = count - 1;
    else if (pos >= count)
        pos = 0;
    *cursor = pos;

    if (count <= visibleRows)
    {
        *scroll = 0;
        return;
    }
    if (pos < *scroll)
        *scroll = pos;
    else if (pos >= *scroll + visibleRows)
        *scroll = pos - visibleRows + 1;
    if (*scroll > count - visibleRows)
        *scroll = count - visibleRows;
}

u8 Pokelocator_GetDexState(enum Species species)
{
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(species);

    if (dexNum == NATIONAL_DEX_NONE)
        return POKELOCATOR_DEX_UNKNOWN;
    if (GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT))
        return POKELOCATOR_DEX_CAUGHT;
    if (GetSetPokedexFlag(dexNum, FLAG_GET_SEEN))
        return POKELOCATOR_DEX_SEEN;
    return POKELOCATOR_DEX_UNKNOWN;
}

// Default reveal policy for secret methods: only species the player has already seen.
bool8 Pokelocator_IsSpeciesRevealed(enum Species species)
{
    return Pokelocator_GetDexState(species) != POKELOCATOR_DEX_UNKNOWN;
}

// Collects the headers of every map in the region-map section of (mapGroup, mapNum), so
// cave floors can be browsed together. The player's own map is flagged in zone->current.
// Returns the number of maps that have encounter data.
u8 Pokelocator_BuildZone(struct PokelocatorZone *zone, u16 mapGroup, u16 mapNum)
{
    const struct MapHeader *here = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    bool8 isPlayerMap = mapGroup == gSaveBlock1Ptr->location.mapGroup && mapNum == gSaveBlock1Ptr->location.mapNum;
    u16 i;

    zone->count = 0;
    zone->current = 0;
    zone->mapSec = here->regionMapSectionId;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED) && zone->count < POKELOCATOR_MAX_ZONE_MAPS; i++)
    {
        const struct WildPokemonHeader *header = &gWildMonHeaders[i];
        u16 headerId = i;

        // Several consecutive headers for one map are alternate tables (Altering Cave).
        if (i > 0 && header->mapGroup == gWildMonHeaders[i - 1].mapGroup && header->mapNum == gWildMonHeaders[i - 1].mapNum)
            continue;
        if (Overworld_GetMapHeaderByGroupAndId(header->mapGroup, header->mapNum)->regionMapSectionId != zone->mapSec)
            continue;

        if (header->mapGroup == mapGroup && header->mapNum == mapNum)
        {
            // GetCurrentMapWildMonHeaderId picks the right alternate table for the player's map.
            if (isPlayerMap && GetCurrentMapWildMonHeaderId() != HEADER_NONE)
                headerId = GetCurrentMapWildMonHeaderId();
            zone->current = zone->count;
        }
        zone->headerIds[zone->count++] = headerId;
    }
    return zone->count;
}

// Read-only query for modifiers other systems may expose. Rules still undefined: the
// answer is UNDEFINED and the UI shows nothing.
u8 Pokelocator_GetModifierStatus(enum PokelocatorModifierId id, u8 mapSec)
{
    if (id >= POKELOCATOR_MODIFIER_COUNT || sModifierProviders[id].getStatus == NULL)
        return POKELOCATOR_MODIFIER_UNDEFINED;
    return sModifierProviders[id].getStatus(mapSec);
}
