#ifndef GUARD_GENESIS_FREE_TEST_H
#define GUARD_GENESIS_FREE_TEST_H
#include "genesis_api.h"
#define GENESIS_FREE_CHUNK 32
/* Partition a map axis into chunks in [8,32], without tiny tail chunks. */
u16 GenesisFree_ChunkExtent(u16 remaining);
/* Open shared central lanes on all four edges, then join all regions. */
gbool GenesisFree_Stitch(GenesisMap *m);
/* Native terrain output: no original terrain dependency. */
u16 GenesisFree_RenderCell(const GenesisWorld *w, u16 x, u16 y,
                           u16 floor, u16 vegetation, u16 rock, u16 water,
                           gbool ocean, gbool cave);
#endif
