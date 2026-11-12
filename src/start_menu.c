#include "global.h"
#include "item.h"
#include "item_icon.h"
#include "config/save.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "bg.h"
#include "debug.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_object_lock.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_weather.h"
#include "gear.h"
#include "field_screen_effect.h"
#include "frontier_pass.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item_menu.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "menu.h"
#include "new_game.h"
#include "option_menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokenav.h"
#include "safari_zone.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "shop.h"
#include "horizon_market.h"
#include "sound.h"
#include "sprite.h"
#include "start_menu.h"
#include "hfc_watch.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_card.h"
#include "window.h"
#include "union_room.h"
#include "dexnav.h"
#include "wild_encounter.h"
#include "constants/battle_frontier.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Menu actions
enum
{
    MENU_ACTION_POKEDEX,
    MENU_ACTION_POKEMON,
    MENU_ACTION_BAG,
    MENU_ACTION_POKENAV,
    MENU_ACTION_PLAYER,
    MENU_ACTION_SAVE,
    MENU_ACTION_OPTION,
    MENU_ACTION_EXIT,
    MENU_ACTION_RETIRE_SAFARI,
    MENU_ACTION_PLAYER_LINK,
    MENU_ACTION_REST_FRONTIER,
    MENU_ACTION_RETIRE_FRONTIER,
    MENU_ACTION_PYRAMID_BAG,
    MENU_ACTION_DEBUG,
    MENU_ACTION_DEXNAV,
    MENU_ACTION_CRAFTING,
    MENU_ACTION_HFC_WATCH,
};

// Save status
enum
{
    SAVE_IN_PROGRESS,
    SAVE_SUCCESS,
    SAVE_CANCELED,
    SAVE_ERROR
};

// IWRAM common
COMMON_DATA bool8 (*gMenuCallback)(void) = NULL;

// EWRAM
EWRAM_DATA static u8 sSafariBallsWindowId = 0;
EWRAM_DATA static u8 sBattlePyramidFloorWindowId = 0;
EWRAM_DATA static u8 sUnholyCraftWindowId = 0;
EWRAM_DATA static u8 sUnholyCraftIconSpriteId = 0;
EWRAM_DATA static u16 sUnholyCraftSavedPalette[16] = {0};
EWRAM_DATA static u8 sUnholyCraftRecipeCursor = 0;
EWRAM_DATA static u8 sUnholyCraftGearCursor = 0;
EWRAM_DATA static u8 sUnholyCraftPartyCursor = 0;
EWRAM_DATA static u8 sUnholyCraftMode = 0; // 0 = fabricate, 1 = equip
EWRAM_DATA static u8 sUnholyCraftStatus = 0;
EWRAM_DATA static bool8 sUnholyCraftConfirm = FALSE;
EWRAM_DATA static u8 sStartMenuCursorPos = 0;
EWRAM_DATA static u8 sNumStartMenuActions = 0;
EWRAM_DATA static u8 sCurrentStartMenuActions[11] = {0};
EWRAM_DATA static s8 sInitStartMenuData[2] = {0};

EWRAM_DATA static u8 (*sSaveDialogCallback)(void) = NULL;
EWRAM_DATA static u8 sSaveDialogTimer = 0;
EWRAM_DATA static bool8 sSavingComplete = FALSE;
EWRAM_DATA static u8 sSaveInfoWindowId = 0;

// Menu action callbacks
static bool8 StartMenuPokedexCallback(void);
static bool8 StartMenuPokemonCallback(void);
static bool8 StartMenuBagCallback(void);
static bool8 StartMenuPokeNavCallback(void);
static bool8 StartMenuPlayerNameCallback(void);
static bool8 StartMenuSaveCallback(void);
static bool8 StartMenuOptionCallback(void);
static bool8 StartMenuExitCallback(void);
static bool8 StartMenuSafariZoneRetireCallback(void);
static bool8 StartMenuLinkModePlayerNameCallback(void);
static bool8 StartMenuBattlePyramidRetireCallback(void);
static bool8 StartMenuBattlePyramidBagCallback(void);
static bool8 StartMenuDebugCallback(void);
static bool8 StartMenuDexNavCallback(void);
static bool8 StartMenuCraftingCallback(void);
static bool8 StartMenuHfcWatchCallback(void);
static void Task_UnholyCraftingMenu(u8 taskId);
static EWRAM_DATA MainCallback sUnholyCraftOnClose = NULL;
static void CloseUnholyCraftingMenu(void);

// Menu callbacks
static bool8 SaveStartCallback(void);
static bool8 SaveCallback(void);
static bool8 BattlePyramidRetireStartCallback(void);
static bool8 BattlePyramidRetireReturnCallback(void);
static bool8 BattlePyramidRetireCallback(void);
static bool8 HandleStartMenuInput(void);

// Save dialog callbacks
static u8 SaveConfirmSaveCallback(void);
static u8 SaveYesNoCallback(void);
static u8 SaveConfirmInputCallback(void);
static u8 SaveFileExistsCallback(void);
static u8 SaveConfirmOverwriteDefaultNoCallback(void);
static u8 SaveConfirmOverwriteCallback(void);
static u8 SaveOverwriteInputCallback(void);
static u8 SaveSavingMessageCallback(void);
static u8 SaveDoSaveCallback(void);
static u8 SaveSuccessCallback(void);
static u8 SaveReturnSuccessCallback(void);
static u8 SaveErrorCallback(void);
static u8 SaveReturnErrorCallback(void);
static u8 BattlePyramidConfirmRetireCallback(void);
static u8 BattlePyramidRetireYesNoCallback(void);
static u8 BattlePyramidRetireInputCallback(void);

// Task callbacks
static void StartMenuTask(u8 taskId);
static void SaveGameTask(u8 taskId);
static void Task_SaveAfterLinkBattle(u8 taskId);
static void Task_WaitForBattleTowerLinkSave(u8 taskId);
static bool8 FieldCB_ReturnToFieldStartMenu(void);

static const struct WindowTemplate sWindowTemplate_SafariBalls = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 9,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const u8 *const sPyramidFloorNames[FRONTIER_STAGES_PER_CHALLENGE + 1] =
{
    gText_Floor1,
    gText_Floor2,
    gText_Floor3,
    gText_Floor4,
    gText_Floor5,
    gText_Floor6,
    gText_Floor7,
    gText_Peak
};

static const struct WindowTemplate sWindowTemplate_PyramidFloor = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 10,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const struct WindowTemplate sWindowTemplate_PyramidPeak = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 12,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const u8 sText_MenuDebug[] = _("DEBUG");
#define TAG_UNHOLY_CRAFT_ICON 9190
#define UNHOLY_CRAFT_MATERIAL_LIMIT 3
#define UNHOLY_CRAFT_RECIPE_COUNT 8

enum UnholyCraftStatus
{
    CRAFT_STATUS_READY,
    CRAFT_STATUS_CONFIRM,
    CRAFT_STATUS_MATERIALS,
    CRAFT_STATUS_BADGE,
    CRAFT_STATUS_SPACE,
    CRAFT_STATUS_COMPLETED,
    CRAFT_STATUS_SHARE_UPGRADED,
    CRAFT_STATUS_SHARE_OWNED,
    CRAFT_STATUS_EMPTY_SLOT,
    CRAFT_STATUS_NO_GEAR,
    CRAFT_STATUS_EQUIPPED,
    CRAFT_STATUS_ALREADY,
    CRAFT_STATUS_ERROR,
};

struct UnholyCraftMaterial
{
    enum Item itemId;
    u8 quantity;
};

struct UnholyCraftRecipe
{
    enum Item result;
    const u8 *description;
    u16 badgeFlag; // Zero = no badge requirement.
    u8 materialCount;
    struct UnholyCraftMaterial materials[UNHOLY_CRAFT_MATERIAL_LIMIT];
};

static const u8 sUnholyCraftTitle[] = _("CRAFTING");
static const u8 sUnholyCraftRequirements[] = _("MATERIALS / UNLOCK");
static const u8 sUnholyCraftSkinEclipse[] = _("ECLIPSE");
static const u8 sUnholyCraftSkinDaylight[] = _("DAYLIGHT");
static const u8 sUnholyCraftControls[] = _("L/R: RECIPE A: CRAFT SEL: EQUIP");
static const u8 sUnholyEquipControls[] = _("A: SWAP SEL: CRAFT B: EXIT");
static const u8 sUnholyCraftTextColors[] = {1, 2, 3};

static const u8 sUnholyRecipeExp[] = _("Shares 100% EXP with\neligible party Pokemon.");
static const u8 sUnholyRecipeOverloadedExp[] = _("Bonus EXP and training.\nWarms party eggs.");
static const u8 sUnholyRecipeVisor[] = _("Ghost type matchups.\nBuilt from astral relics.");
static const u8 sUnholyRecipeHeart[] = _("Physical attack gear.\nAlternates overload.");
static const u8 sUnholyRecipeMask[] = _("Dark damage gear.\nPowerful even recharging.");
static const u8 sUnholyRecipeScepter[] = _("Dark type matchups.\nRequires Mind Badge.");
static const u8 sUnholyRecipeImpulse[] = _("Special attack gear.\nAlternates overload.");
static const u8 sUnholyRecipeIce[] = _("Ice attacks freeze more.\nIce Pokemon regenerate.");
static const u8 sUnholyCraftReady[] = _("Select a recipe.");
static const u8 sUnholyCraftConfirmText[] = _("Press A again to make it.");
static const u8 sUnholyCraftNeedMats[] = _("Missing materials.");
static const u8 sUnholyCraftNeedBadge[] = _("Badge not yet earned.");
static const u8 sUnholyCraftNoSpace[] = _("No room in the Bag.");
static const u8 sUnholyCraftDone[] = _("Crafted! Gear added to Bag.");
static const u8 sUnholyCraftNoMon[] = _("Empty party slot.");
static const u8 sUnholyCraftNoGear[] = _("That Gear is not in Bag.");
static const u8 sUnholyCraftEquipped[] = _("Gear installed in own slot.");
static const u8 sUnholyCraftAlready[] = _("Already equipped.");
static const u8 sUnholyCraftError[] = _("Unable to finish. Try again.");

static const u8 *const sUnholyCraftStatuses[] =
{
    [CRAFT_STATUS_READY] = sUnholyCraftReady,
    [CRAFT_STATUS_CONFIRM] = sUnholyCraftConfirmText,
    [CRAFT_STATUS_MATERIALS] = sUnholyCraftNeedMats,
    [CRAFT_STATUS_BADGE] = sUnholyCraftNeedBadge,
    [CRAFT_STATUS_SPACE] = sUnholyCraftNoSpace,
    [CRAFT_STATUS_COMPLETED] = sUnholyCraftDone,
    [CRAFT_STATUS_SHARE_UPGRADED] = COMPOUND_STRING("Crafted! EXP Share upgraded."),
    [CRAFT_STATUS_SHARE_OWNED] = COMPOUND_STRING("You already have this upgrade."),
    [CRAFT_STATUS_EMPTY_SLOT] = sUnholyCraftNoMon,
    [CRAFT_STATUS_NO_GEAR] = sUnholyCraftNoGear,
    [CRAFT_STATUS_EQUIPPED] = sUnholyCraftEquipped,
    [CRAFT_STATUS_ALREADY] = sUnholyCraftAlready,
    [CRAFT_STATUS_ERROR] = sUnholyCraftError,
};

// Every ingredient is consumed; badges only unlock specific recipes.
// The resulting Gear item can be installed via the EQUIP tab without
// replacing a held item. Repeating a recipe creates another physical copy.
static const struct UnholyCraftRecipe sUnholyCraftRecipes[UNHOLY_CRAFT_RECIPE_COUNT] =
{
    { ITEM_SUPER_EXP_SHARE, sUnholyRecipeExp, FLAG_BADGE01_GET, 1,
      {{ ITEM_EXP_SHARE, 1 }} },
    { ITEM_OVERLOADED_EXP_SHARE, sUnholyRecipeOverloadedExp, FLAG_BADGE04_GET, 2,
      {{ ITEM_SUPER_EXP_SHARE, 1 }, { ITEM_NUGGET, 1 }} },
    { ITEM_GEAR_VISOR_COSMICO, sUnholyRecipeVisor, 0, 3,
      {{ ITEM_SPELL_TAG, 2 }, { ITEM_MOON_STONE, 2 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_CORAZON_TITAN, sUnholyRecipeHeart, 0, 3,
      {{ ITEM_MUSCLE_BAND, 1 }, { ITEM_METAL_COAT, 2 }, { ITEM_HARD_STONE, 2 }} },
    { ITEM_GEAR_SKI_MASK, sUnholyRecipeMask, 0, 3,
      {{ ITEM_BLACK_GLASSES, 1 }, { ITEM_DARK_GEM, 3 }, { ITEM_DUSK_STONE, 1 }} },
    { ITEM_GEAR_CETRO_SINIESTRO, sUnholyRecipeScepter, FLAG_BADGE07_GET, 2,
      {{ ITEM_BLACK_GLASSES, 1 }, { ITEM_SPELL_TAG, 2 }} },
    { ITEM_GEAR_IMPULSO_COSMICO, sUnholyRecipeImpulse, 0, 3,
      {{ ITEM_WISE_GLASSES, 1 }, { ITEM_STAR_PIECE, 2 }, { ITEM_MOON_STONE, 1 }} },
    { ITEM_GEAR_PERMAFROST_FRAGMENT, sUnholyRecipeIce, 0, 3,
      {{ ITEM_ICE_STONE, 2 }, { ITEM_NEVER_MELT_ICE, 1 }, { ITEM_MYSTIC_WATER, 1 }} },
};

// IDs 1..6 match the stable Gear enum values. A Gear is a tangible item in
// the Bag, but lives in a separate Pokemon field after being installed.
static const enum Item sUnholyGearItems[GEAR_COUNT] =
{
    [GEAR_NONE] = ITEM_NONE,
    [GEAR_VISOR_COSMICO] = ITEM_GEAR_VISOR_COSMICO,
    [GEAR_CORAZON_TITAN] = ITEM_GEAR_CORAZON_TITAN,
    [GEAR_SKI_MASK] = ITEM_GEAR_SKI_MASK,
    [GEAR_CETRO_SINIESTRO] = ITEM_GEAR_CETRO_SINIESTRO,
    [GEAR_IMPULSO_COSMICO] = ITEM_GEAR_IMPULSO_COSMICO,
    [GEAR_PERMAFROST_FRAGMENT] = ITEM_GEAR_PERMAFROST_FRAGMENT,
};
static const struct WindowTemplate sUnholyCraftWindow =
{
    .bg = 0, .tilemapLeft = 2, .tilemapTop = 1,
    .width = 26, .height = 17, .paletteNum = 14, .baseBlock = 0x100,
};

static const struct MenuAction sStartMenuItems[] =
{
    [MENU_ACTION_POKEDEX]         = {gText_MenuPokedex, {.u8_void = StartMenuPokedexCallback}},
    [MENU_ACTION_POKEMON]         = {gText_MenuPokemon, {.u8_void = StartMenuPokemonCallback}},
    [MENU_ACTION_BAG]             = {gText_MenuBag,     {.u8_void = StartMenuBagCallback}},
    [MENU_ACTION_POKENAV]         = {gText_MenuPokenav, {.u8_void = StartMenuPokeNavCallback}},
    [MENU_ACTION_PLAYER]          = {gText_MenuPlayer,  {.u8_void = StartMenuPlayerNameCallback}},
    [MENU_ACTION_SAVE]            = {gText_MenuSave,    {.u8_void = StartMenuSaveCallback}},
    [MENU_ACTION_OPTION]          = {gText_MenuOption,  {.u8_void = StartMenuOptionCallback}},
    [MENU_ACTION_EXIT]            = {gText_MenuExit,    {.u8_void = StartMenuExitCallback}},
    [MENU_ACTION_RETIRE_SAFARI]   = {gText_MenuRetire,  {.u8_void = StartMenuSafariZoneRetireCallback}},
    [MENU_ACTION_PLAYER_LINK]     = {gText_MenuPlayer,  {.u8_void = StartMenuLinkModePlayerNameCallback}},
    [MENU_ACTION_REST_FRONTIER]   = {gText_MenuRest,    {.u8_void = StartMenuSaveCallback}},
    [MENU_ACTION_RETIRE_FRONTIER] = {gText_MenuRetire,  {.u8_void = StartMenuBattlePyramidRetireCallback}},
    [MENU_ACTION_PYRAMID_BAG]     = {gText_MenuBag,     {.u8_void = StartMenuBattlePyramidBagCallback}},
    [MENU_ACTION_DEBUG]           = {sText_MenuDebug,   {.u8_void = StartMenuDebugCallback}},
    [MENU_ACTION_DEXNAV]          = {gText_MenuDexNav,  {.u8_void = StartMenuDexNavCallback}},
    [MENU_ACTION_CRAFTING]       = {gText_MenuCrafting, {.u8_void = StartMenuCraftingCallback}},
    [MENU_ACTION_HFC_WATCH]      = {gText_MenuHfcWatch, {.u8_void = StartMenuHfcWatchCallback}},
};

static const struct BgTemplate sBgTemplates_LinkBattleSave[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sWindowTemplates_LinkBattleSave[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x194
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sSaveInfoWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 14,
    .height = 10,
    .paletteNum = 15,
    .baseBlock = 8
};

// Local functions
static void BuildStartMenuActions(void);
static void AddStartMenuAction(u8 action);
static void BuildNormalStartMenu(void);
static void BuildDebugStartMenu(void);
static void BuildSafariZoneStartMenu(void);
static void BuildLinkModeStartMenu(void);
static void BuildUnionRoomStartMenu(void);
static void BuildBattlePikeStartMenu(void);
static void BuildBattlePyramidStartMenu(void);
static void BuildMultiPartnerRoomStartMenu(void);
static void ShowSafariBallsWindow(void);
static void ShowPyramidFloorWindow(void);
static void RemoveExtraStartMenuWindows(void);
static bool32 PrintStartMenuActions(s8 *pIndex, u32 count);
static bool32 InitStartMenuStep(void);
static void InitStartMenu(void);
static void CreateStartMenuTask(TaskFunc followupFunc);
static void InitSave(void);
static u8 RunSaveCallback(void);
static void ShowSaveMessage(const u8 *message, u8 (*saveCallback)(void));
static void HideSaveMessageWindow(void);
static void HideSaveInfoWindow(void);
static void SaveStartTimer(void);
static bool8 SaveSuccesTimer(void);
static bool8 SaveErrorTimer(void);
static void InitBattlePyramidRetire(void);
static void VBlankCB_LinkBattleSave(void);
static bool32 InitSaveWindowAfterLinkBattle(u8 *par1);
static void CB2_SaveAfterLinkBattle(void);
static void ShowSaveInfoWindow(void);
static void RemoveSaveInfoWindow(void);
static void HideStartMenuWindow(void);
static void HideStartMenuDebug(void);

static void BuildStartMenuActions(void)
{
    sNumStartMenuActions = 0;

    if (IsOverworldLinkActive() == TRUE)
    {
        BuildLinkModeStartMenu();
    }
    else if (InUnionRoom() == TRUE)
    {
        BuildUnionRoomStartMenu();
    }
    else if (GetSafariZoneFlag() == TRUE)
    {
        BuildSafariZoneStartMenu();
    }
    else if (InBattlePike())
    {
        BuildBattlePikeStartMenu();
    }
    else if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        BuildBattlePyramidStartMenu();
    }
    else if (InMultiPartnerRoom())
    {
        BuildMultiPartnerRoomStartMenu();
    }
    else
    {
        if (DEBUG_OVERWORLD_MENU == TRUE && DEBUG_OVERWORLD_IN_MENU == TRUE)
            BuildDebugStartMenu();
        else
            BuildNormalStartMenu();
    }
}

static void UnholyCraftRefreshIcon(enum Item itemId)
{
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_UNHOLY_CRAFT_ICON);
        FreeSpritePaletteByTag(TAG_UNHOLY_CRAFT_ICON);
        DestroySprite(&gSprites[sUnholyCraftIconSpriteId]);
        sUnholyCraftIconSpriteId = MAX_SPRITES;
    }

    if (itemId == ITEM_NONE)
        return;

    sUnholyCraftIconSpriteId = AddItemIconSprite(TAG_UNHOLY_CRAFT_ICON, TAG_UNHOLY_CRAFT_ICON, itemId);
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        // Item sprites start near (0, 0); x2/y2 are relative offsets.
        // Use absolute screen coordinates so the item remains visible.
        gSprites[sUnholyCraftIconSpriteId].x = 190;
        gSprites[sUnholyCraftIconSpriteId].y = 58;
        gSprites[sUnholyCraftIconSpriteId].x2 = 0;
        gSprites[sUnholyCraftIconSpriteId].y2 = 0;
    }
}

static void UnholyCraftPrint(u8 x, u8 y, const u8 *text)
{
    AddTextPrinterParameterized4(sUnholyCraftWindowId, FONT_NARROW, x, y, 0, 0,
                                 sUnholyCraftTextColors, TEXT_SKIP_DRAW, text);
}

static void UnholyCraftPrintMaterial(u8 row, const struct UnholyCraftMaterial *ingredient)
{
    u16 owned = CountTotalItemQuantityInBag(ingredient->itemId);

    StringCopy(gStringVar4, GetItemName(ingredient->itemId));
    StringAppend(gStringVar4, COMPOUND_STRING("  "));
    ConvertIntToDecimalStringN(gStringVar2, owned, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar4, gStringVar2);
    StringAppend(gStringVar4, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(gStringVar3, ingredient->quantity, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringAppend(gStringVar4, gStringVar3);
    if (owned >= ingredient->quantity)
        StringAppend(gStringVar4, COMPOUND_STRING(" OK"));
    UnholyCraftPrint(8, 82 + row * 12, gStringVar4);
}

static void UnholyCraftRenderMenu(void)
{
    const u8 *skin = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? sUnholyCraftSkinDaylight : sUnholyCraftSkinEclipse;
    enum Item iconItem;

    FillWindowPixelBuffer(sUnholyCraftWindowId, PIXEL_FILL(1));
    DrawUnholyUiPanelAccents(sUnholyCraftWindowId, 26 * 8, 17 * 8, 19);
    UnholyCraftPrint(4, 3, sUnholyCraftTitle);
    UnholyCraftPrint(137, 3, skin);

    if (sUnholyCraftMode == 0)
    {
        const struct UnholyCraftRecipe *recipe = &sUnholyCraftRecipes[sUnholyCraftRecipeCursor];

        StringCopy(gStringVar4, COMPOUND_STRING("> "));
        StringAppend(gStringVar4, GetItemName(recipe->result));
        UnholyCraftPrint(4, 23, gStringVar4);
        UnholyCraftPrint(8, 40, sUnholyCraftStatus == CRAFT_STATUS_READY ? recipe->description : sUnholyCraftStatuses[sUnholyCraftStatus]);
        UnholyCraftPrint(4, 69, sUnholyCraftRequirements);

        for (u8 i = 0; i < recipe->materialCount; i++)
            UnholyCraftPrintMaterial(i, &recipe->materials[i]);
        if (recipe->badgeFlag)
            UnholyCraftPrint(8, 82 + recipe->materialCount * 12,
                             FlagGet(recipe->badgeFlag) ? COMPOUND_STRING("BADGE: OK") : COMPOUND_STRING("BADGE: LOCKED"));
        UnholyCraftPrint(4, 122, sUnholyCraftControls);
        iconItem = recipe->result;
    }
    else
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sUnholyCraftPartyCursor];
        enum Gear existing = Gear_GetEquipped(mon);
        enum Gear chosen = sUnholyCraftGearCursor;

        UnholyCraftPrint(4, 22, COMPOUND_STRING("EQUIP / REMOVE GEAR"));
        StringCopy(gStringVar4, chosen == GEAR_NONE ? COMPOUND_STRING("> REMOVE GEAR") : GetItemName(sUnholyGearItems[chosen]));
        UnholyCraftPrint(6, 39, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("PARTY "));
        ConvertIntToDecimalStringN(gStringVar3, sUnholyCraftPartyCursor + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(gStringVar4, gStringVar3);
        StringAppend(gStringVar4, COMPOUND_STRING(": "));
        if (GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
        {
            GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
            StringAppend(gStringVar4, gStringVar1);
        }
        else
            StringAppend(gStringVar4, COMPOUND_STRING("EMPTY"));
        UnholyCraftPrint(6, 57, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("CURRENT: "));
        StringAppend(gStringVar4, existing == GEAR_NONE ? COMPOUND_STRING("NONE") : gGearsInfo[existing].name);
        UnholyCraftPrint(6, 73, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("IN BAG: "));
        ConvertIntToDecimalStringN(gStringVar3, chosen == GEAR_NONE ? 0 : CountTotalItemQuantityInBag(sUnholyGearItems[chosen]), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, chosen == GEAR_NONE ? COMPOUND_STRING("N/A") : gStringVar3);
        UnholyCraftPrint(6, 89, gStringVar4);

        if (sUnholyCraftStatus != CRAFT_STATUS_READY)
            UnholyCraftPrint(6, 105, sUnholyCraftStatuses[sUnholyCraftStatus]);
        else
            UnholyCraftPrint(6, 105, COMPOUND_STRING("L/R: GEAR  UP/DOWN: PARTY"));
        UnholyCraftPrint(4, 122, sUnholyEquipControls);
        iconItem = chosen == GEAR_NONE ? ITEM_NONE : sUnholyGearItems[chosen];
    }

    CopyWindowToVram(sUnholyCraftWindowId, COPYWIN_FULL);
    UnholyCraftRefreshIcon(iconItem);
}

static u8 UnholyCraftCheckRecipe(const struct UnholyCraftRecipe *recipe)
{
    if (recipe->badgeFlag != 0 && !FlagGet(recipe->badgeFlag))
        return CRAFT_STATUS_BADGE;
    if (recipe->result == ITEM_OVERLOADED_EXP_SHARE && CheckBagHasItem(recipe->result, 1))
        return CRAFT_STATUS_SHARE_OWNED;

    for (u8 i = 0; i < recipe->materialCount; i++)
    {
        if (!CheckBagHasItem(recipe->materials[i].itemId, recipe->materials[i].quantity))
            return CRAFT_STATUS_MATERIALS;
    }

    if (!CheckBagHasSpace(recipe->result, 1)
     && !(recipe->result == ITEM_OVERLOADED_EXP_SHARE && CountTotalItemQuantityInBag(ITEM_SUPER_EXP_SHARE) == 1))
        return CRAFT_STATUS_SPACE;
    return CRAFT_STATUS_READY;
}

static u8 UnholyCraftFabricate(const struct UnholyCraftRecipe *recipe)
{
    u8 status = UnholyCraftCheckRecipe(recipe);

    if (status != CRAFT_STATUS_READY)
        return status;

    for (u8 i = 0; i < recipe->materialCount; i++)
    {
        if (!RemoveBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity))
        {
            // Guard even against unexpected inventory failure: all-or-nothing.
            while (i > 0)
            {
                i--;
                AddBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity);
            }
            return CRAFT_STATUS_ERROR;
        }
    }

    if (!AddBagItem(recipe->result, 1))
    {
        for (u8 i = 0; i < recipe->materialCount; i++)
            AddBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity);
        return CRAFT_STATUS_SPACE;
    }

    if (recipe->result == ITEM_OVERLOADED_EXP_SHARE)
    {
        if (gSaveBlock1Ptr->registeredItem == ITEM_SUPER_EXP_SHARE)
            gSaveBlock1Ptr->registeredItem = ITEM_OVERLOADED_EXP_SHARE;
        FlagClear(FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED);
        FlagClear(I_EXP_SHARE_FLAG);
        FlagSet(FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED);
        return CRAFT_STATUS_SHARE_UPGRADED;
    }
    return CRAFT_STATUS_COMPLETED;
}

static u8 UnholyCraftSwapGear(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sUnholyCraftPartyCursor];
    enum Gear selected = sUnholyCraftGearCursor;
    enum Gear previous;
    enum Item incoming, outgoing;

    if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
        return CRAFT_STATUS_EMPTY_SLOT;

    previous = Gear_GetEquipped(mon);
    if (previous == selected)
        return CRAFT_STATUS_ALREADY;

    incoming = sUnholyGearItems[selected];
    outgoing = sUnholyGearItems[previous];

    // Do not duplicate a Gear: remove its tangible inventory copy before equipping.
    if (incoming != ITEM_NONE && !RemoveBagItem(incoming, 1))
        return CRAFT_STATUS_NO_GEAR;

    // On replacement or removal the old Gear returns to the Bag.
    // If this fails, put the new one back and leave Pokemon data unchanged.
    if (outgoing != ITEM_NONE && !AddBagItem(outgoing, 1))
    {
        if (incoming != ITEM_NONE)
            AddBagItem(incoming, 1);
        return CRAFT_STATUS_SPACE;
    }

    Gear_SetEquipped(mon, selected);
    return CRAFT_STATUS_EQUIPPED;
}

static void ReturnFromUnholyCrafting(void)
{
    MainCallback onClose = sUnholyCraftOnClose;

    sUnholyCraftOnClose = NULL;
    if (onClose != NULL)
        onClose();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

void OpenUnholyCraftingMenu(MainCallback onClose)
{
    sUnholyCraftOnClose = onClose;
    sUnholyCraftMode = 0;
    sUnholyCraftGearCursor = GEAR_VISOR_COSMICO;
    sUnholyCraftStatus = CRAFT_STATUS_READY;
    sUnholyCraftConfirm = FALSE;
    sUnholyCraftIconSpriteId = MAX_SPRITES;

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(14)], sUnholyCraftSavedPalette, sizeof(sUnholyCraftSavedPalette));
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    sUnholyCraftWindowId = AddWindow(&sUnholyCraftWindow);
    if (sUnholyCraftWindowId == 0xFF)
    {
        LoadPalette(sUnholyCraftSavedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
        FadeScreen(FADE_FROM_BLACK, 0);
        ReturnFromUnholyCrafting();
        return;
    }
    SetStandardWindowBorderStyle(sUnholyCraftWindowId, FALSE);
    PutWindowTilemap(sUnholyCraftWindowId);
    UnholyCraftRenderMenu();
    CreateTask(Task_UnholyCraftingMenu, 8);
    FadeScreen(FADE_FROM_BLACK, 0);
}

static void CloseUnholyCraftingMenu(void)
{
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_UNHOLY_CRAFT_ICON);
        FreeSpritePaletteByTag(TAG_UNHOLY_CRAFT_ICON);
        DestroySprite(&gSprites[sUnholyCraftIconSpriteId]);
        sUnholyCraftIconSpriteId = MAX_SPRITES;
    }
    ClearStdWindowAndFrameToTransparent(sUnholyCraftWindowId, TRUE);
    RemoveWindow(sUnholyCraftWindowId);
    LoadPalette(sUnholyCraftSavedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
}

static void Task_UnholyCraftingMenu(u8 taskId)
{
    bool8 changed = FALSE;

    if (gPaletteFade.active)
        return;

    if (JOY_NEW(B_BUTTON))
    {
        if (sUnholyCraftConfirm)
        {
            sUnholyCraftConfirm = FALSE;
            sUnholyCraftStatus = CRAFT_STATUS_READY;
            UnholyCraftRenderMenu();
            return;
        }
        CloseUnholyCraftingMenu();
        DestroyTask(taskId);
        ReturnFromUnholyCrafting();
        return;
    }

    if (JOY_NEW(SELECT_BUTTON))
    {
        sUnholyCraftMode ^= 1;
        changed = TRUE;
    }
    else if (JOY_NEW(DPAD_LEFT) || JOY_NEW(L_BUTTON))
    {
        if (sUnholyCraftMode == 0)
            sUnholyCraftRecipeCursor = (sUnholyCraftRecipeCursor + UNHOLY_CRAFT_RECIPE_COUNT - 1) % UNHOLY_CRAFT_RECIPE_COUNT;
        else
            sUnholyCraftGearCursor = (sUnholyCraftGearCursor + GEAR_COUNT - 1) % GEAR_COUNT;
        changed = TRUE;
    }
    else if (JOY_NEW(DPAD_RIGHT) || JOY_NEW(R_BUTTON))
    {
        if (sUnholyCraftMode == 0)
            sUnholyCraftRecipeCursor = (sUnholyCraftRecipeCursor + 1) % UNHOLY_CRAFT_RECIPE_COUNT;
        else
            sUnholyCraftGearCursor = (sUnholyCraftGearCursor + 1) % GEAR_COUNT;
        changed = TRUE;
    }
    else if (sUnholyCraftMode && JOY_NEW(DPAD_UP))
    {
        sUnholyCraftPartyCursor = (sUnholyCraftPartyCursor + PARTY_SIZE - 1) % PARTY_SIZE;
        changed = TRUE;
    }
    else if (sUnholyCraftMode && JOY_NEW(DPAD_DOWN))
    {
        sUnholyCraftPartyCursor = (sUnholyCraftPartyCursor + 1) % PARTY_SIZE;
        changed = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sUnholyCraftMode)
        {
            sUnholyCraftStatus = UnholyCraftSwapGear();
            PlaySE(sUnholyCraftStatus == CRAFT_STATUS_EQUIPPED ? SE_SUCCESS : SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
        else if (sUnholyCraftRecipeCursor == 0)
        {
            CloseUnholyCraftingMenu();
            DestroyTask(taskId);
        sUnholyCraftOnClose = NULL;
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            ScriptContext_SetupScript(EventScript_CraftSuperExpShare_Recipe);
            return;
        }
        else if (!sUnholyCraftConfirm)
        {
            sUnholyCraftStatus = UnholyCraftCheckRecipe(&sUnholyCraftRecipes[sUnholyCraftRecipeCursor]);
            if (sUnholyCraftStatus == CRAFT_STATUS_READY)
            {
                sUnholyCraftConfirm = TRUE;
                sUnholyCraftStatus = CRAFT_STATUS_CONFIRM;
            }
            else
                PlaySE(SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
        else
        {
            sUnholyCraftStatus = UnholyCraftFabricate(&sUnholyCraftRecipes[sUnholyCraftRecipeCursor]);
            sUnholyCraftConfirm = FALSE;
            PlaySE(sUnholyCraftStatus == CRAFT_STATUS_COMPLETED || sUnholyCraftStatus == CRAFT_STATUS_SHARE_UPGRADED
                ? SE_SUCCESS : SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
    }

    if (changed)
    {
        sUnholyCraftConfirm = FALSE;
        sUnholyCraftStatus = CRAFT_STATUS_READY;
        PlaySE(SE_SELECT);
        UnholyCraftRenderMenu();
    }
}

static bool8 StartMenuCraftingCallback(void)
{
    if (!gPaletteFade.active)
    {
        RemoveExtraStartMenuWindows();
        ClearStdWindowAndFrameToTransparent(GetStartMenuWindowId(), FALSE);
        RemoveStartMenuWindow();
        OpenUnholyCraftingMenu(NULL);
        return TRUE;
    }
    return FALSE;
}

// Same hand-off as the crafting menu: drop the START window, the hub opens over the field.
static bool8 StartMenuHfcWatchCallback(void)
{
    if (!gPaletteFade.active)
    {
        RemoveExtraStartMenuWindows();
        ClearStdWindowAndFrameToTransparent(GetStartMenuWindowId(), FALSE);
        RemoveStartMenuWindow();
        HfcWatch_OpenHub(TRUE);
        return TRUE;
    }
    return FALSE;
}

static void AddStartMenuAction(u8 action)
{
    AppendToList(sCurrentStartMenuActions, &sNumStartMenuActions, action);
}

static void BuildNormalStartMenu(void)
{
    bool8 hasWatch = HfcWatch_GetTier() != HFC_WATCH_TIER_NONE;

    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEDEX);

    if (DN_FLAG_DEXNAV_GET != 0 && FlagGet(DN_FLAG_DEXNAV_GET))
        AddStartMenuAction(MENU_ACTION_DEXNAV);

    if (FlagGet(FLAG_SYS_POKEMON_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEMON);

    AddStartMenuAction(MENU_ACTION_BAG);
    // With a watch, Horizon and PokeNav live inside its hub (one entry instead of two).
    if (hasWatch)
        AddStartMenuAction(MENU_ACTION_HFC_WATCH);
    if (!hasWatch && FlagGet(FLAG_CRAFTING_UNLOCKED) == TRUE)
        AddStartMenuAction(MENU_ACTION_CRAFTING);

    if (!hasWatch && FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKENAV);

    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_SAVE);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildDebugStartMenu(void)
{
    bool8 hasWatch = HfcWatch_GetTier() != HFC_WATCH_TIER_NONE;

    AddStartMenuAction(MENU_ACTION_DEBUG);
    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEDEX);
    if (FlagGet(FLAG_SYS_POKEMON_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);
    if (hasWatch)
        AddStartMenuAction(MENU_ACTION_HFC_WATCH);
    if (!hasWatch && FlagGet(FLAG_CRAFTING_UNLOCKED) == TRUE)
        AddStartMenuAction(MENU_ACTION_CRAFTING);
    if (!hasWatch && FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKENAV);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_SAVE);
    AddStartMenuAction(MENU_ACTION_OPTION);
}

static void BuildSafariZoneStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_RETIRE_SAFARI);
    AddStartMenuAction(MENU_ACTION_POKEDEX);
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildLinkModeStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);

    if (FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
    {
        AddStartMenuAction(MENU_ACTION_POKENAV);
    }

    AddStartMenuAction(MENU_ACTION_PLAYER_LINK);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildUnionRoomStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);

    if (FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
    {
        AddStartMenuAction(MENU_ACTION_POKENAV);
    }

    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildBattlePikeStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEDEX);
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildBattlePyramidStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PYRAMID_BAG);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_REST_FRONTIER);
    AddStartMenuAction(MENU_ACTION_RETIRE_FRONTIER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildMultiPartnerRoomStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void ShowSafariBallsWindow(void)
{
    sSafariBallsWindowId = AddWindow(&sWindowTemplate_SafariBalls);
    PutWindowTilemap(sSafariBallsWindowId);
    DrawStdWindowFrame(sSafariBallsWindowId, FALSE);
    if (IS_FRLG)
    {
        ConvertIntToDecimalStringN(gStringVar1, gSafariZoneStepCounter, STR_CONV_MODE_RIGHT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, 600, STR_CONV_MODE_RIGHT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar3, gNumSafariBalls, STR_CONV_MODE_RIGHT_ALIGN, 2);
        StringExpandPlaceholders(gStringVar4, gText_MenuSafariStats);
        AddTextPrinterParameterized(sSafariBallsWindowId, FONT_NORMAL, gStringVar4, 4, 3, 0xFF, NULL);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, gNumSafariBalls, STR_CONV_MODE_RIGHT_ALIGN, 2);
        StringExpandPlaceholders(gStringVar4, gText_SafariBallStock);
        AddTextPrinterParameterized(sSafariBallsWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sSafariBallsWindowId, COPYWIN_GFX);
}

static void ShowPyramidFloorWindow(void)
{
    if (gSaveBlock2Ptr->frontier.curChallengeBattleNum == FRONTIER_STAGES_PER_CHALLENGE)
        sBattlePyramidFloorWindowId = AddWindow(&sWindowTemplate_PyramidPeak);
    else
        sBattlePyramidFloorWindowId = AddWindow(&sWindowTemplate_PyramidFloor);

    PutWindowTilemap(sBattlePyramidFloorWindowId);
    DrawStdWindowFrame(sBattlePyramidFloorWindowId, FALSE);
    StringCopy(gStringVar1, sPyramidFloorNames[gSaveBlock2Ptr->frontier.curChallengeBattleNum]);
    StringExpandPlaceholders(gStringVar4, gText_BattlePyramidFloor);
    AddTextPrinterParameterized(sBattlePyramidFloorWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sBattlePyramidFloorWindowId, COPYWIN_GFX);
}

static void RemoveExtraStartMenuWindows(void)
{
    if (GetSafariZoneFlag())
    {
        ClearStdWindowAndFrameToTransparent(sSafariBallsWindowId, FALSE);
        CopyWindowToVram(sSafariBallsWindowId, COPYWIN_GFX);
        RemoveWindow(sSafariBallsWindowId);
    }
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        ClearStdWindowAndFrameToTransparent(sBattlePyramidFloorWindowId, FALSE);
        RemoveWindow(sBattlePyramidFloorWindowId);
    }
}

static bool32 PrintStartMenuActions(s8 *pIndex, u32 count)
{
    s8 index = *pIndex;

    do
    {
        if (sStartMenuItems[sCurrentStartMenuActions[index]].func.u8_void == StartMenuPlayerNameCallback)
        {
            PrintPlayerNameOnWindow(GetStartMenuWindowId(), sStartMenuItems[sCurrentStartMenuActions[index]].text, 8, (index << 4) + 9);
        }
        else
        {
            StringExpandPlaceholders(gStringVar4, sStartMenuItems[sCurrentStartMenuActions[index]].text);
            AddTextPrinterParameterized(GetStartMenuWindowId(), FONT_NORMAL, gStringVar4, 8, (index << 4) + 9, TEXT_SKIP_DRAW, NULL);
        }

        if (sCurrentStartMenuActions[index] == MENU_ACTION_HFC_WATCH
         && HorizonMarketHasUnseenMutation())
        {
            // The standard menu palette already includes saturated red in color 4.
            // A three-pixel notification dot fits in the seven-tile START menu.
            FillWindowPixelRect(GetStartMenuWindowId(), PIXEL_FILL(4), 51, (index << 4) + 13, 3, 3);
            CopyWindowToVram(GetStartMenuWindowId(), COPYWIN_GFX);
        }

        index++;
        if (index >= sNumStartMenuActions)
        {
            *pIndex = index;
            return TRUE;
        }

        count--;
    }
    while (count != 0);

    *pIndex = index;
    return FALSE;
}

static bool32 InitStartMenuStep(void)
{
    s8 state = sInitStartMenuData[0];

    switch (state)
    {
    case 0:
        sInitStartMenuData[0]++;
        break;
    case 1:
        BuildStartMenuActions();
        sInitStartMenuData[0]++;
        break;
    case 2:
        LoadMessageBoxAndBorderGfx();
        DrawStdWindowFrame(AddStartMenuWindow(sNumStartMenuActions), FALSE);
        sInitStartMenuData[1] = 0;
        sInitStartMenuData[0]++;
        break;
    case 3:
        if (GetSafariZoneFlag())
            ShowSafariBallsWindow();
        if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
            ShowPyramidFloorWindow();
        sInitStartMenuData[0]++;
        break;
    case 4:
        if (PrintStartMenuActions(&sInitStartMenuData[1], 2))
            sInitStartMenuData[0]++;
        break;
    case 5:
        sStartMenuCursorPos = InitMenuNormal(GetStartMenuWindowId(), FONT_NORMAL, 0, 9, 16, sNumStartMenuActions, sStartMenuCursorPos);
        CopyWindowToVram(GetStartMenuWindowId(), COPYWIN_MAP);
        return TRUE;
    }

    return FALSE;
}

static void InitStartMenu(void)
{
    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    while (!InitStartMenuStep())
        ;
}

static void StartMenuTask(u8 taskId)
{
    if (InitStartMenuStep() == TRUE)
        SwitchTaskToFollowupFunc(taskId);
}

static void CreateStartMenuTask(TaskFunc followupFunc)
{
    u8 taskId;

    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    taskId = CreateTask(StartMenuTask, 0x50);
    SetTaskFuncWithFollowupFunc(taskId, StartMenuTask, followupFunc);
}

static bool8 FieldCB_ReturnToFieldStartMenu(void)
{
    if (InitStartMenuStep() == FALSE)
    {
        return FALSE;
    }

    ReturnToFieldOpenStartMenu();
    return TRUE;
}

void ShowReturnToFieldStartMenu(void)
{
    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    gFieldCallback2 = FieldCB_ReturnToFieldStartMenu;
}

void Task_ShowStartMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
    case 0:
        if (InUnionRoom() == TRUE)
            SetUsingUnionRoomStartMenu();

        gMenuCallback = HandleStartMenuInput;
        task->data[0]++;
        break;
    case 1:
        if (gMenuCallback() == TRUE)
            DestroyTask(taskId);
        break;
    }
}

void ShowStartMenu(void)
{
    if (!IsOverworldLinkActive())
    {
        FreezeObjectEvents();
        PlayerFreeze();
        StopPlayerAvatar();
    }
    CreateStartMenuTask(Task_ShowStartMenu);
    LockPlayerFieldControls();
}

static bool8 HandleStartMenuInput(void)
{
    if (JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        sStartMenuCursorPos = Menu_MoveCursor(-1);
    }

    if (JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        sStartMenuCursorPos = Menu_MoveCursor(1);
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sStartMenuItems[sCurrentStartMenuActions[sStartMenuCursorPos]].func.u8_void == StartMenuPokedexCallback)
        {
            if (GetNationalPokedexCount(FLAG_GET_SEEN) == 0)
                return FALSE;
        }
        if (sCurrentStartMenuActions[sStartMenuCursorPos] == MENU_ACTION_DEXNAV
          && MapHasNoEncounterData())
            return FALSE;

        gMenuCallback = sStartMenuItems[sCurrentStartMenuActions[sStartMenuCursorPos]].func.u8_void;

        if (gMenuCallback != StartMenuSaveCallback
            && gMenuCallback != StartMenuExitCallback
            && gMenuCallback != StartMenuDebugCallback
            && gMenuCallback != StartMenuHfcWatchCallback
            && gMenuCallback != StartMenuSafariZoneRetireCallback
            && gMenuCallback != StartMenuBattlePyramidRetireCallback)
        {
           FadeScreen(FADE_TO_BLACK, 0);
        }

        return FALSE;
    }

    if (JOY_NEW(START_BUTTON | B_BUTTON))
    {
        RemoveExtraStartMenuWindows();
        HideStartMenu();
        return TRUE;
    }

    return FALSE;
}

bool8 StartMenuPokedexCallback(void)
{
    if (!gPaletteFade.active)
    {
        IncrementGameStat(GAME_STAT_CHECKED_POKEDEX);
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_OpenPokedex);

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPokemonCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_PartyMenuFromStartMenu); // Display party menu

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuBagCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_BagMenuFromStartMenu); // Display bag menu

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPokeNavCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitPokeNav);  // Display PokéNav

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPlayerNameCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();

        if (IsOverworldLinkActive() || InUnionRoom())
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card
        else if (FlagGet(FLAG_SYS_FRONTIER_PASS))
            ShowFrontierPass(CB2_ReturnToFieldWithOpenMenu); // Display frontier pass
        else
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuSaveCallback(void)
{
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        RemoveExtraStartMenuWindows();

    gMenuCallback = SaveStartCallback; // Display save menu

    return FALSE;
}

static bool8 StartMenuOptionCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitOptionMenu); // Display option menu
        gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuExitCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenu(); // Hide start menu

    return TRUE;
}

static bool8 StartMenuDebugCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenuDebug(); // Hide start menu without enabling movement

    if (DEBUG_OVERWORLD_MENU)
    {
        FreezeObjectEvents();
        Debug_ShowMainMenu();
    }

return TRUE;
}

static bool8 StartMenuSafariZoneRetireCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenu();
    SafariZoneRetirePrompt();

    return TRUE;
}

static void HideStartMenuDebug(void)
{
    PlaySE(SE_SELECT);
    ClearStdWindowAndFrame(GetStartMenuWindowId(), TRUE);
    RemoveStartMenuWindow();
}

static bool8 StartMenuLinkModePlayerNameCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        CleanupOverworldWindowsAndTilemaps();
        ShowTrainerCardInLink(gLocalLinkPlayerId, CB2_ReturnToFieldWithOpenMenu);

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuBattlePyramidRetireCallback(void)
{
    gMenuCallback = BattlePyramidRetireStartCallback; // Confirm retire

    return FALSE;
}

// Functionally unused
void ShowBattlePyramidStartMenu(void)
{
    ClearDialogWindowAndFrameToTransparent(0, FALSE);
    ScriptUnfreezeObjectEvents();
    CreateStartMenuTask(Task_ShowStartMenu);
    LockPlayerFieldControls();
}

static bool8 StartMenuBattlePyramidBagCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_PyramidBagMenuFromStartMenu);

        return TRUE;
    }

    return FALSE;
}

static bool8 SaveStartCallback(void)
{
    InitSave();
    gMenuCallback = SaveCallback;

    return FALSE;
}

static bool8 SaveCallback(void)
{
    switch (RunSaveCallback())
    {
    case SAVE_IN_PROGRESS:
        return FALSE;
    case SAVE_CANCELED: // Back to start menu
        ClearDialogWindowAndFrameToTransparent(0, FALSE);
        InitStartMenu();
        gMenuCallback = HandleStartMenuInput;
        return FALSE;
    case SAVE_SUCCESS:
    case SAVE_ERROR:    // Close start menu
        ClearDialogWindowAndFrameToTransparent(0, TRUE);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        SoftResetInBattlePyramid();
        return TRUE;
    }

    return FALSE;
}

static bool8 BattlePyramidRetireStartCallback(void)
{
    InitBattlePyramidRetire();
    gMenuCallback = BattlePyramidRetireCallback;

    return FALSE;
}

static bool8 BattlePyramidRetireReturnCallback(void)
{
    InitStartMenu();
    gMenuCallback = HandleStartMenuInput;

    return FALSE;
}

static bool8 BattlePyramidRetireCallback(void)
{
    switch (RunSaveCallback())
    {
    case SAVE_SUCCESS: // No (Stay in battle pyramid)
        RemoveExtraStartMenuWindows();
        gMenuCallback = BattlePyramidRetireReturnCallback;
        return FALSE;
    case SAVE_IN_PROGRESS:
        return FALSE;
    case SAVE_CANCELED: // Yes (Retire from battle pyramid)
        ClearDialogWindowAndFrameToTransparent(0, TRUE);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        ScriptContext_SetupScript(BattlePyramid_Retire);
        return TRUE;
    }

    return FALSE;
}

static void InitSave(void)
{
    SaveMapView();
    sSaveDialogCallback = SaveConfirmSaveCallback;
    sSavingComplete = FALSE;
}

static u8 RunSaveCallback(void)
{
    // True if text is still printing
    if (RunTextPrintersAndIsPrinter0Active() == TRUE)
    {
        return SAVE_IN_PROGRESS;
    }

    sSavingComplete = FALSE;
    return sSaveDialogCallback();
}

void SaveGame(void)
{
    InitSave();
    CreateTask(SaveGameTask, 0x50);
}

static void ShowSaveMessage(const u8 *message, u8 (*saveCallback)(void))
{
    StringExpandPlaceholders(gStringVar4, message);
    LoadMessageBoxAndFrameGfx(0, TRUE);
    AddTextPrinterForMessage(TRUE);
    sSavingComplete = TRUE;
    sSaveDialogCallback = saveCallback;
}

static void SaveGameTask(u8 taskId)
{
    u8 status = RunSaveCallback();

    switch (status)
    {
    case SAVE_CANCELED:
    case SAVE_ERROR:
        gSpecialVar_Result = 0;
        break;
    case SAVE_SUCCESS:
        gSpecialVar_Result = status;
        break;
    case SAVE_IN_PROGRESS:
        return;
    }

    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void HideSaveMessageWindow(void)
{
    ClearDialogWindowAndFrame(0, TRUE);
}

static void HideSaveInfoWindow(void)
{
    RemoveSaveInfoWindow();
}

static void SaveStartTimer(void)
{
    sSaveDialogTimer = 60;
}

static bool8 SaveSuccesTimer(void)
{
    sSaveDialogTimer--;

    if (JOY_HELD(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        return TRUE;
    }
    if (sSaveDialogTimer == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static bool8 SaveErrorTimer(void)
{
    if (sSaveDialogTimer != 0)
    {
        sSaveDialogTimer--;
    }
    else if (JOY_HELD(A_BUTTON))
    {
        return TRUE;
    }

    return FALSE;
}

static u8 SaveConfirmSaveCallback(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), FALSE);
    RemoveStartMenuWindow();
    ShowSaveInfoWindow();

    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        ShowSaveMessage(gText_BattlePyramidConfirmRest, SaveYesNoCallback);
    }
    else
    {
        ShowSaveMessage(gText_ConfirmSave, SaveYesNoCallback);
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveYesNoCallback(void)
{
    DisplayYesNoMenuDefaultYes(); // Show Yes/No menu
    sSaveDialogCallback = SaveConfirmInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        switch (gSaveFileStatus)
        {
        case SAVE_STATUS_EMPTY:
        case SAVE_STATUS_CORRUPT:
            if (gDifferentSaveFile == FALSE && !SKIP_SAVE_CONFIRMATION)
            {
                sSaveDialogCallback = SaveFileExistsCallback;
                return SAVE_IN_PROGRESS;
            }

            sSaveDialogCallback = SaveSavingMessageCallback;
            return SAVE_IN_PROGRESS;
        default:
            if (SKIP_SAVE_CONFIRMATION)
                sSaveDialogCallback = SaveSavingMessageCallback;
            else
                sSaveDialogCallback = SaveFileExistsCallback;
            return SAVE_IN_PROGRESS;
        }
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveInfoWindow();
        HideSaveMessageWindow();
        return SAVE_CANCELED;
    }

    return SAVE_IN_PROGRESS;
}

// A different save file exists
static u8 SaveFileExistsCallback(void)
{
    if (gDifferentSaveFile == TRUE)
    {
        ShowSaveMessage(gText_DifferentSaveFile, SaveConfirmOverwriteDefaultNoCallback);
    }
    else
    {
        ShowSaveMessage(gText_AlreadySavedFile, SaveConfirmOverwriteCallback);
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmOverwriteDefaultNoCallback(void)
{
    DisplayYesNoMenuWithDefault(1); // Show Yes/No menu (No selected as default)
    sSaveDialogCallback = SaveOverwriteInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmOverwriteCallback(void)
{
    DisplayYesNoMenuDefaultYes(); // Show Yes/No menu
    sSaveDialogCallback = SaveOverwriteInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveOverwriteInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        sSaveDialogCallback = SaveSavingMessageCallback;
        return SAVE_IN_PROGRESS;
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveInfoWindow();
        HideSaveMessageWindow();
        return SAVE_CANCELED;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveSavingMessageCallback(void)
{
    ShowSaveMessage(gText_SavingDontTurnOff, SaveDoSaveCallback);
    return SAVE_IN_PROGRESS;
}

static u8 SaveDoSaveCallback(void)
{
    u8 saveStatus;

    IncrementGameStat(GAME_STAT_SAVED_GAME);
    PausePyramidChallenge();

    if (gDifferentSaveFile == TRUE)
    {
        saveStatus = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
        gDifferentSaveFile = FALSE;
    }
    else
    {
        saveStatus = TrySavingData(SAVE_NORMAL);
    }

    if (saveStatus == SAVE_STATUS_OK)
        ShowSaveMessage(gText_PlayerSavedGame, SaveSuccessCallback);
    else
        ShowSaveMessage(gText_SaveError, SaveErrorCallback);

    SaveStartTimer();
    return SAVE_IN_PROGRESS;
}

static u8 SaveSuccessCallback(void)
{
    if (!IsTextPrinterActiveOnWindow(0))
    {
        PlaySE(SE_SAVE);
        sSaveDialogCallback = SaveReturnSuccessCallback;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveReturnSuccessCallback(void)
{
    if (!IsSEPlaying() && SaveSuccesTimer())
    {
        HideSaveInfoWindow();
        return SAVE_SUCCESS;
    }
    else
    {
        return SAVE_IN_PROGRESS;
    }
}

static u8 SaveErrorCallback(void)
{
    if (!IsTextPrinterActiveOnWindow(0))
    {
        PlaySE(SE_BOO);
        sSaveDialogCallback = SaveReturnErrorCallback;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveReturnErrorCallback(void)
{
    if (!SaveErrorTimer())
    {
        return SAVE_IN_PROGRESS;
    }
    else
    {
        HideSaveInfoWindow();
        return SAVE_ERROR;
    }
}

static void InitBattlePyramidRetire(void)
{
    sSaveDialogCallback = BattlePyramidConfirmRetireCallback;
    sSavingComplete = FALSE;
}

static u8 BattlePyramidConfirmRetireCallback(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), FALSE);
    RemoveStartMenuWindow();
    ShowSaveMessage(gText_BattlePyramidConfirmRetire, BattlePyramidRetireYesNoCallback);

    return SAVE_IN_PROGRESS;
}

static u8 BattlePyramidRetireYesNoCallback(void)
{
    DisplayYesNoMenuWithDefault(1); // Show Yes/No menu (No selected as default)
    sSaveDialogCallback = BattlePyramidRetireInputCallback;

    return SAVE_IN_PROGRESS;
}

static u8 BattlePyramidRetireInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        return SAVE_CANCELED;
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveMessageWindow();
        return SAVE_SUCCESS;
    }

    return SAVE_IN_PROGRESS;
}

static void VBlankCB_LinkBattleSave(void)
{
    TransferPlttBuffer();
}

static bool32 InitSaveWindowAfterLinkBattle(u8 *state)
{
    switch (*state)
    {
    case 0:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
        SetVBlankCallback(NULL);
        ScanlineEffect_Stop();
        DmaClear16(3, PLTT, PLTT_SIZE);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        break;
    case 1:
        ResetSpriteData();
        ResetTasks();
        ResetPaletteFade();
        ScanlineEffect_Clear();
        break;
    case 2:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates_LinkBattleSave, ARRAY_COUNT(sBgTemplates_LinkBattleSave));
        InitWindows(sWindowTemplates_LinkBattleSave);
        LoadUserWindowBorderGfx_(0, 8, BG_PLTT_ID(14));
        Menu_LoadStdPalAt(BG_PLTT_ID(15));
        break;
    case 3:
        ShowBg(0);
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        SetVBlankCallback(VBlankCB_LinkBattleSave);
        EnableInterrupts(1);
        break;
    case 4:
        return TRUE;
    }

    (*state)++;
    return FALSE;
}

void CB2_SetUpSaveAfterLinkBattle(void)
{
    if (InitSaveWindowAfterLinkBattle(&gMain.state))
    {
        CreateTask(Task_SaveAfterLinkBattle, 0x50);
        SetMainCallback2(CB2_SaveAfterLinkBattle);
    }
}

static void CB2_SaveAfterLinkBattle(void)
{
    RunTasks();
    UpdatePaletteFade();
}

static void Task_SaveAfterLinkBattle(u8 taskId)
{
    s16 *state = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        switch (*state)
        {
        case 0:
            FillWindowPixelBuffer(0, PIXEL_FILL(1));
            AddTextPrinterParameterized2(0,
                                        FONT_NORMAL,
                                        gText_SavingDontTurnOffPower,
                                        TEXT_SKIP_DRAW,
                                        NULL,
                                        TEXT_COLOR_DARK_GRAY,
                                        TEXT_COLOR_WHITE,
                                        TEXT_COLOR_LIGHT_GRAY);
            DrawTextBorderOuter(0, 8, 14);
            PutWindowTilemap(0);
            CopyWindowToVram(0, COPYWIN_FULL);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);

            if (gWirelessCommType != 0 && InUnionRoom())
            {
                if (Link_AnyPartnersPlayingFRLG_JP())
                {
                    *state = 1;
                }
                else
                {
                    *state = 5;
                }
            }
            else
            {
                gSoftResetDisabled = TRUE;
                *state = 1;
            }
            break;
        case 1:
            SetContinueGameWarpStatusToDynamicWarp();
            WriteSaveBlock2();
            *state = 2;
            break;
        case 2:
            if (WriteSaveBlock1Sector())
            {
                ClearContinueGameWarpStatus2();
                *state = 3;
                gSoftResetDisabled = FALSE;
            }
            break;
        case 3:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            *state = 4;
            break;
        case 4:
            FreeAllWindowBuffers();
            SetMainCallback2(gMain.savedCallback);
            DestroyTask(taskId);
            break;
        case 5:
            CreateTask(Task_LinkFullSave, 5);
            *state = 6;
            break;
        case 6:
            if (!FuncIsActiveTask(Task_LinkFullSave))
            {
                *state = 3;
            }
            break;
        }
    }
}

static void ShowSaveInfoWindow(void)
{
    struct WindowTemplate saveInfoWindow = sSaveInfoWindowTemplate;
    enum Gender gender;
    u8 color;
    u32 xOffset;
    u32 yOffset;

    if (!FlagGet(FLAG_SYS_POKEDEX_GET))
    {
        saveInfoWindow.height -= 2;
    }

    sSaveInfoWindowId = AddWindow(&saveInfoWindow);
    DrawStdWindowFrame(sSaveInfoWindowId, FALSE);

    gender = gSaveBlock2Ptr->playerGender;
    color = TEXT_COLOR_RED;  // Red when female, blue when male.

    if (gender == MALE)
        color = TEXT_COLOR_BLUE;

    // Print region name
    yOffset = 1;
    BufferSaveMenuText(SAVE_MENU_LOCATION, gStringVar4, TEXT_COLOR_GREEN);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, 0, yOffset, TEXT_SKIP_DRAW, NULL);

    // Print player name
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingPlayer, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_NAME, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    PrintPlayerNameOnWindow(sSaveInfoWindowId, gStringVar4, xOffset, yOffset);

    // Print badge count
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingBadges, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_BADGES, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);

    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
    {
        // Print Pokédex count
        yOffset += 16;
        AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingPokedex, 0, yOffset, TEXT_SKIP_DRAW, NULL);
        BufferSaveMenuText(SAVE_MENU_CAUGHT, gStringVar4, color);
        xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
        AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);
    }

    // Print play time
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingTime, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_PLAY_TIME, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sSaveInfoWindowId, COPYWIN_GFX);
}

static void RemoveSaveInfoWindow(void)
{
    ClearStdWindowAndFrame(sSaveInfoWindowId, FALSE);
    RemoveWindow(sSaveInfoWindowId);
}

static void Task_WaitForBattleTowerLinkSave(u8 taskId)
{
    if (!FuncIsActiveTask(Task_LinkFullSave))
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

#define tInBattleTower data[2]

void SaveForBattleTowerLink(void)
{
    u8 taskId = CreateTask(Task_LinkFullSave, 5);
    gTasks[taskId].tInBattleTower = TRUE;
    gTasks[CreateTask(Task_WaitForBattleTowerLinkSave, 6)].data[1] = taskId;
}

#undef tInBattleTower

static void HideStartMenuWindow(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), TRUE);
    RemoveStartMenuWindow();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void HideStartMenu(void)
{
    PlaySE(SE_SELECT);
    HideStartMenuWindow();
}

void AppendToList(u8 *list, u8 *pos, u8 newEntry)
{
    list[*pos] = newEntry;
    (*pos)++;
}

static bool8 StartMenuDexNavCallback(void)
{
    CreateTask(Task_OpenDexNavFromStartMenu, 0);
    return TRUE;
}

void Script_ForceSaveGame(struct ScriptContext *ctx)
{
    SaveGame();
    ShowSaveInfoWindow();
    gMenuCallback = SaveCallback;
    sSaveDialogCallback = SaveSavingMessageCallback;
}
