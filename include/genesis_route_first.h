#ifndef GUARD_GENESIS_ROUTE_FIRST_H
#define GUARD_GENESIS_ROUTE_FIRST_H
#include "genesis_core.h"

enum {
    GENESIS_TERRAIN_BALANCED, GENESIS_TERRAIN_FOREST, GENESIS_TERRAIN_PLAINS,
    GENESIS_TERRAIN_TRAILS, GENESIS_TERRAIN_CLEARINGS, GENESIS_TERRAIN_NATIVE,
    GENESIS_TERRAIN_COUNT
};

static inline u8 GenesisRoute_Profile(u8 profile)
{
    return profile < GENESIS_TERRAIN_COUNT ? profile : GENESIS_TERRAIN_BALANCED;
}

// Coherent integer interpolation, no per-cell independent random noise.
// 0..255 samples; 8-cell patches, with continuous boundaries.
static inline u32 GenesisRoute_Lerp(u32 a, u32 b, u32 f, u32 span)
{
    return (a * (span - f) + b * f) / span;
}
static inline u32 GenesisRoute_Noise(u32 seed, u16 x, u16 y)
{
    u32 gx = x / 8u, gy = y / 8u, fx = x % 8u, fy = y % 8u;
    u32 a = GenesisRng_Hash(seed, gx, gy) & 255u;
    u32 b = GenesisRng_Hash(seed, gx + 1u, gy) & 255u;
    u32 c = GenesisRng_Hash(seed, gx, gy + 1u) & 255u;
    u32 d = GenesisRng_Hash(seed, gx + 1u, gy + 1u) & 255u;
    return GenesisRoute_Lerp(GenesisRoute_Lerp(a, b, fx, 8u),
                             GenesisRoute_Lerp(c, d, fx, 8u), fy, 8u);
}

// Winding two-cell tracks across large vegetation patches. A native route
// determines the long axis; offset changes gradually between 8-cell anchors.
// The band pitch leaves vegetation islands between the tracks.
// Return minimum required same-elevation grass neighbors, or 0 for no edit.
// Evaluate spatial intent before expensive native neighbor/event checks.
static inline u8 GenesisRoute_Intent(u32 seed, u16 x, u16 y,
                                    u16 w, u16 h, u8 profile)
{
    u32 along = w >= h ? x : y;
    u32 across = w >= h ? y : x;
    u32 band = across / 16u;
    u32 a, b, center, pos, threshold;
    profile = GenesisRoute_Profile(profile);
    if (profile == GENESIS_TERRAIN_NATIVE)
        return 0;
    a = 5u + GenesisRng_Hash(seed ^ 0x524F5554u, along / 8u, band) % 7u;
    b = 5u + GenesisRng_Hash(seed ^ 0x524F5554u, along / 8u + 1u, band) % 7u;
    center = GenesisRoute_Lerp(a, b, along % 8u, 8u);
    pos = across % 16u;
    if (profile == GENESIS_TERRAIN_FOREST)
        return (pos == center && !(band & 1u)) ? 3 : 0;
    if (profile != GENESIS_TERRAIN_CLEARINGS
        && (pos == center || pos == center + 1u
            || (profile == GENESIS_TERRAIN_PLAINS && pos == center + 2u)
            || (profile == GENESIS_TERRAIN_TRAILS && pos == center - 1u)))
        return 3;
    if (profile == GENESIS_TERRAIN_TRAILS)
        return 0;
    threshold = profile == GENESIS_TERRAIN_PLAINS ? 180u
              : profile == GENESIS_TERRAIN_CLEARINGS ? 105u : 64u;
    return GenesisRoute_Noise(seed ^ 0x434C4541u, x, y) < threshold ? 6 : 0;
}

static inline gbool GenesisRoute_Clear(u32 seed, u16 x, u16 y,
                                       u16 w, u16 h, u8 tallNeighbors)
{
    u8 need = GenesisRoute_Intent(seed, x, y, w, h, GENESIS_TERRAIN_BALANCED);
    return (gbool)(need != 0 && tallNeighbors >= need);
}

// Narrow, explicit edit contract. Collision and elevation bits are immutable.
static inline u16 GenesisRoute_ReplaceGrass(u16 entry, u16 tallId, u16 floorId,
                                            u16 idMask, u16 collisionMask)
{
    if ((entry & collisionMask) || (entry & idMask) != tallId)
        return entry;
    return (u16)((entry & ~idMask) | (floorId & idMask));
}
#endif
