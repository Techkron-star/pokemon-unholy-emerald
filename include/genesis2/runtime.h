#ifndef GUARD_GENESIS2_RUNTIME_H
#define GUARD_GENESIS2_RUNTIME_H
#include "global.h"
void G2_StartNewGame(void);
void G2_CommitNewGame(void);
void G2_LoadWorld(void);
void G2_RuntimeTick(void);
bool32 G2_RuntimeComplete(void);
bool32 G2_ApplyCurrentMap(void);
bool32 G2_IsGeneratedCell(s32 x, s32 y);
bool32 G2_HasWorldRecord(void);
struct G2_RuntimeStats {
    u32 seed, mapsReady, ticks, maxTickVBlanks, cacheHits, cacheMisses;
    u32 publishedCells, maxPublishVBlanks, status;
};
extern struct G2_RuntimeStats gGenesis2Stats;
#endif
