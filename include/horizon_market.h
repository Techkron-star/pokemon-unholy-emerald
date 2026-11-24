#ifndef GUARD_HORIZON_MARKET_H
#define GUARD_HORIZON_MARKET_H

#include "pokemon.h"

#define HORIZON_MARKET_OFFER_COUNT 6

// The same daily stock exposes more slots as the watch evolves. These pure
// policies also apply to Horizon's purchases and mutation rolls.
u8 HorizonMarketOfferCountForTier(u8 watchTier);
u8 HorizonMarketMutationGradeForTier(u8 watchTier);
u8 HorizonMarketGetVisibleOfferCount(void);

enum HorizonMarketPurchaseResult
{
    HORIZON_MARKET_PURCHASE_OK,
    HORIZON_MARKET_PURCHASE_SOLD_OUT,
    HORIZON_MARKET_PURCHASE_NO_MONEY,
    HORIZON_MARKET_PURCHASE_NO_SPACE,
    HORIZON_MARKET_PURCHASE_FAILED,
};

void HorizonMarketRefresh(void);
bool8 HorizonMarketHasUnseenMutation(void);
void HorizonMarketMarkPokemonSeen(void);
enum Species HorizonMarketGetSpecies(u8 slot);
bool8 HorizonMarketOfferSold(u8 slot);
enum PokemonBadge HorizonMarketGetMutation(u8 slot);
u32 HorizonMarketGetPrice(u8 slot);
enum HorizonMarketPurchaseResult HorizonMarketPurchase(u8 slot);
const u8 *HorizonMarketMutationName(enum PokemonBadge mutation);

#endif // GUARD_HORIZON_MARKET_H
