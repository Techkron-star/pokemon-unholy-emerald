#include "global.h"
#include "hfc_watch.h"
#include "test/test.h"

// Tests for the HFC Watch tier logic. The hub window needs mGBA (see DISENO.md).

TEST("HFC Watch tier is the best watch owned")
{
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, FALSE, FALSE), HFC_WATCH_TIER_NONE);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, FALSE, FALSE), HFC_WATCH_TIER_POKE);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, TRUE, FALSE), HFC_WATCH_TIER_SUPER);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, TRUE, TRUE), HFC_WATCH_TIER_HF);
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, FALSE, TRUE), HFC_WATCH_TIER_HF);
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, TRUE, FALSE), HFC_WATCH_TIER_SUPER);
}

TEST("HFC Watch lower tiers are the HF feature set with entries masked out")
{
    u8 poke = HfcWatch_GetTierMask(HFC_WATCH_TIER_POKE);
    u8 super = HfcWatch_GetTierMask(HFC_WATCH_TIER_SUPER);
    u8 hf = HfcWatch_GetTierMask(HFC_WATCH_TIER_HF);

    EXPECT_EQ(HfcWatch_GetTierMask(HFC_WATCH_TIER_NONE), 0);
    EXPECT_EQ(poke & hf, poke);
    EXPECT_EQ(super & hf, super);
    EXPECT_EQ(poke & super, poke);
    EXPECT_EQ(hf, (1 << HFC_WATCH_FEATURE_COUNT) - 1);
}

TEST("HFC Watch tiers add the features the design assigns")
{
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_HORIZON_SHOP));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_POKENAV));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_POKELOCATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_POKELOCATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_MUTATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_SIMULATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_NONE, HFC_WATCH_FEATURE_HORIZON_SHOP));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_COUNT));
}

TEST("HFC Watch opens the basic PokeNav at zero badges and lists working tools")
{
    u8 list[HFC_WATCH_FEATURE_COUNT];

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_NONE, 0, list), 0);

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_POKE, 0, list), 2);
    EXPECT_EQ(list[0], HFC_WATCH_FEATURE_HORIZON_SHOP);
    EXPECT_EQ(list[1], HFC_WATCH_FEATURE_POKENAV);

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_SUPER, 0, list), 3);
    EXPECT_EQ(list[2], HFC_WATCH_FEATURE_POKELOCATOR);

    // The HF hub can launch three tools while the paid services are pending.
    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_HF, 8, list), 3);
}

TEST("HFC Watch paid services require the HF tier and their own badge thresholds")
{
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_SIMULATOR, 8), HFC_WATCH_WRONG_TIER);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_MUTATOR, 8), HFC_WATCH_WRONG_TIER);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR, 3), HFC_WATCH_NEEDS_BADGES);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR, 4), HFC_WATCH_SERVICE_PENDING);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR, 7), HFC_WATCH_NEEDS_BADGES);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR, 8), HFC_WATCH_SERVICE_PENDING);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_COUNT, 8), HFC_WATCH_WRONG_TIER);
}
