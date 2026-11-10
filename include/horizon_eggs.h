#ifndef GUARD_HORIZON_EGGS_H
#define GUARD_HORIZON_EGGS_H

#include "horizon_market.h"

enum HorizonEggTier
{
    HORIZON_EGG_BASIC,
    HORIZON_EGG_MEDIUM,
    HORIZON_EGG_HIGH,
    HORIZON_EGG_MYTHIC,
    HORIZON_EGG_LEGENDARY,
    HORIZON_EGG_TIER_COUNT,
};

enum HorizonEggQuality
{
    HORIZON_EGG_STANDARD,
    HORIZON_EGG_RADIANT,
    HORIZON_EGG_PRISMATIC,
};

void HorizonEggsRollStock(void);
u8 HorizonEggGetQuality(u8 tier);
bool8 HorizonEggIsSold(u8 tier);
u32 HorizonEggGetPrice(u8 tier);
const u8 *HorizonEggTierName(u8 tier);
const u8 *HorizonEggQualityName(u8 quality);
enum HorizonMarketPurchaseResult HorizonEggPurchase(u8 tier);

#endif // GUARD_HORIZON_EGGS_H
