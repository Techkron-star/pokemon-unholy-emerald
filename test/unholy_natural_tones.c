#include "global.h"
#include "unholy_natural_tones.h"
#include "test/test.h"

TEST("Natural Pokémon tones are deterministic and independent of the game RNG")
{
    u32 personalities[] = {0, 1, 20, 0x12345678u, 0xFEDCBA98u};

    for (u32 i = 0; i < ARRAY_COUNT(personalities); i++)
    {
        u8 tone = NaturalTone_FromPersonality(personalities[i]);
        EXPECT_LT(tone, 5);
        EXPECT_EQ(tone, NaturalTone_FromPersonality(personalities[i]));
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
    EXPECT_EQ(NaturalTone_AdjustColor(0, NATURAL_TONE_WARM), 0);
    EXPECT_EQ(NaturalTone_AdjustColor(0x7FFF, NATURAL_TONE_DARK), 0x7FFF);
    EXPECT_EQ(NaturalTone_AdjustColor(middle | 0x8000, NATURAL_TONE_LIGHT),
              NaturalTone_AdjustColor(middle, NATURAL_TONE_LIGHT) | 0x8000);
}
