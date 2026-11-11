#ifndef GUARD_HORIZON_GOODS_H
#define GUARD_HORIZON_GOODS_H

#define HORIZON_GOODS_ITEM_COUNT 8
#define HORIZON_GOODS_DISC_COUNT 6

// Called from Horizon's existing saved-market refresh. Generates a new seed
// only when the main market really rotates, or once for an older save.
void HorizonGoodsRollSeeds(void);
void HorizonGoodsEnsureSeeds(void);

// ITEM_NONE-terminated lists; caller may pass directly to SetShopItemsForSale.
// Reopening does NOT consume global RNG and does NOT change stock.
const u16 *HorizonGoodsGetItems(void);
const u16 *HorizonGoodsGetDiscs(void);

#endif // GUARD_HORIZON_GOODS_H
