#include "global.h"
#include "hfc_watch.h"

// Data side of the HFC Watch. The HF tier is the complete device; the lower tiers are
// the same table with the higher entries masked out.

struct HfcWatchFeatureInfo
{
    u8 minTier;
    u8 minBadges;
    bool8 hasLauncher;
};

static const struct HfcWatchFeatureInfo sFeatures[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = { HFC_WATCH_TIER_POKE,  0, TRUE },
    [HFC_WATCH_FEATURE_POKENAV]      = { HFC_WATCH_TIER_POKE,  0, TRUE },
    [HFC_WATCH_FEATURE_CRAFTING]     = { HFC_WATCH_TIER_POKE,  0, TRUE },
    [HFC_WATCH_FEATURE_POKELOCATOR]  = { HFC_WATCH_TIER_SUPER, 0, TRUE },
    [HFC_WATCH_FEATURE_STORAGE]     = { HFC_WATCH_TIER_SUPER, 2, TRUE },
    [HFC_WATCH_FEATURE_FREQUENCY]   = { HFC_WATCH_TIER_HF,    3, TRUE },
    [HFC_WATCH_FEATURE_RAID]        = { HFC_WATCH_TIER_HF,    5, TRUE },
    [HFC_WATCH_FEATURE_ARCHIVE]     = { HFC_WATCH_TIER_HF,    0, TRUE },
    [HFC_WATCH_FEATURE_MUTATOR]      = { HFC_WATCH_TIER_HF,    8, TRUE },
    [HFC_WATCH_FEATURE_SIMULATOR]    = { HFC_WATCH_TIER_HF,    4, TRUE },
};

const u8 *HfcWatch_GetRequirement(u8 tier, u8 feature, struct HfcWatchProgress progress)
{
    static const u8 *const badges[] =
    {
        COMPOUND_STRING("0 BADGES"), COMPOUND_STRING("1 BADGE"), COMPOUND_STRING("2 BADGES"),
        COMPOUND_STRING("3 BADGES"), COMPOUND_STRING("4 BADGES"), COMPOUND_STRING("5 BADGES"),
        COMPOUND_STRING("6 BADGES"), COMPOUND_STRING("7 BADGES"), COMPOUND_STRING("8 BADGES"),
    };
    if (feature >= HFC_WATCH_FEATURE_COUNT)
        return COMPOUND_STRING("UNAVAILABLE");
    switch (HfcWatch_GetAccess(tier, feature, progress))
    {
    case HFC_WATCH_WRONG_TIER:
        return sFeatures[feature].minTier == HFC_WATCH_TIER_HF ? COMPOUND_STRING("REQUIRES HF POKé WATCH")
             : sFeatures[feature].minTier == HFC_WATCH_TIER_SUPER ? COMPOUND_STRING("REQUIRES SUPER POKé WATCH")
             : COMPOUND_STRING("REQUIRES POKé WATCH");
    case HFC_WATCH_NEEDS_BADGES: return badges[sFeatures[feature].minBadges];
    case HFC_WATCH_NEEDS_UNLOCK: return COMPOUND_STRING("UNLOCK CRAFTING AFTER ROXANNE");
    case HFC_WATCH_SERVICE_PENDING: return COMPOUND_STRING("COMING SOON");
    default: return COMPOUND_STRING("READY");
    }
}

// The best watch owned wins; lower ones in the bag are ignored.
u8 HfcWatch_TierFromItems(bool8 hasPoke, bool8 hasSuper, bool8 hasHF)
{
    if (hasHF)
        return HFC_WATCH_TIER_HF;
    if (hasSuper)
        return HFC_WATCH_TIER_SUPER;
    if (hasPoke)
        return HFC_WATCH_TIER_POKE;
    return HFC_WATCH_TIER_NONE;
}

u16 HfcWatch_GetTierMask(u8 tier)
{
    u16 mask = 0;

    if (tier == HFC_WATCH_TIER_NONE || tier >= HFC_WATCH_TIER_COUNT)
        return 0;
    for (u8 i = 0; i < HFC_WATCH_FEATURE_COUNT; i++)
    {
        if (sFeatures[i].minTier <= tier)
            mask |= HFC_WATCH_FEATURE_BIT(i);
    }
    return mask;
}

bool8 HfcWatch_TierHasFeature(u8 tier, u8 feature)
{
    return feature < HFC_WATCH_FEATURE_COUNT
        && tier < HFC_WATCH_TIER_COUNT
        && tier >= sFeatures[feature].minTier;
}

enum HfcWatchAccess HfcWatch_GetAccess(u8 tier, u8 feature, struct HfcWatchProgress progress)
{
    if (!HfcWatch_TierHasFeature(tier, feature))
        return HFC_WATCH_WRONG_TIER;
    if (progress.badges < sFeatures[feature].minBadges)
        return HFC_WATCH_NEEDS_BADGES;
    if (feature == HFC_WATCH_FEATURE_CRAFTING && !progress.craftingUnlocked)
        return HFC_WATCH_NEEDS_UNLOCK;
    if (!sFeatures[feature].hasLauncher)
        return HFC_WATCH_SERVICE_PENDING;
    return HFC_WATCH_READY;
}

u8 HfcWatch_BuildMenu(u8 tier, struct HfcWatchProgress progress, u8 *featuresOut)
{
    u8 i, count = 0;

    for (i = 0; i < HFC_WATCH_FEATURE_COUNT; i++)
    {
        if (HfcWatch_GetAccess(tier, i, progress) == HFC_WATCH_READY)
            featuresOut[count++] = i;
    }
    return count;
}
