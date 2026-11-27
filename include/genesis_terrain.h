#ifndef GUARD_GENESIS_TERRAIN_H
#define GUARD_GENESIS_TERRAIN_H
#include "genesis_core.h"
enum { GENESIS_TERRAIN_OTHER, GENESIS_TERRAIN_GROUND, GENESIS_TERRAIN_WALL, GENESIS_TERRAIN_WATER };
#define GENESIS_TERRAIN_CLASS_MASK 0x03u
#define GENESIS_TERRAIN_GENERATED 0x08u
#define GENESIS_TERRAIN_PACKED_BYTES(n) (((n) + 1u) / 2u)

/* Two semantic cells per byte. Callers must allocate PACKED_BYTES(cellCount)
 * and pass indices within that cell count. Preserve the adjacent nibble. */
static inline u8 GenesisTerrain_Get(const u8 *grid, u32 index)
{
    return (u8)((grid[index >> 1] >> ((index & 1u) * 4u)) & 0x0Fu);
}
static inline void GenesisTerrain_Set(u8 *grid, u32 index, u8 value)
{
    u8 shift = (u8)((index & 1u) * 4u);
    u8 mask = (u8)(0x0Fu << shift);
    grid[index >> 1] = (u8)((grid[index >> 1] & (u8)~mask) | ((value & 0x0Fu) << shift));
}
#define GENESIS_TERRAIN_NO_TILE 0xFFFFu
/* Bits N,E,S,W,NE,SE,SW,NW. No wrapping across map edges. */
typedef struct { u16 tile[256]; u16 votes[256]; } GenesisTerrainRules;
void GenesisTerrain_Clear(GenesisTerrainRules *r);
u8 GenesisTerrain_Mask(const u8 *grid, u16 w, u16 h, u16 x, u16 y, u8 target);
void GenesisTerrain_Observe(GenesisTerrainRules *r, u8 mask, u16 tile);
u16 GenesisTerrain_Select(const GenesisTerrainRules *r, u8 mask, u16 fallback, gbool approximate);
u16 GenesisTerrain_ReplaceId(u16 cell, u16 tile);
#endif
