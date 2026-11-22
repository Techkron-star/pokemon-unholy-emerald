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
    EXPECT_EQ(NaturalTone_AdjustColor(0, NATURAL_TONE_PEARL), 0);
    EXPECT_EQ(NaturalTone_AdjustColor(0x7FFF, NATURAL_TONE_COMPLEMENT), 0x7FFF);
    EXPECT_EQ(NaturalTone_AdjustColor(middle | 0x8000, NATURAL_TONE_LIGHT),
              NaturalTone_AdjustColor(middle, NATURAL_TONE_LIGHT) | 0x8000);
}

TEST("Chromatic tones rotate and complement main colors while keeping grayscale")
{
    const u16 orange = (26 | (12 << 5) | (3 << 10));
    const u16 gray = (16 | (16 << 5) | (16 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(orange, NATURAL_TONE_SHIFT_GREEN), 3 | (26 << 5) | (12 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(orange, NATURAL_TONE_SHIFT_VIOLET), 12 | (3 << 5) | (26 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(orange, NATURAL_TONE_COMPLEMENT), 3 | (17 << 5) | (26 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(gray, NATURAL_TONE_COMPLEMENT), gray);
}

TEST("A shiny may have a pearl-white phenotype without losing its shiny status")
{
    const u16 bodyGray = (12 | (12 << 5) | (12 << 10));
    bool32 foundPearl = FALSE;

    for (u32 personality = 0; personality < 1024; personality++)
    {
        if (NaturalTone_FromShinyPersonality(personality) == NATURAL_TONE_PEARL)
        {
            foundPearl = TRUE;
            break;
        }
    }
    EXPECT_EQ(foundPearl, TRUE);
    EXPECT_EQ(NaturalTone_AdjustColor(bodyGray, NATURAL_TONE_PEARL),
              23 | (23 << 5) | (23 << 10));
}
