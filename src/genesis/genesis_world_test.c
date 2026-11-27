#include "genesis_world_test.h"
#include "genesis_api.h"
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

#define TEST_CHUNK 32
#define TEST_MARGIN MAP_OFFSET
#define NO_TILE 0xFFFFu

GENESIS_DECLARE_BUFFERS(sWorldTestBuffers, TEST_CHUNK, TEST_CHUNK);
static EWRAM_DATA GenesisWorld sWorldTest = {0};
static EWRAM_DATA u16 sTileCounts[1024] = {0};
EWRAM_DATA struct GenesisWorldTestStats gGenesisWorldTestStats = {0};

static u16 LiveCell(s32 x, s32 y)
{
    return gBackupMapLayout.map[(u32)(y + MAP_OFFSET) * (u32)gBackupMapLayout.width
                                + (u32)(x + MAP_OFFSET)];
}

static gbool Near(s32 x, s32 y, s32 ax, s32 ay, s32 rx, s32 ry)
{
    return (gbool)(x >= ax - rx && x <= ax + rx && y >= ay - ry && y <= ay + ry);
}

static gbool Protected(const struct MapHeader *header, s32 x, s32 y)
{
    const struct MapEvents *e = header->events;
    const struct MapLayout *l = header->mapLayout;
    u16 i;
    if (x < TEST_MARGIN || y < TEST_MARGIN
        || x >= l->width - TEST_MARGIN || y >= l->height - TEST_MARGIN)
        return GTRUE;
    if (!e)
        return GFALSE;
    for (i = 0; i < e->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *o = &e->objectEvents[i];
        // Cloned objects use the same coordinates but a different union.
        s32 rx = 5, ry = 5;
        if (o->kind == OBJ_KIND_NORMAL)
        {
            rx += o->movementRangeX;
            ry += o->movementRangeY;
        }
        if (Near(x, y, o->x, o->y, rx, ry))
            return GTRUE;
    }
    for (i = 0; i < e->warpCount; i++)
        if (Near(x, y, e->warps[i].x, e->warps[i].y, 4, 4))
            return GTRUE;
    for (i = 0; i < e->coordEventCount; i++)
        if (Near(x, y, e->coordEvents[i].x, e->coordEvents[i].y, 5, 5))
            return GTRUE;
    for (i = 0; i < e->bgEventCount; i++)
        if (Near(x, y, e->bgEvents[i].x, e->bgEvents[i].y, 3, 3))
            return GTRUE;
    return GFALSE;
}

// For caves, use only neutral tiles already authored in the same layout.
// No placeholder IDs and no replacement of ice, ledges, warp or puzzle tiles.
static u16 DominantCaveTile(const struct MapLayout *l, gbool blocked)
{
    s32 x, y;
    u16 i, best = NO_TILE, count = 0;
    for (i = 0; i < 1024; i++)
        sTileCounts[i] = 0;
    for (y = 0; y < l->height; y++)
    {
        for (x = 0; x < l->width; x++)
        {
            u16 entry = l->map[(u32)y * (u32)l->width + (u32)x];
            u16 id = entry & MAPGRID_METATILE_ID_MASK;
            u16 coll = entry & MAPGRID_COLLISION_MASK;
            u32 behavior;
            if (blocked ? coll != MAPGRID_COLLISION_MASK : coll != 0)
                continue;
            behavior = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
            if (behavior != MB_NORMAL && (blocked || behavior != MB_CAVE))
                continue;
            sTileCounts[id]++;
        }
    }
    for (i = 0; i < 1024; i++)
    {
        if (sTileCounts[i] > count)
        {
            count = sTileCounts[i];
            best = i;
        }
    }
    return best;
}

static gbool Candidate(u16 entry, u16 floorA, u16 floorB)
{
    u16 id = entry & MAPGRID_METATILE_ID_MASK;
    return (gbool)(!(entry & MAPGRID_COLLISION_MASK) && (id == floorA || id == floorB));
}

void GenesisWorldTest_Apply(void)
{
#if GENESIS_WORLD_TEST_ENABLED
    const struct MapHeader *header = &gMapHeader;
    const struct MapLayout *l = header->mapLayout;
    GenesisConfig cfg = {0};
    u16 floorA, floorB, rock;
    s32 bx, by, x, y;
    gbool cave, allowObstacles, allowGrass;
    gGenesisWorldTestStats = (struct GenesisWorldTestStats){0};
    if (GAME_VERSION != VERSION_EMERALD)
        return;
    if (!l || !l->map || l->isFrlg || !gBackupMapLayout.map
        || l->width <= 2 * TEST_MARGIN || l->height <= 2 * TEST_MARGIN
        || gBackupMapLayout.width != l->width + MAP_OFFSET_W
        || gBackupMapLayout.height != l->height + MAP_OFFSET_H
        || (u32)gBackupMapLayout.width * (u32)gBackupMapLayout.height > MAX_MAP_DATA_SIZE)
        return;
    // Keep the starter rescue exactly authored; fixed scripted movement paths
    // cannot safely be inferred from an event template.
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE101)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE101)
        && !FlagGet(FLAG_RESCUED_BIRCH))
        return;
    cave = (gbool)(header->mapType == MAP_TYPE_UNDERGROUND);
    if (cave)
    {
        floorA = DominantCaveTile(l, GFALSE);
        floorB = floorA;
        rock = DominantCaveTile(l, GTRUE);
        if (floorA == NO_TILE || rock == NO_TILE || floorA == rock)
            return;
    }
    else
    {
        if (header->mapType != MAP_TYPE_ROUTE && header->mapType != MAP_TYPE_OCEAN_ROUTE
            && header->mapType != MAP_TYPE_TOWN && header->mapType != MAP_TYPE_CITY)
            return;
        if (l->primaryTileset != &gTileset_General)
            return;
        floorA = METATILE_General_Grass;
        floorB = METATILE_General_TallGrass;
        rock = METATILE_General_RockWall_RockBase;
    }
    allowGrass = (gbool)(!cave && (header->mapType == MAP_TYPE_ROUTE
                                 || header->mapType == MAP_TYPE_OCEAN_ROUTE));
    // Do not add blockers in towns/cities or maps with coordinate scripts.
    // In caves with triggers the entire map is kept authored.
    allowObstacles = (gbool)((cave || allowGrass)
                      && (!header->events || header->events->coordEventCount == 0));
    if (!allowObstacles && !allowGrass)
        return;
    cfg.w = cfg.h = TEST_CHUNK;
    cfg.biome = cave ? GENESIS_BIOME_CAVE : GENESIS_BIOME_FOREST;
    gGenesisWorldTestStats.mapSeed = GenesisRng_Hash(GENESIS_WORLD_TEST_SEED,
                                                     header->mapLayoutId, 0);
    for (by = TEST_MARGIN; by < l->height - TEST_MARGIN; by += TEST_CHUNK)
    {
        for (bx = TEST_MARGIN; bx < l->width - TEST_MARGIN; bx += TEST_CHUNK)
        {
            gbool found = GFALSE;
            for (y = by; y < by + TEST_CHUNK && y < l->height - TEST_MARGIN && !found; y++)
                for (x = bx; x < bx + TEST_CHUNK && x < l->width - TEST_MARGIN; x++)
                    if (Candidate(LiveCell(x, y), floorA, floorB) && !Protected(header, x, y))
                    {
                        found = GTRUE;
                        break;
                    }
            if (!found)
                continue;
            cfg.master_seed = GenesisRng_Hash3(gGenesisWorldTestStats.mapSeed, bx, by, 1);
            if (!GenesisGenerate(&sWorldTest, &cfg, &sWorldTestBuffers))
            {
                gGenesisWorldTestStats.failedChunks++;
                continue; // leave the original chunk unchanged
            }
            gGenesisWorldTestStats.generatedChunks++;
            for (y = by; y < by + TEST_CHUNK && y < l->height - TEST_MARGIN; y++)
            {
                for (x = bx; x < bx + TEST_CHUNK && x < l->width - TEST_MARGIN; x++)
                {
                    u16 old = LiveCell(x, y), next = old;
                    u16 elevation = old & MAPGRID_ELEVATION_MASK;
                    u32 k = (u32)(y - by) * TEST_CHUNK + (u32)(x - bx);
                    if (!Candidate(old, floorA, floorB) || Protected(header, x, y))
                        continue;
                    if (!sWorldTest.map.mask[k] && allowObstacles
                        && GenesisWorldTest_CanBlock(gBackupMapLayout.map,
                           (u16)gBackupMapLayout.width, (u16)gBackupMapLayout.height,
                           (u16)(x + MAP_OFFSET), (u16)(y + MAP_OFFSET), floorA, floorB))
                    {
                        next = (u16)(elevation | MAPGRID_COLLISION_MASK | rock);
                        gGenesisWorldTestStats.addedObstacles++;
                    }
                    else if (allowGrass)
                    {
                        // Procedural grass/ground, with original elevation and collision.
                        u16 id = sWorldTest.map.cells[k] & MAPGRID_METATILE_ID_MASK;
                        next = (u16)(elevation | (id == sWorldTest.profile.metatile_vegetation
                                                  ? floorB : floorA));
                    }
                    if (next != old)
                    {
                        MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, next);
                        gGenesisWorldTestStats.changedCells++;
                    }
                }
            }
        }
    }
#endif
}
#else
struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void) {}
#endif
