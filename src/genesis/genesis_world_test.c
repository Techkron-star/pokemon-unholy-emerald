#include "genesis_world_test.h"
#include "genesis_route.h"
#ifndef GENESIS_BUILD_HOST
#include "fieldmap.h"
#include "global.fieldmap.h"
#include "event_data.h"
#include "tilesets.h"
#include "constants/map_types.h"
#include "constants/metatile_labels.h"
#include "constants/maps.h"
#include "constants/event_objects.h"
#include "constants/flags.h"

#define TEST_SEAM_MARGIN MAP_OFFSET
// Only a changed-cell bitset survives map initialization. The immutable ROM
// layout is our template; no chunk buffers, adjacency histograms or new walls.
static EWRAM_DATA u8 sGenerated[(MAX_MAP_DATA_SIZE + 7u) / 8u] = {0};
static EWRAM_DATA u16 sRouteW = 0, sRouteH = 0, sRouteLayout = 0;
EWRAM_DATA struct GenesisWorldTestStats gGenesisWorldTestStats = {0};

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



gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y)
{
    u32 k;
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    if (!sRouteW || !sRouteH || sRouteLayout != gMapHeader.mapLayoutId
        || x < 0 || y < 0 || x >= sRouteW || y >= sRouteH)
        return GFALSE;
    k = (u32)y * sRouteW + (u32)x;
    return (gbool)((sGenerated[k >> 3] >> (k & 7u)) & 1u);
}

void GenesisWorldTest_Apply(void)
{
#if GENESIS_WORLD_TEST_ENABLED
    const struct MapHeader *header = &gMapHeader;
    const struct MapLayout *l = header->mapLayout;
    const u16 floor = 0x3000u | METATILE_General_Grass;
    const u16 grass = 0x3000u | METATILE_General_TallGrass;
    u16 x, y, w, h;
    u32 k, seed;
    gGenesisWorldTestStats = (struct GenesisWorldTestStats){0};
    sRouteW = sRouteH = sRouteLayout = 0;
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
    // Caves/ocean routes are deliberately restored to authored geometry until
    // their own progression-aware grammars are implemented.
    gGenesisWorldTestStats.result = GENESIS_TEST_MAP_TYPE;
    if (header->mapType != MAP_TYPE_ROUTE)
        return;
    gGenesisWorldTestStats.result = GENESIS_TEST_TILESET;
    if (l->primaryTileset != &gTileset_General)
        return;
    gGenesisWorldTestStats.connectedSides = ConnectedSides(header);
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE101)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE101)
        && !FlagGet(FLAG_RESCUED_BIRCH))
    {
        gGenesisWorldTestStats.result = GENESIS_TEST_STORY;
        return;
    }
    w = (u16)l->width;
    h = (u16)l->height;
    seed = GenesisRng_Hash(GENESIS_WORLD_TEST_SEED, header->mapLayoutId, 0);
    gGenesisWorldTestStats.mapSeed = seed;
    gGenesisWorldTestStats.result = GENESIS_TEST_NO_CANDIDATES;
    sRouteW = w;
    sRouteH = h;
    sRouteLayout = header->mapLayoutId;
    for (k = 0; k < ((u32)w * h + 7u) / 8u; ++k)
        sGenerated[k] = 0;
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            u16 original, live, next;
            k = (u32)y * w + x;
            original = l->map[k];
            if (original != floor && original != grass)
                continue;
            ++gGenesisWorldTestStats.candidateCells;
            if (Protected(header, x, y))
            {
                ++gGenesisWorldTestStats.protectedCells;
                continue;
            }
            // Arrival tile and scripted/native mutations are never overwritten.
            if (gSaveBlock1Ptr && x == gSaveBlock1Ptr->pos.x && y == gSaveBlock1Ptr->pos.y)
            {
                ++gGenesisWorldTestStats.landingRepairs;
                continue;
            }
            live = gBackupMapLayout.map[(u32)(y + MAP_OFFSET) * (u32)gBackupMapLayout.width
                                         + (u32)(x + MAP_OFFSET)];
            if (live != original)
                continue;
            next = GenesisRoute_PlanCell(l->map, w, h, x, y, floor, grass, seed);
            if (next == original)
                continue;
            // Defense in depth: no generated collision/elevation changes.
            if ((next & 0xFC00u) != (original & 0xFC00u))
                continue;
            MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, next);
            sGenerated[k >> 3] |= (u8)(1u << (k & 7u));
            ++gGenesisWorldTestStats.changedCells;
            ++gGenesisWorldTestStats.replacedCells;
        }
    }
    if (gGenesisWorldTestStats.changedCells)
        gGenesisWorldTestStats.result = GENESIS_TEST_OK;
    else if (gGenesisWorldTestStats.candidateCells)
        gGenesisWorldTestStats.result = GENESIS_TEST_UNCHANGED;
#endif
}
#else
struct GenesisWorldTestStats gGenesisWorldTestStats;
void GenesisWorldTest_Apply(void) {}
gbool GenesisWorldTest_IsGeneratedCell(s32 x, s32 y) { (void)x; (void)y; return GFALSE; }
#endif
