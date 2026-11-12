#ifndef GUARD_HFC_WATCH_H
#define GUARD_HFC_WATCH_H

// HFC Watch: one wrist device in three tiers. Every tier is the same device with a
// capped feature set, so the only data is "which feature needs which tier".
enum HfcWatchTier
{
    HFC_WATCH_TIER_NONE,
    HFC_WATCH_TIER_POKE,   // Poke Watch
    HFC_WATCH_TIER_SUPER,  // Super Poke Watch
    HFC_WATCH_TIER_HF,     // HF Poke Watch (full set)
    HFC_WATCH_TIER_COUNT
};

enum HfcWatchFeature
{
    HFC_WATCH_FEATURE_HORIZON_SHOP,
    HFC_WATCH_FEATURE_POKENAV,
    HFC_WATCH_FEATURE_POKELOCATOR,
    HFC_WATCH_FEATURE_MUTATOR,
    HFC_WATCH_FEATURE_SIMULATOR,
    HFC_WATCH_FEATURE_COUNT
};

enum HfcWatchAccess
{
    HFC_WATCH_WRONG_TIER,
    HFC_WATCH_NEEDS_BADGES,
    HFC_WATCH_SERVICE_PENDING,
    HFC_WATCH_READY,
};

#define HFC_WATCH_FEATURE_BIT(feature) (1 << (feature))

// Pure logic (src/hfc_watch_data.c): no hardware, no UI, no save access.
u8 HfcWatch_TierFromItems(bool8 hasPoke, bool8 hasSuper, bool8 hasHF);
u8 HfcWatch_GetTierMask(u8 tier);                       // features the tier includes
bool8 HfcWatch_TierHasFeature(u8 tier, u8 feature);
enum HfcWatchAccess HfcWatch_GetAccess(u8 tier, u8 feature, u8 badges);
u8 HfcWatch_BuildMenu(u8 tier, u8 badges, u8 *featuresOut);

// Game side (src/hfc_watch.c).
u8 HfcWatch_GetTier(void);
void HfcWatch_OpenHub(bool8 fromStartMenu);

#endif // GUARD_HFC_WATCH_H
