#include "global.h"
#include "palette.h"
#include "unholy_natural_tones.h"

// Weights sum to 100. A Pokémon's personality fixes its palette for life.
// Shinies remain shiny, but their shiny species palette may also be hue-shifted.
static const u8 sNormalToneWeights[NATURAL_TONE_COUNT] = {25, 10, 10, 7, 8, 10, 10, 7, 7, 3, 3};
static const u8 sShinyToneWeights[NATURAL_TONE_COUNT]  = {25,  5,  5, 5, 5, 10, 10, 10, 10, 7, 8};

static u8 NaturalTone_ClampChannel(s32 value)
{
    if (value < 0)
        return 0;
    if (value > 31)
        return 31;
    return value;
}

static u8 NaturalTone_Select(u32 personality, const u8 *weights)
{
    u32 hash = personality ^ (personality >> 16);
    u32 roll;
    u32 i;

    hash ^= hash >> 13;
    hash *= 0x5BD1E995u;
    hash ^= hash >> 15;
    roll = hash % 100;

    for (i = 0; i < NATURAL_TONE_COUNT; i++)
    {
        if (roll < weights[i])
            return i;
        roll -= weights[i];
    }

    return NATURAL_TONE_STANDARD;
}

u8 NaturalTone_FromPersonality(u32 personality)
{
    return NaturalTone_Select(personality, sNormalToneWeights);
}

u8 NaturalTone_FromShinyPersonality(u32 personality)
{
    return NaturalTone_Select(personality, sShinyToneWeights);
}

// Shift hue around the RGB hexagon while retaining the original minimum,
// maximum and shading: red can become magenta/violet without becoming white.
// Hue uses 6 sectors x 256 steps (60 degrees per sector), all integer math.
static u16 NaturalTone_ShiftHue(u16 color, s32 red, s32 green, s32 blue, s32 shift)
{
    s32 hi = max(red, max(green, blue));
    s32 lo = min(red, min(green, blue));
    s32 chroma = hi - lo;
    s32 hue;
    s32 sector;
    s32 fraction;
    s32 cross;

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
        red = hi;
        green = lo + cross;
        blue = lo;
        break;
    case 1:
        red = hi - cross;
        green = hi;
        blue = lo;
        break;
    case 2:
        red = lo;
        green = hi;
        blue = lo + cross;
        break;
    case 3:
        red = lo;
        green = hi - cross;
        blue = hi;
        break;
    case 4:
        red = lo + cross;
        green = lo;
        blue = hi;
        break;
    default:
        red = hi;
        green = lo;
        blue = hi - cross;
        break;
    }

    return (color & 0x8000) | red | (green << 5) | (blue << 10);
}

u16 NaturalTone_AdjustColor(u16 color, u8 tone)
{
    s32 red = color & 31;
    s32 green = (color >> 5) & 31;
    s32 blue = (color >> 10) & 31;
    s32 sum = red + green + blue;

    // Preserve near-black outlines, near-white highlights and the high bit.
    if (tone == NATURAL_TONE_STANDARD || sum <= 6 || sum >= 87)
        return color;

    switch (tone)
    {
    case NATURAL_TONE_WARM:
        red += 2;
        blue -= 2;
        break;
    case NATURAL_TONE_COOL:
        red -= 2;
        blue += 2;
        break;
    case NATURAL_TONE_LIGHT:
        red++;
        green++;
        blue++;
        break;
    case NATURAL_TONE_DARK:
        red--;
        green--;
        blue--;
        break;
    case NATURAL_TONE_HUE_PLUS_30:
        return NaturalTone_ShiftHue(color, red, green, blue, 128);
    case NATURAL_TONE_HUE_MINUS_30:
        return NaturalTone_ShiftHue(color, red, green, blue, -128);
    case NATURAL_TONE_HUE_PLUS_60:
        return NaturalTone_ShiftHue(color, red, green, blue, 256);
    case NATURAL_TONE_HUE_MINUS_60:
        return NaturalTone_ShiftHue(color, red, green, blue, -256);
    case NATURAL_TONE_HUE_PLUS_90:
        return NaturalTone_ShiftHue(color, red, green, blue, 384);
    case NATURAL_TONE_HUE_MINUS_90:
        return NaturalTone_ShiftHue(color, red, green, blue, -384);
    default:
        return color;
    }

    return (color & 0x8000)
         | NaturalTone_ClampChannel(red)
         | (NaturalTone_ClampChannel(green) << 5)
         | (NaturalTone_ClampChannel(blue) << 10);

}

void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, bool32 isShiny, bool32 isEgg)
{
    u8 tone;
    u16 colors[16];
    u32 i;

    if (isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors))
        return;

    // Shiny status, sparkles and encounter probability remain untouched.
    // Only the already loaded normal/shiny species sprite colors change.
    tone = isShiny ? NaturalTone_FromShinyPersonality(personality)
                   : NaturalTone_FromPersonality(personality);
    if (tone == NATURAL_TONE_STANDARD)
        return;

    // Start from the freshly loaded palette, not a prior tinted sprite.
    colors[0] = gPlttBufferUnfaded[paletteOffset];
    for (i = 1; i < ARRAY_COUNT(colors); i++)
        colors[i] = NaturalTone_AdjustColor(gPlttBufferUnfaded[paletteOffset + i], tone);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}
