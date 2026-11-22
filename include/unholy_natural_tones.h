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

#endif // GUARD_UNHOLY_NATURAL_TONES_H
