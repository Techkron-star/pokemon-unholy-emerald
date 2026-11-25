#include "global.h"
#include "palette.h"
#include "unholy_natural_tones.h"

// VERSION 1 — immutable. The 64-bit code is a bijective encoding of the
// original-trainer ID and personality, not the species, level, nickname or
// current owner. Two identical (PID, OT ID) pairs, e.g. cloned Pokémon, are
// indistinguishable without adding a new persistent field to the save.
static u32 NaturalTone_Mix32(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352Du;
    value ^= value >> 15;
    value *= 0x846CA68Bu;
    value ^= value >> 16;
    return value;
}

u64 NaturalTone_GetIdentityCode(u32 personality, u32 otId)
{
    // Mix32 is a permutation; combining the two results never folds the
    // existing 64-bit identity into a 32-bit hash.
    u32 lower = NaturalTone_Mix32(personality ^ 0xC0A10B13u);
    u32 upper = NaturalTone_Mix32(otId ^ 0x726F6D31u);
    return ((u64)upper << 32) | lower;
}

void NaturalTone_GetProfile(u32 personality, u32 otId, bool32 isShiny, struct NaturalToneProfile *profile)
{
    u64 identity = NaturalTone_GetIdentityCode(personality, otId);
    u32 lower = identity;
    u32 upper = identity >> 32;
    u32 category = NaturalTone_Mix32(lower ^ (upper << 11 | upper >> 21) ^ 0x43485231u);
    u32 hue = NaturalTone_Mix32(upper ^ (lower >> 7 | lower << 25) ^ 0x68756531u);
    u32 detail = NaturalTone_Mix32(category ^ hue ^ 0x64657431u);
    u32 roll = category % 1000;

    profile->microSeed = lower ^ (upper << 7 | upper >> 25);

    // Continuous hue shifts are the default. Opposite-wheel hues are rare;
    // an even rarer luminance inversion enables dark shiny body -> white.
    if (roll < (isShiny ? 880 : 900))
    {
        profile->mode = NATURAL_TONE_NEAR;
        profile->hueShift = (s32)(hue % 641) - 320; // approx. ±75 degrees
    }
    else if (roll < (isShiny ? 975 : 990))
    {
        profile->mode = NATURAL_TONE_COMPLEMENT;
        profile->hueShift = 683 + hue % 171; // approx. 160..200 degrees
    }
    else
    {
        profile->mode = NATURAL_TONE_LIGHT_INVERSION;
        profile->hueShift = (s32)(hue % 129) - 64; // subtle colored accents
    }

    profile->saturationPercent = (s32)(detail % 13) - 6;
    profile->brightnessSteps = (s32)((detail >> 8) % 3) - 1;
}

// Rotate a saturated RGB555 color around the hue wheel without changing its
// darkest or lightest channel. The grayscale/shadows remain recognizable.
static u16 NaturalTone_ShiftHue(u16 color, s32 red, s32 green, s32 blue, s32 shift)
{
    s32 hi = max(red, max(green, blue));
    s32 lo = min(red, min(green, blue));
    s32 chroma = hi - lo;
    s32 hue, sector, fraction, cross;

    if (chroma < 4)
        return color;

    if (hi == red)
        hue = 256 * (green - blue) / chroma;
    else if (hi == green)
        hue = 512 + 256 * (blue - red) / chroma;
    else
        hue = 1024 + 256 * (red - green) / chroma;

    if (hue < 0)
        hue += 1536;
    hue += shift;
    if (hue < 0)
        hue += 1536;
    else if (hue >= 1536)
        hue -= 1536;

    sector = hue / 256;
    fraction = hue % 256;
    cross = chroma * fraction / 256;

    switch (sector)
    {
    case 0:
        red = hi; green = lo + cross; blue = lo;
        break;
    case 1:
        red = hi - cross; green = hi; blue = lo;
        break;
    case 2:
        red = lo; green = hi; blue = lo + cross;
        break;
    case 3:
        red = lo; green = hi - cross; blue = hi;
        break;
    case 4:
        red = lo + cross; green = lo; blue = hi;
        break;
    default:
        red = hi; green = lo; blue = hi - cross;
        break;
    }
    return (color & 0x8000) | red | (green << 5) | (blue << 10);
}

static u8 NaturalTone_ClampChannel(s32 value)
{
    if (value < 0)
        return 0;
    if (value > 31)
        return 31;
    return value;
}

static u16 NaturalTone_FineTuneColor(u16 original, u16 mapped, u32 paletteIndex, const struct NaturalToneProfile *profile)
{
    u32 fine = NaturalTone_Mix32(profile->microSeed ^ (paletteIndex * 0x9E3779B9u));
    s32 srcRed = original & 31;
    s32 srcGreen = (original >> 5) & 31;
    s32 srcBlue = (original >> 10) & 31;
    s32 chroma = max(srcRed, max(srcGreen, srcBlue)) - min(srcRed, min(srcGreen, srcBlue));
    s32 red = mapped & 31;
    s32 green = (mapped >> 5) & 31;
    s32 blue = (mapped >> 10) & 31;

    // One RGB555 step at most per palette shade. This avoids collapsing
    // hundreds of distinct 64-bit identities into identical visible palettes
    // without adding new random noise, using save bytes or damaging outlines.
    if (chroma < 4)
    {
        s32 delta = (s32)(fine % 3) - 1;
        red += delta;
        green += delta;
        blue += delta;
    }
    else
    {
        red += (s32)(fine % 3) - 1;
        blue += (s32)((fine >> 3) % 3) - 1;
    }

    return (mapped & 0x8000)
         | NaturalTone_ClampChannel(red)
         | (NaturalTone_ClampChannel(green) << 5)
         | (NaturalTone_ClampChannel(blue) << 10);
}

u16 NaturalTone_AdjustColor(u16 color, u32 paletteIndex, const struct NaturalToneProfile *profile)
{
    u16 original = color;
    s32 red = color & 31;
    s32 green = (color >> 5) & 31;
    s32 blue = (color >> 10) & 31;
    s32 avg, gray, result;

    // The transparent index, outlines and white highlights never change.
    if (paletteIndex == 0 || red + green + blue <= 6 || red + green + blue >= 87)
        return color;

    if (profile->mode == NATURAL_TONE_LIGHT_INVERSION
     && max(red, max(green, blue)) - min(red, min(green, blue)) <= 6)
    {
        gray = (red * 3 + green * 4 + blue) / 8;
        if (gray <= 5)
            return color;
        result = 17 + gray * 14 / 31;
        color = (color & 0x8000) | result | (result << 5) | (result << 10);
    }
    else
    {
        color = NaturalTone_ShiftHue(color, red, green, blue, profile->hueShift);
        red = color & 31;
        green = (color >> 5) & 31;
        blue = (color >> 10) & 31;

        avg = (red + green + blue) / 3;
        red = avg + (red - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;
        green = avg + (green - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;
        blue = avg + (blue - avg) * (100 + profile->saturationPercent) / 100 + profile->brightnessSteps;

        color = (color & 0x8000)
              | NaturalTone_ClampChannel(red)
              | (NaturalTone_ClampChannel(green) << 5)
              | (NaturalTone_ClampChannel(blue) << 10);
    }

    return NaturalTone_FineTuneColor(original, color, paletteIndex, profile);
}

void NaturalTone_ApplyLoadedPalette(u32 paletteOffset, u32 personality, u32 otId, bool32 isShiny, bool32 isEgg)
{
    struct NaturalToneProfile profile;
    u16 colors[16];
    u32 i;

    if (isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors))
        return;

    NaturalTone_GetProfile(personality, otId, isShiny, &profile);

    // Recompute only from the canonical normal/shiny palette, never from a
    // previously tinted buffer. Do not alter the transparency entry.
    colors[0] = gPlttBufferUnfaded[paletteOffset];
    for (i = 1; i < ARRAY_COUNT(colors); i++)
        colors[i] = NaturalTone_AdjustColor(gPlttBufferUnfaded[paletteOffset + i], i, &profile);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}

// V2 is independent of V1's frozen identity and palette vectors.
static s32 V2_Wrap(s32 hue)
{
    while (hue < -768) hue += 1536;
    while (hue > 768) hue -= 1536;
    return hue;
}
static s32 V2_Distance(s32 a, s32 b)
{
    s32 d = V2_Wrap(a - b);
    return d < 0 ? -d : d;
}
static s32 V2_Chroma(u16 c)
{
    s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    return max(r, max(g, b)) - min(r, min(g, b));
}
static s32 V2_Hue(u16 c)
{
    s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    s32 hi = max(r, max(g, b)), ch = V2_Chroma(c);
    if (!ch) return 0;
    if (hi == r) return V2_Wrap(256 * (g - b) / ch);
    if (hi == g) return V2_Wrap(512 + 256 * (b - r) / ch);
    return V2_Wrap(1024 + 256 * (r - g) / ch);
}
static bool32 V2_Protected(u16 c)
{
    s32 sum = (c & 31) + ((c >> 5) & 31) + ((c >> 10) & 31);
    return sum <= 6 || sum >= 87;
}
void NaturalTone_GetV2Profile(u32 personality, u32 otId, bool32 isShiny, struct NaturalToneV2Profile *profile)
{
    u64 identity = NaturalTone_GetIdentityCode(personality, otId);
    u32 lo = identity, hi = identity >> 32;
    u32 seed = NaturalTone_Mix32(lo ^ (hi << 13 | hi >> 19) ^ 0x56324348u);
    u32 harmony = NaturalTone_Mix32(seed ^ 0x4841524Du);
    u32 finish = NaturalTone_Mix32(seed ^ 0x46494E32u) % 10000;
    u32 scope = NaturalTone_Mix32(seed ^ 0x53434F50u) % 100;
    u32 order = NaturalTone_Mix32(seed ^ 0x524F4C45u);
    u32 j;
    u8 temp;
    s32 shift = (s32)(harmony % 513) - 256;
    s32 direction = (harmony & 0x10000u) ? 1 : -1;
    static const s16 offsets[5][2] = {{128,-160},{768,128},{512,-512},{640,-640},{64,-256}};
    u32 scheme = (harmony >> 10) % 5;
    u32 i;
    if ((harmony >> 20) % 100 < (isShiny ? 25 : 15))
        shift = (s32)(NaturalTone_Mix32(seed ^ 0x48554532u) % 1536) - 768;
    profile->hue[0] = shift;
    profile->hue[1] = V2_Wrap(shift + direction * offsets[scheme][0]);
    profile->hue[2] = V2_Wrap(shift + direction * offsets[scheme][1]);
    for (i = 0; i < 3; i++)
    {
        u32 detail = NaturalTone_Mix32(seed ^ ((i + 1) * 0x9E3779B9u));
        profile->hue[i] = V2_Wrap(profile->hue[i] + (s32)(detail % 65) - 32);
        profile->saturation[i] = (s32)((detail >> 8) % 25) - 12;
        profile->brightness[i] = (s32)((detail >> 16) % 5) - 2;
    }
    if (finish < 5129) profile->finish = NATURAL_FINISH_NATURAL;
    else if (finish < 6329) profile->finish = NATURAL_FINISH_PASTEL;
    else if (finish < 7029) profile->finish = NATURAL_FINISH_COPPER;
    else if (finish < 7529) profile->finish = NATURAL_FINISH_OBSIDIAN;
    else if (finish < 7929) profile->finish = NATURAL_FINISH_PORCELAIN;
    else if (finish < 8129) profile->finish = NATURAL_FINISH_MELANISM;
    else if (finish < 8229) profile->finish = NATURAL_FINISH_SOLAR;
    else if (finish < 8249) profile->finish = NATURAL_FINISH_MAGNETAR;
    else if (finish < 8259) profile->finish = NATURAL_FINISH_QUASAR;
    else if (finish < 8309) profile->finish = NATURAL_FINISH_COSMIC;
    else if (finish < 8459) profile->finish = NATURAL_FINISH_GHOSTLY;
    else if (finish < 8509) profile->finish = NATURAL_FINISH_INFERNAL;
    else if (finish < 8510) profile->finish = NATURAL_FINISH_GODLY;
    else if (finish < 8810) profile->finish = NATURAL_FINISH_AQUA;
    else if (finish < 9110) profile->finish = NATURAL_FINISH_JUNGLE;
    else if (finish < 9310) profile->finish = NATURAL_FINISH_NIGHTLY;
    else if (finish < 9340) profile->finish = NATURAL_FINISH_ALBINISM;
    else if (finish < 9440) profile->finish = NATURAL_FINISH_SPECTRAL;
    else if (finish < 9540) profile->finish = NATURAL_FINISH_GOLDEN;
    else if (finish < 9580) profile->finish = NATURAL_FINISH_IRIDESCENT;
    else if (finish < 9700) profile->finish = NATURAL_FINISH_CRIMSON;
    else if (finish < 9800) profile->finish = NATURAL_FINISH_GLACIAL;
    else if (finish < 9900) profile->finish = NATURAL_FINISH_TOXIC;
    else profile->finish = NATURAL_FINISH_LUNAR;
    // 40% one zone, 30% two, 15% all available zones, 10% global,
    // 5% accent-first one/two zones. Finishes are independent of scope.
    profile->scope = scope < 40 ? 1 : scope < 70 ? 2 : scope < 85 ? 3
        : scope < 95 ? 4 : 1 + ((order >> 16) & 1);
    for (i = 0; i < 3; i++) profile->roleOrder[i] = i;
    for (i = 2; i > 0; i--)
    {
        j = order % (i + 1);
        temp = profile->roleOrder[i];
        profile->roleOrder[i] = profile->roleOrder[j];
        profile->roleOrder[j] = temp;
        order >>= 8;
    }
    if (scope >= 95)
    {
        // Prefer accent (role 2) but fall back to another available role.
        for (i = 0; i < 3; i++)
            if (profile->roleOrder[i] == 2)
            {
                temp = profile->roleOrder[0];
                profile->roleOrder[0] = 2;
                profile->roleOrder[i] = temp;
                break;
            }
    }
}
u32 NaturalTone_GetV2RoleMask(const u16 *base)
{
    s32 anchor[3] = {0};
    u32 count = 0, mask = 0, i, j;
    // Select separated hue anchors; keep shade ramps in a single role.
    for (j = 0; j < 3; j++)
    {
        s32 best = 0, score = -1;
        for (i = 1; i < 16; i++)
        {
            s32 ch = V2_Chroma(base[i]), hue = V2_Hue(base[i]), sep = 768;
            u32 k;
            if (V2_Protected(base[i]) || ch < 4) continue;
            for (k = 0; k < count; k++) sep = min(sep, V2_Distance(hue, anchor[k]));
            if (count && sep < 160) continue;
            if (ch * (count ? sep : 256) > score)
            {
                best = hue;
                score = ch * (count ? sep : 256);
            }
        }
        if (score < 0) break;
        anchor[count++] = best;
    }
    for (i = 1; i < 16; i++)
    {
        u32 role = 1;
        s32 dist = 769, hue = V2_Hue(base[i]);
        if (V2_Protected(base[i])) continue;
        if (V2_Chroma(base[i]) >= 4)
            for (j = 0; j < count; j++)
            {
                s32 d = V2_Distance(hue, anchor[j]);
                if (d < dist) { dist = d; role = j + 1; }
            }
        mask |= role << (2 * i);
    }
    return mask;
}
// Candidate semantic masks derived from repository palettes. Exact signatures
// keep alternate graphics and unrelated species on the automatic fallback.
// Mask 0 entries remain untouched for curated partial and global modes.
static const struct
{
    u16 colors[15]; // Skip index zero, which can differ at runtime.
    u32 roles;
} sCuratedV2Palettes[] =
{
    // zigzagoon/normal.pal
    { { 0x2d8e, 0x0000, 0x4eb7, 0x677c, 0x4676, 0x35f2, 0x254b, 0x2108, 0x7fff, 0x190a, 0x194d, 0x1484, 0x5f18, 0x0000, 0x6b9e }, 0x48f07d44u },
    // zigzagoon/shiny.pal
    { { 0x256d, 0x0842, 0x4297, 0x63bf, 0x2a3d, 0x21b9, 0x256e, 0x2108, 0x7fff, 0x1154, 0x21b9, 0x1484, 0x5f18, 0x0000, 0x677c }, 0x48f07d44u },
    // zigzagoon/normal_gba.pal
    { { 0x1509, 0x2df5, 0x3e79, 0x4efd, 0x63bf, 0x18e8, 0x256d, 0x46b8, 0x575c, 0x7fff, 0x0094, 0x10f9, 0x3e54, 0x256f, 0x0002 }, 0x168557f4u },
    // zigzagoon/shiny_gba.pal
    { { 0x1509, 0x2198, 0x261b, 0x269d, 0x63bf, 0x14c6, 0x14ec, 0x46b8, 0x575c, 0x7fff, 0x0094, 0x10f9, 0x3e54, 0x1d50, 0x0002 }, 0x168557f4u },
    // alakazam/normal.pal
    { { 0x7fff, 0x6f7b, 0x5ad6, 0x2d6b, 0x2ada, 0x1616, 0x056f, 0x04ca, 0x054d, 0x01d3, 0x57df, 0x0fbf, 0x0ad9, 0x056e, 0x0842 }, 0x156aabf0u },
    // alakazam/shiny.pal
    { { 0x7fff, 0x6f7b, 0x5ad6, 0x2d6b, 0x725f, 0x553b, 0x38b5, 0x284e, 0x050d, 0x01b3, 0x43bf, 0x035f, 0x065a, 0x01b4, 0x0842 }, 0x156aabf0u },
    // alakazam/normal_gba.pal
    { { 0x7fff, 0x6f7b, 0x5ad6, 0x2d6b, 0x2ada, 0x1615, 0x0991, 0x00cc, 0x7c1f, 0x7c1f, 0x3fff, 0x17bf, 0x02da, 0x0191, 0x0842 }, 0x1542abf0u },
    // alakazam/shiny_gba.pal
    { { 0x7fff, 0x6f7b, 0x5ad6, 0x2d6b, 0x5dfc, 0x4957, 0x34b0, 0x140a, 0x7c1f, 0x7c1f, 0x273f, 0x02ba, 0x01d5, 0x008f, 0x0842 }, 0x1542abf0u },
    // rayquaza/mega/normal.pal
    { { 0x0e5b, 0x235d, 0x03de, 0x0d0c, 0x127c, 0x3a09, 0x0000, 0x114e, 0x46eb, 0x2145, 0x2f5d, 0x24e7, 0x5b72, 0x24ff, 0x1cce }, 0xf4961aa8u },
    // rayquaza/mega/shiny.pal
    { { 0x0e5b, 0x235d, 0x03de, 0x1050, 0x1875, 0x2528, 0x0000, 0x114e, 0x35ac, 0x18c6, 0x28ff, 0x18a5, 0x4a51, 0x24ff, 0x1cce }, 0xf4d61fa8u },
};

u32 NaturalTone_GetCuratedRoleMask(const u16 *base)
{
    u32 i, j;
    for (i = 0; i < ARRAY_COUNT(sCuratedV2Palettes); i++)
    {
        for (j = 1; j < 16; j++)
            if ((base[j] & 0x7FFF) != sCuratedV2Palettes[i].colors[j - 1])
                break;
        if (j == 16) return sCuratedV2Palettes[i].roles;
    }
    return 0;
}

u8 NaturalTone_SelectV2Roles(u32 packedRoles, const struct NaturalToneV2Profile *profile)
{
    u8 available = 0, selected = 0;
    u32 i, remaining = profile->scope;
    for (i = 1; i < 16; i++)
    {
        u32 role = (packedRoles >> (2 * i)) & 3;
        if (role) available |= 1u << (role - 1);
    }
    if (remaining >= 3) return available;
    for (i = 0; i < 3 && remaining; i++)
    {
        u8 bit;
        if (profile->roleOrder[i] >= 3) continue;
        bit = 1u << profile->roleOrder[i];
        if ((available & bit) && !(selected & bit))
        {
            selected |= bit;
            remaining--;
        }
    }
    return selected;
}

// Themed targets: each finish has a body, secondary and accent target.
// Scale to the source value before blending so gradients remain visible.
static const u16 sV2ThemeColors[][3] =
{
    { (31 | (18 << 5) | (2 << 10)), (31 | (7 << 5) | (1 << 10)), (31 | (28 << 5) | (12 << 10)) }, // SOLAR
    { (10 | (5 << 5) | (31 << 10)), (29 | (3 << 5) | (26 << 10)), (5 | (17 << 5) | (31 << 10)) }, // MAGNETAR
    { (15 | (27 << 5) | (31 << 10)), (4 | (12 << 5) | (31 << 10)), (30 | (31 << 5) | (31 << 10)) }, // QUASAR
    { (10 | (5 << 5) | (23 << 10)), (22 | (8 << 5) | (28 << 10)), (31 | (17 << 5) | (25 << 10)) }, // COSMIC
    { (14 | (23 << 5) | (21 << 10)), (8 | (17 << 5) | (18 << 10)), (25 | (31 << 5) | (28 << 10)) }, // GHOSTLY
    { (24 | (3 << 5) | (3 << 10)), (31 | (11 << 5) | (1 << 10)), (12 | (3 << 5) | (4 << 10)) }, // INFERNAL
    { (31 | (30 << 5) | (24 << 10)), (31 | (23 << 5) | (5 << 10)), (31 | (31 << 5) | (31 << 10)) }, // GODLY
    { (3 | (15 << 5) | (28 << 10)), (2 | (25 << 5) | (24 << 10)), (17 | (31 << 5) | (28 << 10)) }, // AQUA
    { (8 | (20 << 5) | (4 << 10)), (17 | (23 << 5) | (6 << 10)), (20 | (13 << 5) | (6 << 10)) }, // JUNGLE
    { (5 | (7 << 5) | (19 << 10)), (12 | (14 << 5) | (23 << 10)), (22 | (25 << 5) | (30 << 10)) }, // NIGHTLY
    { (31 | (29 << 5) | (25 << 10)), (30 | (24 << 5) | (23 << 10)), (28 | (8 << 5) | (12 << 10)) }, // ALBINISM
    { (13 | (10 << 5) | (28 << 10)), (7 | (18 << 5) | (29 << 10)), (21 | (24 << 5) | (31 << 10)) }, // SPECTRAL
    { (31 | (22 << 5) | (4 << 10)), (23 | (14 << 5) | (3 << 10)), (31 | (29 << 5) | (14 << 10)) }, // GOLDEN
    { (24 | (7 << 5) | (30 << 10)), (4 | (22 << 5) | (31 << 10)), (8 | (29 << 5) | (18 << 10)) }, // IRIDESCENT
    { (27 | (3 << 5) | (8 << 10)), (18 | (2 << 5) | (6 << 10)), (31 | (13 << 5) | (14 << 10)) }, // CRIMSON
    { (19 | (27 << 5) | (31 << 10)), (8 | (23 << 5) | (30 << 10)), (29 | (31 << 5) | (31 << 10)) }, // GLACIAL
    { (18 | (29 << 5) | (3 << 10)), (20 | (4 << 5) | (26 << 10)), (28 | (31 << 5) | (8 << 10)) }, // TOXIC
    { (16 | (20 << 5) | (30 << 10)), (8 | (10 << 5) | (22 << 10)), (28 | (29 << 5) | (31 << 10)) }, // LUNAR
};

static u16 V2_ThemeColor(u16 mapped, u32 role, const struct NaturalToneV2Profile *p)
{
    u16 target = sV2ThemeColors[p->finish - NATURAL_FINISH_SOLAR][role];
    s32 r = mapped & 31, g = (mapped >> 5) & 31, b = (mapped >> 10) & 31;
    s32 tr = target & 31, tg = (target >> 5) & 31, tb = (target >> 10) & 31;
    s32 hi = max(r, max(g, b)), targetHi = max(tr, max(tg, tb));
    s32 strength = 76 + p->saturation[role] / 2;
    if (p->finish == NATURAL_FINISH_GODLY || p->finish == NATURAL_FINISH_ALBINISM
     || p->finish == NATURAL_FINISH_QUASAR)
        hi += (31 - hi) / 3;
    else if (p->finish == NATURAL_FINISH_NIGHTLY || p->finish == NATURAL_FINISH_COSMIC
          || p->finish == NATURAL_FINISH_INFERNAL)
        hi = hi * 85 / 100;
    tr = tr * hi / targetHi; tg = tg * hi / targetHi; tb = tb * hi / targetHi;
    r = (r * (100 - strength) + tr * strength) / 100 + p->brightness[role];
    g = (g * (100 - strength) + tg * strength) / 100 + p->brightness[role];
    b = (b * (100 - strength) + tb * strength) / 100 + p->brightness[role];
    return (mapped & 0x8000) | NaturalTone_ClampChannel(r)
        | (NaturalTone_ClampChannel(g) << 5) | (NaturalTone_ClampChannel(b) << 10);
}

static u16 V2_Adjust(u16 c, u32 role, const struct NaturalToneV2Profile *p)
{
    s32 r, g, b, gray, sat, light, value;
    u16 mapped;
    if (V2_Protected(c)) return c;
    r = c & 31; g = (c >> 5) & 31; b = (c >> 10) & 31;
    mapped = NaturalTone_ShiftHue(c, r, g, b, p->hue[role]);
    if (p->finish >= NATURAL_FINISH_SOLAR && p->finish < NATURAL_FINISH_COUNT)
        return V2_ThemeColor(mapped, role, p);
    // A selected neutral material can acquire a tint too (e.g. silver
    // spoons). Keep the neutral ramp's value and protected endpoints.
    if (V2_Chroma(c) < 4 && p->finish == NATURAL_FINISH_NATURAL)
    {
        s32 v = max(r, max(g, b)), low = v * 3 / 5;
        u16 seedColor = v | (low << 5) | (low << 10);
        if (v - low >= 4)
            mapped = NaturalTone_ShiftHue(seedColor, v, low, low, p->hue[role]);
    }
    r = mapped & 31; g = (mapped >> 5) & 31; b = (mapped >> 10) & 31;
    gray = (r * 3 + g * 4 + b) / 8;
    sat = 100 + p->saturation[role]; light = p->brightness[role];
    switch (p->finish)
    {
    case NATURAL_FINISH_PASTEL: sat = sat * 62 / 100; light += 3; break;
    case NATURAL_FINISH_COPPER: sat = sat * 80 / 100; break;
    case NATURAL_FINISH_OBSIDIAN: sat = sat * 65 / 100; break;
    case NATURAL_FINISH_PORCELAIN: sat = sat * 45 / 100; break;
    case NATURAL_FINISH_MELANISM: sat = sat * 35 / 100; break;
    default: break;
    }
    r = gray + (r - gray) * sat / 100;
    g = gray + (g - gray) * sat / 100;
    b = gray + (b - gray) * sat / 100;
    if (p->finish == NATURAL_FINISH_COPPER && role == 0)
    {
        r = (r * 3 + min(31, gray + 8)) / 4;
        g = (g * 3 + gray) / 4;
        b = (b * 3 + max(0, gray - 7)) / 4;
    }
    else if (p->finish == NATURAL_FINISH_OBSIDIAN || p->finish == NATURAL_FINISH_MELANISM)
    {
        value = role == 0 ? (p->finish == NATURAL_FINISH_MELANISM ? 55 : 68) : 86;
        r = 2 + r * value / 100; g = 2 + g * value / 100; b = 2 + b * value / 100;
    }
    else if (p->finish == NATURAL_FINISH_PORCELAIN)
    {
        value = role == 0 ? 45 : 20;
        r += (31-r) * value / 100; g += (31-g) * value / 100; b += (31-b) * value / 100;
    }
    return (c & 0x8000) | NaturalTone_ClampChannel(r + light)
        | (NaturalTone_ClampChannel(g + light) << 5) | (NaturalTone_ClampChannel(b + light) << 10);
}
void NaturalTone_BuildV2Palette(const u16 *base, u16 *out, u32 personality, u32 otId, bool32 isShiny, u32 overrides)
{
    struct NaturalToneV2Profile p;
    u32 roles = NaturalTone_GetCuratedRoleMask(base), i;
    u8 selected;
    if (!roles) roles = NaturalTone_GetV2RoleMask(base);
    // Explicit overrides replace individual role assignments; zero means
    // retain inferred or curated role. Protected colors are always preserved.
    for (i = 1; i < 16; i++)
    {
        u32 role = (overrides >> (2 * i)) & 3;
        if (role) roles = (roles & ~(3u << (2 * i))) | (role << (2 * i));
        if (V2_Protected(base[i])) roles &= ~(3u << (2 * i));
    }
    NaturalTone_GetV2Profile(personality, otId, isShiny, &p);
    selected = NaturalTone_SelectV2Roles(roles, &p);
    out[0] = base[0];
    for (i = 1; i < 16; i++)
    {
        u32 role = (roles >> (2 * i)) & 3;
        out[i] = role && (selected & (1u << (role - 1)))
            ? V2_Adjust(base[i], role - 1, &p) : base[i];
    }
}
void NaturalTone_ApplyLoadedPaletteVersion(u32 paletteOffset, u32 personality, u32 otId, bool32 isShiny, bool32 isEgg, bool32 isV2)
{
    u16 colors[16];
    if (!isV2)
    {
        NaturalTone_ApplyLoadedPalette(paletteOffset, personality, otId, isShiny, isEgg);
        return;
    }
    if (isEgg || paletteOffset > PLTT_BUFFER_SIZE - ARRAY_COUNT(colors)) return;
    NaturalTone_BuildV2Palette(&gPlttBufferUnfaded[paletteOffset], colors, personality, otId, isShiny, 0);
    LoadPalette(colors, paletteOffset, sizeof(colors));
}
