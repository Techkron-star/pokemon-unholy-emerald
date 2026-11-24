#ifndef GUARD_SHOP_H
#define GUARD_SHOP_H

extern struct ItemSlot gMartPurchaseHistory[3];

void CreatePokemartMenu(const u16 *itemsForSale);
void CreateHorizonShopMenu(void (*fieldReturn)(void));
// Draws pixel-perfect Eclipse/Daylight chrome in existing GBA windows.
void DrawUnholyUiPanelAccents(u8 windowId, u16 width, u16 height, u8 dividerY);
const u16 *GetUnholyUiPalette(void);
const u16 *GetUnholyUiPaletteForTheme(bool8 daylight);
void CreateDecorationShop1Menu(const u16 *itemsForSale);
void CreateDecorationShop2Menu(const u16 *itemsForSale);
void CB2_ExitSellMenu(void);

#endif // GUARD_SHOP_H
