#include "global.h"
#include "pokelocator.h"
#include "config/pokelocator.h"
#include "dma3.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "region_map.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/map_types.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Pokélocator screen. It follows the on-field window pattern of the crafting menu and
// Horizon: one native BG0 window, the shared Eclipse/Daylight palette in slot 14 (saved
// and restored on exit), and at most POKELOCATOR_VISIBLE_ROWS mon icon sprites.
// The screen only renders when an input changes something, never every frame.

#define POKELOCATOR_VISIBLE_ROWS 3
#define POKELOCATOR_NUM_ICON_PALETTES 6

// Window geometry in pixels. The window sits at tile (2, 1), i.e. pixel (16, 8).
#define WIN_PIXEL_LEFT   16
#define WIN_PIXEL_TOP    8
#define WIN_WIDTH        (26 * 8)
#define WIN_HEIGHT       (17 * 8)
#define HEADER_DIVIDER_Y 20

// Species cards: 3 rows of 30 px at a 32 px pitch, between the two dividers.
#define CARD_X      4
#define CARD_WIDTH  200
#define CARD_TOP    22
#define CARD_PITCH  32
#define CARD_HEIGHT 30

enum PokelocatorMode
{
    MODE_LIST,
    MODE_DETAIL,
};

struct PokelocatorUi
{
    struct PokelocatorList list;
    struct PokelocatorZone zone;
    u8 filteredIndex[POKELOCATOR_MAX_ROWS];
    u8 filteredCount;
    u8 filter;
    u8 cursor;
    u8 scroll;
    u8 mode;
    u8 windowId;
    u8 iconSpriteIds[POKELOCATOR_VISIBLE_ROWS];
    u8 ownedPaletteMask;
    bool8 paletteSaved;
    u16 savedPalette[16];
    void (*onClose)(void);
};

static EWRAM_DATA struct PokelocatorUi sUi = {0};

static const struct WindowTemplate sWindowTemplate =
{
    .bg = 0, .tilemapLeft = 2, .tilemapTop = 1,
    // 64 + 442 tiles ends before the field's standard frame at tile 0x214.
    .width = 26, .height = 17, .paletteNum = 14, .baseBlock = 0x40,
};
STATIC_ASSERT(0x40 + 26 * 17 <= STD_WINDOW_BASE_TILE_NUM, PokelocatorDoesNotOverwriteItsBorder);

static const u8 sTextColors[] = {1, 2, 3};

static const u8 sText_NoWildMons[] = _("NO WILD POKéMON HERE.");
static const u8 sText_FilterAll[] = _("ALL");
static const u8 sText_Caught[] = _("CAUGHT");
static const u8 sText_Seen[] = _("SEEN");
static const u8 sText_Unseen[] = _("UNSEEN");
static const u8 sText_ControlsList[] = _("A:INFO L/R:FILTER B:EXIT");
static const u8 sText_ControlsListAreas[] = _("A:INFO L/R:FILTER <>:AREA B:EXIT");
static const u8 sText_ControlsEmpty[] = _("B:EXIT");
static const u8 sText_ControlsEmptyAreas[] = _("<>:AREA B:EXIT");

static const u8 *const sMethodLabels[POKELOCATOR_METHOD_COUNT] =
{
    [POKELOCATOR_METHOD_LAND]      = COMPOUND_STRING("GRASS"),
    [POKELOCATOR_METHOD_WATER]     = COMPOUND_STRING("SURF"),
    [POKELOCATOR_METHOD_ROCKS]     = COMPOUND_STRING("ROCK SMASH"),
    [POKELOCATOR_METHOD_OLD_ROD]   = COMPOUND_STRING("OLD ROD"),
    [POKELOCATOR_METHOD_GOOD_ROD]  = COMPOUND_STRING("GOOD ROD"),
    [POKELOCATOR_METHOD_SUPER_ROD] = COMPOUND_STRING("SUPER ROD"),
    [POKELOCATOR_METHOD_HIDDEN]    = COMPOUND_STRING("HIDDEN"),
};
static const u8 sText_MethodCave[] = _("CAVE");

static void Task_Pokelocator(u8 taskId);
static void Task_StartSearch(u8 taskId);
extern const u8 EventScript_PokelocatorSearchBattle[];
extern const u8 EventScript_PokelocatorSearchUnavailable[];

// ---------------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------------

// Underground maps call their land encounters "CAVE" instead of "GRASS".
static const u8 *GetMethodLabel(u8 method)
{
    if (method == POKELOCATOR_METHOD_LAND && sUi.zone.count != 0)
    {
        const struct WildPokemonHeader *header = &gWildMonHeaders[sUi.zone.headerIds[sUi.zone.current]];

        if (Overworld_GetMapHeaderByGroupAndId(header->mapGroup, header->mapNum)->mapType == MAP_TYPE_UNDERGROUND)
            return sText_MethodCave;
    }
    return sMethodLabels[method];
}

static const u8 *GetFilterLabel(u8 filter)
{
    return filter == POKELOCATOR_FILTER_ALL ? sText_FilterAll : GetMethodLabel(filter - 1);
}

// Prints text clipped to a pixel width so it can never run into a neighbouring field.
static void PrintFit(u8 x, u8 y, u8 width, u8 fontId, const u8 *colors, const u8 *source)
{
    u8 text[48];
    u8 len = 0;

    while (len < sizeof(text) - 1 && source[len] != EOS)
    {
        text[len] = source[len];
        len++;
    }
    text[len] = EOS;

    while (len > 0 && GetStringWidth(fontId, text, 0) > width)
        text[--len] = EOS;

    AddTextPrinterParameterized4(sUi.windowId, fontId, x, y, 0, 0, colors, TEXT_SKIP_DRAW, text);
}

static void PrintRightAligned(u8 rightX, u8 y, u8 fontId, const u8 *colors, const u8 *text)
{
    s32 width = GetStringWidth(fontId, text, 0);

    AddTextPrinterParameterized4(sUi.windowId, fontId, width < rightX ? rightX - width : 0, y,
                                 0, 0, colors, TEXT_SKIP_DRAW, text);
}

// "20%" for whole percentages, "0.5%" otherwise; "--" when the chance cannot be computed.
static u8 *AppendChance(u8 *dst, const struct PokelocatorRow *row)
{
    u16 tenths;

    if (!Pokelocator_RowHasChance(row))
        return StringCopy(dst, COMPOUND_STRING("--"));

    tenths = Pokelocator_GetChanceTenths(row);
    dst = ConvertIntToDecimalStringN(dst, tenths / 10, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (tenths % 10 != 0)
    {
        dst = StringAppend(dst, COMPOUND_STRING("."));
        dst = ConvertIntToDecimalStringN(dst, tenths % 10, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
    return StringAppend(dst, COMPOUND_STRING("%"));
}

static u8 *AppendLevels(u8 *dst, const struct PokelocatorRow *row)
{
    dst = StringCopy(dst, COMPOUND_STRING("LV "));
    dst = ConvertIntToDecimalStringN(dst, row->minLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (row->maxLevel != row->minLevel)
    {
        dst = StringAppend(dst, COMPOUND_STRING("-"));
        dst = ConvertIntToDecimalStringN(dst, row->maxLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    return dst;
}

static const u8 *GetDexLabel(enum Species species)
{
    switch (Pokelocator_GetDexState(species))
    {
    case POKELOCATOR_DEX_CAUGHT:
        return sText_Caught;
    case POKELOCATOR_DEX_SEEN:
        return sText_Seen;
    default:
        return sText_Unseen;
    }
}

// ---------------------------------------------------------------------------------
// Icons: only the visible rows own a sprite; palettes are loaded on demand and the ones
// this screen loaded are freed again, so nothing outlives the screen.
// ---------------------------------------------------------------------------------

static void DestroyIcons(void)
{
    u8 i;

    for (i = 0; i < POKELOCATOR_VISIBLE_ROWS; i++)
    {
        if (sUi.iconSpriteIds[i] != MAX_SPRITES)
        {
            FreeAndDestroyMonIconSprite(&gSprites[sUi.iconSpriteIds[i]]);
            sUi.iconSpriteIds[i] = MAX_SPRITES;
        }
    }
    for (i = 0; i < POKELOCATOR_NUM_ICON_PALETTES; i++)
    {
        if (sUi.ownedPaletteMask & (1 << i))
            FreeSpritePaletteByTag(gMonIconPaletteTable[i].tag);
    }
    sUi.ownedPaletteMask = 0;
}

static void CreateIcon(u8 slot, enum Species species, s16 x, s16 y)
{
    u8 paletteIndex = GetMonIconPaletteIndexFromSpecies(species);
    u16 tag = gMonIconPaletteTable[paletteIndex].tag;
    u8 spriteId;

    if (IndexOfSpritePaletteTag(tag) == 0xFF)
    {
        LoadMonIconPalette(species);
        // No free palette slot: show the card without an icon instead of garbage colors.
        if (IndexOfSpritePaletteTag(tag) == 0xFF)
            return;
        sUi.ownedPaletteMask |= 1 << paletteIndex;
    }

    spriteId = CreateMonIconNoPersonality(species, SpriteCB_MonIcon, x, y, 0);
    if (spriteId == MAX_SPRITES)
        return;
    // Same priority as the window layer, so the icon is drawn above it.
    gSprites[spriteId].oam.priority = 0;
    sUi.iconSpriteIds[slot] = spriteId;
}

// ---------------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------------

static void DrawCard(u8 x, u8 y, u8 width, u8 height, bool8 selected)
{
    const bool8 daylight = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT);
    const u8 fill = daylight ? (selected ? 9 : 4) : (selected ? 4 : 1);
    const u8 line = daylight ? (selected ? 7 : 3) : (selected ? 10 : 3);
    const u8 shine = daylight ? 6 : 5;

    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(fill), x + 1, y + 1, width - 2, height - 2);
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(line), x, y, width, 1);
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(line), x, y + height - 1, width, 1);
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(line), x, y, 1, height);
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(line), x + width - 1, y, 1, height);
    if (selected)
        FillWindowPixelRect(sUi.windowId, PIXEL_FILL(shine), x + 2, y + 1, width - 4, 2);
}

// Text colors that match the fill of a card, so no rectangle shows behind the letters.
static void GetCardColors(bool8 selected, u8 *colors)
{
    const bool8 daylight = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT);

    colors[0] = daylight ? (selected ? 9 : 4) : (selected ? 4 : 1);
    colors[1] = daylight ? 2 : 10;
    colors[2] = 3;
}

static void DrawHeader(void)
{
    u8 buffer[32];
    u8 *end;

    if (sUi.zone.count != 0)
    {
        end = GetMapName(buffer, sUi.zone.mapSec, 0);
        if (sUi.zone.count > 1)
        {
            end = StringAppend(end, COMPOUND_STRING("  "));
            end = ConvertIntToDecimalStringN(end, sUi.zone.current + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, sUi.zone.count, STR_CONV_MODE_LEFT_ALIGN, 2);
        }
    }
    else
    {
        GetMapName(buffer, GetCurrentRegionMapSectionId(), 0);
    }
    PrintFit(6, 3, 120, FONT_NARROW, sTextColors, buffer);

    if (sUi.mode == MODE_LIST && sUi.list.methodMask != 0)
    {
        StringCopy(buffer, COMPOUND_STRING("L "));
        StringAppend(buffer, GetFilterLabel(sUi.filter));
        StringAppend(buffer, COMPOUND_STRING(" R"));
        PrintRightAligned(WIN_WIDTH - 6, 3, FONT_NARROW, sTextColors, buffer);
    }
}

static void DrawFooter(void)
{
    const bool8 hasAreas = sUi.zone.count > 1;
    const u8 *controls;
    u8 counter[12];
    u8 *end;
    u8 counterWidth = 0;

    if (sUi.mode == MODE_DETAIL && sUi.filteredCount != 0)
    {
        const struct PokelocatorRow *row = &sUi.list.rows[sUi.filteredIndex[sUi.cursor]];
        u16 header = sUi.zone.count ? sUi.zone.headerIds[sUi.zone.current] : HEADER_NONE;
        controls = Pokelocator_GetSearchMessage(Pokelocator_GetSearchStatus(header, row->species, row->method));
    }
    else if (sUi.filteredCount == 0)
        controls = hasAreas ? sText_ControlsEmptyAreas : sText_ControlsEmpty;
    else
        controls = hasAreas ? sText_ControlsListAreas : sText_ControlsList;

    if (sUi.filteredCount != 0)
    {
        end = ConvertIntToDecimalStringN(counter, sUi.cursor + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(end, sUi.filteredCount, STR_CONV_MODE_LEFT_ALIGN, 2);
        counterWidth = GetStringWidth(FONT_SMALL_NARROW, counter, 0) + 6;
        PrintRightAligned(WIN_WIDTH - 6, WIN_HEIGHT - 15, FONT_SMALL_NARROW, sTextColors, counter);
    }
    PrintFit(6, WIN_HEIGHT - 15, WIN_WIDTH - 12 - counterWidth, FONT_SMALL_NARROW, sTextColors, controls);
}

static void DrawListRow(u8 visibleRow, const struct PokelocatorRow *row, bool8 selected)
{
    const u8 y = CARD_TOP + visibleRow * CARD_PITCH;
    const bool8 daylight = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT);
    u8 colors[3];
    u8 buffer[32];
    u8 *end;

    GetCardColors(selected, colors);
    DrawCard(CARD_X, y, CARD_WIDTH, CARD_HEIGHT, selected);

    PrintFit(CARD_X + 38, y + 1, 100, FONT_NARROW, colors, GetSpeciesName(row->species));

    AppendChance(buffer, row);
    PrintRightAligned(CARD_X + CARD_WIDTH - 5, y + 1, FONT_NARROW, colors, buffer);

    end = StringCopy(buffer, GetMethodLabel(row->method));
    end = StringAppend(end, COMPOUND_STRING("  "));
    AppendLevels(end, row);
    PrintFit(CARD_X + 38, y + 18, 100, FONT_SMALL_NARROW, colors, buffer);

    PrintRightAligned(CARD_X + CARD_WIDTH - 5, y + 18, FONT_SMALL_NARROW, colors, GetDexLabel(row->species));
    (void)daylight;
}

static void DrawListMode(void)
{
    u8 i;

    if (sUi.filteredCount == 0)
    {
        PrintFit(CARD_X + 6, CARD_TOP + 36, CARD_WIDTH - 12, FONT_NARROW, sTextColors, sText_NoWildMons);
        return;
    }

    for (i = 0; i < POKELOCATOR_VISIBLE_ROWS; i++)
    {
        u8 position = sUi.scroll + i;
        const struct PokelocatorRow *row;

        if (position >= sUi.filteredCount)
            break;
        row = &sUi.list.rows[sUi.filteredIndex[position]];
        DrawListRow(i, row, position == sUi.cursor);
    }
}

// Detail page: every way the selected species can be found in this area.
static void DrawDetailMode(void)
{
    const struct PokelocatorRow *selected = &sUi.list.rows[sUi.filteredIndex[sUi.cursor]];
    u8 buffer[40];
    u8 *end;
    u8 line = 0;
    u8 i;

    PrintFit(46, CARD_TOP + 2, 150, FONT_NARROW, sTextColors, GetSpeciesName(selected->species));
    PrintFit(4, CARD_TOP + 38, 40, FONT_SMALL_NARROW, sTextColors, GetDexLabel(selected->species));

    for (i = 0; i < sUi.list.count && line < POKELOCATOR_METHOD_COUNT; i++)
    {
        const struct PokelocatorRow *row = &sUi.list.rows[i];

        if (row->species != selected->species)
            continue;

        end = StringCopy(buffer, row->method == selected->method ? COMPOUND_STRING("> ") : COMPOUND_STRING("  "));
        end = StringAppend(end, GetMethodLabel(row->method));
        end = StringAppend(end, COMPOUND_STRING("  "));
        end = AppendLevels(end, row);
        end = StringAppend(end, COMPOUND_STRING("  "));
        AppendChance(end, row);
        PrintFit(46, CARD_TOP + 20 + line * 10, 150, FONT_SMALL_NARROW, sTextColors, buffer);
        line++;
    }

    // Shown only when another system provides modifier rules; hidden while they are undefined.
    if (Pokelocator_GetModifierStatus(POKELOCATOR_MODIFIER_MUTATION, sUi.zone.mapSec) != POKELOCATOR_MODIFIER_UNDEFINED)
    {
        StringCopy(buffer, COMPOUND_STRING("MUTATION MODIFIER: "));
        StringAppend(buffer,
                     Pokelocator_GetModifierStatus(POKELOCATOR_MODIFIER_MUTATION, sUi.zone.mapSec) == POKELOCATOR_MODIFIER_ACTIVE
                     ? COMPOUND_STRING("ON") : COMPOUND_STRING("OFF"));
        PrintFit(46, CARD_TOP + 20 + line * 10, 150, FONT_SMALL_NARROW, sTextColors, buffer);
    }
}

static void RefreshIcons(void)
{
    u8 i;

    DestroyIcons();
    if (sUi.filteredCount == 0)
        return;

    if (sUi.mode == MODE_DETAIL)
    {
        const struct PokelocatorRow *row = &sUi.list.rows[sUi.filteredIndex[sUi.cursor]];

        CreateIcon(0, row->species, WIN_PIXEL_LEFT + CARD_X + 16, WIN_PIXEL_TOP + CARD_TOP + 16);
        return;
    }

    for (i = 0; i < POKELOCATOR_VISIBLE_ROWS; i++)
    {
        u8 position = sUi.scroll + i;

        if (position >= sUi.filteredCount)
            break;
        CreateIcon(i, sUi.list.rows[sUi.filteredIndex[position]].species,
                   WIN_PIXEL_LEFT + CARD_X + 18, WIN_PIXEL_TOP + CARD_TOP + CARD_PITCH * i + CARD_HEIGHT / 2);
    }
}

static void Render(void)
{
    FillWindowPixelBuffer(sUi.windowId, PIXEL_FILL(1));
    DrawUnholyUiPanelAccents(sUi.windowId, WIN_WIDTH, WIN_HEIGHT, HEADER_DIVIDER_Y);
    DrawHeader();
    if (sUi.mode == MODE_DETAIL && sUi.filteredCount != 0)
        DrawDetailMode();
    else
        DrawListMode();
    DrawFooter();
    CopyWindowToVram(sUi.windowId, COPYWIN_FULL);
    RefreshIcons();
}

// ---------------------------------------------------------------------------------
// State changes
// ---------------------------------------------------------------------------------

static void RefilterList(void)
{
    sUi.filteredCount = Pokelocator_BuildFilteredIndex(&sUi.list, sUi.filter, sUi.filteredIndex);
    sUi.cursor = 0;
    sUi.scroll = 0;
}

static void LoadZoneMap(u8 zoneIndex)
{
    sUi.zone.current = zoneIndex;
    if (sUi.zone.count != 0)
        Pokelocator_BuildListForHeader(&sUi.list, sUi.zone.headerIds[zoneIndex], Pokelocator_IsSpeciesRevealed);
    else
        Pokelocator_ClearList(&sUi.list);
    sUi.filter = POKELOCATOR_FILTER_ALL;
    sUi.mode = MODE_LIST;
    RefilterList();
}

static void ReturnFromLocator(void)
{
    void (*onClose)(void) = sUi.onClose;
    sUi.onClose = NULL;
    if (onClose != NULL)
        onClose();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

static void CloseScreen(u8 taskId, bool8 returnToCaller)
{
    DestroyIcons();
    ClearStdWindowAndFrameToTransparent(sUi.windowId, TRUE);
    RemoveWindow(sUi.windowId);
    sUi.windowId = WINDOW_NONE;
    if (sUi.paletteSaved)
    {
        LoadPalette(sUi.savedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
        sUi.paletteSaved = FALSE;
    }
    DestroyTask(taskId);
    if (returnToCaller)
        ReturnFromLocator();
    else
        sUi.onClose = NULL; // Search continues on the field, not in WATCH/START.
}

static void Task_StartSearch(u8 taskId)
{
    if (gPaletteFade.active || IsDma3ManagerBusyWithBgCopy())
        return;
    const s16 *data = gTasks[taskId].data;
    bool32 ready = Pokelocator_CreateSearchMon((u16)data[0], (u16)data[1], data[2]);
    ScriptContext_SetupScript(ready ? EventScript_PokelocatorSearchBattle : EventScript_PokelocatorSearchUnavailable);
    DestroyTask(taskId);
}

static void Task_Pokelocator(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (JOY_NEW(B_BUTTON))
    {
        if (sUi.mode == MODE_DETAIL)
        {
            sUi.mode = MODE_LIST;
            PlaySE(SE_SELECT);
            Render();
        }
        else
        {
            PlaySE(SE_SELECT);
            CloseScreen(taskId, TRUE);
        }
        return;
    }

    if (JOY_NEW(A_BUTTON) && sUi.mode == MODE_LIST && sUi.filteredCount != 0)
    {
        sUi.mode = MODE_DETAIL;
        PlaySE(SE_SELECT);
        Render();
        return;
    }

    if (JOY_NEW(A_BUTTON) && sUi.mode == MODE_DETAIL && sUi.filteredCount != 0)
    {
        const struct PokelocatorRow *row = &sUi.list.rows[sUi.filteredIndex[sUi.cursor]];
        u16 header = sUi.zone.count ? sUi.zone.headerIds[sUi.zone.current] : HEADER_NONE;
        if (Pokelocator_GetSearchStatus(header, row->species, row->method) != POKELOCATOR_SEARCH_READY)
        {
            PlaySE(SE_FAILURE);
            Render();
            return;
        }
        u8 searchTask = CreateTask(Task_StartSearch, 8);
        if (searchTask == TASK_NONE)
        {
            PlaySE(SE_FAILURE);
            return;
        }
        gTasks[searchTask].data[0] = header;
        gTasks[searchTask].data[1] = row->species;
        gTasks[searchTask].data[2] = row->method;
        PlaySE(SE_SELECT);
        CloseScreen(taskId, FALSE);
        return;
    }

    if (JOY_REPEAT(DPAD_UP) || JOY_REPEAT(DPAD_DOWN))
    {
        if (sUi.filteredCount > 1)
        {
            Pokelocator_MoveCursor(&sUi.cursor, &sUi.scroll, sUi.filteredCount, POKELOCATOR_VISIBLE_ROWS,
                                   JOY_REPEAT(DPAD_UP) ? -1 : 1);
            PlaySE(SE_SELECT);
            Render();
        }
        return;
    }

    if (sUi.mode != MODE_LIST)
        return;

    if (JOY_NEW(L_BUTTON) || JOY_NEW(R_BUTTON))
    {
        u8 next = Pokelocator_NextFilter(sUi.list.methodMask, sUi.filter, JOY_NEW(R_BUTTON) ? 1 : -1);

        if (next != sUi.filter)
        {
            sUi.filter = next;
            RefilterList();
            PlaySE(SE_SELECT);
            Render();
        }
        return;
    }

    if ((JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_RIGHT)) && sUi.zone.count > 1)
    {
        u8 next = JOY_NEW(DPAD_RIGHT) ? (sUi.zone.current + 1) % sUi.zone.count
                                      : (sUi.zone.current + sUi.zone.count - 1) % sUi.zone.count;

        LoadZoneMap(next);
        PlaySE(SE_SELECT);
        Render();
    }
}

// Opens the screen over the field. Controls stay locked until the screen closes.
void Pokelocator_OpenFromField(void (*onClose)(void))
{
    u8 i;

    HideMapNamePopUpWindow();
    LockPlayerFieldControls();

    memset(&sUi, 0, sizeof(sUi));
    sUi.onClose = onClose;
    for (i = 0; i < POKELOCATOR_VISIBLE_ROWS; i++)
        sUi.iconSpriteIds[i] = MAX_SPRITES;

    // Zone browsing and the table lookup happen once here, never per frame.
    Pokelocator_BuildZone(&sUi.zone, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    LoadZoneMap(sUi.zone.current);

    // The field owns palette slot 14; take it for the theme and give it back on exit.
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(14)], sUi.savedPalette, sizeof(sUi.savedPalette));
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    sUi.paletteSaved = TRUE;

    sUi.windowId = AddWindow(&sWindowTemplate);
    if (sUi.windowId == WINDOW_NONE)
    {
        LoadPalette(sUi.savedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
        sUi.paletteSaved = FALSE;
        ReturnFromLocator();
        return;
    }

    SetStandardWindowBorderStyle(sUi.windowId, FALSE);
    PutWindowTilemap(sUi.windowId);
    Render();
    CreateTask(Task_Pokelocator, 8);
}
