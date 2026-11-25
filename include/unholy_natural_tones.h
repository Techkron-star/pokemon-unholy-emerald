#ifndef GUARD_UNHOLY_NATURAL_TONES_H
#define GUARD_UNHOLY_NATURAL_TONES_H

// Algorithm V1 is immutable for existing Pokémon: do not change the identity
// mapping or palette math in a later ROM without an explicit migration.
#define NATURAL_TONE_ALGORITHM_VERSION 1

enum NaturalToneMode
{
    NATURAL_TONE_NEAR,
    NATURAL_TONE_COMPLEMENT,
    NATURAL_TONE_LIGHT_INVERSION,
};

struct NaturalToneProfile
{
    u32 microSeed;           // Stable, tiny per-palette-shade variation.
    s16 hueShift;             // Six 256-step RGB-hue sectors.
    s8 saturationPercent;     // -6..+6
    s8 brightnessSteps;       // -1..+1 RGB555 steps
    u8 mode;
};

u64 NaturalTone_GetIdentityCode(u32 personality, u32 otId);
void NaturalTone_GetProfile(u32 personality, u32 otId, bool32 isShiny, struct NaturalToneProfile *profile);
u16 NaturalTone_AdjustColor(u16 color, u32 paletteIndex, const struct NaturalToneProfile *profile);
void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, u32 otId, bool32 isShiny, bool32 isEgg);

#define NATURAL_TONE_LATEST_VERSION 2

enum NaturalToneFinish
{
    NATURAL_FINISH_NATURAL,
    NATURAL_FINISH_PASTEL,
    NATURAL_FINISH_COPPER,
    NATURAL_FINISH_OBSIDIAN,
    NATURAL_FINISH_PORCELAIN,
    NATURAL_FINISH_MELANISM,
    NATURAL_FINISH_SOLAR,
    NATURAL_FINISH_MAGNETAR,
    NATURAL_FINISH_QUASAR,
    NATURAL_FINISH_COSMIC,
    NATURAL_FINISH_GHOSTLY,
    NATURAL_FINISH_INFERNAL,
    NATURAL_FINISH_GODLY,
    NATURAL_FINISH_AQUA,
    NATURAL_FINISH_JUNGLE,
    NATURAL_FINISH_NIGHTLY,
    NATURAL_FINISH_ALBINISM,
    NATURAL_FINISH_SPECTRAL,
    NATURAL_FINISH_GOLDEN,
    NATURAL_FINISH_IRIDESCENT,
    NATURAL_FINISH_CRIMSON,
    NATURAL_FINISH_GLACIAL,
    NATURAL_FINISH_TOXIC,
    NATURAL_FINISH_LUNAR,
    NATURAL_FINISH_COUNT,
};

struct NaturalToneV2Profile
{
    s16 hue[3];
    s8 saturation[3];
    s8 brightness[3];
    u8 finish;
    u8 scope; // Requested region count; 4 means whole palette.
    u8 roleOrder[3]; // Stable permutation of anatomical role IDs 0..2.
};

void NaturalTone_GetV2Profile(u32 personality, u32 otId, bool32 isShiny, struct NaturalToneV2Profile *profile);
// Two bits per palette entry: 0 automatic, 1 body, 2 secondary, 3 accent.
u32 NaturalTone_GetV2RoleMask(const u16 *base);
// A curated signature uses exact RGB555 palette colors; zero means no match.
u32 NaturalTone_GetCuratedRoleMask(const u16 *base);
// Return selected 1-based region bits as bit 0..2, ignoring absent roles.
u8 NaturalTone_SelectV2Roles(u32 packedRoles, const struct NaturalToneV2Profile *profile);
void NaturalTone_BuildV2Palette(const u16 *base, u16 *out, u32 personality, u32 otId, bool32 isShiny, u32 overrides);
void NaturalTone_ApplyLoadedPaletteVersion(u32 paletteOffset, u32 personality, u32 otId, bool32 isShiny, bool32 isEgg, bool32 isV2);

#endif // GUARD_UNHOLY_NATURAL_TONES_H
