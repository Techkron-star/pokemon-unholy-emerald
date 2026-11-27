#ifndef GUARD_GENESIS_WORLD_TEST_H
#define GUARD_GENESIS_WORLD_TEST_H

// EXPERIMENTAL BRANCH ONLY. Never enable in a release or use a valuable save.
#define GENESIS_WORLD_TEST_ENABLED 1
#define GENESIS_WORLD_TEST_SEED 0x47454E33u
#include "genesis_core.h"

enum {
    GENESIS_EDGE_NORTH = 1, GENESIS_EDGE_SOUTH = 2,
    GENESIS_EDGE_WEST = 4, GENESIS_EDGE_EAST = 8,
    GENESIS_EDGE_ALL = 15
};
enum {
    GENESIS_TEST_OK, GENESIS_TEST_INVALID_LAYOUT, GENESIS_TEST_STORY,
    GENESIS_TEST_MAP_TYPE, GENESIS_TEST_TILESET, GENESIS_TEST_NO_FLOOR,
    GENESIS_TEST_NO_CANDIDATES, GENESIS_TEST_GENERATION_FAILED,
    GENESIS_TEST_UNCHANGED, GENESIS_TEST_OTHER_GAME
};
struct GenesisWorldTestStats
{
    u32 mapSeed;
    u32 generatedChunks;
    u32 failedChunks;
    u32 changedCells;
    u32 addedObstacles;
    u32 candidateCells;
    u32 protectedCells;
    u32 emergencyChunks;
    u32 landingRepairs;
    u32 replacedCells;
    u32 openedCells;
    u8 biome;
    u8 connectedSides;
    u8 result;
};
extern struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void);

gbool GenesisWorldTest_ProtectEdge(u16 w, u16 h, u16 x, u16 y,
                                    u8 connectedSides, u8 seamMargin);
// Pure conservative local topology check: removing the center leaves all
// four cardinal neighbors joined by the eight-cell ring.
gbool GenesisWorldTest_CanBlock(const u16 *grid, u16 stride, u16 height,
                               u16 x, u16 y, u16 floorA, u16 floorB);

#endif
