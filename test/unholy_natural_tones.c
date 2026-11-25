#include "global.h"
#include "pokemon.h"
#include "unholy_natural_tones.h"
#include "test/test.h"

TEST("Chromatic identity is an immutable code from both individual IDs")
{
    u64 identity = NaturalTone_GetIdentityCode(0x12345678u, 0x9ABCDEF0u);
    u64 otherPid = NaturalTone_GetIdentityCode(0x12345679u, 0x9ABCDEF0u);
    u64 otherOt = NaturalTone_GetIdentityCode(0x12345678u, 0x9ABCDEF1u);

    EXPECT_EQ(NATURAL_TONE_ALGORITHM_VERSION, 1);
    EXPECT_EQ((u32)identity, 4191506750u);
    EXPECT_EQ((u32)(identity >> 32), 4244140527u);
    EXPECT_EQ((u32)identity, (u32)NaturalTone_GetIdentityCode(0x12345678u, 0x9ABCDEF0u));
    EXPECT_EQ((u32)(identity >> 32), (u32)(NaturalTone_GetIdentityCode(0x12345678u, 0x9ABCDEF0u) >> 32));
    EXPECT_NE((u32)identity, (u32)otherPid);
    EXPECT_NE((u32)(identity >> 32), (u32)(otherOt >> 32));
}

TEST("Chromatic profile stays stable and depends on original trainer ID")
{
    struct NaturalToneProfile a, b, c;

    NaturalTone_GetProfile(0x12345678u, 0x9ABCDEF0u, FALSE, &a);
    NaturalTone_GetProfile(0x12345678u, 0x9ABCDEF0u, FALSE, &b);
    NaturalTone_GetProfile(0x12345678u, 0x9ABCDEF1u, FALSE, &c);

    EXPECT_EQ(a.microSeed, 2246944448u);
    EXPECT_EQ(a.mode, NATURAL_TONE_NEAR);
    EXPECT_EQ(a.hueShift, 299);
    EXPECT_EQ(a.saturationPercent, -2);
    EXPECT_EQ(a.brightnessSteps, 0);
    EXPECT_EQ(a.hueShift, b.hueShift);
    EXPECT_EQ(a.saturationPercent, b.saturationPercent);
    EXPECT_NE(a.hueShift, c.hueShift);
}

TEST("Chromatic hue rotates a red individual to violet and preserves grays")
{
    struct NaturalToneProfile profile = {0};
    const u16 red = 26 | (3 << 5) | (2 << 10);
    const u16 gray = 16 | (16 << 5) | (16 << 10);

    profile.mode = NATURAL_TONE_NEAR;
    profile.hueShift = -384;
    EXPECT_EQ(NaturalTone_AdjustColor(red, 1, &profile), 13 | (2 << 5) | (27 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(gray, 1, &profile), 15 | (15 << 5) | (15 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(gray, 2, &profile), 17 | (17 << 5) | (17 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(0, 1, &profile), 0);
    EXPECT_EQ(NaturalTone_AdjustColor(0x7FFF, 1, &profile), 0x7FFF);
}

TEST("Rare chromatic light inversion can make a dark shiny silver-white")
{
    struct NaturalToneProfile profile = {0};
    const u16 bodyGray = 12 | (12 << 5) | (12 << 10);
    const u16 outline = 2 | (2 << 5) | (2 << 10);
    bool32 found = FALSE;

    profile.mode = NATURAL_TONE_LIGHT_INVERSION;
    EXPECT_EQ(NaturalTone_AdjustColor(bodyGray, 1, &profile),
              21 | (21 << 5) | (21 << 10));
    EXPECT_EQ(NaturalTone_AdjustColor(outline, 1, &profile), outline);
    for (u32 pid = 0; pid < 1024; pid++)
    {
        NaturalTone_GetProfile(pid, 0x9ABCDEF0u, TRUE, &profile);
        if (profile.mode == NATURAL_TONE_LIGHT_INVERSION)
        {
            found = TRUE;
            break;
        }
    }
    EXPECT_EQ(found, TRUE);
}

TEST("Chromatic version is an independent persistent bit")
{
    struct BoxPokemon original = {0}, copy;
    const u8 yes = TRUE, no = FALSE;
    EXPECT_EQ(GetBoxMonData(&original, MON_DATA_CHROMATIC_V2), FALSE);
    SetBoxMonData(&original, MON_DATA_CHROMATIC_V2, &yes);
    EXPECT_EQ(GetBoxMonData(&original, MON_DATA_CHROMATIC_V2), TRUE);
    copy = original;
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_CHROMATIC_V2), TRUE);
    SetBoxMonData(&copy, MON_DATA_CHROMATIC_V2, &no);
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_CHROMATIC_V2), FALSE);
    EXPECT_EQ(GetBoxMonData(&original, MON_DATA_CHROMATIC_V2), TRUE);
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
}

TEST("V2 groups shade ramps and protects transparent and extreme colors")
{
    const u16 palette[16] = {0x1234, 0, 0x7FFF, 25 | (3 << 5) | (2 << 10),
        17 | (2 << 5) | (1 << 10), 2 | (26 << 5) | (3 << 10), 1 | (16 << 5) | (2 << 10),
        2 | (3 << 5) | (25 << 10), 1 | (2 << 5) | (16 << 10)};
    u16 a[16], b[16], c[16];
    u32 mask = NaturalTone_GetV2RoleMask(palette);
    bool32 different = FALSE;
    EXPECT_EQ((mask >> 6) & 3, (mask >> 8) & 3);
    EXPECT_EQ((mask >> 10) & 3, (mask >> 12) & 3);
    EXPECT_NE((mask >> 6) & 3, (mask >> 10) & 3);
    EXPECT_NE((mask >> 10) & 3, (mask >> 14) & 3);
    NaturalTone_BuildV2Palette(palette, a, 0x12345678u, 0x9ABCDEF0u, TRUE, 0);
    NaturalTone_BuildV2Palette(palette, b, 0x12345678u, 0x9ABCDEF0u, TRUE, 0);
    NaturalTone_BuildV2Palette(palette, c, 0x12345679u, 0x9ABCDEF0u, TRUE, 0);
    EXPECT_EQ(a[0], palette[0]);
    EXPECT_EQ(a[1], palette[1]);
    EXPECT_EQ(a[2], palette[2]);
    for (u32 i = 0; i < 16; i++)
    {
        EXPECT_EQ(a[i], b[i]);
        if (a[i] != c[i]) different = TRUE;
    }
    EXPECT_EQ(different, TRUE);
}
