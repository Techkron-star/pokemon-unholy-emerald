#include "genesis_terrain.h"

void GenesisTerrain_Clear(GenesisTerrainRules *r)
{
    u16 i;
    for (i = 0; i < 256; i++)
    {
        r->tile[i] = GENESIS_TERRAIN_NO_TILE;
        r->votes[i] = 0;
    }
}

u8 GenesisTerrain_Mask(const u8 *grid, u16 w, u16 h, u16 x, u16 y, u8 target)
{
    static const s8 dx[8] = {0,1,0,-1,1,1,-1,-1};
    static const s8 dy[8] = {-1,0,1,0,-1,1,1,-1};
    u8 i, mask = 0;
    if (!grid || x >= w || y >= h || target > GENESIS_TERRAIN_WATER)
        return 0;
    for (i = 0; i < 8; i++)
    {
        s32 nx = (s32)x + dx[i], ny = (s32)y + dy[i];
        if (nx >= 0 && ny >= 0 && nx < w && ny < h
            && (GenesisTerrain_Get(grid, (u32)ny * w + (u32)nx) & GENESIS_TERRAIN_CLASS_MASK) == target)
            mask |= (u8)(1u << i);
    }
    return mask;
}

void GenesisTerrain_Observe(GenesisTerrainRules *r, u8 mask, u16 tile)
{
    if (tile > 1023u)
        return;
    /* Bounded, deterministic majority candidate; not a full histogram. */
    if (!r->votes[mask])
    {
        r->tile[mask] = tile;
        r->votes[mask] = 1;
    }
    else if (r->tile[mask] == tile)
    {
        if (r->votes[mask] < 65535u)
            r->votes[mask]++;
    }
    else
        r->votes[mask]--;
}

u16 GenesisTerrain_Select(const GenesisTerrainRules *r, u8 mask, u16 fallback, gbool approximate)
{
    u16 i, best = fallback, bestVotes = 0;
    u8 bestDistance = 5;
    if (r->tile[mask] <= 1023u)
        return r->tile[mask];
    if (!approximate)
        return fallback;
    /* Only compare patterns with identical cardinal adjacency. */
    for (i = mask & 15u; i < 256; i += 16)
    {
        u8 diff = (u8)((i ^ mask) >> 4), d = 0;
        if (r->tile[i] > 1023u)
            continue;
        while (diff) { d += diff & 1u; diff >>= 1; }
        if (d < bestDistance || (d == bestDistance && r->votes[i] > bestVotes))
        {
            bestDistance = d;
            bestVotes = r->votes[i];
            best = r->tile[i];
        }
    }
    return best;
}

u16 GenesisTerrain_ReplaceId(u16 cell, u16 tile)
{
    if (tile > 1023u)
        return cell;
    return (u16)((cell & 0xFC00u) | tile);
}
