#ifndef GUARD_UNHOLY_UI_H
#define GUARD_UNHOLY_UI_H

// Native window chrome. Palette slots are borrowed and restored by each owner.
struct UnholyUiPalette
{
    u16 colors[16];
    u8 slot;
    bool8 saved;
};

enum UnholyUiIcon
{
    UNHOLY_UI_ICON_SHOP,
    UNHOLY_UI_ICON_NAV,
    UNHOLY_UI_ICON_CRAFT,
    UNHOLY_UI_ICON_LOCATOR,
    UNHOLY_UI_ICON_BOXES,
    UNHOLY_UI_ICON_SIGNAL,
    UNHOLY_UI_ICON_RAID,
    UNHOLY_UI_ICON_ARCHIVE,
    UNHOLY_UI_ICON_MUTATOR,
    UNHOLY_UI_ICON_SIMULATOR,
    UNHOLY_UI_ICON_LOCK,
    UNHOLY_UI_ICON_COIN,
    UNHOLY_UI_ICON_COUNT
};

void UnholyUi_AcquirePalette(struct UnholyUiPalette *lease, u8 slot);
void UnholyUi_ReleasePalette(struct UnholyUiPalette *lease);
void UnholyUi_Print(u8 window, u8 font, u16 x, u16 y, u16 width, const u8 *text);
void UnholyUi_PrintOnFill(u8 window, u8 font, u16 x, u16 y, u16 width, u8 fill, const u8 *text);
void UnholyUi_Frame(u8 window, const u8 *title);
void UnholyUi_Footer(u8 window, const u8 *actions);
void UnholyUi_Card(u8 window, u16 x, u16 y, u16 width, u16 height, bool8 selected);
void UnholyUi_ChoiceRow(u8 window, u16 y, const u8 *label, const u8 *detail, bool8 selected);
void UnholyUi_Bar(u8 window, u16 x, u16 y, u16 width, u32 value, u32 maximum, u8 color);
u8 UnholyUi_CardFill(bool8 selected);
u8 UnholyUi_AccentColor(void);
void UnholyUi_Icon(u8 window, u16 x, u16 y, u8 icon, u8 color);

#endif
