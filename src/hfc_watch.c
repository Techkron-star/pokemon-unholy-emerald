#include "global.h"
#include "hfc_watch.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_weather.h"
#include "horizon_market.h"
#include "item.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokelocator.h"
#include "pokenav.h"
#include "script.h"
#include "shop.h"
#include "start_menu.h"
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

static EWRAM_DATA u8 sHubWindowId;
static EWRAM_DATA u8 sHubFeatures[HFC_WATCH_FEATURE_COUNT] = {0};

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

static void Task_LaunchHfcWatchFeature(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    u8 feature = gTasks[taskId].data[1];
    CloseHubWindow();
    DestroyTask(taskId);
    Launch(feature);
}

static void Task_HfcWatchHub(u8 taskId)
{
    s8 input;

    if (gPaletteFade.active)
        return;

    input = Menu_ProcessInput();
    if (input == MENU_NOTHING_CHOSEN)
        return;

    if (input != MENU_B_PRESSED)
    {
        u8 feature = sHubFeatures[input];

        if (feature == HFC_WATCH_FEATURE_HORIZON_SHOP || feature == HFC_WATCH_FEATURE_POKENAV)
        {
            gTasks[taskId].data[1] = feature;
            gTasks[taskId].func = Task_LaunchHfcWatchFeature;
            FadeScreen(FADE_TO_BLACK, 0);
        }
        else
        {
            CloseHubWindow();
            DestroyTask(taskId);
            Launch(feature);
        }
        return;
    }

    bool8 fromStartMenu = gTasks[taskId].data[0];
    CloseHubWindow();
    DestroyTask(taskId);
    if (fromStartMenu)
        ShowStartMenu();
    else
        ReturnToField();
}

void HfcWatch_OpenHub(bool8 fromStartMenu)
{
    struct WindowTemplate template = {0};
    u8 badges = 0;
    u8 taskId, i;

    if (FuncIsActiveTask(Task_HfcWatchHub))
        return;

    FreezeObjectEvents();
    LockPlayerFieldControls();

    for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        if (FlagGet(flag))
            badges++;

    u8 hubCount = HfcWatch_BuildMenu(HfcWatch_GetTier(), badges, sHubFeatures);
    if (hubCount == 0)
    {
        ReturnToField();
        return;
    }

    taskId = CreateTask(Task_HfcWatchHub, HUB_TASK_PRIORITY);
    if (gTasks[taskId].func != Task_HfcWatchHub)
    {
        ReturnToField();
        return;
    }
    gTasks[taskId].data[0] = fromStartMenu;

    template.bg = 0;
    template.tilemapLeft = 1;
    template.tilemapTop = 1;
    template.width = HUB_WINDOW_WIDTH;
    template.height = 2 * hubCount + 2;
    template.paletteNum = 15;
    template.baseBlock = 8;

    sHubWindowId = AddWindow(&template);
    if (sHubWindowId == WINDOW_NONE)
    {
        DestroyTask(taskId);
        ReturnToField();
        return;
    }

    LoadMessageBoxAndBorderGfx();
    SetStandardWindowBorderStyle(sHubWindowId, FALSE);
    for (i = 0; i < hubCount; i++)
    {
        AddTextPrinterParameterized(sHubWindowId, FONT_NORMAL, sFeatureNames[sHubFeatures[i]], 8, (i << 4) + 9, 0, NULL);
        if (sHubFeatures[i] == HFC_WATCH_FEATURE_HORIZON_SHOP && HorizonMarketHasUnseenMutation())
            FillWindowPixelRect(sHubWindowId, PIXEL_FILL(4), 109, (i << 4) + 13, 3, 3);
    }
    InitMenuNormal(sHubWindowId, FONT_NORMAL, 0, 9, 16, hubCount, 0);
    PutWindowTilemap(sHubWindowId);
    CopyWindowToVram(sHubWindowId, COPYWIN_FULL);
}
