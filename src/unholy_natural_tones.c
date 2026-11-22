#include "global.h"
#include "palette.h"
#include "unholy_natural_tones.h"

// The personality ID fixes the tint across saves, evolution and encounters.
// More dramatic variants are rarer among ordinary Pokémon; shinies can also
// have alternate shiny palettes (including a porcelain-white appearance).
static const u8 sNormalToneWeights[NATURAL_TONE_COUNT] = {30, 10, 10, 10, 10, 10, 10, 7, 3};
static const u8 sShinyToneWeights[NATURAL_TONE_COUNT]  = {30,  5,  5,  5,  5, 12, 12, 13, 13};

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

u16 NaturalTone_AdjustColor(u16 color, u8 tone)
{
    s32 red = color & 31;
    s32 green = (color >> 5) & 31;
    s32 blue = (color >> 10) & 31;
    s32 sum = red + green + blue;
    s32 max = max(red, max(green, blue));
    s32 min = min(red, min(green, blue));
    s32 oldRed = red;
    s32 oldGreen = green;
    s32 oldBlue = blue;
    s32 gray;
    s32 pearl;

    // Never recolor transparent entries (handled by caller), black outlines,
    // near-white highlights or the reserved high bit.
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
    case NATURAL_TONE_SHIFT_GREEN:
        // Cycle channels: orange/red body colors may turn green or turquoise.
        if (max - min < 4)
            return color;
        red = oldBlue;
        green = oldRed;
        blue = oldGreen;
        break;
    case NATURAL_TONE_SHIFT_VIOLET:
        // Reverse channel cycle: orange/red body colors may turn violet.
        if (max - min < 4)
            return color;
        red = oldGreen;
        green = oldBlue;
        blue = oldRed;
        break;
    case NATURAL_TONE_COMPLEMENT:
        // Complement the chromatic components without inverting grayscale
        // outlines, eye whites or every pixel's brightness.
        if (max - min < 4)
            return color;
        red = max + min - oldRed;
        green = max + min - oldGreen;
        blue = max + min - oldBlue;
        break;
    case NATURAL_TONE_PEARL:
        // Turn primary colors into pale, shaded ivory/silver. Retain a hint
        // of the original hue in accents and do not flatten every shade.
        gray = (oldRed * 3 + oldGreen * 4 + oldBlue) / 8;
        pearl = gray < 8 ? 8 + gray * 2 : 22 + (gray - 8) * 9 / 23;
        red = pearl + (oldRed - gray) / 8;
        green = pearl + (oldGreen - gray) / 8;
        blue = pearl + (oldBlue - gray) / 8;
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

    if (isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors))
        return;

    // Shiny palettes get their own fixed tone distribution and retain their
    // shiny flag, sparkle and encounter rate. Only their sprite colors change.
    tone = isShiny ? NaturalTone_FromShinyPersonality(personality)
                   : NaturalTone_FromPersonality(personality);
    if (tone == NATURAL_TONE_STANDARD)
        return;

    // Always start from the freshly loaded species palette; avoid repeated
    // tinting after a Pokémon swap or palette reuse.
    colors[0] = gPlttBufferUnfaded[paletteOffset];
    for (i = 1; i < ARRAY_COUNT(colors); i++)
        colors[i] = NaturalTone_AdjustColor(gPlttBufferUnfaded[paletteOffset + i], tone);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}
