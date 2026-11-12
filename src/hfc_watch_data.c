#include "global.h"
#include "hfc_watch.h"

// Data side of the HFC Watch. The HF tier is the complete device; the lower tiers are
// the same table with the higher entries masked out.

struct HfcWatchFeatureInfo
{
    u8 minTier;
    // A feature is only offered when something real can be launched. The Mutator and the
    // Simulator have no rules (cost, odds, unlocks) in DISENO.md yet, so they stay
    // listed here for the tier layout but never appear in the hub.
    bool8 hasLauncher;
};

static const struct HfcWatchFeatureInfo sFeatures[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = { HFC_WATCH_TIER_POKE,  TRUE },
    [HFC_WATCH_FEATURE_POKENAV]      = { HFC_WATCH_TIER_POKE,  TRUE },
    [HFC_WATCH_FEATURE_POKELOCATOR]  = { HFC_WATCH_TIER_SUPER, TRUE },
    [HFC_WATCH_FEATURE_MUTATOR]      = { HFC_WATCH_TIER_HF,    FALSE },
    [HFC_WATCH_FEATURE_SIMULATOR]    = { HFC_WATCH_TIER_HF,    FALSE },
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

bool8 HfcWatch_FeatureHasLauncher(u8 feature)
{
    return feature < HFC_WATCH_FEATURE_COUNT && sFeatures[feature].hasLauncher;
}

// Lists, in order, what the hub shows: included in the tier, launchable, and not blocked
// by story progress (the PokeNav entry needs the PokeNav itself, so the watch cannot
// grant it early). Returns the number of entries.
u8 HfcWatch_BuildMenu(u8 tier, bool8 hasPokenav, u8 *featuresOut)
{
    u8 mask = HfcWatch_GetTierMask(tier);
    u8 i, count = 0;

    for (i = 0; i < HFC_WATCH_FEATURE_COUNT; i++)
    {
        if (!(mask & HFC_WATCH_FEATURE_BIT(i)) || !sFeatures[i].hasLauncher)
            continue;
        if (i == HFC_WATCH_FEATURE_POKENAV && !hasPokenav)
            continue;
        featuresOut[count++] = i;
    }
    return count;
}
