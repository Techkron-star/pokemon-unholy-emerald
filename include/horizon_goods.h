#ifndef GUARD_HORIZON_GOODS_H
#define GUARD_HORIZON_GOODS_H

#include "horizon_market.h"

#define HORIZON_GOODS_ITEM_COUNT 12
#define HORIZON_GOODS_DISC_COUNT 12

enum HorizonDealGrade
{
    HORIZON_DEAL_NORMAL,
    HORIZON_DEAL_PROMO,
    HORIZON_DEAL_CHROMATIC,
};

struct HorizonGoodsOffer
{
    u16 itemId;
    u8 quantity;
    u8 grade;
    u32 price;
};

u8 HorizonGoodsItemCountForTier(u8 watchTier);
u8 HorizonGoodsDiscCountForTier(u8 watchTier);
void HorizonGoodsRollSeeds(void);
void HorizonGoodsEnsureSeeds(void);
const u16 *HorizonGoodsGetItems(void);
const u16 *HorizonGoodsGetDiscs(void);
u32 HorizonGoodsReferencePrice(enum Item item);
const struct HorizonGoodsOffer *HorizonGoodsGetOffer(bool8 discs, u8 slot);
bool32 HorizonGoodsOfferSold(bool8 discs, u8 slot);
enum HorizonMarketPurchaseResult HorizonGoodsPurchase(bool8 discs, u8 slot, enum Item expected, u32 price);

#endif
