#include "global.h"
#include "palette.h"
#include "unholy_natural_tones.h"

// VERSION 1 — immutable. The 64-bit code is a bijective encoding of the
// original-trainer ID and personality, not the species, level, nickname or
// current owner. Two identical (PID, OT ID) pairs, e.g. cloned Pokémon, are
// indistinguishable without adding a new persistent field to the save.
static u32 NaturalTone_Mix32(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352Du;
    value ^= value >> 15;
    value *= 0x846CA68Bu;
    value ^= value >> 16;
    return value;
}

u64 NaturalTone_GetIdentityCode(u32 personality, u32 otId)
{
    // Mix32 is a permutation; combining the two results never folds the
    // existing 64-bit identity into a 32-bit hash.
    u32 lower = NaturalTone_Mix32(personality ^ 0xC0A10B13u);
    u32 upper = NaturalTone_Mix32(otId ^ 0x726F6D31u);
    return ((u64)upper << 32) | lower;
}

void NaturalTone_GetProfile(u32 personality, u32 otId, bool32 isShiny, struct NaturalToneProfile *profile)
{
    u64 identity = NaturalTone_GetIdentityCode(personality, otId);
    u32 lower = identity;
    u32 upper = identity >> 32;
    u32 category = NaturalTone_Mix32(lower ^ (upper << 11 | upper >> 21) ^ 0x43485231u);
    u32 hue = NaturalTone_Mix32(upper ^ (lower >> 7 | lower << 25) ^ 0x68756531u);
    u32 detail = NaturalTone_Mix32(category ^ hue ^ 0x64657431u);
    u32 roll = category % 1000;

    // Continuous hue shifts are the default. Opposite-wheel hues are rare;
    // an even rarer luminance inversion enables dark shiny body -> white.
    if (roll < (isShiny ? 880 : 900))
    {
        profile->mode = NATURAL_TONE_NEAR;
        profile->hueShift = (s32)(hue % 641) - 320; // approx. ±75 degrees
    }
    else if (roll < (isShiny ? 975 : 990))
    {
        profile->mode = NATURAL_TONE_COMPLEMENT;
        profile->hueShift = 683 + hue % 171; // approx. 160..200 degrees
    }
    else
    {
        profile->mode = NATURAL_TONE_LIGHT_INVERSION;
        profile->hueShift = (s32)(hue % 129) - 64; // subtle colored accents
    }

    profile->saturationPercent = (s32)(detail % 13) - 6;
    profile->brightnessSteps = (s32)((detail >> 8) % 3) - 1;
}

// Rotate a saturated RGB555 color around the hue wheel without changing its
// darkest or lightest channel. The grayscale/shadows remain recognizable.
static u16 NaturalTone_ShiftHue(u16 color, s32 red, s32 green, s32 blue, s32 shift)
{
    s32 hi = max(red, max(green, blue));
    s32 lo = min(red, min(green, blue));
    s32 chroma = hi - lo;
    s32 hue, sector, fraction, cross;

    if (chroma < 4)
        return color;

    if (hi == red)
        hue = 256 * (green - blue) / chroma;
    else if (hi == green)
        hue = 512 + 256 * (blue - red) / chroma;
    else
        hue = 1024 + 256 * (red - green) / chroma;

    if (hue < 0)
        hue += 1536;
    hue += shift;
    if (hue < 0)
        hue += 1536;
    else if (hue >= 1536)
        hue -= 1536;

    sector = hue / 256;
    fraction = hue % 256;
    cross = chroma * fraction / 256;

    switch (sector)
    {
    case 0:
        red = hi; green = lo + cross; blue = lo;
        break;
    case 1:
        red = hi - cross; green = hi; blue = lo;
        break;
    case 2:
        red = lo; green = hi; blue = lo + cross;
        break;
    case 3:
        red = lo; green = hi - cross; blue = hi;
        break;
    case 4:
        red = lo + cross; green = lo; blue = hi;
        break;
    default:
        red = hi; green = lo; blue = hi - cross;
        break;
    }
    return (color & 0x8000) | red | (green << 5) | (blue << 10);
}

static u8 NaturalTone_ClampChannel(s32 value)
{
    if (value < 0)
        return 0;
    if (value > 31)
        return 31;
    return value;
}

u16 NaturalTone_AdjustColor(u16 color, const struct NaturalToneProfile *profile)
{
    s32 red = color & 31;
    s32 green = (color >> 5) & 31;
    s32 blue = (color >> 10) & 31;
    s32 avg, gray, result;

    // Preserve outlines and white highlights; palette entry zero is skipped
    // separately by NaturalTone_ApplyLoadedPalette.
    if (red + green + blue <= 6 || red + green + blue >= 87)
        return color;

    if (profile->mode == NATURAL_TONE_LIGHT_INVERSION
     && max(red, max(green, blue)) - min(red, min(green, blue)) <= 6)
    {
        // Lighten neutral body shades while keeping their ordering and the
        // original black outline. This permits a white/gray shiny Charizard.
        gray = (red * 3 + green * 4 + blue) / 8;
        if (gray <= 5)
            return color; // Low-value eye detail and soft outlines.
        result = 17 + gray * 14 / 31;
        return (color & 0x8000) | result | (result << 5) | (result << 10);
    }

    color = NaturalTone_ShiftHue(color, red, green, blue, profile->hueShift);
    red = color & 31;
    green = (color >> 5) & 31;
    blue = (color >> 10) & 31;

    avg = (red + green + blue) / 3;
    red = avg + (red - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;
    green = avg + (green - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;
    blue = avg + (blue - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;

    return (color & 0x8000)
         | NaturalTone_ClampChannel(red)
         | (NaturalTone_ClampChannel(green) << 5)
         | (NaturalTone_ClampChannel(blue) << 10);
}

void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, u32 otId, bool32 isShiny, bool32 isEgg)
{
    struct NaturalToneProfile profile;
    u16 colors[16];
    u32 i;

    if (isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors))
        return;

    NaturalTone_GetProfile(personality, otId, isShiny, &profile);

    // Recompute only from the canonical normal/shiny palette, never from a
    // previously tinted buffer. Do not alter the transparency entry.
    colors[0] = gPlttBufferUnfaded[paletteOffset];
    for (i = 1; i < ARRAY_COUNT(colors); i++)
        colors[i] = NaturalTone_AdjustColor(gPlttBufferUnfaded[paletteOffset + i], &profile);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}
