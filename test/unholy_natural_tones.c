#include "global.h"
#include "unholy_natural_tones.h"
#include "test/test.h"

TEST("Natural Pokémon tones remain deterministic and independent of the game RNG")
{
    u32 personalities[] = {0, 1, 20, 0x12345678u, 0xFEDCBA98u};

    for (u32 i = 0; i < ARRAY_COUNT(personalities); i++)
    {
        u8 normal = NaturalTone_FromPersonality(personalities[i]);
        u8 shiny = NaturalTone_FromShinyPersonality(personalities[i]);
        EXPECT_LT(normal, NATURAL_TONE_COUNT);
        EXPECT_LT(shiny, NATURAL_TONE_COUNT);
        EXPECT_EQ(normal, NaturalTone_FromPersonality(personalities[i]));
        EXPECT_EQ(shiny, NaturalTone_FromShinyPersonality(personalities[i]));
    }
}

TEST("Natural Pokémon tones change 15-bit colors subtly and keep outlines")
{
    const u16 middle = (10 | (20 << 5) | (5 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(middle, NATURAL_TONE_STANDARD), middle);
    EXPECT_EQ(NaturalTone_AdjustColor(middle, NATURAL_TONE_WARM), 12 | (20 << 5) | (3 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(middle, NATURAL_TONE_COOL), 8 | (20 << 5) | (7 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(middle, NATURAL_TONE_LIGHT), 11 | (21 << 5) | (6 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(middle, NATURAL_TONE_DARK), 9 | (19 << 5) | (4 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(0, NATURAL_TONE_HUE_MINUS_90), 0);
    EXPECT_EQ(NaturalTone_AdjustColor(0x7FFF, NATURAL_TONE_HUE_PLUS_90), 0x7FFF);
    EXPECT_EQ(NaturalTone_AdjustColor(middle | 0x8000, NATURAL_TONE_LIGHT),
              NaturalTone_AdjustColor(middle, NATURAL_TONE_LIGHT) | 0x8000);
}

TEST("Hue rotation can turn red into violet while preserving main brightness")
{
    const u16 red = (26 | (3 << 5) | (2 << 10));
    const u16 gray = (16 | (16 << 5) | (16 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(red, NATURAL_TONE_HUE_MINUS_90),
              14 | (2 << 5) | (26 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(red, NATURAL_TONE_HUE_PLUS_60),
              26 | (26 << 5) | (2 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(gray, NATURAL_TONE_HUE_PLUS_90), gray);
}

TEST("Shiny Pokémon can receive chromatic hues without changing shiny status")
{
    bool32 foundViolet = FALSE;

    for (u32 personality = 0; personality < 1024; personality++)
    {
        if (NaturalTone_FromShinyPersonality(personality) == NATURAL_TONE_HUE_MINUS_90)
        {
            foundViolet = TRUE;
            break;
        }
    }
    EXPECT_EQ(foundViolet, TRUE);
}
