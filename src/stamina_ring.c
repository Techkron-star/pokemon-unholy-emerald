#include "global.h"
#include "config/stamina.h"
#include "stamina_ring.h"

// Clockwise from 12 o'clock. Five concentric radii (9..13px), precomputed
// for the GBA: no floating-point math, trigonometry or per-frame allocations.
// Each 32x32 sprite renders a Zelda-style three-pixel-thick hollow gauge.
#define STAMINA_RING_TICKS STAMINA_RING_SEGMENTS
static const s8 sRingPoint[STAMINA_RING_TICKS][10] =
{
{   0,  -9,   0, -10,   0, -11,   0, -12,   0, -13 },
{   1,  -9,   1, -10,   1, -11,   1, -12,   1, -13 },
{   2,  -9,   2, -10,   2, -11,   2, -12,   3, -13 },
{   3,  -9,   3, -10,   3, -11,   3, -11,   4, -12 },
{   3,  -8,   4,  -9,   4, -10,   5, -11,   5, -12 },
{   4,  -8,   5,  -9,   5, -10,   6, -11,   6, -11 },
{   5,  -7,   6,  -8,   6,  -9,   7, -10,   7, -11 },
{   6,  -7,   6,  -8,   7,  -9,   8,  -9,   8, -10 },
{   6,  -6,   7,  -7,   8,  -8,   8,  -8,   9,  -9 },
{   7,  -6,   8,  -6,   9,  -7,   9,  -8,  10,  -8 },
{   7,  -5,   8,  -6,   9,  -6,  10,  -7,  11,  -7 },
{   8,  -4,   9,  -5,  10,  -5,  11,  -6,  11,  -6 },
{   8,  -3,   9,  -4,  10,  -4,  11,  -5,  12,  -5 },
{   9,  -3,  10,  -3,  11,  -3,  11,  -3,  12,  -4 },
{   9,  -2,  10,  -2,  11,  -2,  12,  -2,  13,  -3 },
{   9,  -1,  10,  -1,  11,  -1,  12,  -1,  13,  -1 },
{   9,   0,  10,   0,  11,   0,  12,   0,  13,   0 },
{   9,   1,  10,   1,  11,   1,  12,   1,  13,   1 },
{   9,   2,  10,   2,  11,   2,  12,   2,  13,   3 },
{   9,   3,  10,   3,  11,   3,  11,   3,  12,   4 },
{   8,   3,   9,   4,  10,   4,  11,   5,  12,   5 },
{   8,   4,   9,   5,  10,   5,  11,   6,  11,   6 },
{   7,   5,   8,   6,   9,   6,  10,   7,  11,   7 },
{   7,   6,   8,   6,   9,   7,   9,   8,  10,   8 },
{   6,   6,   7,   7,   8,   8,   8,   8,   9,   9 },
{   6,   7,   6,   8,   7,   9,   8,   9,   8,  10 },
{   5,   7,   6,   8,   6,   9,   7,  10,   7,  11 },
{   4,   8,   5,   9,   5,  10,   6,  11,   6,  11 },
{   3,   8,   4,   9,   4,  10,   5,  11,   5,  12 },
{   3,   9,   3,  10,   3,  11,   3,  11,   4,  12 },
{   2,   9,   2,  10,   2,  11,   2,  12,   3,  13 },
{   1,   9,   1,  10,   1,  11,   1,  12,   1,  13 },
{   0,   9,   0,  10,   0,  11,   0,  12,   0,  13 },
{  -1,   9,  -1,  10,  -1,  11,  -1,  12,  -1,  13 },
{  -2,   9,  -2,  10,  -2,  11,  -2,  12,  -3,  13 },
{  -3,   9,  -3,  10,  -3,  11,  -3,  11,  -4,  12 },
{  -3,   8,  -4,   9,  -4,  10,  -5,  11,  -5,  12 },
{  -4,   8,  -5,   9,  -5,  10,  -6,  11,  -6,  11 },
{  -5,   7,  -6,   8,  -6,   9,  -7,  10,  -7,  11 },
{  -6,   7,  -6,   8,  -7,   9,  -8,   9,  -8,  10 },
{  -6,   6,  -7,   7,  -8,   8,  -8,   8,  -9,   9 },
{  -7,   6,  -8,   6,  -9,   7,  -9,   8, -10,   8 },
{  -7,   5,  -8,   6,  -9,   6, -10,   7, -11,   7 },
{  -8,   4,  -9,   5, -10,   5, -11,   6, -11,   6 },
{  -8,   3,  -9,   4, -10,   4, -11,   5, -12,   5 },
{  -9,   3, -10,   3, -11,   3, -11,   3, -12,   4 },
{  -9,   2, -10,   2, -11,   2, -12,   2, -13,   3 },
{  -9,   1, -10,   1, -11,   1, -12,   1, -13,   1 },
{  -9,   0, -10,   0, -11,   0, -12,   0, -13,   0 },
{  -9,  -1, -10,  -1, -11,  -1, -12,  -1, -13,  -1 },
{  -9,  -2, -10,  -2, -11,  -2, -12,  -2, -13,  -3 },
{  -9,  -3, -10,  -3, -11,  -3, -11,  -3, -12,  -4 },
{  -8,  -3,  -9,  -4, -10,  -4, -11,  -5, -12,  -5 },
{  -8,  -4,  -9,  -5, -10,  -5, -11,  -6, -11,  -6 },
{  -7,  -5,  -8,  -6,  -9,  -6, -10,  -7, -11,  -7 },
{  -7,  -6,  -8,  -6,  -9,  -7,  -9,  -8, -10,  -8 },
{  -6,  -6,  -7,  -7,  -8,  -8,  -8,  -8,  -9,  -9 },
{  -6,  -7,  -6,  -8,  -7,  -9,  -8,  -9,  -8, -10 },
{  -5,  -7,  -6,  -8,  -6,  -9,  -7, -10,  -7, -11 },
{  -4,  -8,  -5,  -9,  -5, -10,  -6, -11,  -6, -11 },
{  -3,  -8,  -4,  -9,  -4, -10,  -5, -11,  -5, -12 },
{  -3,  -9,  -3, -10,  -3, -11,  -3, -11,  -4, -12 },
{  -2,  -9,  -2, -10,  -2, -11,  -2, -12,  -3, -13 },
{  -1,  -9,  -1, -10,  -1, -11,  -1, -12,  -1, -13 }
};

enum
{
    RING_TRANSPARENT,
    RING_EMPTY,
    RING_READY,
    RING_WARNING,
    RING_EXHAUSTED,
    RING_OUTLINE,
};

static void SetRingPixel(u8 *tiles, s32 x, s32 y, u8 color)
{
    u32 byteIndex;
    if (x < 0 || x >= 32 || y < 0 || y >= 32)
        return;
    // OBJ 4bpp tile-major order, NOT linear 32-pixel scanlines.
    byteIndex = ((y >> 3) * 4 + (x >> 3)) * 32
              + ((y & 7) * 4) + ((x & 7) >> 1);
    if (x & 1)
        tiles[byteIndex] = (tiles[byteIndex] & 0x0F) | (color << 4);
    else
        tiles[byteIndex] = (tiles[byteIndex] & 0xF0) | color;
}

static void RenderRingBand(u8 *tiles, u32 first, u32 last, u8 color)
{
    for (u32 i = first; i < last; i++)
    {
        const s8 *point = sRingPoint[i];
        for (u32 radiusIndex = 1; radiusIndex <= 3; radiusIndex++)
        {
            s32 x = 16 + point[radiusIndex * 2];
            s32 y = 16 + point[radiusIndex * 2 + 1];
            SetRingPixel(tiles, x, y, color);
        }
    }
}

u32 StaminaRing_FilledSegments(u32 current, u32 max)
{
    if (max == 0)
        return 0;
    if (current >= max)
        return STAMINA_RING_TICKS;
    // Battle max stamina is bounded well below 2^26; keep rounding stable.
    return (current * STAMINA_RING_TICKS + max / 2) / max;
}

void StaminaRing_Render(u8 *tiles, u32 current, u32 max, bool32 exhausted)
{
    u32 filled;
    u8 color;

    if (tiles == NULL)
        return;

    memset(tiles, 0, STAMINA_RING_TILE_BYTES);
    filled = StaminaRing_FilledSegments(current, max);

    // Dark, transparent-outside silhouette stays visible when stamina hits 0.
    for (u32 i = 0; i < STAMINA_RING_TICKS; i++)
    {
        const s8 *point = sRingPoint[i];
        SetRingPixel(tiles, 16 + point[0], 16 + point[1], RING_OUTLINE);
        SetRingPixel(tiles, 16 + point[8], 16 + point[9], RING_OUTLINE);
    }
    RenderRingBand(tiles, 0, STAMINA_RING_TICKS, RING_EMPTY);

    if (exhausted || max == 0 || current * 4 <= max)
        color = RING_EXHAUSTED;
    else if (current * 2 <= max)
        color = RING_WARNING;
    else
        color = RING_READY;

    RenderRingBand(tiles, 0, filled, color);
}
