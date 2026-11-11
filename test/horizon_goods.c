#include "global.h"
#include "horizon_goods.h"
#include "event_data.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/vars.h"

TEST("Horizon goods lists are deterministic for the same saved market")
{
    u16 oldSeedA = VarGet(VAR_HORIZON_GOODS_SEED_A);
    u16 oldSeedB = VarGet(VAR_HORIZON_GOODS_SEED_B);
    u16 oldBadges = VarGet(VAR_HORIZON_MARKET_BADGES);
    u16 oldPeriod = VarGet(VAR_HORIZON_MARKET_PERIOD);
    u16 items[HORIZON_GOODS_ITEM_COUNT + 1];
    u16 discs[HORIZON_GOODS_DISC_COUNT + 1];
    const u16 *actual;

    VarSet(VAR_HORIZON_GOODS_SEED_A, 0x4A31);
    VarSet(VAR_HORIZON_GOODS_SEED_B, 0x9B27);
    VarSet(VAR_HORIZON_MARKET_BADGES, 0);
    VarSet(VAR_HORIZON_MARKET_PERIOD, 0);

    actual = HorizonGoodsGetItems();
    for (u8 i = 0; i <= HORIZON_GOODS_ITEM_COUNT; i++)
        items[i] = actual[i];

    actual = HorizonGoodsGetDiscs();
    for (u8 i = 0; i <= HORIZON_GOODS_DISC_COUNT; i++)
        discs[i] = actual[i];

    // Nothing changes when the user reopens the same two shops.
    actual = HorizonGoodsGetItems();
    for (u8 i = 0; i <= HORIZON_GOODS_ITEM_COUNT; i++)
        EXPECT_EQ(actual[i], items[i]);

    actual = HorizonGoodsGetDiscs();
    for (u8 i = 0; i <= HORIZON_GOODS_DISC_COUNT; i++)
        EXPECT_EQ(actual[i], discs[i]);

    EXPECT_EQ(items[0], ITEM_POKE_BALL);
    EXPECT_EQ(items[1], ITEM_POTION);
    EXPECT_EQ(items[2], ITEM_ESCAPE_ROPE);
    EXPECT_EQ(items[HORIZON_GOODS_ITEM_COUNT], ITEM_NONE);
    EXPECT_EQ(discs[HORIZON_GOODS_DISC_COUNT], ITEM_NONE);

    for (u8 i = 3; i < HORIZON_GOODS_ITEM_COUNT; i++)
    {
        EXPECT_NE(items[i], ITEM_NONE);
        EXPECT_NE(items[i], ITEM_ULTRA_BALL);
        EXPECT_NE(items[i], ITEM_RARE_CANDY);
        for (u8 j = i + 1; j < HORIZON_GOODS_ITEM_COUNT; j++)
            EXPECT_NE(items[i], items[j]);
    }

    for (u8 i = 0; i < HORIZON_GOODS_DISC_COUNT; i++)
    {
        EXPECT_NE(discs[i], ITEM_NONE);
        EXPECT_NE(discs[i], ITEM_TM26); // TM26 requires seven badges.
        for (u8 j = i + 1; j < HORIZON_GOODS_DISC_COUNT; j++)
            EXPECT_NE(discs[i], discs[j]);
    }

    // Restore the surrounding test world's market state.
    VarSet(VAR_HORIZON_GOODS_SEED_A, oldSeedA);
    VarSet(VAR_HORIZON_GOODS_SEED_B, oldSeedB);
    VarSet(VAR_HORIZON_MARKET_BADGES, oldBadges);
    VarSet(VAR_HORIZON_MARKET_PERIOD, oldPeriod);
}
