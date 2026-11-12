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
    [HFC_WATCH_FEATURE_POKELOCATOR]  = { HFC_WATCH_TIER_SUPER, 0, TRUE },
    [HFC_WATCH_FEATURE_MUTATOR]      = { HFC_WATCH_TIER_HF,    8, FALSE },
    [HFC_WATCH_FEATURE_SIMULATOR]    = { HFC_WATCH_TIER_HF,    4, FALSE },
};

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

u8 HfcWatch_GetTierMask(u8 tier)
{
    u8 mask = 0, i;

    if (tier == HFC_WATCH_TIER_NONE || tier >= HFC_WATCH_TIER_COUNT)
        return 0;
    for (i = 0; i < HFC_WATCH_FEATURE_COUNT; i++)
    {
        if (sFeatures[i].minTier <= tier)
            mask |= HFC_WATCH_FEATURE_BIT(i);
    }
    return mask;
}

bool8 HfcWatch_TierHasFeature(u8 tier, u8 feature)
{
    if (feature >= HFC_WATCH_FEATURE_COUNT)
        return FALSE;
    return (HfcWatch_GetTierMask(tier) & HFC_WATCH_FEATURE_BIT(feature)) != 0;
}

enum HfcWatchAccess HfcWatch_GetAccess(u8 tier, u8 feature, u8 badges)
{
    if (!HfcWatch_TierHasFeature(tier, feature))
        return HFC_WATCH_WRONG_TIER;
    if (badges < sFeatures[feature].minBadges)
        return HFC_WATCH_NEEDS_BADGES;
    if (!sFeatures[feature].hasLauncher)
        return HFC_WATCH_SERVICE_PENDING;
    return HFC_WATCH_READY;
}

u8 HfcWatch_BuildMenu(u8 tier, u8 badges, u8 *featuresOut)
{
    u8 i, count = 0;

    for (i = 0; i < HFC_WATCH_FEATURE_COUNT; i++)
    {
        if (HfcWatch_GetAccess(tier, i, badges) == HFC_WATCH_READY)
            featuresOut[count++] = i;
    }
    return count;
}
