#ifndef GUARD_GENESIS2_WORLD_H
#define GUARD_GENESIS2_WORLD_H
#include <stdint.h>
#include <stddef.h>
#include "genesis2/modules.h"

#define GENESIS2_VERSION UINT32_C(0x020100)
#define G2_CATALOG_VERSION UINT32_C(2)
#define G2_MAX_NATIVE_CELLS 10240u
#define G2_MAP_OFFSET_W 15u
#define G2_MAP_OFFSET_H 14u
#define G2_ID_MASK 0x03FFu
#define G2_COLLISION_MASK 0x0C00u
#define G2_GRASS 0x001u
#define G2_TALL 0x00Du
#define G2_SAND 0x121u

enum G2_Phase {
    G2_QUEUED, G2_RENDER, G2_VALIDATE, G2_READY, G2_FAILED,
    G2_PLAN_INIT, G2_PLAN_FLOOD, G2_PLAN_ROOTS, G2_PLAN_PATHS,
    G2_PLAN_TRACE, G2_PLAN_PAINT, G2_PLAN_COMPOSE, G2_PLAN_NAV_INIT,
    G2_PLAN_NAV, G2_PLAN_NAV_CHECK, G2_PLAN_REPAIR, G2_PLAN_MODULE_CHECK, G2_PLAN_FALLBACK
};
enum G2_PlanFlags { G2_SOFT=1, G2_PORT=2, G2_CORRIDOR=4, G2_MODULE=8 };
#define G2_NO_NODE UINT16_C(0xFFFF)
struct G2_Placement { uint16_t origin, module; };
struct G2_Plan {
    uint8_t *flags;
    uint16_t *region, *parent, *owner;
    struct G2_Placement *placements;
    uint32_t digest, corridorCells, softCells, removedModules;
    uint16_t regions, placementCount, placementCapacity;
    uint8_t grammar, fallback;
};
/* One reusable workspace, owned and restored with the world. Every phase
 * retains its cursors; no recursive solver or per-visit allocation. */
struct G2_Work {
    uint32_t *distance;
    uint16_t *heap, *position, *queue, *roots;
    uint8_t *seen;
    uint32_t capacity, scan, head, tail, heapCount, cursor, traceSteps;
    uint16_t floodRegion, root, trace;
    uint32_t rootHash;
    uint8_t rootIsPort;
};

/* Immutable native assets and reviewed protection. No serialized pointers. */
struct G2_Map {
    uint16_t mapId, layoutId, width, height;
    int32_t worldX, worldY;
    uint32_t domain, assetHash;
    const uint16_t *native;
    const uint8_t *editable;
    /* NULL disables new geometry. The reviewed mask is a subset of editable
     * ordinary passable grass at elevation 3, away from special movement. */
    const uint8_t *topology;
    const struct G2_Catalog *catalog;
};
struct G2_Entry {
    uint16_t *tiles;
    struct G2_Edit { uint16_t sourceIndex, liveIndex; } *edits;
    uint32_t cursor, crc, fingerprint, changed;
    uint8_t phase;
    uint16_t encounterAnchor;
    struct G2_Plan plan;
};
struct G2_World {
    const struct G2_Map *maps;
    struct G2_Entry *entries;
    uint32_t seed, version, catalogVersion, mapCount, current;
    uint32_t ticks, workUnits, maxTickUnits, failed;
    struct G2_Work work;
};

/* Caller owns reusable storage; no hidden heap, MMIO, game RNG or globals. */
int G2_MapValid(const struct G2_Map *map);
size_t G2_StorageBytes(const struct G2_Map *maps, uint32_t count);
int G2_Begin(struct G2_World *world, const struct G2_Map *maps, uint32_t count,
             struct G2_Entry *entries, void *storage, size_t bytes, uint32_t seed);
uint32_t G2_Tick(struct G2_World *world, uint32_t budget);
int G2_Complete(const struct G2_World *world);
const struct G2_Entry *G2_FindReady(const struct G2_World *world, uint16_t mapId,
                                  uint16_t layoutId, uint16_t w, uint16_t h);
uint32_t G2_Hash(uint32_t seed, int32_t x, int32_t y);
uint8_t G2_Climate(uint32_t seed, int32_t x, int32_t y, unsigned shift);
uint16_t G2_RenderCell(const struct G2_Map *map, uint32_t seed, uint32_t k);
int G2_HasTopology(const struct G2_Map *map);
void G2_PlanStep(struct G2_World *world, struct G2_Entry *entry, const struct G2_Map *map);
int G2_ModuleCellValid(const struct G2_Map *map, const struct G2_Entry *entry,
                       uint32_t index, uint16_t value);
#endif
