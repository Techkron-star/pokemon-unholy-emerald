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

void UnholyUi_Hint(u8 window, u16 x, u16 y, u16 width, const u8 *text)
{
    // Dim text index: 11 (muted lavender) in Eclipse, 10 (moss) in Daylight.
    u8 dim = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? 10 : 11;
    const u8 colors[] = {1, dim, 1};
    u8 buffer[96];
    u16 length = 0;
    u16 windowWidth = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;

    if (x >= windowWidth)
        return;
    width = min(width, windowWidth - x);
    buffer[0] = EOS;
    while (*text != EOS && length + 1 < sizeof(buffer))
    {
        buffer[length] = *text;
        buffer[length + 1] = EOS;
        if (GetStringWidth(FONT_SMALL_NARROW, buffer, 0) > width)
        {
            buffer[length] = EOS;
            break;
        }
        length++;
        text++;
    }
    AddTextPrinterParameterized4(window, FONT_SMALL_NARROW, x, y, 0, 0, colors, TEXT_SKIP_DRAW, buffer);
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

static u8 IconColor(char code)
{
    bool8 daylight = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT);

    switch (code)
    {
    case 'o': return 2;                  // same index as the text: always readable
    case 'a': return daylight ? 6 : 7;   // soft gold / cool silver, as the panel accent
    case 'b': return daylight ? 7 : 4;   // turquoise / deep blue
    case 'r': return 13;                 // alert red in both themes
    default:  return 0;
    }
}

void UnholyUi_Icon(u8 window, u16 x, u16 y, const char *const rows[UNHOLY_UI_ICON_SIZE])
{
    u16 right = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 bottom = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;

    if (x + UNHOLY_UI_ICON_SIZE > right || y + UNHOLY_UI_ICON_SIZE > bottom)
        return;
    for (u8 row = 0; row < UNHOLY_UI_ICON_SIZE; row++)
    {
        // One fill per horizontal run keeps redraws cheap.
        for (u8 col = 0; col < UNHOLY_UI_ICON_SIZE;)
        {
            u8 length = 1;

            if (rows[row][col] == '.')
            {
                col++;
                continue;
            }
            while (col + length < UNHOLY_UI_ICON_SIZE && rows[row][col + length] == rows[row][col])
                length++;
            FillWindowPixelRect(window, PIXEL_FILL(IconColor(rows[row][col])), x + col, y + row, length, 1);
            col += length;
        }
    }
}

void UnholyUi_Frame(u8 window, const u8 *title)
{
    u16 width = GetWindowAttribute(window, WINDOW_WIDTH) * TILE_WIDTH;
    u16 height = GetWindowAttribute(window, WINDOW_HEIGHT) * TILE_HEIGHT;

    FillWindowPixelBuffer(window, PIXEL_FILL(1));
    DrawUnholyUiPanelAccents(window, width, height, 19);
    UnholyUi_Print(window, FONT_SMALL, 7, 4, width - 14, title);
}

void UnholyUi_Bar(u8 window, u16 x, u16 y, u16 width, u32 value, u32 maximum, u8 color)
{
    FillWindowPixelRect(window, PIXEL_FILL(3), x, y, width, 3);
    if (maximum != 0 && value != 0)
        FillWindowPixelRect(window, PIXEL_FILL(color), x, y, max(1, min(value, maximum) * width / maximum), 3);
}
