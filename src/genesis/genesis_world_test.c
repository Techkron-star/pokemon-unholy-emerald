#include "genesis_world_test.h"
#include "genesis_route_first.h"
#ifndef GENESIS_BUILD_HOST
#include "fieldmap.h"
#include "global.fieldmap.h"
#include "event_data.h"
#include "tilesets.h"
#include "constants/map_types.h"
#include "constants/metatile_behaviors.h"
#include "constants/metatile_labels.h"
#include "constants/maps.h"
#include "constants/event_objects.h"
#include "constants/flags.h"

#define TEST_SEAM_MARGIN MAP_OFFSET
// One bit per changed cell; no chunk scratch or learned wall palettes.
static EWRAM_DATA u8 sChanged[(MAX_MAP_DATA_SIZE + 7u) / 8u] = {0};
static EWRAM_DATA u16 sTerrainW = 0, sTerrainH = 0, sTerrainLayout = 0;
EWRAM_DATA struct GenesisWorldTestStats gGenesisWorldTestStats = {0};

struct GenesisBakedEdit { u16 x, y, oldEntry, newEntry; };
struct GenesisBakedMap {
    u16 mapId, width, height;
    u32 count;
    const struct GenesisBakedEdit *edits;
};
#include "genesis_baked.generated.h"

gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y)
{
    u32 k;
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    if (!sTerrainW || !sTerrainH || sTerrainLayout != gMapHeader.mapLayoutId
        || x < 0 || y < 0 || x >= sTerrainW || y >= sTerrainH)
        return GFALSE;
    k = (u32)y * sTerrainW + (u32)x;
    return (gbool)((sChanged[k / 8u] >> (k % 8u)) & 1u);
}

void GenesisWorldTest_Apply(void)
{
#if GENESIS_WORLD_TEST_ENABLED
    const struct MapHeader *header = &gMapHeader;
    const struct MapLayout *l = header->mapLayout;
    u32 i;
    u16 mapId;
    const struct GenesisBakedMap *baked = NULL;
    gGenesisWorldTestStats = (struct GenesisWorldTestStats){0};
    sTerrainW = sTerrainH = sTerrainLayout = 0;
    gGenesisWorldTestStats.result = GENESIS_TEST_OTHER_GAME;
    if (GAME_VERSION != VERSION_EMERALD)
        return;
    gGenesisWorldTestStats.result = GENESIS_TEST_INVALID_LAYOUT;
    if (!l || !l->map || l->isFrlg || !gBackupMapLayout.map
        || l->width < GENESIS_MIN_DIM || l->height < GENESIS_MIN_DIM
        || gBackupMapLayout.width != l->width + MAP_OFFSET_W
        || gBackupMapLayout.height != l->height + MAP_OFFSET_H
        || (u32)gBackupMapLayout.width * (u32)gBackupMapLayout.height > MAX_MAP_DATA_SIZE)
        return;
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE101)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE101)
        && !FlagGet(FLAG_RESCUED_BIRCH))
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_STORY;
        return;
    }
    // Authored caves, ocean routes, towns, interiors and puzzles are restored.
    // Never apply land-route rules to a different movement domain.
    if (header->mapType != MAP_TYPE_ROUTE)
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_MAP_TYPE;
        return;
    }
    if (l->primaryTileset != &gTileset_General)
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_TILESET;
        return;
    }
    if (gSaveBlock2Ptr->optionsGenesisTerrain == GENESIS_TERRAIN_NATIVE)
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_UNCHANGED;
        return;
    }
    mapId = ((u16)gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    for (i = 0; i < ARRAY_COUNT(sBakedMaps); ++i)
        if (sBakedMaps[i].mapId == mapId)
        {
            baked = &sBakedMaps[i];
            break;
        }
    gGenesisWorldTestStats.result = GENESIS_TEST_NO_CANDIDATES;
    if (baked == NULL || baked->width != l->width || baked->height != l->height)
        return;
    sTerrainW = (u16)l->width;
    sTerrainH = (u16)l->height;
    sTerrainLayout = header->mapLayoutId;
    for (i = 0; i < ((u32)sTerrainW * sTerrainH + 7u) / 8u; ++i)
        sChanged[i] = 0;
    // No terrain noise, neighbor scanning, or event loops on the GBA.
    // Validate the expected native cell before applying the sparse edit.
    for (i = 0; i < baked->count; ++i)
    {
        const struct GenesisBakedEdit *e = &baked->edits[i];
        u32 k = (u32)e->y * sTerrainW + e->x;
        u32 live = (u32)(e->y + MAP_OFFSET) * gBackupMapLayout.width + e->x + MAP_OFFSET;
        if (e->x >= sTerrainW || e->y >= sTerrainH
            || l->map[k] != e->oldEntry || gBackupMapLayout.map[live] != e->oldEntry
            || ((e->oldEntry ^ e->newEntry) & ~MAPGRID_METATILE_ID_MASK)
            || (e->oldEntry & MAPGRID_COLLISION_MASK))
            continue;
        ++gGenesisWorldTestStats.candidateCells;
        gBackupMapLayout.map[live] = e->newEntry;
        sChanged[k / 8u] |= (u8)(1u << (k % 8u));
        ++gGenesisWorldTestStats.changedCells;
        ++gGenesisWorldTestStats.replacedCells;
    }
    gGenesisWorldTestStats.generatedChunks = 1;
    gGenesisWorldTestStats.result = gGenesisWorldTestStats.changedCells
                                  ? GENESIS_TEST_OK : GENESIS_TEST_UNCHANGED;
#endif
}

#else
struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void) {}
gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y) { (void)x; (void)y; return GFALSE; }
#endif
