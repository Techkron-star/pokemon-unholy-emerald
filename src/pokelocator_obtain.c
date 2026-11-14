#include "global.h"
#include "pokelocator.h"
#include "battle_setup.h"
#include "battle_pyramid.h"
#include "caps.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "safari_zone.h"
#include "script_pokemon_util.h"
#include "config/wild_encounter.h"
#include "constants/items.h"
#include "constants/trainer_types.h"

// Hidden encounters use this game's map trainers, never imported trainer IDs.
static bool32 HiddenUnlocked(u16 header)
{
    if (!FlagGet(FLAG_IS_CHAMPION))
        return FALSE;
    const struct WildPokemonHeader *wild = &gWildMonHeaders[header];
    const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(wild->mapGroup, wild->mapNum);
    if (map->events == NULL)
        return TRUE;
    for (u8 i = 0; i < map->events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *object = &map->events->objectEvents[i];
        if ((object->trainerType == TRAINER_TYPE_NORMAL || object->trainerType == TRAINER_TYPE_BURIED
          || object->trainerType == TRAINER_TYPE_SEE_ALL_DIRECTIONS)
         && object->script != NULL && !GetTrainerFlagFromScriptPointer(object->script))
            return FALSE;
    }
    return TRUE;
}

bool32 Pokelocator_IsMethodUnlocked(u16 header, u8 method)
{
    if (Pokelocator_GetSearchInfo(header, method) == NULL)
        return FALSE;
    switch (method)
    {
    case POKELOCATOR_METHOD_WATER: return IsFieldMoveUnlocked(FIELD_MOVE_SURF);
    case POKELOCATOR_METHOD_ROCKS: return IsFieldMoveUnlocked(FIELD_MOVE_ROCK_SMASH);
    case POKELOCATOR_METHOD_OLD_ROD: return CheckBagHasItem(ITEM_OLD_ROD, 1);
    case POKELOCATOR_METHOD_GOOD_ROD: return CheckBagHasItem(ITEM_GOOD_ROD, 1);
    case POKELOCATOR_METHOD_SUPER_ROD: return CheckBagHasItem(ITEM_SUPER_ROD, 1);
    case POKELOCATOR_METHOD_HIDDEN: return HiddenUnlocked(header);
    default: return TRUE;
    }
}

void Pokelocator_BuildAvailableList(struct PokelocatorList *list, u16 header)
{
    Pokelocator_ClearList(list);
    if (Pokelocator_GetSearchInfo(header, POKELOCATOR_METHOD_LAND) == NULL
     && Pokelocator_GetSearchInfo(header, POKELOCATOR_METHOD_WATER) == NULL
     && Pokelocator_GetSearchInfo(header, POKELOCATOR_METHOD_OLD_ROD) == NULL
     && Pokelocator_GetSearchInfo(header, POKELOCATOR_METHOD_ROCKS) == NULL
     && Pokelocator_GetSearchInfo(header, POKELOCATOR_METHOD_HIDDEN) == NULL)
        return;
    Pokelocator_BuildListForHeader(list, header, NULL);
    u8 count = 0;
    list->methodMask = 0;
    for (u8 i = 0; i < list->count; i++)
        if (Pokelocator_IsMethodUnlocked(header, list->rows[i].method))
        {
            list->rows[count++] = list->rows[i];
            list->methodMask |= 1 << list->rows[i].method;
        }
    list->count = count;
}

u16 Pokelocator_CountFreeSlots(void)
{
    u16 free = 0;
    for (u8 i = 0; i < PARTY_SIZE; i++)
        free += GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) == SPECIES_NONE;
    for (u8 box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (u8 slot = 0; slot < IN_BOX_COUNT; slot++)
            free += GetBoxMonDataAt(box, slot, MON_DATA_SPECIES) == SPECIES_NONE;
    return free;
}

u8 Pokelocator_PrepareObtain(struct PokelocatorObtainPlan *plan, u16 header, u8 group, u8 kind, u16 species)
{
    memset(plan, 0, sizeof(*plan));
    plan->header = header;
    plan->group = group;
    plan->kind = kind;
    plan->selectedSpecies = species;
    plan->level = GetCurrentLevelCap();
    if (header != GetCurrentMapWildMonHeaderId())
        return POKELOCATOR_OBTAIN_WRONG_AREA;
    if (gMain.inBattle || GetSafariZoneFlag() || InBattlePyramid()
     || FlagGet(WE_FLAG_NO_ENCOUNTER) || FlagGet(WE_FLAG_NO_CATCHING)
     || AreLegendariesInSootopolisPreventingEncounters())
        return POKELOCATOR_OBTAIN_BLOCKED;
    if (group == POKELOCATOR_GROUP_ALL || group >= POKELOCATOR_GROUP_COUNT || kind > POKELOCATOR_OBTAIN_NEW)
        return POKELOCATOR_OBTAIN_EMPTY;
    struct PokelocatorList list;
    u8 index[POKELOCATOR_MAX_ROWS];
    Pokelocator_BuildAvailableList(&list, header);
    u8 count = Pokelocator_BuildCatalogIndex(&list, group,
        kind == POKELOCATOR_OBTAIN_NEW ? POKELOCATOR_COLLECTION_MISSING : POKELOCATOR_COLLECTION_ALL,
        Pokelocator_GetDexState, index);
    for (u8 i = 0; i < count; i++)
        if (kind != POKELOCATOR_OBTAIN_ONE || list.rows[index[i]].species == species)
            plan->species[plan->count++] = list.rows[index[i]].species;
    if (!plan->count)
        return POKELOCATOR_OBTAIN_EMPTY;
    if (Pokelocator_CountFreeSlots() < plan->count)
        return POKELOCATOR_OBTAIN_NO_SPACE;
    return POKELOCATOR_OBTAIN_READY;
}

struct PokelocatorObtainResult Pokelocator_ExecuteObtain(const struct PokelocatorObtainPlan *plan)
{
    struct PokelocatorObtainPlan current;
    struct PokelocatorObtainResult result = {0};
    result.status = Pokelocator_PrepareObtain(&current, plan->header, plan->group, plan->kind, plan->selectedSpecies);
    if (result.status != POKELOCATOR_OBTAIN_READY)
        return result;
    // No RNG or gifts until the exact confirmed catalog and level are revalidated.
    if (memcmp(plan, &current, sizeof(current)))
    {
        result.status = POKELOCATOR_OBTAIN_CHANGED;
        return result;
    }
    for (u8 i = 0; i < current.count; i++)
    {
        u32 delivery = ScriptGiveMon(current.species[i], current.level, ITEM_NONE);
        if (delivery == MON_CANT_GIVE)
        {
            result.status = POKELOCATOR_OBTAIN_NO_SPACE;
            break;
        }
        if (delivery == MON_GIVEN_TO_PARTY) result.party++;
        else result.pc++;
    }
    return result;
}

const u8 *Pokelocator_GetObtainMessage(u8 status)
{
    switch (status)
    {
    case POKELOCATOR_OBTAIN_READY: return COMPOUND_STRING("FREE - NO BALLS OR POINTS USED");
    case POKELOCATOR_OBTAIN_WRONG_AREA: return COMPOUND_STRING("GO TO THIS AREA FIRST");
    case POKELOCATOR_OBTAIN_BLOCKED: return COMPOUND_STRING("OBTAINING IS BLOCKED HERE");
    case POKELOCATOR_OBTAIN_EMPTY: return COMPOUND_STRING("NO ELIGIBLE SPECIES IN THIS GROUP");
    case POKELOCATOR_OBTAIN_NO_SPACE: return COMPOUND_STRING("NOT ENOUGH PARTY / PC SPACE");
    default: return COMPOUND_STRING("CATALOG CHANGED - REVIEW AGAIN");
    }
}
