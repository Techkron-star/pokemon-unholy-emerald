#include "global.h"
#include "unholy_ui.h"
#include "event_data.h"
#include "menu.h"
#include "palette.h"
#include "shop.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"

void UnholyUi_AcquirePalette(struct UnholyUiPalette *lease, u8 slot)
{
    if (lease->saved)
        return;
    lease->slot = slot;
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(slot)], lease->colors, sizeof(lease->colors));
    lease->saved = TRUE;
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(slot), PLTT_SIZE_4BPP);
}

void UnholyUi_ReleasePalette(struct UnholyUiPalette *lease)
{
    if (!lease->saved)
        return;
    LoadPalette(lease->colors, BG_PLTT_ID(lease->slot), PLTT_SIZE_4BPP);
    lease->saved = FALSE;
}

void UnholyUi_PrintOnFill(u8 window, u8 font, u16 x, u16 y, u16 width, u8 fill, const u8 *source)
{
    u8 text[192];
    u16 length = 0;
    u16 windowWidth = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 windowHeight = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;
    const u8 colors[] = {fill, 2, fill};

    if (x >= windowWidth || y + GetFontAttribute(font, FONTATTR_MAX_LETTER_HEIGHT) > windowHeight)
        return;
    width = min(width, windowWidth - x);
    text[0] = EOS;
    // Prefer a narrower font of the same family over cutting the text short;
    // the clipping loop below is only the last resort.
    font = GetFontIdToFit(source, font, 0, width);
    // Copy whole control sequences so clipping never leaves an incomplete font code.
    while (*source != EOS && *source != CHAR_NEWLINE)
    {
        u8 bytes = *source == EXT_CTRL_CODE_BEGIN ? 1 + GetExtCtrlCodeLength(source[1]) : 1;
        if (length + bytes >= sizeof(text))
            break;
        memcpy(&text[length], source, bytes);
        text[length + bytes] = EOS;
        if (GetStringWidth(font, text, 0) > width)
        {
            text[length] = EOS;
            break;
        }
        length += bytes;
        source += bytes;
    }
    AddTextPrinterParameterized4(window, font, x, y, 0, 0, colors, TEXT_SKIP_DRAW, text);
}

void UnholyUi_Print(u8 window, u8 font, u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_PrintOnFill(window, font, x, y, width, 1, text);
}

u8 UnholyUi_CardFill(bool8 selected)
{
    return FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? (selected ? 9 : 4) : (selected ? 4 : 1);
}

void UnholyUi_Card(u8 window, u16 x, u16 y, u16 width, u16 height, bool8 selected)
{
    u16 right = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 bottom = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;
    u8 line = selected ? (FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? 7 : 5) : 3;

    if (width < 2 || height < 2 || x + width > right || y + height > bottom)
        return;
    FillWindowPixelRect(window, PIXEL_FILL(UnholyUi_CardFill(selected)), x, y, width, height);
    FillWindowPixelRect(window, PIXEL_FILL(line), x, y, width, 1);
    FillWindowPixelRect(window, PIXEL_FILL(line), x, y + height - 1, width, 1);
    FillWindowPixelRect(window, PIXEL_FILL(line), x, y, selected ? 2 : 1, height);
    FillWindowPixelRect(window, PIXEL_FILL(line), x + width - 1, y, 1, height);
}

void UnholyUi_Frame(u8 window, const u8 *title)
{
    u16 width = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 height = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;

    FillWindowPixelBuffer(window, PIXEL_FILL(1));
    DrawUnholyUiPanelAccents(window, width, height, 19);
    UnholyUi_Print(window, FONT_SMALL, 7, 4, width - 14, title);
}

// Pixel icons: 8x8, one bit per pixel (bit 7 is the left column). Drawn with
// window rectangles, so they need no sprites, tiles or palette slots.
static const u8 sIcons[UNHOLY_UI_ICON_COUNT][8] =
{
    [UNHOLY_UI_ICON_SHOP]      = {0x3C, 0x42, 0x42, 0xFF, 0xFF, 0x7E, 0x7E, 0x3C},
    [UNHOLY_UI_ICON_NAV]       = {0x3C, 0x7E, 0x66, 0x66, 0x7E, 0x3C, 0x18, 0x18},
    [UNHOLY_UI_ICON_CRAFT]     = {0x7C, 0xFE, 0xFE, 0x7C, 0x10, 0x10, 0x10, 0x10},
    [UNHOLY_UI_ICON_LOCATOR]   = {0x18, 0x18, 0x3C, 0xE7, 0xE7, 0x3C, 0x18, 0x18},
    [UNHOLY_UI_ICON_BOXES]     = {0xFF, 0xFF, 0x81, 0xBD, 0x81, 0x81, 0x81, 0xFF},
    [UNHOLY_UI_ICON_SIGNAL]    = {0x24, 0x42, 0x99, 0x5A, 0x18, 0x18, 0x18, 0x3C},
    [UNHOLY_UI_ICON_RAID]      = {0x10, 0x92, 0x54, 0x38, 0xFE, 0x38, 0x54, 0x92},
    [UNHOLY_UI_ICON_ARCHIVE]   = {0x7E, 0x81, 0xBD, 0x81, 0xBD, 0x81, 0x81, 0x7E},
    [UNHOLY_UI_ICON_MUTATOR]   = {0x06, 0x0C, 0x18, 0x3E, 0x7C, 0x18, 0x30, 0x60},
    [UNHOLY_UI_ICON_SIMULATOR] = {0x03, 0x03, 0x0F, 0x0F, 0x3F, 0x3F, 0xFF, 0xFF},
    [UNHOLY_UI_ICON_LOCK]      = {0x3C, 0x42, 0x42, 0xFF, 0xFF, 0xF7, 0xF7, 0xFF},
    [UNHOLY_UI_ICON_COIN]      = {0x3C, 0x7E, 0xE7, 0xE7, 0xE7, 0xE7, 0x7E, 0x3C},
};

u8 UnholyUi_AccentColor(void)
{
    return FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? 6 : 7;
}

void UnholyUi_Icon(u8 window, u16 x, u16 y, u8 icon, u8 color)
{
    u16 right = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 bottom = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;

    if (icon >= UNHOLY_UI_ICON_COUNT || x + 8 > right || y + 8 > bottom)
        return;
    for (u8 row = 0; row < 8; row++)
    {
        u8 bits = sIcons[icon][row];

        // Merge neighbouring pixels into runs to keep the draw count low.
        for (u8 col = 0; col < 8; col++)
        {
            u8 length = 0;

            while (col + length < 8 && (bits & (0x80 >> (col + length))))
                length++;
            if (length != 0)
            {
                FillWindowPixelRect(window, PIXEL_FILL(color), x + col, y + row, length, 1);
                col += length;
            }
        }
    }
}

#define FOOTER_MAX_KEYS 6
#define FOOTER_TEXT_LENGTH 32

struct FooterKey
{
    u8 key[8];
    u8 label[FOOTER_TEXT_LENGTH];
};

static bool8 FooterIsGap(const u8 *text)
{
    return text[0] == CHAR_SPACE && text[1] == CHAR_SPACE;
}

// Splits "A:OPEN  B:BACK" into key/label pairs. Anything that does not follow
// that shape (plain sentences, control codes, overlong parts) is left to the
// plain footer so no instruction is ever dropped.
static u8 FooterParse(const u8 *text, struct FooterKey *keys)
{
    u8 count = 0;

    while (*text != EOS)
    {
        const u8 *start = text;
        u8 length;

        if (count >= FOOTER_MAX_KEYS)
            return 0;
        while (*text != EOS && *text != CHAR_COLON && *text != EXT_CTRL_CODE_BEGIN && !FooterIsGap(text))
            text++;
        length = text - start;
        if (*text != CHAR_COLON || length == 0 || length >= sizeof(keys[count].key))
            return 0;
        memcpy(keys[count].key, start, length);
        keys[count].key[length] = EOS;
        text++;
        start = text;
        while (*text != EOS && !FooterIsGap(text))
        {
            if (*text == EXT_CTRL_CODE_BEGIN || *text == CHAR_NEWLINE)
                return 0;
            text++;
        }
        length = text - start;
        if (length == 0 || length >= FOOTER_TEXT_LENGTH)
            return 0;
        memcpy(keys[count].label, start, length);
        keys[count].label[length] = EOS;
        count++;
        while (*text == CHAR_SPACE)
            text++;
    }
    return count;
}

// Key caps: the button name on an accent chip with rounded corners, then its
// action. Returns FALSE (drawing nothing) when it would not fit.
static bool8 FooterDrawKeys(u8 window, u16 width, u16 y, const u8 *actions)
{
    struct FooterKey keys[FOOTER_MAX_KEYS];
    u8 count = FooterParse(actions, keys);
    u8 accent = UnholyUi_AccentColor();
    const u8 chipColors[] = {accent, 1, accent};
    u16 total = 0, x = 7;

    if (count == 0)
        return FALSE;
    for (u8 i = 0; i < count; i++)
        total += GetStringWidth(FONT_SMALL_NARROW, keys[i].key, 0) + 4 + 3
               + GetStringWidth(FONT_SMALL_NARROW, keys[i].label, 0) + (i != 0 ? 6 : 0);
    if (total > width)
        return FALSE;
    for (u8 i = 0; i < count; i++)
    {
        u16 chip = GetStringWidth(FONT_SMALL_NARROW, keys[i].key, 0) + 4;

        if (i != 0)
            x += 6;
        FillWindowPixelRect(window, PIXEL_FILL(accent), x, y, chip, 12);
        AddTextPrinterParameterized4(window, FONT_SMALL_NARROW, x + 2, y, 0, 0, chipColors, TEXT_SKIP_DRAW, keys[i].key);
        FillWindowPixelRect(window, PIXEL_FILL(1), x, y, 1, 1);
        FillWindowPixelRect(window, PIXEL_FILL(1), x + chip - 1, y, 1, 1);
        FillWindowPixelRect(window, PIXEL_FILL(1), x, y + 11, 1, 1);
        FillWindowPixelRect(window, PIXEL_FILL(1), x + chip - 1, y + 11, 1, 1);
        x += chip + 3;
        UnholyUi_Print(window, FONT_SMALL_NARROW, x, y, width + 7 - x, keys[i].label);
        x += GetStringWidth(FONT_SMALL_NARROW, keys[i].label, 0);
    }
    return TRUE;
}

void UnholyUi_Footer(u8 window, const u8 *actions)
{
    u16 width = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH - 14;
    u16 bottom = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;
    u8 font = GetFontIdToFit(actions, FONT_SMALL_NARROW, 0, width);
    // Each glyph blits 12 or 15 pixels. Leave the two-pixel frame intact.
    u8 inset = font == FONT_SMALL_NARROWER ? 17 : 14;

    // Key caps need the 12-pixel font; longer footers keep the compact text.
    if (FooterDrawKeys(window, width, bottom - 14, actions))
        return;
    UnholyUi_Print(window, font, 7, bottom - inset, width, actions);
}

void UnholyUi_ChoiceRow(u8 window, u16 y, const u8 *label, const u8 *detail, bool8 selected)
{
    u16 width = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH - 12;
    u8 fill = UnholyUi_CardFill(selected);
    u16 labelWidth = width - 10;
    UnholyUi_Card(window, 6, y, width, 13, selected);
    if (detail != NULL)
    {
        u16 detailWidth = min(width / 2, GetStringWidth(FONT_SMALL_NARROW, detail, 0));
        UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, width + 1 - detailWidth, y + 1, detailWidth, fill, detail);
        labelWidth -= detailWidth + 5;
    }
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 11, y + 1, labelWidth, fill, label);
}

void UnholyUi_Bar(u8 window, u16 x, u16 y, u16 width, u32 value, u32 maximum, u8 color)
{
    FillWindowPixelRect(window, PIXEL_FILL(3), x, y, width, 3);
    if (maximum != 0 && value != 0)
        FillWindowPixelRect(window, PIXEL_FILL(color), x, y, max(1, min(value, maximum) * width / maximum), 3);
}
