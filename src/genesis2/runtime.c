#include "global.h"
#include "genesis2/runtime.h"
#include "genesis2/world.h"
#include "genesis_route_first.h"
#include "gbax/gbax.h"
#include "main.h"
#include "random.h"
#include "fieldmap.h"
#include "unholy_save.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "genesis2/assets.generated.h"

#define G2_RECORD_MAGIC 0x47325731u
#define G2_TICK_UNITS 128u
#define G2_TICK_SCANLINES 55u /* ~4 ms plus at most one bounded microstep */
/* Sole root owner. Other engines must reserve siblings through this owner,
 * never start independent arenas at WORK_BASE. SAVX is never scratch. */
struct G2_Runtime {
    GBAXArena root, genesis;
    struct G2_World world;
    struct G2_Entry entries[ARRAY_COUNT(sG2Maps)];
};
struct G2_Record { u32 magic, version, catalogVersion, seed; };
STATIC_ASSERT(UNHOLY_SAVE4_GENESIS_OFFSET + sizeof(struct G2_Record)
              <= UNHOLY_SAVE4_COFRADIA_OFFSET, GenesisRecordFitsSave4);
static EWRAM_DATA struct G2_Runtime *sRuntime = NULL;
static EWRAM_DATA u32 sPendingSeed = 0;
static EWRAM_DATA bool8 sIntroPending = FALSE;
static EWRAM_DATA const struct G2_Map *sPublished = NULL;
static EWRAM_DATA const struct G2_Entry *sPublishedEntry = NULL;
#define G2_PUBLICATION_BYTES ((G2_MAX_NATIVE_CELLS / 4u + 7u) / 8u)
static EWRAM_DATA u8 sPublishedModules[G2_PUBLICATION_BYTES] = {0};
static EWRAM_DATA u8 sRejectedModules[G2_PUBLICATION_BYTES] = {0};
EWRAM_DATA struct G2_RuntimeStats gGenesis2Stats = {0};

static const struct G2_Record *ReadRecord(void)
{
    const struct SaveBlock4 *block = &gSaveBlock3Ptr->unholySave4;
    const struct G2_Record *record;
    /* Read-only. Continuing never initializes a damaged/unknown save. */
    if (!UnholySave4_Validate(block)) return NULL;
    record = (const void *)(block->payload + UNHOLY_SAVE4_GENESIS_OFFSET);
    if (record->magic != G2_RECORD_MAGIC || record->version != GENESIS2_VERSION
        || record->catalogVersion != G2_CATALOG_VERSION) return NULL;
    return record;
}

bool32 G2_HasWorldRecord(void)
{
    const struct SaveBlock4 *block = &gSaveBlock3Ptr->unholySave4;
    const struct G2_Record *r = (const void *)(block->payload + UNHOLY_SAVE4_GENESIS_OFFSET);
    return r->magic == G2_RECORD_MAGIC;
}

static void BeginWorld(u32 seed)
{
    GBAXInfo info;
    GBAXArena root, child;
    size_t bytes = G2_StorageBytes(sG2Maps, ARRAY_COUNT(sG2Maps));
    void *tiles;
    sRuntime = NULL; sPublished = NULL; sPublishedEntry = NULL;
    memset(&gGenesis2Stats, 0, sizeof(gGenesis2Stats));
    gGenesis2Stats.seed = seed;
    gGenesis2Stats.status = 1;
    if (GAME_VERSION != VERSION_EMERALD || !bytes || !GBAXDetect(&info)
        || !GBAXArenaInit(&root, &info)
        || !GBAXReserve(&root, &child, sizeof(struct G2_Runtime) + bytes + 16u, 16)) return;
    sRuntime = GBAXAlloc(&child, sizeof(*sRuntime), 4);
    if (!sRuntime) return;
    sRuntime->root = root;
    tiles = GBAXAlloc(&child, bytes, 4);
    sRuntime->genesis = child;
    if (!tiles || !G2_Begin(&sRuntime->world, sG2Maps, ARRAY_COUNT(sG2Maps),
                           sRuntime->entries, tiles, bytes, seed))
    { sRuntime = NULL; return; }
    gGenesis2Stats.status = 2;
}

void G2_StartNewGame(void)
{
    sPendingSeed = Random32();
    sIntroPending = TRUE;
    BeginWorld(sPendingSeed);
}

void G2_CommitNewGame(void)
{
    struct SaveBlock4 *block;
    struct G2_Record *record;
    if (!sIntroPending || GAME_VERSION != VERSION_EMERALD) return;
    block = UnholySave4_Get();
    if (block)
    {
        record = (void *)(block->payload + UNHOLY_SAVE4_GENESIS_OFFSET);
        *record = (struct G2_Record){G2_RECORD_MAGIC, GENESIS2_VERSION,
                                    G2_CATALOG_VERSION, sPendingSeed};
        UnholySave4_Commit();
    }
    sIntroPending = FALSE;
}

void G2_LoadWorld(void)
{
    const struct G2_Record *record = ReadRecord();
    sIntroPending = FALSE;
    if (!record)
    {
        sRuntime = NULL; sPublished = NULL; sPublishedEntry = NULL;
        gGenesis2Stats.status = 0;
        return;
    }
    BeginWorld(record->seed);
}

static u32 ScanClock(void)
{
    u32 frame, after, line;
    const volatile u32 *counter = &gMain.vblankCounter1;
    /* VBlank starts at line 160. Combine a coherent frame counter with the
     * scanline phase; VCOUNT alone would lose time across frame boundaries.
     * If interrupts are temporarily delayed, a backwards clock only causes
     * a conservative early yield. No game timer is reserved or modified. */
    do
    {
        frame = *counter;
        line = REG_VCOUNT;
        after = *counter;
    } while (frame != after);
    return frame * 228u + (line >= 160u ? line - 160u : line + 68u);
}

void G2_RuntimeTick(void)
{
    u32 before, clock, units;
    if (!sRuntime || G2_Complete(&sRuntime->world)) return;
    before = gMain.vblankCounter1;
    clock = ScanClock();
    for (units = 0; units < G2_TICK_UNITS && !G2_Complete(&sRuntime->world); ++units)
    {
        G2_Tick(&sRuntime->world, 1);
        if (ScanClock() - clock >= G2_TICK_SCANLINES) break;
    }
    ++gGenesis2Stats.ticks;
    if (gMain.vblankCounter1 - before > gGenesis2Stats.maxTickVBlanks)
        gGenesis2Stats.maxTickVBlanks = gMain.vblankCounter1 - before;
    gGenesis2Stats.mapsReady = sRuntime->world.current - sRuntime->world.failed;
    if (G2_Complete(&sRuntime->world)) gGenesis2Stats.status = sRuntime->world.failed ? 4 : 3;
}

bool32 G2_RuntimeComplete(void)
{ return !sRuntime || G2_Complete(&sRuntime->world); }

static u32 LiveIndex(const struct G2_Map *m, u32 k)
{ return (k / m->width + MAP_OFFSET) * gBackupMapLayout.width + k % m->width + MAP_OFFSET; }

static bool32 ActorInModule(const struct G2_Map *m, const struct G2_Placement *p,
                            const struct G2_Module *module)
{
    u32 i, list;
    s32 ox = p->origin % m->width, oy = p->origin / m->width;
    for (list = 0; list < 2; ++list)
        for (i = 0; i < OBJECT_EVENTS_COUNT; ++i)
        {
            const struct ObjectEvent *actor = list ? &gSaveBlock1Ptr->objectEvents[i] : &gObjectEvents[i];
            s32 x = actor->currentCoords.x - MAP_OFFSET, y = actor->currentCoords.y - MAP_OFFSET;
            if (actor->active && actor->mapNum == gSaveBlock1Ptr->location.mapNum
                && actor->mapGroup == gSaveBlock1Ptr->location.mapGroup
                && x >= ox && y >= oy && x < ox + module->width && y < oy + module->height) return TRUE;
        }
    return FALSE;
}

static void RejectModule(const struct G2_Map *m, const struct G2_Entry *e, u32 index)
{
    const struct G2_Placement *p = &e->plan.placements[index];
    const struct G2_Module *module = &m->catalog->modules[p->module];
    u32 dx,dy;
    sRejectedModules[index / 8u] |= (u8)(1u << (index % 8u));
    sPublishedModules[index / 8u] &= (u8)~(1u << (index % 8u));
    for (dy = 0; dy < module->height; ++dy)
        for (dx = 0; dx < module->width; ++dx)
        {
            u32 k = (p->origin / m->width + dy) * m->width + p->origin % m->width + dx;
            u32 live = LiveIndex(m,k);
            if (gBackupMapLayout.map[live] == e->tiles[k] && e->tiles[k] != m->native[k])
                gBackupMapLayout.map[live] = m->native[k];
        }
}

static void PublishModule(const struct G2_Map *m, const struct G2_Entry *e, u32 index, const struct MapLayout *l)
{
    const struct G2_Placement *p = &e->plan.placements[index];
    const struct G2_Module *module = &m->catalog->modules[p->module];
    u32 dx,dy;
    bool32 native = TRUE, generated = TRUE;
    s32 px = gSaveBlock1Ptr->pos.x, py = gSaveBlock1Ptr->pos.y;
    /* A native-mode save or a warp must never leave the player inside a tree. */
    if (module->kind == G2_MODULE_TREE && ((px >= (s32)(p->origin % m->width)
        && py >= (s32)(p->origin / m->width)
        && px < (s32)(p->origin % m->width + module->width)
        && py < (s32)(p->origin / m->width + module->height)) || ActorInModule(m,p,module)))
    { RejectModule(m,e,index); return; }
    for (dy = 0; dy < module->height; ++dy)
        for (dx = 0; dx < module->width; ++dx)
        {
            u32 k = (p->origin / m->width + dy) * m->width + p->origin % m->width + dx;
            u16 value = gBackupMapLayout.map[LiveIndex(m,k)];
            if (l->map[k] != m->native[k]) { native = generated = FALSE; }
            if (value != m->native[k]) native = FALSE;
            if (value != e->tiles[k]) generated = FALSE;
        }
    if (!native && !generated) { RejectModule(m,e,index); return; }
    sPublishedModules[index / 8u] |= (u8)(1u << (index % 8u));
    if (generated) return;
    /* Logical atomic publication: the whole footprint passed preflight. */
    for (dy = 0; dy < module->height; ++dy)
        for (dx = 0; dx < module->width; ++dx)
        {
            u32 k = (p->origin / m->width + dy) * m->width + p->origin % m->width + dx;
            gBackupMapLayout.map[LiveIndex(m,k)] = e->tiles[k];
            if (e->tiles[k] != m->native[k]) ++gGenesis2Stats.publishedCells;
        }
}

static s32 PublishedIndex(s32 x, s32 y)
{
    x -= MAP_OFFSET; y -= MAP_OFFSET;
    if (!sPublished || !sPublishedEntry || sPublished->layoutId != gMapHeader.mapLayoutId
        || x < 0 || y < 0 || x >= sPublished->width || y >= sPublished->height) return -1;
    return y * sPublished->width + x;
}

void G2_PrepareSavedView(s32 x, s32 y, const u16 *view, u32 width, u32 height)
{
    u32 dx,dy;
    if (!sPublished || !sPublishedEntry || !sPublishedEntry->plan.owner || !view
        || width > MAP_OFFSET_W || height > MAP_OFFSET_H) return;
    for (dy = 0; dy < height; ++dy)
        for (dx = 0; dx < width; ++dx)
        {
            s32 k = PublishedIndex(x + (s32)dx,y + (s32)dy);
            if (k >= 0 && sPublishedEntry->plan.owner[k]
                && view[dy * width + dx] != sPublishedEntry->tiles[k])
                RejectModule(sPublished,sPublishedEntry,sPublishedEntry->plan.owner[k] - 1u);
        }
}

bool32 G2_ShouldRestoreSavedCell(s32 x, s32 y, u16 value)
{
    s32 k = PublishedIndex(x,y);
    if (k >= 0 && gSaveBlock2Ptr->optionsGenesisTerrain == GENESIS_TERRAIN_NATIVE
        && value == sPublishedEntry->tiles[k] && value != sPublished->native[k]) return FALSE;
    if (k >= 0 && sPublishedEntry->plan.owner && sPublishedEntry->plan.owner[k])
    {
        u32 index = sPublishedEntry->plan.owner[k] - 1u;
        if ((sRejectedModules[index / 8u] & (1u << (index % 8u)))
            && value == sPublishedEntry->tiles[k] && value != sPublished->native[k]) return FALSE;
    }
    return TRUE;
}

void G2_ReconcileCurrentModules(void)
{
    u32 i;
    if (!sPublished || !sPublishedEntry || !sPublishedEntry->plan.placementCount
        || sPublished->layoutId != gMapHeader.mapLayoutId) return;
    for (i = 0; i < sPublishedEntry->plan.placementCount; ++i)
    {
        const struct G2_Placement *p = &sPublishedEntry->plan.placements[i];
        const struct G2_Module *module = &sPublished->catalog->modules[p->module];
        u32 x,y;
        bool32 intact = TRUE;
        if (!(sPublishedModules[i / 8u] & (1u << (i % 8u)))) continue;
        if (module->kind == G2_MODULE_TREE && ActorInModule(sPublished,p,module)) intact = FALSE;
        for (y = 0; y < module->height; ++y)
            for (x = 0; x < module->width; ++x)
            {
                u32 k = (p->origin / sPublished->width + y) * sPublished->width + p->origin % sPublished->width + x;
                if (gBackupMapLayout.map[LiveIndex(sPublished,k)] != sPublishedEntry->tiles[k]) intact = FALSE;
            }
        if (!intact) RejectModule(sPublished,sPublishedEntry,i);
    }
}

bool32 G2_ApplyCurrentMap(void)
{
    const struct MapLayout *l = gMapHeader.mapLayout;
    const struct G2_Entry *entry;
    const struct G2_Map *m = NULL;
    u32 i, before;
    u16 mapId = ((u16)gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    sPublished = NULL; sPublishedEntry = NULL;
    memset(sPublishedModules,0,sizeof(sPublishedModules));
    memset(sRejectedModules,0,sizeof(sRejectedModules));
    if (!sRuntime || !l || !gBackupMapLayout.map
        || gBackupMapLayout.width != l->width + MAP_OFFSET_W
        || gBackupMapLayout.height != l->height + MAP_OFFSET_H
        || (u32)gBackupMapLayout.width * gBackupMapLayout.height > MAX_MAP_DATA_SIZE)
        return FALSE;
    if (mapId == MAP_ROUTE101 && !FlagGet(FLAG_RESCUED_BIRCH)) return FALSE;
    entry = G2_FindReady(&sRuntime->world, mapId, gMapHeader.mapLayoutId, l->width, l->height);
    if (!entry) { ++gGenesis2Stats.cacheMisses; return FALSE; }
    for (i = 0; i < ARRAY_COUNT(sG2Maps); ++i)
        if (sG2Maps[i].mapId == mapId) { m = &sG2Maps[i]; break; }
    if (!m) return FALSE;
    ++gGenesis2Stats.cacheHits;
    if (gSaveBlock2Ptr->optionsGenesisTerrain == GENESIS_TERRAIN_NATIVE)
    {
        sPublished = m; sPublishedEntry = entry;
        for (i = 0; i < entry->plan.placementCount; ++i) RejectModule(m,entry,i);
        return TRUE;
    }
    before = gMain.vblankCounter1;
    /* No noise, CRC, allocation, event scans or persistence on map entry.
     * Native script edits survive because publication compares live cells. */
    for (i = 0; i < entry->plan.placementCount; ++i) PublishModule(m,entry,i,l);
    for (i = 0; i < entry->changed; ++i)
    {
        u32 k = entry->edits[i].sourceIndex;
        u32 live = entry->edits[i].liveIndex;
        if (entry->plan.owner && entry->plan.owner[k]) continue;
        if (l->map[k] == m->native[k] && gBackupMapLayout.map[live] == m->native[k])
        { gBackupMapLayout.map[live] = entry->tiles[k]; ++gGenesis2Stats.publishedCells; }
    }
    sPublished = m; sPublishedEntry = entry;
    if (gMain.vblankCounter1 - before > gGenesis2Stats.maxPublishVBlanks)
        gGenesis2Stats.maxPublishVBlanks = gMain.vblankCounter1 - before;
    return TRUE;
}

bool32 G2_IsGeneratedCell(s32 x, s32 y)
{
    s32 k = PublishedIndex(x,y);
    if (k < 0) return FALSE;
    if (gSaveBlock2Ptr->optionsGenesisTerrain == GENESIS_TERRAIN_NATIVE) return FALSE;
    if (sPublishedEntry->plan.owner && sPublishedEntry->plan.owner[k])
    {
        u32 index = sPublishedEntry->plan.owner[k] - 1u;
        return (sPublishedModules[index / 8u] >> (index % 8u)) & 1u;
    }
    return sPublishedEntry->tiles[k] != sPublished->native[k];
}
