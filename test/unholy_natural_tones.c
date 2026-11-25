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

TEST("V2 chooses only available regions in stable priority order")
{
    struct NaturalToneV2Profile p = {0};
    u32 roles = (1u << 2) | (2u << 4) | (3u << 6);
    p.roleOrder[0] = 2; p.roleOrder[1] = 0; p.roleOrder[2] = 1;
    p.scope = 1;
    EXPECT_EQ(NaturalTone_SelectV2Roles(roles, &p), 4);
    EXPECT_EQ(NaturalTone_SelectV2Roles(roles & ~(3u << 6), &p), 1);
    p.scope = 2;
    EXPECT_EQ(NaturalTone_SelectV2Roles(roles, &p), 5);
    p.scope = 3;
    EXPECT_EQ(NaturalTone_SelectV2Roles(roles, &p), 7);
    p.scope = 4;
    EXPECT_EQ(NaturalTone_SelectV2Roles(roles, &p), 7);
}

TEST("V2 partial variants preserve unselected palette regions exactly")
{
    const u16 base[16] = {0x1234, 0, 0x7FFF, 25 | (3 << 5) | (2 << 10),
        17 | (2 << 5) | (1 << 10), 2 | (26 << 5) | (3 << 10),
        1 | (16 << 5) | (2 << 10), 2 | (3 << 5) | (25 << 10),
        1 | (2 << 5) | (16 << 10)};
    u16 out[16];
    u32 pid, i, roles = NaturalTone_GetV2RoleMask(base);
    bool32 found = FALSE;
    struct NaturalToneV2Profile p;
    for (pid = 0; pid < 128; pid++)
    {
        u8 selected;
        NaturalTone_GetV2Profile(pid, 0x9ABCDEF0u, FALSE, &p);
        if (p.scope != 1) continue;
        found = TRUE;
        selected = NaturalTone_SelectV2Roles(roles, &p);
        NaturalTone_BuildV2Palette(base, out, pid, 0x9ABCDEF0u, FALSE, 0);
        for (i = 0; i < 16; i++)
        {
            u32 role = (roles >> (2 * i)) & 3;
            if (!role || !(selected & (1u << (role - 1))))
                EXPECT_EQ(out[i], base[i]);
        }
    }
    EXPECT_EQ(found, TRUE);
}

TEST("Alakazam canonical palette identifies silver accent separately")
{
    u16 base[16] = {0, 0x7fff, 0x6f7b, 0x5ad6, 0x2d6b, 0x2ada, 0x1616, 0x56f, 0x4ca, 0x54d, 0x1d3, 0x57df, 0xfbf, 0xad9, 0x56e, 0x842};
    u32 mask = NaturalTone_GetCuratedRoleMask(base);
    EXPECT_EQ(mask, 0x156aabf0u);
    EXPECT_EQ((mask >> 4) & 3, 3);
    EXPECT_EQ((mask >> 24) & 3, 1);
    base[12] ^= 1;
    EXPECT_EQ(NaturalTone_GetCuratedRoleMask(base), 0);
}

TEST("V2 recolor is alias-safe and keeps ordered selected shade ramps")
{
    const u16 base[16] = {0x1234, 0, 0x7FFF,
        8 | (3 << 5) | (2 << 10), 16 | (6 << 5) | (4 << 10),
        24 | (9 << 5) | (6 << 10)};
    u16 a[16], b[16];
    u32 pid, i;
    // Force the three shades into one region, independent of fallback.
    const u32 roles = (1u << 6) | (1u << 8) | (1u << 10);
    for (pid = 0; pid < 128; pid++)
    {
        for (i = 0; i < 16; i++) b[i] = base[i];
        NaturalTone_BuildV2Palette(base, a, pid, 0x9ABCDEF0u, FALSE, roles);
        NaturalTone_BuildV2Palette(b, b, pid, 0x9ABCDEF0u, FALSE, roles);
        for (i = 0; i < 16; i++) EXPECT_EQ(a[i], b[i]);
        for (i = 3; i < 5; i++)
        {
            u32 x = max(a[i] & 31, max((a[i] >> 5) & 31, (a[i] >> 10) & 31));
            u32 y = max(a[i+1] & 31, max((a[i+1] >> 5) & 31, (a[i+1] >> 10) & 31));
            EXPECT_EQ(x <= y, TRUE);
        }
    }
}
