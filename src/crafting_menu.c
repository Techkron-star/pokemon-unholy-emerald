#include "global.h"
#include "crafting_menu.h"
#include "event_data.h"
#include "item.h"
#include "item_icon.h"
#include "menu.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/songs.h"

// Crafting screen: recipe list on the left, result + materials panel on the right,
// hint/result line at the bottom. A crafts the selected recipe, B leaves.

#define CRAFT_ICON_TAG 9110 // immune to time blending

#define CRAFT_MAX_INGREDIENTS 2

struct CraftIngredient
{
    enum Item item;
    u16 count;
};

struct CraftRecipe
{
    enum Item result;
    u16 badgeFlag;           // 0 = no badge needed
    const u8 *badgeName;
    struct CraftIngredient ingredients[CRAFT_MAX_INGREDIENTS];
};

enum {
    WIN_RECIPES,
    WIN_PANEL,
    WIN_HINT,
};

static const u8 sText_BadgeStone[] = _("STONE BADGE");
static const u8 sText_Exit[] = _("EXIT");
static const u8 sText_Hint[] = _("A: CRAFT   B: BACK");
static const u8 sText_NeedBadge[] = _("You need the required BADGE.");
static const u8 sText_NeedItems[] = _("Missing materials.");
static const u8 sText_NoRoom[] = _("No room in your BAG.");
static const u8 sText_Crafted[] = _("Crafted successfully!");
static const u8 sText_Ok[] = _("OK");
static const u8 sText_Need[] = _("NEED");
static const u8 sText_Slash[] = _("/");

static const struct CraftRecipe sRecipes[] =
{
    {
        .result = ITEM_SUPER_EXP_SHARE,
        .badgeFlag = FLAG_BADGE01_GET,
        .badgeName = sText_BadgeStone,
        .ingredients = { { ITEM_EXP_SHARE, 1 } },
    },
};

#define NUM_RECIPES ARRAY_COUNT(sRecipes)
#define ROW_EXIT NUM_RECIPES
#define NUM_ROWS (NUM_RECIPES + 1)

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_RECIPES] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 12,
        .height = 12,
        .paletteNum = 15,
        .baseBlock = 0x0008,
    },
    [WIN_PANEL] = {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 1,
        .width = 15,
        .height = 12,
        .paletteNum = 15,
        .baseBlock = 0x0098,
    },
    [WIN_HINT] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 14,
        .width = 28,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x014C,
    },
};

static EWRAM_DATA u8 sWindowIds[3] = {0};
static EWRAM_DATA u8 sIconSpriteId = 0;
static EWRAM_DATA u8 sLastPos = 0;

static void RemoveIcon(void)
{
    if (sIconSpriteId == SPRITE_NONE)
        return;

    FreeSpriteTilesByTag(CRAFT_ICON_TAG);
    FreeSpritePaletteByTag(CRAFT_ICON_TAG);
    DestroySprite(&gSprites[sIconSpriteId]);
    sIconSpriteId = SPRITE_NONE;
}

static void PrintHint(const u8 *text)
{
    FillWindowPixelBuffer(sWindowIds[WIN_HINT], PIXEL_FILL(1));
    AddTextPrinterParameterized(sWindowIds[WIN_HINT], FONT_NORMAL, text, 4, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWindowIds[WIN_HINT], COPYWIN_GFX);
}

static bool32 HasIngredients(const struct CraftRecipe *recipe)
{
    u32 i;

    for (i = 0; i < CRAFT_MAX_INGREDIENTS; i++)
    {
        if (recipe->ingredients[i].count != 0
         && CountTotalItemQuantityInBag(recipe->ingredients[i].item) < recipe->ingredients[i].count)
            return FALSE;
    }
    return TRUE;
}

static void PrintPanelLine(u8 y, const u8 *label, const u8 *status)
{
    AddTextPrinterParameterized(sWindowIds[WIN_PANEL], FONT_NARROW, label, 4, y, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sWindowIds[WIN_PANEL], FONT_NARROW, status, 84, y, TEXT_SKIP_DRAW, NULL);
}

static void ShowRecipe(u8 row)
{
    const struct CraftRecipe *recipe;
    u32 i;
    u8 y = 52;

    RemoveIcon();
    FillWindowPixelBuffer(sWindowIds[WIN_PANEL], PIXEL_FILL(1));

    if (row < NUM_RECIPES)
    {
        u8 spriteId;

        recipe = &sRecipes[row];
        spriteId = AddItemIconSprite(CRAFT_ICON_TAG, CRAFT_ICON_TAG, recipe->result);
        if (spriteId != MAX_SPRITES)
        {
            sIconSpriteId = spriteId;
            gSprites[spriteId].x2 = 172;
            gSprites[spriteId].y2 = 28;
        }

        CopyItemName(recipe->result, gStringVar1);
        AddTextPrinterParameterized(sWindowIds[WIN_PANEL], FONT_NARROW, gStringVar1, 4, 36, TEXT_SKIP_DRAW, NULL);

        if (recipe->badgeFlag != 0)
        {
            PrintPanelLine(y, recipe->badgeName, FlagGet(recipe->badgeFlag) ? sText_Ok : sText_Need);
            y += 14;
        }

        for (i = 0; i < CRAFT_MAX_INGREDIENTS; i++)
        {
            if (recipe->ingredients[i].count == 0)
                continue;

            CopyItemName(recipe->ingredients[i].item, gStringVar1);
            ConvertIntToDecimalStringN(gStringVar2, CountTotalItemQuantityInBag(recipe->ingredients[i].item), STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(gStringVar2, sText_Slash);
            ConvertIntToDecimalStringN(gStringVar3, recipe->ingredients[i].count, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(gStringVar2, gStringVar3);
            PrintPanelLine(y, gStringVar1, gStringVar2);
            y += 14;
        }
    }

    CopyWindowToVram(sWindowIds[WIN_PANEL], COPYWIN_GFX);
}

static void CloseCraftingMenu(u8 taskId)
{
    u32 i;

    RemoveIcon();
    for (i = 0; i < ARRAY_COUNT(sWindowIds); i++)
    {
        ClearStdWindowAndFrameToTransparent(sWindowIds[i], TRUE);
        RemoveWindow(sWindowIds[i]);
    }
    DestroyTask(taskId);
    UnlockPlayerFieldControls();
}

static void TryCraft(u8 row)
{
    const struct CraftRecipe *recipe = &sRecipes[row];
    u32 i;

    if (recipe->badgeFlag != 0 && !FlagGet(recipe->badgeFlag))
    {
        PlaySE(SE_FAILURE);
        PrintHint(sText_NeedBadge);
        return;
    }
    if (!HasIngredients(recipe))
    {
        PlaySE(SE_FAILURE);
        PrintHint(sText_NeedItems);
        return;
    }
    if (!CheckBagHasSpace(recipe->result, 1))
    {
        PlaySE(SE_FAILURE);
        PrintHint(sText_NoRoom);
        return;
    }

    for (i = 0; i < CRAFT_MAX_INGREDIENTS; i++)
    {
        if (recipe->ingredients[i].count != 0)
            RemoveBagItem(recipe->ingredients[i].item, recipe->ingredients[i].count);
    }
    AddBagItem(recipe->result, 1);
    PlaySE(SE_SUCCESS);
    PrintHint(sText_Crafted);
    ShowRecipe(row);
}

static void Task_CraftingMenu(u8 taskId)
{
    s8 inputCode = Menu_ProcessInputNoWrap();
    u8 pos = Menu_GetCursorPos();

    if (pos != sLastPos)
    {
        sLastPos = pos;
        ShowRecipe(pos);
        PrintHint(sText_Hint);
    }

    switch (inputCode)
    {
    case MENU_NOTHING_CHOSEN:
        break;
    case MENU_B_PRESSED:
    case ROW_EXIT:
        PlaySE(SE_SELECT);
        CloseCraftingMenu(taskId);
        break;
    default:
        TryCraft(inputCode);
        break;
    }
}

void CreateCraftingMenu(void)
{
    u32 i;

    LockPlayerFieldControls();
    for (i = 0; i < ARRAY_COUNT(sWindowIds); i++)
    {
        sWindowIds[i] = AddWindow(&sWindowTemplates[i]);
        SetStandardWindowBorderStyle(sWindowIds[i], FALSE);
    }
    sIconSpriteId = SPRITE_NONE;
    sLastPos = 0;

    for (i = 0; i < NUM_RECIPES; i++)
    {
        CopyItemName(sRecipes[i].result, gStringVar4);
        AddTextPrinterParameterized(sWindowIds[WIN_RECIPES], FONT_NARROW, gStringVar4, 8, i * 16 + 1, TEXT_SKIP_DRAW, NULL);
    }
    AddTextPrinterParameterized(sWindowIds[WIN_RECIPES], FONT_NORMAL, sText_Exit, 8, ROW_EXIT * 16 + 1, TEXT_SKIP_DRAW, NULL);
    InitMenuInUpperLeftCornerNormal(sWindowIds[WIN_RECIPES], NUM_ROWS, 0);

    ShowRecipe(0);
    PrintHint(sText_Hint);
    for (i = 0; i < ARRAY_COUNT(sWindowIds); i++)
    {
        PutWindowTilemap(sWindowIds[i]);
        CopyWindowToVram(sWindowIds[i], COPYWIN_FULL);
    }

    CreateTask(Task_CraftingMenu, 8);
}
