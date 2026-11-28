#include "genesis_route.h"

#define PATCH_SIZE 6u

static u16 Lerp(u16 a, u16 b, u16 fraction)
{
    return (u16)(((u32)a * (PATCH_SIZE - fraction) + (u32)b * fraction) / PATCH_SIZE);
}

// Continuous value field across the whole route, not independent chunk RNG.
static u16 PatchField(u32 seed, u16 x, u16 y)
{
    u16 gx = x / PATCH_SIZE, gy = y / PATCH_SIZE;
    u16 fx = x % PATCH_SIZE, fy = y % PATCH_SIZE;
    u16 a = (u16)(GenesisRng_Hash(seed, gx, gy) & 255u);
    u16 b = (u16)(GenesisRng_Hash(seed, gx + 1, gy) & 255u);
    u16 c = (u16)(GenesisRng_Hash(seed, gx, gy + 1) & 255u);
    u16 d = (u16)(GenesisRng_Hash(seed, gx + 1, gy + 1) & 255u);
    return Lerp(Lerp(a, b, fx), Lerp(c, d, fx), fy);
}

u16 GenesisRoute_PlanCell(const u16 *authored, u16 w, u16 h, u16 x, u16 y,
                          u16 floor, u16 grass, u32 seed)
{
    u16 original, field, nearGrass = 0;
    s32 dx, dy;
    if (!authored || !w || !h || x >= w || y >= h)
        return 0;
    original = authored[(u32)y * w + x];
    if (floor == grass || (floor & 0x0C00u)
        || ((floor ^ grass) & 0xFC00u)
        || (original != floor && original != grass))
        return original;
    // No new encounters at the edge of a path, cliff, ledge, water or
    // special metatile: only the interior of an authored grass meadow.
    if (x == 0 || y == 0 || (u32)x + 1 >= w || (u32)y + 1 >= h)
        return original;
    for (dy = -1; dy <= 1; ++dy)
    {
        for (dx = -1; dx <= 1; ++dx)
        {
            u16 neighbor = authored[(u32)((s32)y + dy) * w + (u32)((s32)x + dx)];
            if (neighbor != floor && neighbor != grass)
                return original;
            if (dx || dy)
                nearGrass += neighbor == grass;
        }
    }
    field = PatchField(seed, x, y);
    // Preserve the core of every existing substantial grass patch. Open
    // coherent clearings along its edges; don't erase isolated grass either.
    if (original == grass)
    {
        if (nearGrass > 0 && nearGrass < 8 && field < 110u)
            return floor;
        return grass;
    }
    // Expand grass only next to the native habitat, never into unrelated
    // clearings or a one-cell-wide path. New patches remain clustered.
    // Preserve narrow bare corridors bordered by tall grass on both sides.
    if ((authored[(u32)y * w + x - 1] == grass && authored[(u32)y * w + x + 1] == grass)
        || (authored[(u32)(y - 1) * w + x] == grass && authored[(u32)(y + 1) * w + x] == grass))
        return floor;
    if (nearGrass >= 2 && field > 150u)
        return grass;
    return floor;
}
