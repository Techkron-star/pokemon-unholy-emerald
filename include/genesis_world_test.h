#ifndef GUARD_GENESIS_WORLD_TEST_H
#define GUARD_GENESIS_WORLD_TEST_H

// EXPERIMENTAL BRANCH ONLY. Never enable in a release or use a valuable save.
#define GENESIS_WORLD_TEST_ENABLED 1
#define GENESIS_WORLD_TEST_SEED 0x47454E31u
#include "genesis_core.h"

struct GenesisWorldTestStats
{
    u32 mapSeed;
    u32 generatedChunks;
    u32 failedChunks;
    u32 changedCells;
    u32 addedObstacles;
};
extern struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void);

// Pure conservative local topology check: removing the center leaves all
// four cardinal neighbors joined by the eight-cell ring.
gbool GenesisWorldTest_CanBlock(const u16 *grid, u16 stride, u16 height,
                               u16 x, u16 y, u16 floorA, u16 floorB);

#endif
