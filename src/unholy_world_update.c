#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "menu.h"
#include "overworld.h"
#include "region_map.h"
#include "script_menu.h"
#include "string_util.h"
#include "text.h"
#include "unholy_world_update.h"
#include "window.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/vars.h"

#if !IS_FRLG
#include "data/map_group_count.h"
#endif

#define UNHOLY_WORLD_VERSION_LATEST 1
#define UNHOLY_WORLD_MAPS_PER_CHUNK 4

#if !IS_FRLG
struct WorldUpdateAudit
{
    u16 totalMaps;
    u16 scannedMaps;
    u16 routeSections;
    u16 unclaimedBalls;
    u16 visibleNpcs;
    u16 appearedBalls;
    u16 appearedNpcs;
    u16 repairedBalls;
    u16 repairedNpcs;
    u16 errors;
    u16 lastMapSection;
    u16 mapGroup;
    u16 mapNum;
    u8 windowId;
    bool8 active;
};

static EWRAM_DATA struct WorldUpdateAudit sWorldAudit = {0};

static const u8 sText_AuditTitle[] = _("ACTUALIZANDO MUNDO");
static const u8 sText_AuditAreas[] = _("Zonas: {STR_VAR_2}/{STR_VAR_3}");
static const u8 sText_AuditRoute[] = _("Zona: {STR_VAR_1}");
static const u8 sText_AuditPending[] = _("Comprobando objetos...");

static void WorldUpdate_DrawAuditProgress(void)
{
    u8 windowId = sWorldAudit.windowId;

    if (windowId == WINDOW_NONE)
        return;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sText_AuditTitle, 8, 5, TEXT_SKIP_DRAW, NULL);

    ConvertIntToDecimalStringN(gStringVar2, sWorldAudit.scannedMaps, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, sWorldAudit.totalMaps, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringExpandPlaceholders(gStringVar4, sText_AuditAreas);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 8, 23, TEXT_SKIP_DRAW, NULL);

    GetMapName(gStringVar1, sWorldAudit.lastMapSection, 0);
    StringExpandPlaceholders(gStringVar4, sText_AuditRoute);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 8, 43, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sText_AuditPending, 8, 61, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void WorldUpdate_ScanMap(u16 group, u16 num)
{
    const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(group, num);
    const struct MapEvents *events;

    sWorldAudit.scannedMaps++;
    if (map == NULL || map->events == NULL)
    {
        sWorldAudit.errors++;
        return;
    }

    sWorldAudit.lastMapSection = map->regionMapSectionId;
    if (map->mapType == MAP_TYPE_ROUTE || map->mapType == MAP_TYPE_OCEAN_ROUTE)
        sWorldAudit.routeSections++;

    events = map->events;
    if (events->objectEventCount != 0 && events->objectEvents == NULL)
    {
        sWorldAudit.errors++;
        return;
    }
    if (events->objectEventCount > OBJECT_EVENT_TEMPLATES_COUNT)
        sWorldAudit.errors++;

    // Other maps are immutable ROM headers, not independent save copies.
    // Check every template now; normal map loads instantiate them from ROM.
    for (u16 i = 0; i < events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *obj = &events->objectEvents[i];

        if (obj->graphicsId == OBJ_EVENT_GFX_ITEM_BALL)
        {
            if (obj->script == NULL)
                sWorldAudit.errors++;
            if (obj->script == Common_EventScript_FindItem)
            {
                if (obj->flagId == 0 || obj->flagId >= FLAGS_COUNT
                 || obj->trainerRange_berryTreeId <= ITEM_NONE
                 || obj->trainerRange_berryTreeId >= ITEMS_COUNT)
                    sWorldAudit.errors++;
                else if (!FlagGet(obj->flagId))
                    sWorldAudit.unclaimedBalls++;
            }
        }
        else if (obj->script != NULL && obj->flagId < FLAGS_COUNT
              && (obj->flagId == 0 || !FlagGet(obj->flagId)))
        {
            sWorldAudit.visibleNpcs++;
        }
        else if (obj->flagId >= FLAGS_COUNT)
        {
            sWorldAudit.errors++;
        }
    }
}

static void WorldUpdate_AppendNumber(u16 value)
{
    ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringAppend(gStringVar4, gStringVar1);
}

static bool8 WorldUpdate_TemplateChanged(const struct ObjectEventTemplate *saved,
                                         const struct ObjectEventTemplate *rom)
{
    return saved->localId != rom->localId
        || saved->graphicsId != rom->graphicsId
        || saved->x != rom->x || saved->y != rom->y
        || saved->script != rom->script
        || saved->flagId != rom->flagId
        || saved->trainerRange_berryTreeId != rom->trainerRange_berryTreeId;
}

static void WorldUpdate_DespawnWithoutClaiming(u8 localId)
{
    for (u8 j = 0; j < OBJECT_EVENTS_COUNT; j++)
    {
        struct ObjectEvent *objectEvent = &gObjectEvents[j];

        if (objectEvent->active && objectEvent->localId == localId
         && objectEvent->mapNum == gSaveBlock1Ptr->location.mapNum
         && objectEvent->mapGroup == gSaveBlock1Ptr->location.mapGroup)
            RemoveObjectEvent(objectEvent);
    }
}

static void ReconcileWorldEvent(u16 hideFlag, bool8 shouldAppear)
{
    if (shouldAppear)
        FlagClear(hideFlag);
    else
        FlagSet(hideFlag);
}

static void ReconcileClaimableWorldEvent(u16 hideFlag, u16 claimedFlag, bool8 unlocked)
{
    // Do not clear a picked-up item's hide flag on a later update or visit.
    ReconcileWorldEvent(hideFlag, unlocked && !FlagGet(claimedFlag));
}
#endif

void UnholyWorldUpdate_Reconcile(void)
{
#if !IS_FRLG
    u16 version = VarGet(VAR_UNHOLY_WORLD_VERSION);

    // Future migrations go here in ascending version order. Upgrading from a
    // legacy save does not clear or reassign any existing reward flags.
    if (version < UNHOLY_WORLD_VERSION_LATEST)
        VarSet(VAR_UNHOLY_WORLD_VERSION, UNHOLY_WORLD_VERSION_LATEST);

    // Map-event hide flags are synchronized before object templates spawn.
    // The claimed flags are separate, persistent and never cleared here.
    ReconcileClaimableWorldEvent(FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE,
                                FLAG_UNHOLY_WORLD_ROUTE102_BALL_CLAIMED,
                                FlagGet(FLAG_BADGE01_GET));
    ReconcileWorldEvent(FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE, FlagGet(FLAG_BADGE02_GET));
    ReconcileClaimableWorldEvent(FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE,
                                FLAG_UNHOLY_WORLD_ROUTE104_BALL_CLAIMED,
                                FlagGet(FLAG_BADGE03_GET));
    ReconcileWorldEvent(FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE, FlagGet(FLAG_SYS_GAME_CLEAR));
#endif
}

void UnholyWorldUpdate_BeginAudit(struct ScriptContext *ctx)
{
#if !IS_FRLG
    bool8 beforeRoute102Ball = FlagGet(FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE);
    bool8 beforeRoute104Ball = FlagGet(FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE);
    bool8 beforeRoute102Npc = FlagGet(FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE);
    bool8 beforeRoute110Npc = FlagGet(FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE);

    memset(&sWorldAudit, 0, sizeof(sWorldAudit));
    sWorldAudit.windowId = WINDOW_NONE;
    sWorldAudit.lastMapSection = gMapHeader.regionMapSectionId;
    sWorldAudit.active = TRUE;

    // Count *every* map in every group before starting. Work on the headers
    // in small batches; never move the player into a map just to inspect it.
    for (u16 group = 0; group < MAP_GROUPS_COUNT; group++)
        sWorldAudit.totalMaps += MAP_GROUP_COUNT[group];

    UnholyWorldUpdate_Reconcile();
    sWorldAudit.appearedBalls += beforeRoute102Ball && !FlagGet(FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE);
    sWorldAudit.appearedBalls += beforeRoute104Ball && !FlagGet(FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE);
    sWorldAudit.appearedNpcs += beforeRoute102Npc && !FlagGet(FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE);
    sWorldAudit.appearedNpcs += beforeRoute110Npc && !FlagGet(FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE);

    sWorldAudit.windowId = CreateWindowFromRect(2, 3, 26, 11);
    if (sWorldAudit.windowId != WINDOW_NONE)
        WorldUpdate_DrawAuditProgress();
#else
    UnholyWorldUpdate_Reconcile();
#endif
}

void UnholyWorldUpdate_ProcessChunk(struct ScriptContext *ctx)
{
    gSpecialVar_Result = TRUE;
#if !IS_FRLG
    if (!sWorldAudit.active)
        return;

    for (u8 processed = 0; processed < UNHOLY_WORLD_MAPS_PER_CHUNK
                     && sWorldAudit.mapGroup < MAP_GROUPS_COUNT;)
    {
        if (sWorldAudit.mapNum >= MAP_GROUP_COUNT[sWorldAudit.mapGroup])
        {
            sWorldAudit.mapGroup++;
            sWorldAudit.mapNum = 0;
            continue;
        }

        WorldUpdate_ScanMap(sWorldAudit.mapGroup, sWorldAudit.mapNum++);
        processed++;
    }

    // The final group may end exactly on a chunk boundary.
    while (sWorldAudit.mapGroup < MAP_GROUPS_COUNT
        && sWorldAudit.mapNum >= MAP_GROUP_COUNT[sWorldAudit.mapGroup])
    {
        sWorldAudit.mapGroup++;
        sWorldAudit.mapNum = 0;
    }

    WorldUpdate_DrawAuditProgress();
    gSpecialVar_Result = (sWorldAudit.mapGroup >= MAP_GROUPS_COUNT);
#endif
}

void UnholyWorldUpdate_RefreshCurrentMap(struct ScriptContext *ctx)
{
    UnholyWorldUpdate_Reconcile();
#if !IS_FRLG
    // Scripted refresh happens with field controls locked during the audit.
    // The normal map-load hooks already handle every future map transition.
    // Only reconcile objects owned by World Update, leaving story objects,
    // followers, native pickup flags and all unrelated events untouched.
    const u16 worldHideFlags[] =
    {
        FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE,
        FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE,
        FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE,
        FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE,
    };

    if (gMapHeader.events == NULL)
        return;

    // The current map is the only map whose object templates are cached in
    // the save. Repair stale fixed pickups here, and all eight New Mauville
    // balls (including scripted Voltorb), without touching claim flags.
    for (u16 i = 0; i < gMapHeader.events->objectEventCount
                     && i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        const struct ObjectEventTemplate *rom = &gMapHeader.events->objectEvents[i];
        struct ObjectEventTemplate *saved = &gSaveBlock1Ptr->objectEventTemplates[i];
        bool8 newMauvilleBall = rom->graphicsId == OBJ_EVENT_GFX_ITEM_BALL
            && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_NEW_MAUVILLE_INSIDE)
            && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_NEW_MAUVILLE_INSIDE);
        bool8 corruptNativeBall = rom->graphicsId == OBJ_EVENT_GFX_ITEM_BALL
            && rom->script == Common_EventScript_FindItem && rom->flagId != 0
            && (saved->localId != rom->localId || saved->graphicsId != rom->graphicsId
             || saved->script == NULL || saved->script != rom->script
             || saved->flagId == 0 || saved->trainerRange_berryTreeId <= ITEM_NONE
             || saved->trainerRange_berryTreeId >= ITEMS_COUNT);
        bool8 worldEvent = FALSE;

        for (u8 j = 0; j < ARRAY_COUNT(worldHideFlags); j++)
        {
            if (rom->flagId == worldHideFlags[j])
                worldEvent = TRUE;
        }

        if (newMauvilleBall || corruptNativeBall
         || (worldEvent && WorldUpdate_TemplateChanged(saved, rom)))
        {
            if (WorldUpdate_TemplateChanged(saved, rom) && sWorldAudit.active)
            {
                if (rom->graphicsId == OBJ_EVENT_GFX_ITEM_BALL)
                    sWorldAudit.repairedBalls++;
                else
                    sWorldAudit.repairedNpcs++;
            }
            // The regular RemoveObjectEventByLocalIdAndMap sets a claim flag;
            // here we must despawn WITHOUT claiming the reward or the NPC.
            WorldUpdate_DespawnWithoutClaiming(rom->localId);
            *saved = *rom;
        }
    }

    for (u16 i = 0; i < gMapHeader.events->objectEventCount
                     && i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        const struct ObjectEventTemplate *object = &gSaveBlock1Ptr->objectEventTemplates[i];

        for (u8 j = 0; j < ARRAY_COUNT(worldHideFlags); j++)
        {
            if (object->flagId == worldHideFlags[j] && FlagGet(object->flagId))
            {
                RemoveObjectEventByLocalIdAndMap(object->localId,
                    gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
                break;
            }
        }
    }
    // The spawner checks for already-active local IDs, so this does not
    // duplicate NPCs or Poké Balls when the button is pressed repeatedly.
    TrySpawnObjectEvents(0, 0);
#endif
}

void UnholyWorldUpdate_FinishAudit(struct ScriptContext *ctx)
{
#if !IS_FRLG
    if (sWorldAudit.windowId != WINDOW_NONE)
    {
        ClearToTransparentAndRemoveWindow(sWorldAudit.windowId);
        sWorldAudit.windowId = WINDOW_NONE;
    }
    sWorldAudit.active = FALSE;

    // A genuine changelog: count new visibility transitions and repaired
    // templates separately from pre-existing, unclaimed map rewards.
    StringCopy(gStringVar4, sWorldAudit.errors
        ? COMPOUND_STRING("Revision con incidencias.\nZonas: ")
        : COMPOUND_STRING("Mundo verificado.\nZonas: "));
    WorldUpdate_AppendNumber(sWorldAudit.scannedMaps);
    StringAppend(gStringVar4, COMPOUND_STRING("/"));
    WorldUpdate_AppendNumber(sWorldAudit.totalMaps);
    StringAppend(gStringVar4, COMPOUND_STRING(".\pTramos de ruta: "));
    WorldUpdate_AppendNumber(sWorldAudit.routeSections);
    StringAppend(gStringVar4, COMPOUND_STRING(".\nPoke Balls sin recoger: "));
    WorldUpdate_AppendNumber(sWorldAudit.unclaimedBalls);
    StringAppend(gStringVar4, COMPOUND_STRING(".\pNPCs visibles: "));
    WorldUpdate_AppendNumber(sWorldAudit.visibleNpcs);
    StringAppend(gStringVar4, COMPOUND_STRING(".\nNuevas Poke Balls: "));
    WorldUpdate_AppendNumber(sWorldAudit.appearedBalls);
    StringAppend(gStringVar4, COMPOUND_STRING(".\pNuevos NPCs: "));
    WorldUpdate_AppendNumber(sWorldAudit.appearedNpcs);
    StringAppend(gStringVar4, COMPOUND_STRING(".\nBolas reparadas: "));
    WorldUpdate_AppendNumber(sWorldAudit.repairedBalls);
    StringAppend(gStringVar4, COMPOUND_STRING(".\pNPCs reparados: "));
    WorldUpdate_AppendNumber(sWorldAudit.repairedNpcs);
    StringAppend(gStringVar4, COMPOUND_STRING(".\nIncidencias: "));
    WorldUpdate_AppendNumber(sWorldAudit.errors);
    StringAppend(gStringVar4, COMPOUND_STRING(".\pCambios aplicados: "));
    WorldUpdate_AppendNumber(sWorldAudit.appearedBalls + sWorldAudit.appearedNpcs
        + sWorldAudit.repairedBalls + sWorldAudit.repairedNpcs);
    StringAppend(gStringVar4, COMPOUND_STRING(".\nNada recogido se reinicia."));
#else
    StringCopy(gStringVar4, COMPOUND_STRING("Mundo sin cambios."));
#endif
}
