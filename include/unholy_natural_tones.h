#ifndef GUARD_UNHOLY_NATURAL_TONES_H
#define GUARD_UNHOLY_NATURAL_TONES_H

// Cosmetic hues derive from each Pokémon's permanent personality.
// Shiny Pokémon use their canonical shiny palette before color adjustment.
enum NaturalTone
{
    NATURAL_TONE_STANDARD,
    NATURAL_TONE_WARM,
    NATURAL_TONE_COOL,
    NATURAL_TONE_LIGHT,
    NATURAL_TONE_DARK,
    NATURAL_TONE_HUE_PLUS_30,
    NATURAL_TONE_HUE_MINUS_30,
    NATURAL_TONE_HUE_PLUS_60,
    NATURAL_TONE_HUE_MINUS_60,
    NATURAL_TONE_HUE_PLUS_90,
    NATURAL_TONE_HUE_MINUS_90,
    NATURAL_TONE_COMPLEMENT,
    NATURAL_TONE_COUNT,
};

u8 NaturalTone_FromPersonality(u32 personality);
u8 NaturalTone_FromShinyPersonality(u32 personality);
u16 NaturalTone_AdjustColor(u16 color, u8 tone);
void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, bool32 isShiny, bool32 isEgg);

#endif // GUARD_UNHOLY_NATURAL_TONES_H
