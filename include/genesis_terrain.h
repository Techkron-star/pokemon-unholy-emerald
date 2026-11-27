#ifndef GUARD_GENESIS_TERRAIN_H
#define GUARD_GENESIS_TERRAIN_H
#include "genesis_core.h"
enum { GENESIS_TERRAIN_OTHER, GENESIS_TERRAIN_GROUND, GENESIS_TERRAIN_WALL, GENESIS_TERRAIN_WATER };
#define GENESIS_TERRAIN_CLASS_MASK 0x0Fu
#define GENESIS_TERRAIN_GENERATED 0x80u
#define GENESIS_TERRAIN_NO_TILE 0xFFFFu
/* Bits N,E,S,W,NE,SE,SW,NW. No wrapping across map edges. */
typedef struct { u16 tile[256]; u16 votes[256]; } GenesisTerrainRules;
void GenesisTerrain_Clear(GenesisTerrainRules *r);
u8 GenesisTerrain_Mask(const u8 *grid, u16 w, u16 h, u16 x, u16 y, u8 target);
void GenesisTerrain_Observe(GenesisTerrainRules *r, u8 mask, u16 tile);
u16 GenesisTerrain_Select(const GenesisTerrainRules *r, u8 mask, u16 fallback, gbool approximate);
u16 GenesisTerrain_ReplaceId(u16 cell, u16 tile);
#endif
