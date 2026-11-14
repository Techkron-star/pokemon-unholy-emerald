#ifndef GUARD_POKELOCATOR_H
#define GUARD_POKELOCATOR_H

#include "wild_encounter.h"

// Pokélocator: in-game tool that lists the wild Pokémon of the current zone.
// Table helpers are read-only. The explicit Search action creates a normal wild
// encounter from the current map's live table; it is separate from browsing.

// Worst case rows for one map: 12 land + 5 water + 5 rock smash + 10 fishing + 3 hidden = 35.
#define POKELOCATOR_MAX_ROWS      40
// Maps that share one region-map section (cave floors, underwater twin of a route...).
#define POKELOCATOR_MAX_ZONE_MAPS 16

enum PokelocatorMethod
{
    POKELOCATOR_METHOD_LAND,      // tall grass, or cave floor when the map is underground
    POKELOCATOR_METHOD_WATER,     // Surf
    POKELOCATOR_METHOD_ROCKS,     // Rock Smash
    POKELOCATOR_METHOD_OLD_ROD,
    POKELOCATOR_METHOD_GOOD_ROD,
    POKELOCATOR_METHOD_SUPER_ROD,
    POKELOCATOR_METHOD_HIDDEN,    // hidden table; live catalog gates by Champion and map trainers
    POKELOCATOR_METHOD_COUNT
};

// Filter value 0 shows every method; value n + 1 shows only method n.
#define POKELOCATOR_FILTER_ALL 0

enum PokelocatorGroup
{
    POKELOCATOR_GROUP_ALL,
    POKELOCATOR_GROUP_LAND,
    POKELOCATOR_GROUP_WATER,
    POKELOCATOR_GROUP_RODS,
    POKELOCATOR_GROUP_ROCKS,
    POKELOCATOR_GROUP_HONEY,
    POKELOCATOR_GROUP_HIDDEN,
    POKELOCATOR_GROUP_COUNT,
};

#define POKELOCATOR_GROUP_OTHER POKELOCATOR_GROUP_ROCKS

enum PokelocatorCollectionFilter
{
    POKELOCATOR_COLLECTION_ALL,
    POKELOCATOR_COLLECTION_MISSING,
    POKELOCATOR_COLLECTION_CAUGHT,
    POKELOCATOR_COLLECTION_COUNT,
};

enum PokelocatorGridDirection
{
    POKELOCATOR_GRID_LEFT,
    POKELOCATOR_GRID_RIGHT,
    POKELOCATOR_GRID_UP,
    POKELOCATOR_GRID_DOWN,
};

typedef u8 (*PokelocatorDexStateFn)(enum Species species);

enum PokelocatorDexState
{
    POKELOCATOR_DEX_UNKNOWN,
    POKELOCATOR_DEX_SEEN,
    POKELOCATOR_DEX_CAUGHT,
};

// One species found with one method. A species reachable several times through the
// same method (e.g. two Wurmple slots) is merged into a single row.
struct PokelocatorRow
{
    u16 species;
    u8 method;      // enum PokelocatorMethod
    u8 minLevel;
    u8 maxLevel;
    u8 weight;      // summed slot weight
    u8 total;       // total weight of the slot group; 0 means the chance cannot be computed
    u8 padding;
};

struct PokelocatorList
{
    struct PokelocatorRow rows[POKELOCATOR_MAX_ROWS];
    u8 count;
    u8 methodMask;  // bit n set when method n has at least one row
};

// The wild-encounter headers that belong to one region-map section (the "zone").
struct PokelocatorZone
{
    u16 headerIds[POKELOCATOR_MAX_ZONE_MAPS];
    u8 count;
    u8 current;     // index into headerIds of the map the player is standing on
    u8 mapSec;
};

// Decides whether a species of a secret method may be listed. NULL reveals everything.
typedef bool8 (*PokelocatorRevealFn)(enum Species species);

// Read-only modifiers that other systems may expose to the Pokélocator.
enum PokelocatorModifierId
{
    POKELOCATOR_MODIFIER_MUTATION,
    POKELOCATOR_MODIFIER_COUNT
};

enum PokelocatorModifierStatus
{
    POKELOCATOR_MODIFIER_UNDEFINED, // rules not defined yet: nothing is shown
    POKELOCATOR_MODIFIER_INACTIVE,
    POKELOCATOR_MODIFIER_ACTIVE,
};

// Table reading and filtering (pure: no hardware, no RNG).
void Pokelocator_ClearList(struct PokelocatorList *list);
void Pokelocator_AddArea(struct PokelocatorList *list, enum WildPokemonArea area,
                         const struct WildPokemonInfo *info, PokelocatorRevealFn reveal);
void Pokelocator_BuildListForHeader(struct PokelocatorList *list, u32 headerId, PokelocatorRevealFn reveal);
u16 Pokelocator_GetChanceTenths(const struct PokelocatorRow *row); // 0 when unknown, see HasChance
bool8 Pokelocator_RowHasChance(const struct PokelocatorRow *row);
u8 Pokelocator_BuildFilteredIndex(const struct PokelocatorList *list, u8 filter, u8 *indexOut);
u8 Pokelocator_NextFilter(u8 methodMask, u8 filter, s8 direction);
void Pokelocator_MoveCursor(u8 *cursor, u8 *scroll, u8 count, u8 visibleRows, s8 delta);
bool8 Pokelocator_MethodInGroup(u8 method, u8 group);
u8 Pokelocator_BuildCatalogIndex(const struct PokelocatorList *list, u8 group, u8 collection,
                               PokelocatorDexStateFn dexState, u8 *indexOut);
u8 Pokelocator_GetSpeciesMethods(const struct PokelocatorList *list, u16 species, u8 *indexOut);
void Pokelocator_MoveGrid(u8 *cursor, u8 *scroll, u8 count, u8 direction);

// Game-state helpers.
u8 Pokelocator_GetDexState(enum Species species);
bool8 Pokelocator_IsSpeciesRevealed(enum Species species);
u8 Pokelocator_BuildZone(struct PokelocatorZone *zone, u16 mapGroup, u16 mapNum);
u8 Pokelocator_GetModifierStatus(enum PokelocatorModifierId id, u8 mapSec);

enum PokelocatorSearchStatus
{
    POKELOCATOR_SEARCH_READY,
    POKELOCATOR_SEARCH_WRONG_AREA,
    POKELOCATOR_SEARCH_UNAVAILABLE,
    POKELOCATOR_SEARCH_NO_PARTY,
    POKELOCATOR_SEARCH_NEED_LAND,
    POKELOCATOR_SEARCH_NEED_GRASS,
    POKELOCATOR_SEARCH_NEED_SURF,
    POKELOCATOR_SEARCH_NEED_ROD,
    POKELOCATOR_SEARCH_NEED_WATER,
    POKELOCATOR_SEARCH_NEED_ROCK_SMASH,
    POKELOCATOR_SEARCH_BLOCKED,
};

enum WildPokemonArea Pokelocator_GetMethodArea(u8 method);
const struct WildPokemonInfo *Pokelocator_GetSearchInfo(u16 headerId, u8 method);
u16 Pokelocator_GetSearchWeight(const struct WildPokemonInfo *info, u16 species, u8 method);
u8 Pokelocator_SelectSearchSlot(const struct WildPokemonInfo *info, u16 species, u8 method, u16 roll);
u8 Pokelocator_GetSearchStatus(u16 headerId, u16 species, u8 method);
bool32 Pokelocator_CreateSearchMon(u16 headerId, u16 species, u8 method);
const u8 *Pokelocator_GetSearchMessage(u8 status);

enum PokelocatorObtainKind { POKELOCATOR_OBTAIN_ONE, POKELOCATOR_OBTAIN_ALL, POKELOCATOR_OBTAIN_NEW };
enum PokelocatorObtainStatus
{
    POKELOCATOR_OBTAIN_READY, POKELOCATOR_OBTAIN_WRONG_AREA, POKELOCATOR_OBTAIN_BLOCKED,
    POKELOCATOR_OBTAIN_EMPTY, POKELOCATOR_OBTAIN_NO_SPACE, POKELOCATOR_OBTAIN_CHANGED,
};
struct PokelocatorObtainPlan
{
    u16 header, selectedSpecies, species[POKELOCATOR_MAX_ROWS];
    u8 group, kind, count, level;
};
struct PokelocatorObtainResult { u8 status, party, pc; };
bool32 Pokelocator_IsMethodUnlocked(u16 header, u8 method);
void Pokelocator_BuildAvailableList(struct PokelocatorList *list, u16 header);
u16 Pokelocator_CountFreeSlots(void);
u8 Pokelocator_PrepareObtain(struct PokelocatorObtainPlan *plan, u16 header, u8 group, u8 kind, u16 species);
struct PokelocatorObtainResult Pokelocator_ExecuteObtain(const struct PokelocatorObtainPlan *plan);
const u8 *Pokelocator_GetObtainMessage(u8 status);
bool32 Pokelocator_RegisterTarget(u16 species, u8 method);
bool32 Pokelocator_ReadTarget(u16 *species, u8 *method);
bool32 Pokelocator_QueueSearch(u16 header, u16 species, u8 method);
bool32 Pokelocator_TryQuickSearch(void);

// UI entry point, called from the Pokélocator key item while the field is active.
void Pokelocator_OpenFromField(void (*onClose)(void));

#endif // GUARD_POKELOCATOR_H
