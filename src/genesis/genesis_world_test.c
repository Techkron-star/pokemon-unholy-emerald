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

// Read the original layout, never the partially modified live map.
static u16 Original(const struct MapLayout *l, s32 x, s32 y)
{
    return l->map[(u32)y * (u32)l->width + (u32)x];
}

static u8 TallNeighbors(const struct MapLayout *l, s32 x, s32 y, u16 entry)
{
    s32 dx, dy;
    u8 count = 0;
    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx)
        {
            u16 n;
            if ((!dx && !dy) || x + dx < 0 || y + dy < 0
                || x + dx >= l->width || y + dy >= l->height)
                continue;
            n = Original(l, x + dx, y + dy);
            if ((n & MAPGRID_METATILE_ID_MASK) == METATILE_General_TallGrass
                && (n & (MAPGRID_COLLISION_MASK | MAPGRID_ELEVATION_MASK))
                   == (entry & (MAPGRID_COLLISION_MASK | MAPGRID_ELEVATION_MASK)))
                ++count;
        }
    return count;
}

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
    s32 x, y;
    u32 i;
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
    for (i = 0; i < sizeof(sChanged); ++i)
        sChanged[i] = 0;
    sTerrainW = (u16)l->width;
    sTerrainH = (u16)l->height;
    sTerrainLayout = header->mapLayoutId;
    gGenesisWorldTestStats.mapSeed = GenesisRng_Hash(GENESIS_WORLD_TEST_SEED,
                                                     header->mapLayoutId, 0);
    gGenesisWorldTestStats.result = GENESIS_TEST_NO_CANDIDATES;
    // Topology first: the authored walkable graph, walls, ledges, water,
    // road art, elevations and events are immutable. Only neutral/tall grass
    // is eligible, and only tall grass is converted to traversable short
    // grass. New meandering trails/clearings do not remove an old path.
    for (y = 0; y < l->height; ++y)
        for (x = 0; x < l->width; ++x)
        {
            u16 old = Original(l, x, y), next;
            u32 k = (u32)y * sTerrainW + (u32)x;
            if ((old & MAPGRID_METATILE_ID_MASK) != METATILE_General_TallGrass
                || (old & MAPGRID_COLLISION_MASK)
                || LiveCell(x, y) != old
                || MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET) != MB_TALL_GRASS)
                continue;
            ++gGenesisWorldTestStats.candidateCells;
            if (Protected(header, x, y))
            {
                ++gGenesisWorldTestStats.protectedCells;
                continue;
            }
            // Protect exact player arrival from decoration changes too.
            if (gSaveBlock1Ptr && x == gSaveBlock1Ptr->pos.x && y == gSaveBlock1Ptr->pos.y)
            {
                ++gGenesisWorldTestStats.landingRepairs;
                continue;
            }
            if (!GenesisRoute_Clear(gGenesisWorldTestStats.mapSeed, (u16)x, (u16)y,
                                   sTerrainW, sTerrainH, TallNeighbors(l, x, y, old)))
                continue;
            next = GenesisRoute_ReplaceGrass(old, METATILE_General_TallGrass,
                                             METATILE_General_Grass, MAPGRID_METATILE_ID_MASK,
                                             MAPGRID_COLLISION_MASK);
            if (next == old)
                continue;
            MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, next);
            sChanged[k / 8u] |= (u8)(1u << (k % 8u));
            ++gGenesisWorldTestStats.changedCells;
            ++gGenesisWorldTestStats.replacedCells;
        }
    if (gGenesisWorldTestStats.candidateCells)
    {
        gGenesisWorldTestStats.generatedChunks = 1;
        gGenesisWorldTestStats.result = gGenesisWorldTestStats.changedCells
                                      ? GENESIS_TEST_OK : GENESIS_TEST_UNCHANGED;
    }
#endif
}

#else
struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void) {}
gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y) { (void)x; (void)y; return GFALSE; }
#endif
