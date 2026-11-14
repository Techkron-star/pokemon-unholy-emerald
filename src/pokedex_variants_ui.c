#include "global.h"
#include "pokedex_variants.h"
#include "bg.h"
#include "horizon_market.h"
#include "malloc.h"
#include "main.h"
#include "mutation_icons.h"
#include "mutation_tiers.h"
#include "palette.h"
#include "pokemon.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "unholy_ui.h"
#include "window.h"
#include "constants/songs.h"

#define VISIBLE_VARIANTS 6
#define VARIANT_WIDTH 208
#define VARIANT_HEIGHT 112

struct DexVariantSprite
{
    SpriteCallback callback;
    bool8 used, invisible;
};

struct DexVariantUi
{
    struct PokedexVariantCollection collection;
    struct UnholyUiPalette palette;
    struct DexVariantSprite sprites[MAX_SPRITES];
    u16 objPalette[256];
    u16 bgTilemap[BG_SCREEN_SIZE / 2];
    TaskFunc onReturn;
    s32 bgX, bgY;
    u16 scanCursor;
    u8 owner, window, cursor, priority;
    u8 icons[VISIBLE_VARIANTS];
    bool8 detail;
    bool8 scanning;
};

static EWRAM_DATA struct DexVariantUi *sVariants = NULL;

// The regular HGSS Info page occupies BG2 tiles 1..644. BG0 shares its character
// block; use only the inactive Cry/Size region, keeping Info and its footprint intact.
static const struct WindowTemplate sWindow =
{
    .bg = 0, .tilemapLeft = 2, .tilemapTop = 3, .width = 26, .height = 14,
    .paletteNum = 14, .baseBlock = 645,
};
STATIC_ASSERT(645 + 26 * 14 <= 1024, DexVariantTilesStayInCharacterMemory);

static void Task_Variants(u8 taskId);
static void Task_WaitForVariants(u8 taskId) {}

static void Print(u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_Print(sVariants->window, FONT_SMALL_NARROW, x, y, width, text);
}

static void DestroyIcons(void)
{
    for (u8 i = 0; i < VISIBLE_VARIANTS; i++)
    {
        MutationIcon_Destroy(sVariants->icons[i]);
        sVariants->icons[i] = MAX_SPRITES;
    }
}

static u8 *AppendSigned(u8 *text, s16 value)
{
    if (value < 0) text = StringAppend(text, COMPOUND_STRING("-"));
    else if (value > 0) text = StringAppend(text, COMPOUND_STRING("+"));
    return ConvertIntToDecimalStringN(text, value < 0 ? -value : value, STR_CONV_MODE_LEFT_ALIGN, 4);
}

static void FormatRange(u8 *text, const struct PokedexVariantRange *range)
{
    text[0] = EOS;
    text = AppendSigned(text, range->minimum);
    if (range->minimum != range->maximum)
    {
        text = StringAppend(text, COMPOUND_STRING(".."));
        text = AppendSigned(text, range->maximum);
    }
    StringAppend(text, COMPOUND_STRING("%"));
}

static void DrawGrid(void)
{
    u8 text[80];
    const struct PokedexVariantCollection *collection = &sVariants->collection;
    u8 *end = StringCopy(text, COMPOUND_STRING("OWNED "));
    end = ConvertIntToDecimalStringN(end, collection->total, STR_CONV_MODE_LEFT_ALIGN, 4);
    end = StringAppend(end, COMPOUND_STRING("  NORMAL "));
    end = ConvertIntToDecimalStringN(end, collection->normal, STR_CONV_MODE_LEFT_ALIGN, 4);
    end = StringAppend(end, COMPOUND_STRING("  COMBO "));
    ConvertIntToDecimalStringN(end, collection->combined, STR_CONV_MODE_LEFT_ALIGN, 4);
    Print(5, 20, 198, text);

    u8 first = sVariants->cursor / VISIBLE_VARIANTS * VISIBLE_VARIANTS;
    for (u8 i = 0; i < VISIBLE_VARIANTS && first + i < POKEDEX_VARIANT_COUNT; i++)
    {
        u8 index = first + i;
        u16 badge = PokedexVariants_BadgeAt(index);
        u8 x = 4 + i % 3 * 68, y = 34 + i / 3 * 28;
        bool8 selected = index == sVariants->cursor;
        u8 fill = UnholyUi_CardFill(selected);
        UnholyUi_Card(sVariants->window, x, y, 65, 26, selected);
        const u8 *name = badge == POKEMON_BADGE_OVERLOADED ? COMPOUND_STRING("OVERLOAD") : HorizonMarketMutationName(badge);
        UnholyUi_PrintOnFill(sVariants->window, FONT_SMALL_NARROW, x + 18, y + 1, 46, fill,
                            name);
        end = StringCopy(text, COMPOUND_STRING("x"));
        end = ConvertIntToDecimalStringN(end, collection->variants[index].count, STR_CONV_MODE_LEFT_ALIGN, 4);
        end = StringAppend(end, COMPOUND_STRING(" "));
        enum MutationTier tier = MutationTierFromBadge(badge);
        StringAppend(end, tier == MUTATION_TIER_OMEGA ? COMPOUND_STRING("O") : MutationTierGetName(tier));
        UnholyUi_PrintOnFill(sVariants->window, FONT_SMALL_NARROW, x + 18, y + 13, 46, fill, text);
        sVariants->icons[i] = MutationIcon_Create(badge, 16 + x + 10, 24 + y + 13, 0);
    }
    Print(5, 96, 171, COMPOUND_STRING("+:CHOOSE  A:BONUS  B:INFO"));
    end = ConvertIntToDecimalStringN(text, first / VISIBLE_VARIANTS + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringAppend(end, COMPOUND_STRING("/2"));
    Print(184, 96, 19, text);
}

static void DrawDetail(void)
{
    static const u8 *const labels[] =
    {
        COMPOUND_STRING("HP"), COMPOUND_STRING("ATK"), COMPOUND_STRING("DEF"),
        COMPOUND_STRING("SPE"), COMPOUND_STRING("SPA"), COMPOUND_STRING("SPD"),
    };
    u8 text[80];
    const struct PokedexVariantBucket *bucket = &sVariants->collection.variants[sVariants->cursor];
    if (!bucket->count)
    {
        Print(8, 35, 192, COMPOUND_STRING("NO OWNED INDIVIDUALS OF THIS VARIANT"));
        Print(8, 53, 192, COMPOUND_STRING("CHECK YOUR PARTY AND POKéMON BOXES"));
    }
    else
    {
        u8 *end = StringCopy(text, COMPOUND_STRING("OWNED "));
        end = ConvertIntToDecimalStringN(end, bucket->count, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringAppend(end, COMPOUND_STRING(" - TOTAL STAT BONUSES"));
        Print(5, 20, 198, text);
        for (u8 stat = 0; stat < NUM_STATS; stat++)
        {
            u8 x = 5 + stat % 2 * 102, y = 36 + stat / 2 * 15;
            Print(x, y, 24, labels[stat]);
            FormatRange(text, &bucket->bonuses[stat]);
            Print(x + 26, y, 72, text);
        }
        u8 *value = StringCopy(text, COMPOUND_STRING("STA "));
        FormatRange(value, &bucket->bonuses[NUM_STATS]);
        Print(5, 82, 198, text);
    }
    // Bonuses belong to whole individuals, so combinations legitimately contribute.
    Print(5, 96, 198, COMPOUND_STRING("COMBOS ADD; L/R:VARIANT  B:GRID"));
}

static void Render(void)
{
    DestroyIcons();
    u8 text[24];
    u8 window = sVariants->window;
    u16 badge = PokedexVariants_BadgeAt(sVariants->cursor);
    UnholyUi_Frame(window, sVariants->detail ? HorizonMarketMutationName(badge) : COMPOUND_STRING("VARIANTS"));
    if (sVariants->detail)
        StringAppend(StringCopy(text, COMPOUND_STRING("TIER ")), MutationTierGetName(MutationTierFromBadge(badge)));
    Print(112, 6, 90, sVariants->detail ? text : GetSpeciesName(sVariants->collection.species));
    if (sVariants->scanning)
    {
        Print(8, 34, 192, COMPOUND_STRING("READING PARTY AND POKéMON BOXES"));
        UnholyUi_Bar(window, 8, 63, 192, sVariants->scanCursor, PokedexVariants_ScanTotal(), 5);
        Print(5, 96, 198, COMPOUND_STRING("B:CANCEL"));
    }
    else if (sVariants->detail)
    {
        // Real font glyphs are 12 px, with the final line ending above the border.
        FillWindowPixelRect(window, PIXEL_FILL(1), 5, VARIANT_HEIGHT - 19, VARIANT_WIDTH - 10, 1);
        DrawDetail();
    }
    else
        DrawGrid();
    CopyWindowToVram(window, COPYWIN_FULL);
}

static void Close(u8 taskId)
{
    DestroyIcons();
    HideBg(0);
    RemoveWindow(sVariants->window);
    CpuCopy16(sVariants->bgTilemap, GetBgTilemapBuffer(0), BG_SCREEN_SIZE);
    CopyBgTilemapBufferToVram(0);
    SetBgAttribute(0, BG_ATTR_PRIORITY, sVariants->priority);
    ChangeBgX(0, sVariants->bgX, BG_COORD_SET);
    ChangeBgY(0, sVariants->bgY, BG_COORD_SET);
    UnholyUi_ReleasePalette(&sVariants->palette);
    LoadPalette(sVariants->objPalette, OBJ_PLTT_ID(0), sizeof(sVariants->objPalette));
    for (u8 i = 0; i < MAX_SPRITES; i++)
    {
        if (!sVariants->sprites[i].used)
            continue;
        gSprites[i].callback = sVariants->sprites[i].callback;
        gSprites[i].invisible = sVariants->sprites[i].invisible;
    }
    gTasks[sVariants->owner].func = sVariants->onReturn;
    Free(sVariants);
    sVariants = NULL;
    DestroyTask(taskId);
}

static void Task_Variants(u8 taskId)
{
    if (IsDma3ManagerBusyWithBgCopy())
        return;
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (!sVariants->scanning && sVariants->detail)
        {
            sVariants->detail = FALSE;
            Render();
        }
        else Close(taskId);
        return;
    }
    if (sVariants->scanning)
    {
        sVariants->scanning = !PokedexVariants_ScanStep(&sVariants->collection, &sVariants->scanCursor, 8);
        Render();
        return;
    }
    if (JOY_NEW(A_BUTTON) && !sVariants->detail)
        sVariants->detail = TRUE;
    else if (sVariants->detail && JOY_NEW(L_BUTTON | R_BUTTON | DPAD_LEFT | DPAD_RIGHT))
        sVariants->cursor = (sVariants->cursor + (JOY_NEW(L_BUTTON | DPAD_LEFT) ? POKEDEX_VARIANT_COUNT - 1 : 1)) % POKEDEX_VARIANT_COUNT;
    else if (!sVariants->detail && JOY_REPEAT(DPAD_ANY))
    {
        s8 delta = JOY_REPEAT(DPAD_LEFT) ? -1 : JOY_REPEAT(DPAD_RIGHT) ? 1 : JOY_REPEAT(DPAD_UP) ? -3 : 3;
        sVariants->cursor = (sVariants->cursor + POKEDEX_VARIANT_COUNT + delta) % POKEDEX_VARIANT_COUNT;
    }
    else return;
    PlaySE(SE_SELECT);
    Render();
}

bool32 PokedexVariants_Open(u16 species, u8 ownerTaskId)
{
    if (sVariants || ownerTaskId >= NUM_TASKS || !gTasks[ownerTaskId].isActive || !GetBgTilemapBuffer(0))
        return FALSE;
    sVariants = AllocZeroed(sizeof(*sVariants));
    if (!sVariants)
        return FALSE;
    sVariants->window = AddWindow(&sWindow);
    if (sVariants->window == WINDOW_NONE)
    {
        Free(sVariants);
        sVariants = NULL;
        return FALSE;
    }
    u8 task = CreateTask(Task_Variants, 0);
    if (task == TASK_NONE)
    {
        RemoveWindow(sVariants->window);
        Free(sVariants);
        sVariants = NULL;
        return FALSE;
    }
    for (u8 i = 0; i < VISIBLE_VARIANTS; i++) sVariants->icons[i] = MAX_SPRITES;
    PokedexVariants_Init(&sVariants->collection, species);
    sVariants->scanning = TRUE;
    sVariants->owner = ownerTaskId;
    sVariants->onReturn = gTasks[ownerTaskId].func;
    gTasks[ownerTaskId].func = Task_WaitForVariants;
    sVariants->priority = GetBgAttribute(0, BG_ATTR_PRIORITY);
    sVariants->bgX = GetBgX(0);
    sVariants->bgY = GetBgY(0);
    CpuCopy16(GetBgTilemapBuffer(0), sVariants->bgTilemap, BG_SCREEN_SIZE);
    CpuCopy16(&gPlttBufferUnfaded[OBJ_PLTT_ID(0)], sVariants->objPalette, sizeof(sVariants->objPalette));
    for (u8 i = 0; i < MAX_SPRITES; i++)
    {
        struct Sprite *sprite = &gSprites[i];
        sVariants->sprites[i] = (struct DexVariantSprite){.used = sprite->inUse, .invisible = sprite->invisible, .callback = sprite->callback};
        if (sprite->inUse)
        {
            sprite->callback = SpriteCallbackDummy;
            sprite->invisible = TRUE;
        }
    }
    UnholyUi_AcquirePalette(&sVariants->palette, 14);
    SetBgAttribute(0, BG_ATTR_PRIORITY, 0);
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    PutWindowTilemap(sVariants->window);
    Render();
    ShowBg(0);
    return TRUE;
}
