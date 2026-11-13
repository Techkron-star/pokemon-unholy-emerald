#ifndef GUARD_UNHOLY_UI_H
#define GUARD_UNHOLY_UI_H

// Native window chrome. Palette slots are borrowed and restored by each owner.
struct UnholyUiPalette
{
    u16 colors[16];
    u8 slot;
    bool8 saved;
};

void UnholyUi_AcquirePalette(struct UnholyUiPalette *lease, u8 slot);
void UnholyUi_ReleasePalette(struct UnholyUiPalette *lease);
void UnholyUi_Print(u8 window, u8 font, u16 x, u16 y, u16 width, const u8 *text);
void UnholyUi_PrintOnFill(u8 window, u8 font, u16 x, u16 y, u16 width, u8 fill, const u8 *text);
void UnholyUi_Frame(u8 window, const u8 *title);
void UnholyUi_Card(u8 window, u16 x, u16 y, u16 width, u16 height, bool8 selected);
void UnholyUi_Bar(u8 window, u16 x, u16 y, u16 width, u32 value, u32 maximum, u8 color);
u8 UnholyUi_CardFill(bool8 selected);

#endif
