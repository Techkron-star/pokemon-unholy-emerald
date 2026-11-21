#ifndef GUARD_UNHOLY_NATURAL_TONES_H
#define GUARD_UNHOLY_NATURAL_TONES_H

// A small, deterministic color variation based on a Pokémon's personality.
// Natural tones never replace the canonical shiny or egg palettes.
enum NaturalTone
{
    NATURAL_TONE_STANDARD,
    NATURAL_TONE_WARM,
    NATURAL_TONE_COOL,
    NATURAL_TONE_LIGHT,
    NATURAL_TONE_DARK,
};

u8 NaturalTone_FromPersonality(u32 personality);
u16 NaturalTone_AdjustColor(u16 color, u8 tone);
void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, bool32 isShiny, bool32 isEgg);

#endif // GUARD_UNHOLY_NATURAL_TONES_H
