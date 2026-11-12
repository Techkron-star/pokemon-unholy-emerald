#include "global.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "horizon_market.h"
#include "item.h"
#include "menu.h"
#include "mutation_tiers.h"
#include "overworld.h"
#include "palette.h"
#include "pokelocator.h"
#include "pokenav.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "start_menu.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "new_game.h"

#define HUB_TASK_PRIORITY 8
#define HUB_ROW_HEIGHT 12
#define HUB_ROW_TOP 4

static const u8 *const sFeatureNames[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = COMPOUND_STRING("HORIZON"),
    [HFC_WATCH_FEATURE_POKENAV] = COMPOUND_STRING("POKéNAV"),
    [HFC_WATCH_FEATURE_CRAFTING] = COMPOUND_STRING("CRAFTING"),
    [HFC_WATCH_FEATURE_POKELOCATOR] = COMPOUND_STRING("LOCATOR"),
    [HFC_WATCH_FEATURE_STORAGE] = COMPOUND_STRING("POKéMON BOXES"),
    [HFC_WATCH_FEATURE_FREQUENCY] = COMPOUND_STRING("MUTANT SIGNAL"),
    [HFC_WATCH_FEATURE_RAID] = COMPOUND_STRING("DAILY RAID"),
    [HFC_WATCH_FEATURE_ARCHIVE] = COMPOUND_STRING("ANOMALY ARCHIVE"),
};

static EWRAM_DATA u8 sWatchWindowId;
static EWRAM_DATA u8 sHubFeatures[HFC_WATCH_FEATURE_COUNT];
static EWRAM_DATA u8 sLastFeature;
static EWRAM_DATA bool8 sReturnToStart;
static EWRAM_DATA enum HfcRaidDifficulty sRaidDifficulty;
static EWRAM_DATA bool8 sRaidCaptureSelected;
static EWRAM_DATA enum HfcRaidRewardError sRaidRewardError;

static void OpenHub(void);

u8 HfcWatch_GetTier(void)
{
    return HfcWatch_TierFromItems(CheckBagHasItem(ITEM_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_SUPER_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_HF_POKE_WATCH, 1));
}

u8 HfcWatch_CountBadges(void)
{
    u8 badges = 0;

    for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        if (FlagGet(flag))
            badges++;
    return badges;
}

void ScriptUpgradeRegisteredHfcWatch(struct ScriptContext *ctx)
{
    enum Item registered = gSaveBlock1Ptr->registeredItem;

    Script_RequestEffects(SCREFF_V1 | SCREFF_SAVE);
    if (registered == ITEM_POKE_WATCH || registered == ITEM_SUPER_POKE_WATCH)
        gSaveBlock1Ptr->registeredItem = gSpecialVar_0x8004;
}

static bool8 OpenWatchWindow(u8 width, u8 height)
{
    struct WindowTemplate template =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = width,
        .height = height,
        .paletteNum = 15,
        .baseBlock = 8,
    };

    sWatchWindowId = AddWindow(&template);
    if (sWatchWindowId == WINDOW_NONE)
        return FALSE;

    LoadMessageBoxAndBorderGfx();
    SetStandardWindowBorderStyle(sWatchWindowId, FALSE);
    PutWindowTilemap(sWatchWindowId);
    return TRUE;
}

static void CloseWatchWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sWatchWindowId, TRUE);
    RemoveWindow(sWatchWindowId);
    sWatchWindowId = WINDOW_NONE;
}

static void ReturnFromWatch(void)
{
    if (sReturnToStart)
        ShowStartMenu();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

static void FieldCB_ReturnToWatch(void)
{
    FadeInFromBlack();
    OpenHub();
}

static void PrintWatchText(const u8 *text, u8 x, u8 y)
{
    AddTextPrinterParameterized(sWatchWindowId, FONT_SMALL, text, x, y, TEXT_SKIP_DRAW, NULL);
}

static void PrintRaidCaptureChance(u16 basisPoints, u8 y)
{
    u8 text[48];
    u8 *cursor = StringCopy(text, COMPOUND_STRING("Capture chance  "));
    cursor = ConvertUIntToDecimalStringN(cursor, basisPoints / 100, STR_CONV_MODE_LEFT_ALIGN, 3);
    cursor = StringCopy(cursor, COMPOUND_STRING("."));
    cursor = ConvertUIntToDecimalStringN(cursor, basisPoints % 100, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringCopy(cursor, COMPOUND_STRING("%"));
    PrintWatchText(text, 8, y);
}

static void RenderRaidSelection(void)
{
    static const u8 *const sMessages[] =
    {
        [HFC_RAID_NOT_ON_ROUTE] = COMPOUND_STRING("Travel to an outdoor route."),
        [HFC_RAID_USED_TODAY] = COMPOUND_STRING("Today's raid has been used."),
        [HFC_RAID_NEEDS_PARTY] = COMPOUND_STRING("You need two Pokémon ready to fight."),
        [HFC_RAID_UNAVAILABLE] = COMPOUND_STRING("The raid is unavailable here."),
    };
    enum HfcWatchRaidStatus status = HfcWatch_GetRaidStatus();
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(sRaidDifficulty);
    u8 text[48];
    u8 *cursor;

    if (status != HFC_RAID_READY)
    {
        PrintWatchText(sMessages[status], 8, 23);
        PrintWatchText(COMPOUND_STRING("B  Back"), 8, 94);
        return;
    }

    PrintWatchText(info->name, 8, 19);
    PrintWatchText(COMPOUND_STRING("L/R  Difficulty"), 101, 19);
    if (!HfcWatch_IsRaidDifficultyUnlocked(sRaidDifficulty, HfcWatch_CountBadges()))
    {
        cursor = StringCopy(text, COMPOUND_STRING("Unlocks at "));
        cursor = ConvertUIntToDecimalStringN(cursor, info->minBadges, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(cursor, COMPOUND_STRING(" badges."));
        PrintWatchText(text, 8, 42);
        PrintWatchText(COMPOUND_STRING("B  Back"), 8, 94);
        return;
    }

    u16 day = VarGet(VAR_DAYS);
    u32 trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    struct HfcRaidOpponent boss = HfcWatch_GetRaidOpponent(day, trainerId, sRaidDifficulty, HFC_RAID_BOSS, 0);
    cursor = StringCopy(text, COMPOUND_STRING("Boss  "));
    cursor = StringCopy(cursor, GetSpeciesName(boss.species));
    cursor = StringCopy(cursor, COMPOUND_STRING("  Lv. "));
    ConvertUIntToDecimalStringN(cursor, boss.level, STR_CONV_MODE_LEFT_ALIGN, 2);
    PrintWatchText(text, 8, 34);
    PrintWatchText(COMPOUND_STRING("Two doubles, then the mutated boss."), 8, 49);
    PrintRaidCaptureChance(HfcWatch_GetRaidCaptureChance(day, trainerId, sRaidDifficulty), 64);
    PrintWatchText(COMPOUND_STRING("A  Start raid     B  Back"), 8, 94);
}

static void RenderRaidReward(void)
{
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(HfcWatch_GetActiveRaidDifficulty());

    PrintWatchText(COMPOUND_STRING("Raid won. Choose one reward."), 8, 17);
    PrintWatchText(sRaidCaptureSelected ? COMPOUND_STRING("  Loot    > Capture") : COMPOUND_STRING("> Loot      Capture"), 8, 31);
    if (sRaidCaptureSelected)
    {
        PrintRaidCaptureChance(HfcWatch_GetActiveRaidCaptureChance(), 48);
        PrintWatchText(COMPOUND_STRING("A failed roll ends the raid."), 8, 62);
    }
    else
    {
        for (u8 i = 0; i < HFC_RAID_LOOT_COUNT; i++)
        {
            u8 text[40];
            u8 *cursor = CopyItemName(info->loot[i].item, text);
            cursor = StringCopy(cursor, COMPOUND_STRING("  x"));
            ConvertUIntToDecimalStringN(cursor, info->loot[i].quantity, STR_CONV_MODE_LEFT_ALIGN, 2);
            PrintWatchText(text, 8, 45 + 11 * i);
        }
    }

    if (sRaidRewardError == HFC_RAID_REWARD_BAG_FULL)
        PrintWatchText(COMPOUND_STRING("Make room in the Bag."), 8, 80);
    else if (sRaidRewardError == HFC_RAID_REWARD_STORAGE_FULL)
        PrintWatchText(COMPOUND_STRING("Make room for a Pokémon."), 8, 80);
    PrintWatchText(COMPOUND_STRING("L/R Choose   A Confirm   B Back"), 8, 94);
}

static void RenderRaid(void)
{
    const u8 *message;

    switch (HfcWatch_GetRaidState())
    {
    case HFC_RAID_IDLE:
        RenderRaidSelection();
        return;
    case HFC_RAID_NEXT_BATTLE:
        message = HfcWatch_GetRaidStage() == HFC_RAID_BOSS
            ? COMPOUND_STRING("Round 3/3   Mutated boss")
            : COMPOUND_STRING("Round 2/3   Double battle");
        PrintWatchText(message, 8, 22);
        PrintWatchText(COMPOUND_STRING("Your party keeps its HP and PP."), 8, 42);
        PrintWatchText(COMPOUND_STRING("A  Continue     B  Abandon"), 8, 94);
        return;
    case HFC_RAID_CHOOSE_REWARD:
        RenderRaidReward();
        return;
    case HFC_RAID_LOOT_RECEIVED:
        message = COMPOUND_STRING("Loot added to your Bag.");
        break;
    case HFC_RAID_CAPTURED:
        message = COMPOUND_STRING("The mutant joined your collection!");
        break;
    case HFC_RAID_CAPTURE_FAILED:
        message = COMPOUND_STRING("The mutant escaped.");
        break;
    default:
        message = COMPOUND_STRING("The raid ended.");
        break;
    }

    PrintWatchText(message, 8, 23);
    PrintWatchText(COMPOUND_STRING("B  Back"), 8, 94);
}

static void RenderFrequency(void)
{
    u8 remaining = HfcWatch_GetFrequencyRemaining();
    u8 text[56];
    u8 *cursor;

    if (remaining != 0)
    {
        cursor = ConvertUIntToDecimalStringN(text, remaining, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(cursor, COMPOUND_STRING(" wild Pokémon left here."));
        PrintWatchText(text, 8, 21);
        PrintWatchText(COMPOUND_STRING("B  Back"), 8, 58);
    }
    else if (HfcWatch_CanActivateFrequency())
    {
        cursor = StringCopy(text, COMPOUND_STRING("Tier C/B mutation odds +"));
        cursor = ConvertUIntToDecimalStringN(cursor, HfcWatch_GetFrequencyBoost(HfcWatch_CountBadges()), STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(cursor, COMPOUND_STRING("%"));
        PrintWatchText(text, 8, 21);
        PrintWatchText(COMPOUND_STRING("10 wild Pokémon here, once daily."), 8, 35);
        PrintWatchText(COMPOUND_STRING("A  Activate     B  Back"), 8, 58);
    }
    else
    {
        const u8 *message = FlagGet(FLAG_DAILY_HFC_FREQUENCY)
                          ? COMPOUND_STRING("Today's signal has been used.")
                          : COMPOUND_STRING("Find an area with wild encounters.");
        PrintWatchText(message, 8, 23);
        PrintWatchText(COMPOUND_STRING("B  Back"), 8, 58);
    }
}

static void RenderArchive(void)
{
    static const enum PokemonBadge sArchiveOrder[] =
    {
        POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER,
        POKEMON_BADGE_SUPER, POKEMON_BADGE_OVERLOADED,
        POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
        POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
    };
    u16 registered = HfcWatch_ReadArchive();

    for (u8 i = 0; i < ARRAY_COUNT(sArchiveOrder); i++)
    {
        enum PokemonBadge mutation = sArchiveOrder[i];
        u8 y = 18 + HUB_ROW_HEIGHT * i;
        PrintWatchText(HorizonMarketMutationName(mutation), 8, y);
        PrintWatchText(MutationTierGetName(MutationTierFromBadge(mutation)), 84, y);
        PrintWatchText((registered & mutation) ? COMPOUND_STRING("RECORDED") : COMPOUND_STRING("--"), 125, y);
    }
    PrintWatchText(COMPOUND_STRING("B  Back"), 8, 130);
}

static void RenderPanel(u8 feature)
{
    FillWindowPixelBuffer(sWatchWindowId, PIXEL_FILL(1));
    PrintWatchText(sFeatureNames[feature], 8, 4);

    switch (feature)
    {
    case HFC_WATCH_FEATURE_RAID:
        RenderRaid();
        break;
    case HFC_WATCH_FEATURE_FREQUENCY:
        RenderFrequency();
        break;
    case HFC_WATCH_FEATURE_ARCHIVE:
        RenderArchive();
        break;
    }
    CopyWindowToVram(sWatchWindowId, COPYWIN_FULL);
}

static void CloseWatchPanel(u8 taskId)
{
    CloseWatchWindow();
    DestroyTask(taskId);
}

static void HandleRaidPanelInput(u8 taskId)
{
    enum HfcRaidState state = HfcWatch_GetRaidState();

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (state != HFC_RAID_CHOOSE_REWARD)
            HfcWatch_ResetRaid();
        CloseWatchPanel(taskId);
        OpenHub();
        return;
    }

    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        if (state == HFC_RAID_IDLE && HfcWatch_GetRaidStatus() == HFC_RAID_READY)
        {
            u8 step = JOY_NEW(DPAD_LEFT | L_BUTTON) ? HFC_RAID_DIFFICULTY_COUNT - 1 : 1;
            sRaidDifficulty = (sRaidDifficulty + step) % HFC_RAID_DIFFICULTY_COUNT;
        }
        else if (state == HFC_RAID_CHOOSE_REWARD)
        {
            sRaidCaptureSelected = !sRaidCaptureSelected;
            sRaidRewardError = HFC_RAID_REWARD_OK;
        }
        else
            return;

        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
        return;
    }

    if (!JOY_NEW(A_BUTTON))
        return;

    if ((state == HFC_RAID_IDLE && HfcWatch_StartDailyRaid(sRaidDifficulty))
     || (state == HFC_RAID_NEXT_BATTLE && HfcWatch_ContinueRaid()))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
    }
    else if (state == HFC_RAID_CHOOSE_REWARD)
    {
        sRaidRewardError = HfcWatch_ClaimRaidReward(sRaidCaptureSelected);
        PlaySE(sRaidRewardError == HFC_RAID_REWARD_OK ? SE_SELECT : SE_FAILURE);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
    }
    else if (state == HFC_RAID_NEXT_BATTLE)
    {
        PlaySE(SE_FAILURE);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
    }
}

static void Task_HfcWatchPanel(u8 taskId)
{
    u8 feature = gTasks[taskId].data[0];
    if (gPaletteFade.active)
        return;

    if (feature == HFC_WATCH_FEATURE_RAID)
        HandleRaidPanelInput(taskId);
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
        OpenHub();
    }
    else if (JOY_NEW(A_BUTTON) && feature == HFC_WATCH_FEATURE_FREQUENCY && HfcWatch_ActivateFrequency())
    {
        PlaySE(SE_SELECT);
        RenderPanel(feature);
    }
}

static void OpenPanel(u8 feature)
{
    u8 height = 9;
    if (feature == HFC_WATCH_FEATURE_ARCHIVE)
        height = 18;
    else if (feature == HFC_WATCH_FEATURE_RAID)
        height = 14;

    u8 taskId = CreateTask(Task_HfcWatchPanel, HUB_TASK_PRIORITY);
    if (gTasks[taskId].func != Task_HfcWatchPanel)
    {
        ReturnFromWatch();
        return;
    }
    if (!OpenWatchWindow(24, height))
    {
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }

    gTasks[taskId].data[0] = feature;
    RenderPanel(feature);
}

void HfcWatch_OpenRaidAfterBattle(void)
{
    sRaidCaptureSelected = FALSE;
    sRaidRewardError = HFC_RAID_REWARD_OK;
    FreezeObjectEvents();
    LockPlayerFieldControls();
    Overworld_PlaySpecialMapMusic();
    FadeInFromBlack();
    OpenPanel(HFC_WATCH_FEATURE_RAID);
}

static void Task_LaunchHfcWatchFeature(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    CloseWatchWindow();
    DestroyTask(taskId);

    if (sLastFeature == HFC_WATCH_FEATURE_CRAFTING)
    {
        OpenUnholyCraftingMenu(OpenHub);
        return;
    }

    PlayRainStoppingSoundEffect();
    CleanupOverworldWindowsAndTilemaps();

    switch (sLastFeature)
    {
    case HFC_WATCH_FEATURE_HORIZON_SHOP:
        CreateHorizonShopMenu(FieldCB_ReturnToWatch);
        FadeScreen(FADE_FROM_BLACK, 0);
        break;
    case HFC_WATCH_FEATURE_POKENAV:
        OpenPokeNavWithFieldReturn(FieldCB_ReturnToWatch);
        break;
    case HFC_WATCH_FEATURE_STORAGE:
        OpenPokemonStorageFromField(FieldCB_ReturnToWatch);
        break;
    }
}

static void Task_HfcWatchHub(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    s8 input = Menu_ProcessInput();
    if (input == MENU_NOTHING_CHOSEN)
        return;
    if (input == MENU_B_PRESSED)
    {
        CloseWatchWindow();
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }

    sLastFeature = sHubFeatures[input];
    switch (sLastFeature)
    {
    case HFC_WATCH_FEATURE_HORIZON_SHOP:
    case HFC_WATCH_FEATURE_POKENAV:
    case HFC_WATCH_FEATURE_CRAFTING:
    case HFC_WATCH_FEATURE_STORAGE:
        gTasks[taskId].func = Task_LaunchHfcWatchFeature;
        FadeScreen(FADE_TO_BLACK, 0);
        break;
    case HFC_WATCH_FEATURE_POKELOCATOR:
        CloseWatchWindow();
        DestroyTask(taskId);
        Pokelocator_OpenFromField(OpenHub);
        break;
    default:
        CloseWatchWindow();
        DestroyTask(taskId);
        OpenPanel(sLastFeature);
        break;
    }
}

static void OpenHub(void)
{
    struct HfcWatchProgress progress =
    {
        .badges = HfcWatch_CountBadges(),
        .craftingUnlocked = FlagGet(FLAG_CRAFTING_UNLOCKED),
    };
    u8 hubCount = HfcWatch_BuildMenu(HfcWatch_GetTier(), progress, sHubFeatures);
    u8 initialCursor = 0;
    u8 taskId;

    if (hubCount == 0)
    {
        ReturnFromWatch();
        return;
    }
    taskId = CreateTask(Task_HfcWatchHub, HUB_TASK_PRIORITY);
    if (gTasks[taskId].func != Task_HfcWatchHub)
    {
        ReturnFromWatch();
        return;
    }
    if (!OpenWatchWindow(17, (HUB_ROW_HEIGHT * hubCount + 15) / 8))
    {
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }

    FreezeObjectEvents();
    LockPlayerFieldControls();
    for (u8 i = 0; i < hubCount; i++)
    {
        u8 y = HUB_ROW_TOP + HUB_ROW_HEIGHT * i;
        u8 feature = sHubFeatures[i];

        PrintWatchText(sFeatureNames[feature], 8, y);
        if (feature == HFC_WATCH_FEATURE_HORIZON_SHOP && HorizonMarketHasUnseenMutation())
            FillWindowPixelRect(sWatchWindowId, PIXEL_FILL(4), 126, y + 4, 3, 3);
        if (feature == sLastFeature)
            initialCursor = i;
    }
    InitMenuNormal(sWatchWindowId, FONT_SMALL, 0, HUB_ROW_TOP, HUB_ROW_HEIGHT, hubCount, initialCursor);
    CopyWindowToVram(sWatchWindowId, COPYWIN_FULL);
}

void HfcWatch_OpenHub(bool8 fromStartMenu)
{
    if (FuncIsActiveTask(Task_HfcWatchHub) || FuncIsActiveTask(Task_LaunchHfcWatchFeature)
     || FuncIsActiveTask(Task_HfcWatchPanel))
        return;

    sReturnToStart = fromStartMenu;
    sLastFeature = HFC_WATCH_FEATURE_HORIZON_SHOP;
    sRaidDifficulty = HFC_RAID_STANDARD;
    OpenHub();
}
