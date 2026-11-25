#include "global.h"
#include "horizon_goods.h"
#include "hfc_watch.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "unholy_save.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/vars.h"

TEST("Horizon V2 keeps twelve goods and discs stable and buys a complete offer only once")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u16 oldSeedA = VarGet(VAR_HORIZON_GOODS_SEED_A);
    u16 oldSeedB = VarGet(VAR_HORIZON_GOODS_SEED_B);
    u16 oldBadges = VarGet(VAR_HORIZON_MARKET_BADGES);
    u16 oldPeriod = VarGet(VAR_HORIZON_MARKET_PERIOD);
    u16 oldStatus = VarGet(VAR_HORIZON_MARKET_STATUS);
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    u16 items[HORIZON_GOODS_ITEM_COUNT + 1];
    u16 discs[HORIZON_GOODS_DISC_COUNT + 1];
    struct HorizonGoodsOffer offers[HORIZON_GOODS_ITEM_COUNT];

    SetBagItemsPointers();
    bool32 hadWatch = CheckBagHasItem(ITEM_HF_POKE_WATCH, 1);
    if (!hadWatch)
        EXPECT(AddBagItem(ITEM_HF_POKE_WATCH, 1));
    UnholySave4_Reset();
    VarSet(VAR_HORIZON_GOODS_SEED_A, 0x4A31);
    VarSet(VAR_HORIZON_GOODS_SEED_B, 0x9B27);
    VarSet(VAR_HORIZON_MARKET_BADGES, 8);
    VarSet(VAR_HORIZON_MARKET_PERIOD, 0);
    VarSet(VAR_HORIZON_MARKET_STATUS, HFC_WATCH_TIER_HF << 13);

    memcpy(items, HorizonGoodsGetItems(), sizeof(items));
    memcpy(discs, HorizonGoodsGetDiscs(), sizeof(discs));
    EXPECT_EQ(items[0], ITEM_POKE_BALL);
    EXPECT_EQ(items[1], ITEM_POTION);
    EXPECT_EQ(items[2], ITEM_ESCAPE_ROPE);
    EXPECT_EQ(items[3], ITEM_LINKING_CORD);
    EXPECT(items[4] >= ITEM_NORMAL_GEM && items[4] <= ITEM_FAIRY_GEM);
    EXPECT_EQ(items[12], ITEM_NONE);
    EXPECT_EQ(discs[12], ITEM_NONE);
    for (u8 i = 0; i < 12; i++)
    {
        EXPECT_NE(items[i], ITEM_NONE);
        EXPECT_NE(discs[i], ITEM_NONE);
        EXPECT_NE(items[i], ITEM_MEGA_RING);
        EXPECT_NE(items[i], ITEM_Z_POWER_RING);
        EXPECT_NE(items[i], ITEM_DYNAMAX_BAND);
        EXPECT_NE(items[i], ITEM_TERA_ORB);
        const struct HorizonGoodsOffer *offer = HorizonGoodsGetOffer(FALSE, i);
        EXPECT(offer != NULL);
        offers[i] = *offer;
        EXPECT(offer->quantity >= 1 && offer->quantity <= 99);
        EXPECT(offer->price > 0);
        EXPECT(offer->price < HorizonGoodsReferencePrice(offer->itemId) || HorizonGoodsReferencePrice(offer->itemId) == 1);
        for (u8 j = i + 1; j < 12; j++)
        {
            EXPECT_NE(items[i], items[j]);
            EXPECT_NE(discs[i], discs[j]);
        }
    }

    EXPECT_EQ(memcmp(items, HorizonGoodsGetItems(), sizeof(items)), 0);
    EXPECT_EQ(memcmp(discs, HorizonGoodsGetDiscs(), sizeof(discs)), 0);
    for (u8 i = 0; i < 12; i++)
    {
        const struct HorizonGoodsOffer *offer = HorizonGoodsGetOffer(FALSE, i);
        EXPECT_EQ(offer->itemId, offers[i].itemId);
        EXPECT_EQ(offer->price, offers[i].price);
        EXPECT_EQ(offer->quantity, offers[i].quantity);
        EXPECT_EQ(offer->grade, offers[i].grade);
    }

    u16 before = CountTotalItemQuantityInBag(ITEM_POKE_BALL);
    SetMoney(&gSaveBlock1Ptr->money, 0);
    EXPECT_EQ(HorizonGoodsPurchase(FALSE, 0, ITEM_POKE_BALL, offers[0].price), HORIZON_MARKET_PURCHASE_NO_MONEY);
    EXPECT(!HorizonGoodsOfferSold(FALSE, 0));
    SetMoney(&gSaveBlock1Ptr->money, 100000);
    EXPECT_EQ(HorizonGoodsPurchase(FALSE, 0, ITEM_POKE_BALL, offers[0].price), HORIZON_MARKET_PURCHASE_OK);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), before + offers[0].quantity);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 100000 - offers[0].price);
    EXPECT(HorizonGoodsOfferSold(FALSE, 0));
    EXPECT_EQ(HorizonGoodsPurchase(FALSE, 0, ITEM_POKE_BALL, offers[0].price), HORIZON_MARKET_PURCHASE_SOLD_OUT);
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));

    RemoveBagItem(ITEM_POKE_BALL, offers[0].quantity);
    if (!hadWatch)
        RemoveBagItem(ITEM_HF_POKE_WATCH, 1);
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
    gSaveBlock3Ptr->unholySave4 = oldSave;
    VarSet(VAR_HORIZON_GOODS_SEED_A, oldSeedA);
    VarSet(VAR_HORIZON_GOODS_SEED_B, oldSeedB);
    VarSet(VAR_HORIZON_MARKET_BADGES, oldBadges);
    VarSet(VAR_HORIZON_MARKET_PERIOD, oldPeriod);
    VarSet(VAR_HORIZON_MARKET_STATUS, oldStatus);
}
