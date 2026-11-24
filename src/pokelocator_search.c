#include "global.h"
#include "bg.h"
#include "pokelocator.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "field_move.h"
#include "field_player_avatar.h"
#include "hfc_watch_services.h"
#include "item.h"
#include "item_use.h"
#include "main.h"
#include "menu.h"
#include "fieldmap.h"
#include "event_object_movement.h"
#include "metatile_behavior.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "safari_zone.h"
#include "config/wild_encounter.h"
#include "constants/items.h"
#include "constants/vars.h"
#include "dma3.h"
#include "event_object_lock.h"
#include "hfc_watch.h"
#include "palette.h"
#include "script.h"
#include "task.h"
#include "string_util.h"

static bool32 HasUsableMon(void)
{
    for (u8 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) > 0)
            return TRUE;
    }
    return FALSE;
}

static bool32 CanRockSmash(void)
{
    if (!IsFieldMoveUnlocked(FIELD_MOVE_ROCK_SMASH))
        return FALSE;
    for (u8 i = 0; i < PARTY_SIZE; i++)
        if (!GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_IS_EGG)
         && MonKnowsMove(&gParties[B_TRAINER_PLAYER][i], FieldMove_GetMoveId(FIELD_MOVE_ROCK_SMASH)))
            return TRUE;
    return FALSE;
}

u8 Pokelocator_GetSearchStatus(u16 headerId, u16 species, u8 method)
{
    if (headerId != GetCurrentMapWildMonHeaderId())
        return POKELOCATOR_SEARCH_WRONG_AREA;
    const struct WildPokemonInfo *info = Pokelocator_GetSearchInfo(headerId, method);
    if (Pokelocator_GetSearchWeight(info, species, method) == 0
     || !Pokelocator_IsMethodUnlocked(headerId, method))
        return POKELOCATOR_SEARCH_UNAVAILABLE;
    if (gMain.inBattle || GetSafariZoneFlag() || InBattlePyramid()
     || FlagGet(WE_FLAG_NO_ENCOUNTER) || FlagGet(WE_FLAG_NO_CATCHING)
     || AreLegendariesInSootopolisPreventingEncounters())
        return POKELOCATOR_SEARCH_BLOCKED;
    if (!HasUsableMon())
        return POKELOCATOR_SEARCH_NO_PARTY;
    bool32 inWater = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER);
    bool32 waterTarget = method == POKELOCATOR_METHOD_WATER
                     || (method == POKELOCATOR_METHOD_HIDDEN && info->encounterRate == 1);
    if (method == POKELOCATOR_METHOD_LAND || (method == POKELOCATOR_METHOD_HIDDEN && !waterTarget))
    {
        s16 x, y;
        PlayerGetDestCoords(&x, &y);
        if (!MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y)))
            return POKELOCATOR_SEARCH_NEED_GRASS;
    }
    if (waterTarget && !inWater)
        return POKELOCATOR_SEARCH_NEED_SURF;
    if ((method == POKELOCATOR_METHOD_LAND || method == POKELOCATOR_METHOD_ROCKS
      || (method == POKELOCATOR_METHOD_HIDDEN && !waterTarget)) && inWater)
        return POKELOCATOR_SEARCH_NEED_LAND;
    if (method >= POKELOCATOR_METHOD_OLD_ROD && method <= POKELOCATOR_METHOD_SUPER_ROD)
    {
        const u16 rods[] = {ITEM_OLD_ROD, ITEM_GOOD_ROD, ITEM_SUPER_ROD};
        if (!CheckBagHasItem(rods[method - POKELOCATOR_METHOD_OLD_ROD], 1))
            return POKELOCATOR_SEARCH_NEED_ROD;
        if (!CanFishFromCurrentPosition())
            return POKELOCATOR_SEARCH_NEED_WATER;
    }
    if (method == POKELOCATOR_METHOD_ROCKS && !CanRockSmash())
        return POKELOCATOR_SEARCH_NEED_ROCK_SMASH;
    return POKELOCATOR_SEARCH_READY;
}

bool32 Pokelocator_CreateSearchMon(u16 headerId, u16 species, u8 method)
{
    // Revalidate after the menu closes: no stale map/time/table may create a mon.
    if (Pokelocator_GetSearchStatus(headerId, species, method) != POKELOCATOR_SEARCH_READY)
        return FALSE;
    const struct WildPokemonInfo *info = Pokelocator_GetSearchInfo(headerId, method);
    u16 total = Pokelocator_GetSearchWeight(info, species, method);
    if (total == 0)
        return FALSE;
    u8 slot = Pokelocator_SelectSearchSlot(info, species, method, Random() % total);
    if (slot == 0xFF)
        return FALSE;
    enum WildPokemonArea area = Pokelocator_GetMethodArea(method);
    u8 level = ChooseWildMonLevel(info->wildPokemon, slot, area);
    gIsFishingEncounter = area == WILD_AREA_FISHING;
    gIsSurfingEncounter = area == WILD_AREA_WATER || (area == WILD_AREA_HIDDEN && info->encounterRate == 1);
    CreateWildMonWithFrequency(species, level, HfcWatch_ConsumeFrequencyEncounter());
    return TRUE;
}

const u8 *Pokelocator_GetSearchMessage(u8 status)
{
    switch (status)
    {
    case POKELOCATOR_SEARCH_READY: return COMPOUND_STRING("A:SEARCH UP/DN B:BACK");
    case POKELOCATOR_SEARCH_WRONG_AREA: return COMPOUND_STRING("GO TO THIS AREA FIRST");
    case POKELOCATOR_SEARCH_NO_PARTY: return COMPOUND_STRING("NEED A HEALTHY POKéMON");
    case POKELOCATOR_SEARCH_NEED_GRASS: return COMPOUND_STRING("STAND IN GRASS OR A CAVE");
    case POKELOCATOR_SEARCH_NEED_LAND: return COMPOUND_STRING("RETURN TO LAND FIRST");
    case POKELOCATOR_SEARCH_NEED_SURF: return COMPOUND_STRING("SURF HERE TO SEARCH");
    case POKELOCATOR_SEARCH_NEED_ROD: return COMPOUND_STRING("NEED THIS ROD IN THE BAG");
    case POKELOCATOR_SEARCH_NEED_WATER: return COMPOUND_STRING("FACE FISHABLE WATER");
    case POKELOCATOR_SEARCH_NEED_ROCK_SMASH: return COMPOUND_STRING("NEED USABLE ROCK SMASH");
    case POKELOCATOR_SEARCH_BLOCKED: return COMPOUND_STRING("CANNOT SEARCH HERE");
    default: return COMPOUND_STRING("NOT AVAILABLE RIGHT NOW");
    }
}

extern const u8 EventScript_PokelocatorSearchBattle[];
extern const u8 EventScript_PokelocatorSearchUnavailable[];

bool32 Pokelocator_RegisterTarget(u16 species, u8 method)
{
    if (species == SPECIES_NONE || species >= NUM_SPECIES || species > 0x7FF || method >= POKELOCATOR_METHOD_COUNT)
        return FALSE;
    VarSet(VAR_POKELOCATOR_TARGET, 0x8000 | (method << 11) | species);
    return TRUE;
}

bool32 Pokelocator_ReadTarget(u16 *species, u8 *method)
{
    u16 value = VarGet(VAR_POKELOCATOR_TARGET);
    *species = value & 0x7FF;
    *method = (value >> 11) & 7;
    return (value & 0xC000) == 0x8000 && *species > SPECIES_NONE && *species < NUM_SPECIES && *method < POKELOCATOR_METHOD_COUNT;
}

static void Task_LocatorSearch(u8 taskId)
{
    if (gPaletteFade.active || IsDma3ManagerBusyWithBgCopy())
        return;
    const s16 *data = gTasks[taskId].data;
    bool32 ready = Pokelocator_CreateSearchMon((u16)data[0], (u16)data[1], data[2]);
    if (!ready)
        StringCopy(gStringVar1, Pokelocator_GetSearchMessage(Pokelocator_GetSearchStatus((u16)data[0], (u16)data[1], data[2])));
    ScriptContext_SetupScript(ready ? EventScript_PokelocatorSearchBattle : EventScript_PokelocatorSearchUnavailable);
    DestroyTask(taskId);
}

bool32 Pokelocator_QueueSearch(u16 header, u16 species, u8 method)
{
    if (FuncIsActiveTask(Task_LocatorSearch))
        return FALSE;
    u8 task = CreateTask(Task_LocatorSearch, 8);
    if (task == TASK_NONE)
        return FALSE;
    gTasks[task].data[0] = header;
    gTasks[task].data[1] = species;
    gTasks[task].data[2] = method;
    LockPlayerFieldControls();
    return TRUE;
}

bool32 Pokelocator_TryQuickSearch(void)
{
    u16 species;
    u8 method;
    struct HfcWatchProgress progress = {.badges = HfcWatch_CountBadges()};
    if (!CheckBagHasItem(ITEM_POKELOCATOR, 1)
     && HfcWatch_GetAccess(HfcWatch_GetTier(), HFC_WATCH_FEATURE_POKELOCATOR, progress) != HFC_WATCH_READY)
        return FALSE;
    if (!Pokelocator_ReadTarget(&species, &method))
        return FALSE;
    return Pokelocator_QueueSearch(GetCurrentMapWildMonHeaderId(), species, method);
}
