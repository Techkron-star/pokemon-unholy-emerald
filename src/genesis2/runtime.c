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
    gGenesis2Stats.mapsReady = sRuntime->world.current;
    if (G2_Complete(&sRuntime->world)) gGenesis2Stats.status = sRuntime->world.failed ? 4 : 3;
}

bool32 G2_RuntimeComplete(void)
{ return !sRuntime || G2_Complete(&sRuntime->world); }

bool32 G2_ApplyCurrentMap(void)
{
    const struct MapLayout *l = gMapHeader.mapLayout;
    const struct G2_Entry *entry;
    const struct G2_Map *m = NULL;
    u32 i, before;
    u16 mapId = ((u16)gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    sPublished = NULL; sPublishedEntry = NULL;
    if (!sRuntime || !l || !gBackupMapLayout.map
        || gBackupMapLayout.width != l->width + MAP_OFFSET_W
        || gBackupMapLayout.height != l->height + MAP_OFFSET_H
        || (u32)gBackupMapLayout.width * gBackupMapLayout.height > MAX_MAP_DATA_SIZE)
        return FALSE;
    if (gSaveBlock2Ptr->optionsGenesisTerrain == GENESIS_TERRAIN_NATIVE
        || (mapId == MAP_ROUTE101 && !FlagGet(FLAG_RESCUED_BIRCH))) return FALSE;
    entry = G2_FindReady(&sRuntime->world, mapId, gMapHeader.mapLayoutId, l->width, l->height);
    if (!entry) { ++gGenesis2Stats.cacheMisses; return FALSE; }
    for (i = 0; i < ARRAY_COUNT(sG2Maps); ++i)
        if (sG2Maps[i].mapId == mapId) { m = &sG2Maps[i]; break; }
    if (!m) return FALSE;
    ++gGenesis2Stats.cacheHits;
    before = gMain.vblankCounter1;
    /* No noise, CRC, allocation, event scans or persistence on map entry.
     * Native script edits survive because publication compares live cells. */
    for (i = 0; i < entry->changed; ++i)
    {
        u32 k = entry->edits[i].sourceIndex;
        u32 live = entry->edits[i].liveIndex;
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
    u32 k;
    x -= MAP_OFFSET; y -= MAP_OFFSET;
    if (!sPublished || !sPublishedEntry || sPublished->layoutId != gMapHeader.mapLayoutId
        || x < 0 || y < 0 || x >= sPublished->width || y >= sPublished->height) return FALSE;
    k = (u32)y * sPublished->width + (u32)x;
    return sPublishedEntry->tiles[k] != sPublished->native[k];
}
