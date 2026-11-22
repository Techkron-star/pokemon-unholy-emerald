#ifndef GUARD_UNHOLY_NATURAL_TONES_H
#define GUARD_UNHOLY_NATURAL_TONES_H

// Deterministic cosmetic palettes based on each Pokémon's personality.
// Shiny Pokémon use the original shiny species palette as their base.
enum NaturalTone
{
    NATURAL_TONE_STANDARD,
    NATURAL_TONE_WARM,
    NATURAL_TONE_COOL,
    NATURAL_TONE_LIGHT,
    NATURAL_TONE_DARK,
    NATURAL_TONE_SHIFT_GREEN,
    NATURAL_TONE_SHIFT_VIOLET,
    NATURAL_TONE_COMPLEMENT,
    NATURAL_TONE_PEARL,
    NATURAL_TONE_COUNT,
};

u8 NaturalTone_FromPersonality(u32 personality);
u8 NaturalTone_FromShinyPersonality(u32 personality);
u16 NaturalTone_AdjustColor(u16 color, u8 tone);
void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, bool32 isShiny, bool32 isEgg);

#endif // GUARD_UNHOLY_NATURAL_TONES_H
