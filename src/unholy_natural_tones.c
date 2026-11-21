#include "global.h"
#include "palette.h"
#include "unholy_natural_tones.h"

static u8 NaturalTone_ClampChannel(s32 value)
{
    if (value < 0)
        return 0;
    if (value > 31)
        return 31;
    return value;
}

u8 NaturalTone_FromPersonality(u32 personality)
{
    // Stable across saves, evolution, captures and reloads; no RNG consumed.
    u32 hash = personality ^ (personality >> 16);
    hash ^= hash >> 13;
    hash *= 0x5BD1E995u;
    hash ^= hash >> 15;
    hash %= 20;

    if (hash < 8)
        return NATURAL_TONE_STANDARD;
    if (hash < 11)
        return NATURAL_TONE_WARM;
    if (hash < 14)
        return NATURAL_TONE_COOL;
    if (hash < 17)
        return NATURAL_TONE_LIGHT;
    return NATURAL_TONE_DARK;
}

u16 NaturalTone_AdjustColor(u16 color, u8 tone)
{
    s32 red = color & 31;
    s32 green = (color >> 5) & 31;
    s32 blue = (color >> 10) & 31;
    s32 sum = red + green + blue;

    // Preserve outlines, near-white highlights and the reserved high bit.
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

    if (isShiny || isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors))
        return;

    tone = NaturalTone_FromPersonality(personality);
    if (tone == NATURAL_TONE_STANDARD)
        return;

    // Copy from the freshly loaded original palette, never from a previous
    // tinted result. Palette index zero is transparent in 4bpp sprites.
    colors[0] = gPlttBufferUnfaded[paletteOffset];
    for (i = 1; i < ARRAY_COUNT(colors); i++)
        colors[i] = NaturalTone_AdjustColor(gPlttBufferUnfaded[paletteOffset + i], tone);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}
