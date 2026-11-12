#include "global.h"
#include "hfc_watch.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "field_weather.h"
#include "item.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokelocator.h"
#include "pokenav.h"
#include "script.h"
#include "shop.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"
#include "constants/items.h"

// HFC Watch hub: a small list window over the field, in the same style as the START menu.
// It lists only what the owned tier includes and what can really be launched. The task
// polls input once per frame and only redraws on open, so nothing heavy runs per frame.

#define HUB_TASK_PRIORITY 8
#define HUB_WINDOW_WIDTH  15

static const u8 sText_HorizonShop[] = _("HORIZON");
static const u8 sText_PokeNav[]     = _("POKéNAV");
static const u8 sText_Pokelocator[] = _("LOCATOR");

static const u8 *const sFeatureNames[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = sText_HorizonShop,
    [HFC_WATCH_FEATURE_POKENAV]      = sText_PokeNav,
    [HFC_WATCH_FEATURE_POKELOCATOR]  = sText_Pokelocator,
};

static EWRAM_DATA u8 sHubWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sHubFeatures[HFC_WATCH_FEATURE_COUNT] = {0};
static EWRAM_DATA u8 sHubCount = 0;

u8 HfcWatch_GetTier(void)
{
    return HfcWatch_TierFromItems(CheckBagHasItem(ITEM_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_SUPER_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_HF_POKE_WATCH, 1));
}

static void CloseHubWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sHubWindowId, TRUE);
    RemoveWindow(sHubWindowId);
    sHubWindowId = WINDOW_NONE;
}

static void ReturnToField(void)
{
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

static void Launch(u8 feature)
{
    switch (feature)
    {
    case HFC_WATCH_FEATURE_HORIZON_SHOP:
        // Same hand-off as the START menu entry, which reopens START on exit.
        PlayRainStoppingSoundEffect();
        CleanupOverworldWindowsAndTilemaps();
        CreateHorizonShopMenu();
        FadeScreen(FADE_FROM_BLACK, 0);
        break;
    case HFC_WATCH_FEATURE_POKENAV:
        PlayRainStoppingSoundEffect();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitPokeNav);
        break;
    case HFC_WATCH_FEATURE_POKELOCATOR:
        Pokelocator_OpenFromField(); // takes over the (already locked) player itself
        break;
    default:
        ReturnToField(); // unreachable: the menu only lists launchable features
        break;
    }
}

static void Task_HfcWatchHub(u8 taskId)
{
    s8 input;

    if (gPaletteFade.active)
        return;

    input = Menu_ProcessInput();
    if (input == MENU_NOTHING_CHOSEN)
        return;

    CloseHubWindow();
    DestroyTask(taskId);
    if (input == MENU_B_PRESSED)
        ReturnToField();
    else
        Launch(sHubFeatures[input]);
}

void HfcWatch_OpenHub(void)
{
    struct WindowTemplate template = {0};
    u8 i;

    LockPlayerFieldControls();

    sHubCount = HfcWatch_BuildMenu(HfcWatch_GetTier(), FlagGet(FLAG_SYS_POKENAV_GET), sHubFeatures);
    if (sHubCount == 0)
    {
        ReturnToField();
        return;
    }

    template.bg = 0;
    template.tilemapLeft = 1;
    template.tilemapTop = 1;
    template.width = HUB_WINDOW_WIDTH;
    template.height = 2 * sHubCount + 2;
    template.paletteNum = 15;
    template.baseBlock = 8;

    sHubWindowId = AddWindow(&template);
    if (sHubWindowId == WINDOW_NONE)
    {
        ReturnToField();
        return;
    }

    SetStandardWindowBorderStyle(sHubWindowId, FALSE);
    for (i = 0; i < sHubCount; i++)
        AddTextPrinterParameterized(sHubWindowId, FONT_NORMAL, sFeatureNames[sHubFeatures[i]], 8, (i << 4) + 9, 0, NULL);
    InitMenuNormal(sHubWindowId, FONT_NORMAL, 0, 9, 16, sHubCount, 0);
    PutWindowTilemap(sHubWindowId);
    CopyWindowToVram(sHubWindowId, COPYWIN_FULL);
    CreateTask(Task_HfcWatchHub, HUB_TASK_PRIORITY);
}
