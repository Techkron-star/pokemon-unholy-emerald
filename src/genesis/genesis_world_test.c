#include "genesis_world_test.h"
#include "genesis_api.h"
#include "genesis_free_test.h"
#include "genesis_terrain.h"
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

#define TEST_CHUNK GENESIS_FREE_CHUNK
#define TEST_MARGIN 1
#define TEST_SEAM_MARGIN MAP_OFFSET
#define NO_TILE 0xFFFFu

GENESIS_DECLARE_BUFFERS(sWorldTestBuffers, TEST_CHUNK, TEST_CHUNK);
static EWRAM_DATA GenesisWorld sWorldTest = {0};
static EWRAM_DATA u16 sTileCounts[1024] = {0};
// Immutable semantic snapshot for the final pass. Max native grid already bounded.
static EWRAM_DATA u8 sTerrain[MAX_MAP_DATA_SIZE] = {0};
static EWRAM_DATA GenesisTerrainRules sWallRules = {0};
static EWRAM_DATA GenesisTerrainRules sWaterRules = {0};
static EWRAM_DATA GenesisTerrainRules sFloorRules = {0};
static EWRAM_DATA u16 sTerrainW = 0, sTerrainH = 0, sTerrainLayout = 0;
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

static u8 ConnectedSides(const struct MapHeader *header)
{
    const struct MapConnections *connections = header->connections;
    s32 i;
    u8 sides = 0;
    if (!connections)
        return 0;
    if (!connections->connections || connections->count < 0)
        return GENESIS_EDGE_ALL;
    for (i = 0; i < connections->count; i++)
    {
        switch (connections->connections[i].direction)
        {
        case CONNECTION_NORTH: sides |= GENESIS_EDGE_NORTH; break;
        case CONNECTION_SOUTH: sides |= GENESIS_EDGE_SOUTH; break;
        case CONNECTION_WEST: sides |= GENESIS_EDGE_WEST; break;
        case CONNECTION_EAST: sides |= GENESIS_EDGE_EAST; break;
        default: break;
        }
    }
    return sides;
}

static gbool Protected(const struct MapHeader *header, s32 x, s32 y)
{
    const struct MapEvents *e = header->events;
    const struct MapLayout *l = header->mapLayout;
    u16 i;
    if (GenesisWorldTest_ProtectEdge((u16)l->width, (u16)l->height,
                                   (u16)x, (u16)y,
                                   gGenesisWorldTestStats.connectedSides,
                                   TEST_SEAM_MARGIN))
        return GTRUE;
    if (!e)
        return GFALSE;
    for (i = 0; i < e->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *o = &e->objectEvents[i];
        // Cloned objects use the same coordinates but a different union.
        s32 rx = 2, ry = 2;
        if (o->kind == OBJ_KIND_NORMAL)
        {
            rx += o->movementRangeX;
            ry += o->movementRangeY;
        }
        if (Near(x, y, o->x, o->y, rx, ry))
            return GTRUE;
    }
    for (i = 0; i < e->warpCount; i++)
        if (Near(x, y, e->warps[i].x, e->warps[i].y, 2, 2))
            return GTRUE;
    for (i = 0; i < e->coordEventCount; i++)
        if (Near(x, y, e->coordEvents[i].x, e->coordEvents[i].y, 3, 3))
            return GTRUE;
    for (i = 0; i < e->bgEventCount; i++)
        if (Near(x, y, e->bgEvents[i].x, e->bgEvents[i].y, 1, 1))
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
            if (blocked ? coll == 0 : coll != 0)
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

static u16 CaveFloorEntry(const struct MapLayout *l, u16 floor)
{
    u16 counts[16] = {0};
    u16 i, best = 0;
    u32 k, n = (u32)l->width * (u32)l->height;
    for (k = 0; k < n; k++)
    {
        u16 e = l->map[k];
        if ((e & MAPGRID_METATILE_ID_MASK) == floor && !(e & MAPGRID_COLLISION_MASK))
            counts[e >> 12]++;
    }
    for (i = 1; i < 16; i++)
        if (counts[i] > counts[best])
            best = i;
    return (u16)(floor | (best << 12));
}

static u8 TerrainClass(u16 entry, u32 behavior)
{
    if (entry & MAPGRID_COLLISION_MASK)
        return GENESIS_TERRAIN_WALL;
    switch (behavior)
    {
    case MB_POND_WATER:
    case MB_OCEAN_WATER:
    case MB_DEEP_WATER:
    case MB_INTERIOR_DEEP_WATER:
    case MB_SOOTOPOLIS_DEEP_WATER:
    case MB_SHALLOW_WATER:
        return GENESIS_TERRAIN_WATER;
    case MB_NORMAL:
    case MB_CAVE:
    case MB_TALL_GRASS:
    case MB_LONG_GRASS:
    case MB_SHORT_GRASS:
        return GENESIS_TERRAIN_GROUND;
    default:
        return GENESIS_TERRAIN_OTHER;
    }
}

static void LearnTerrain(const struct MapLayout *l, u16 floor, u16 rock, gbool cave)
{
    u16 w = (u16)l->width, h = (u16)l->height;
    u16 x, y, i;
    u32 floorBehavior = MB_NORMAL;
    u16 floorId = floor & MAPGRID_METATILE_ID_MASK;
    GenesisTerrain_Clear(&sWallRules);
    GenesisTerrain_Clear(&sWaterRules);
    GenesisTerrain_Clear(&sFloorRules);
    // Read the authored geometry before the first generated tile is written.
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            u16 old = LiveCell(x, y);
            u32 behavior = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
            sTerrain[(u32)y * w + x] = TerrainClass(old, behavior);
            if ((old & MAPGRID_METATILE_ID_MASK) == floorId)
                floorBehavior = behavior;
        }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            u32 k = (u32)y * w + x;
            u16 old = LiveCell(x, y), id = old & MAPGRID_METATILE_ID_MASK;
            u32 behavior = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
            u8 mask;
            if (sTerrain[k] == GENESIS_TERRAIN_WALL && behavior == MB_NORMAL
                && ((id ^ rock) & 0x0200u) == 0)
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y, GENESIS_TERRAIN_WALL);
                GenesisTerrain_Observe(&sWallRules, mask, id);
            }
            else if (!cave && sTerrain[k] == GENESIS_TERRAIN_WATER
                && behavior == MB_OCEAN_WATER && (old & MAPGRID_ELEVATION_MASK) == 0x1000u)
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y, GENESIS_TERRAIN_WATER);
                GenesisTerrain_Observe(&sWaterRules, mask, id);
            }
            else if (sTerrain[k] == GENESIS_TERRAIN_GROUND && behavior == floorBehavior
                && ((old ^ floor) & MAPGRID_ELEVATION_MASK) == 0
                && ((id ^ floor) & 0x0200u) == 0)
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y,
                    cave ? GENESIS_TERRAIN_WALL : GENESIS_TERRAIN_WATER);
                if (mask)
                    GenesisTerrain_Observe(&sFloorRules, mask, id);
            }
        }
    for (i = 0; i < 256; i++)
    {
        gGenesisWorldTestStats.learnedPatterns += sWallRules.tile[i] <= 1023u;
        gGenesisWorldTestStats.learnedPatterns += sWaterRules.tile[i] <= 1023u;
        gGenesisWorldTestStats.learnedPatterns += sFloorRules.tile[i] <= 1023u;
    }
}

static void RenderTerrain(const struct MapLayout *l, u16 floor, u16 rock, gbool cave)
{
    u16 x, y, w = (u16)l->width, h = (u16)l->height;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            u8 kind = sTerrain[(u32)y * w + x], mask;
            u16 old, next, id;
            if (!(kind & GENESIS_TERRAIN_GENERATED))
                continue;
            kind &= GENESIS_TERRAIN_CLASS_MASK;
            old = LiveCell(x, y);
            id = old & MAPGRID_METATILE_ID_MASK;
            if (kind == GENESIS_TERRAIN_WALL)
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y, GENESIS_TERRAIN_WALL);
                // Use an observed solid interior as fallback, not an arbitrary edge.
                id = GenesisTerrain_Select(&sWallRules, mask,
                    GenesisTerrain_Select(&sWallRules, 255, rock & MAPGRID_METATILE_ID_MASK, GFALSE), GTRUE);
            }
            else if (kind == GENESIS_TERRAIN_WATER && !cave)
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y, GENESIS_TERRAIN_WATER);
                id = GenesisTerrain_Select(&sWaterRules, mask, id, GFALSE);
            }
            else if (kind == GENESIS_TERRAIN_GROUND && id == (floor & MAPGRID_METATILE_ID_MASK))
            {
                mask = GenesisTerrain_Mask(sTerrain, w, h, x, y,
                    cave ? GENESIS_TERRAIN_WALL : GENESIS_TERRAIN_WATER);
                if (mask)
                    id = GenesisTerrain_Select(&sFloorRules, mask, id, GFALSE);
            }
            next = GenesisTerrain_ReplaceId(old, id);
            if (next != old)
            {
                MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, next);
                gGenesisWorldTestStats.autotiledCells++;
            }
        }
}

// LoadSavedMapView uses padded coordinates. Never reuse a stale layout snapshot.
gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y)
{
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    if (!sTerrainW || !sTerrainH || sTerrainLayout != gMapHeader.mapLayoutId
        || x < 0 || y < 0 || x >= sTerrainW || y >= sTerrainH)
        return GFALSE;
    return (gbool)((sTerrain[(u32)y * sTerrainW + (u32)x] & GENESIS_TERRAIN_GENERATED) != 0);
}

static GenesisBiomeId SelectBiome(u32 seed, gbool cave, gbool ocean)
{
    static const GenesisBiomeId land[] = { GENESIS_BIOME_MEADOW, GENESIS_BIOME_FOREST,
        GENESIS_BIOME_DESERT, GENESIS_BIOME_RUINS };
    static const GenesisBiomeId caves[] = { GENESIS_BIOME_CAVE, GENESIS_BIOME_GLACIER,
        GENESIS_BIOME_VOLCANO, GENESIS_BIOME_RUINS, GENESIS_BIOME_ALIEN };
    if (cave)
        return caves[seed % 5u];
    if (ocean)
        return land[seed % 3u];
    return land[seed % 4u];
}

void GenesisWorldTest_Apply(void)
{
#if GENESIS_WORLD_TEST_ENABLED
    const struct MapHeader *header = &gMapHeader;
    const struct MapLayout *l = header->mapLayout;
    GenesisConfig cfg = {0};
    u16 floor, vegetation, rock, water;
    s32 bx, by, x, y;
    u16 cw, ch;
    gbool cave, ocean;
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
    gGenesisWorldTestStats.connectedSides = ConnectedSides(header);
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE101)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE101)
        && !FlagGet(FLAG_RESCUED_BIRCH))
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_STORY;
        return;
    }
    cave = (gbool)(header->mapType == MAP_TYPE_UNDERGROUND);
    ocean = (gbool)(header->mapType == MAP_TYPE_OCEAN_ROUTE);
    if (!cave && !ocean && header->mapType != MAP_TYPE_ROUTE)
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_MAP_TYPE;
        return;
    }
    if (cave)
    {
        u16 floorId = DominantCaveTile(l, GFALSE);
        u16 rockId = DominantCaveTile(l, GTRUE);
        if (floorId == NO_TILE || rockId == NO_TILE || floorId == rockId)
        {
            gGenesisWorldTestStats.result = GENESIS_TEST_NO_FLOOR;
            return;
        }
        floor = CaveFloorEntry(l, floorId);
        vegetation = floor;
        rock = (u16)((floor & MAPGRID_ELEVATION_MASK) | MAPGRID_COLLISION_MASK | rockId);
        water = rock;
    }
    else
    {
        if (l->primaryTileset != &gTileset_General)
        {
            gGenesisWorldTestStats.result = GENESIS_TEST_TILESET;
            return;
        }
        floor = 0x3000u | METATILE_General_Grass;
        vegetation = 0x3000u | METATILE_General_TallGrass;
        rock = 0x3000u | MAPGRID_COLLISION_MASK | METATILE_General_RockWall_RockBase;
        // Native Route 124 calm-water entry: elevation 1, collision 0.
        water = 0x1000u | METATILE_General_CalmWater;
    }
    LearnTerrain(l, floor, rock, cave);
    sTerrainW = (u16)l->width;
    sTerrainH = (u16)l->height;
    sTerrainLayout = header->mapLayoutId;
    gGenesisWorldTestStats.mapSeed = GenesisRng_Hash(GENESIS_WORLD_TEST_SEED,
                                                     header->mapLayoutId, 0);
    cfg.biome = SelectBiome(gGenesisWorldTestStats.mapSeed, cave, ocean);
    gGenesisWorldTestStats.biome = (u8)cfg.biome;
    gGenesisWorldTestStats.result = GENESIS_TEST_NO_CANDIDATES;
    // Direct generation: every unprotected cell is rewritten, regardless of
    // its original collision, elevation, metatile or terrain class.
    for (by = 0; by < l->height; by += ch)
    {
        ch = GenesisFree_ChunkExtent((u16)(l->height - by));
        if (!ch)
            break;
        for (bx = 0; bx < l->width; bx += cw)
        {
            cw = GenesisFree_ChunkExtent((u16)(l->width - bx));
            if (!cw)
                break;
            cfg.w = cw;
            cfg.h = ch;
            cfg.master_seed = GenesisRng_Hash3(gGenesisWorldTestStats.mapSeed, bx, by, 3);
            if (!GenesisGenerate(&sWorldTest, &cfg, &sWorldTestBuffers)
                || !GenesisFree_Stitch(&sWorldTest.map))
            {
                gGenesisWorldTestStats.failedChunks++;
                continue;
            }
            gGenesisWorldTestStats.generatedChunks++;
            if (sWorldTest.status == GENESIS_STATUS_EMERGENCY)
                gGenesisWorldTestStats.emergencyChunks++;
            for (y = by; y < by + ch; y++)
            {
                for (x = bx; x < bx + cw; x++)
                {
                    u16 old = LiveCell(x, y), next;
                    gGenesisWorldTestStats.candidateCells++;
                    if (Protected(header, x, y))
                    {
                        gGenesisWorldTestStats.protectedCells++;
                        continue;
                    }
                    // Preserve the exact arrival tile, but not the old paths.
                    if (gSaveBlock1Ptr && x == gSaveBlock1Ptr->pos.x && y == gSaveBlock1Ptr->pos.y)
                    {
                        gGenesisWorldTestStats.landingRepairs++;
                        continue;
                    }
                    next = GenesisFree_RenderCell(&sWorldTest, (u16)(x - bx), (u16)(y - by),
                                                   floor, vegetation, rock, water, ocean, cave);
                    sTerrain[(u32)y * sTerrainW + (u32)x] = GENESIS_TERRAIN_GENERATED |
                        ((next & MAPGRID_COLLISION_MASK) ? GENESIS_TERRAIN_WALL :
                         (!cave && next == water) ? GENESIS_TERRAIN_WATER : GENESIS_TERRAIN_GROUND);
                    gGenesisWorldTestStats.replacedCells++;
                    if (next == old)
                        continue;
                    if ((old & MAPGRID_COLLISION_MASK) && !(next & MAPGRID_COLLISION_MASK))
                        gGenesisWorldTestStats.openedCells++;
                    if (!(old & MAPGRID_COLLISION_MASK) && (next & MAPGRID_COLLISION_MASK))
                        gGenesisWorldTestStats.addedObstacles++;
                    MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, next);
                    gGenesisWorldTestStats.changedCells++;
                }
            }
        }
    }
    // Whole-map pass: adjacent chunks share the same semantic neighborhood.
    RenderTerrain(l, floor, rock, cave);
    if (gGenesisWorldTestStats.changedCells || gGenesisWorldTestStats.autotiledCells)
        gGenesisWorldTestStats.result = GENESIS_TEST_OK;
    else if (gGenesisWorldTestStats.failedChunks)
        gGenesisWorldTestStats.result = GENESIS_TEST_GENERATION_FAILED;
    else if (gGenesisWorldTestStats.generatedChunks)
        gGenesisWorldTestStats.result = GENESIS_TEST_UNCHANGED;
#endif
}

#else
struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void) {}
gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y) { (void)x; (void)y; return GFALSE; }
#endif
