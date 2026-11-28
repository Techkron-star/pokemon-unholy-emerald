#include "genesis2/world.h"
#include <string.h>

/* Unsigned arithmetic deliberately wraps, signed arithmetic never does. */
uint32_t G2_Hash(uint32_t seed, int32_t x, int32_t y)
{
    uint32_t h = seed ^ (uint32_t)x * UINT32_C(0x9E3779B1)
                      ^ (uint32_t)y * UINT32_C(0x85EBCA77);
    h ^= h >> 16; h *= UINT32_C(0x7FEB352D);
    h ^= h >> 15; h *= UINT32_C(0x846CA68B);
    return h ^ (h >> 16);
}

static int32_t FloorGrid(int32_t x, uint32_t step)
{
    int32_t q = x / (int32_t)step;
    return q - (x % (int32_t)step < 0);
}

uint8_t G2_Climate(uint32_t seed, int32_t x, int32_t y, unsigned shift)
{
    uint32_t step, fx, fy, a, b, c, d, top, bottom;
    int32_t gx, gy;
    if (shift > 12) shift = 12;
    step = 1u << shift;
    gx = FloorGrid(x, step); gy = FloorGrid(y, step);
    /* Remainders without signed left shifts or potentially overflowing products. */
    fx = (uint32_t)x & (step - 1u); fy = (uint32_t)y & (step - 1u);
    a = G2_Hash(seed, gx, gy) & 255u;
    b = G2_Hash(seed, (int32_t)((uint32_t)gx + 1u), gy) & 255u;
    c = G2_Hash(seed, gx, (int32_t)((uint32_t)gy + 1u)) & 255u;
    d = G2_Hash(seed, (int32_t)((uint32_t)gx + 1u), (int32_t)((uint32_t)gy + 1u)) & 255u;
    top = (a * (step - fx) + b * fx) >> shift;
    bottom = (c * (step - fx) + d * fx) >> shift;
    return (uint8_t)((top * (step - fy) + bottom * fy) >> shift);
}

int G2_MapValid(const struct G2_Map *m)
{
    return m && m->native && m->editable && m->width >= 8 && m->height >= 8
        && (uint32_t)(m->width + G2_MAP_OFFSET_W)
          * (uint32_t)(m->height + G2_MAP_OFFSET_H) <= G2_MAX_NATIVE_CELLS
        && m->worldX >= -1048576 && m->worldX <= 1048576
        && m->worldY >= -1048576 && m->worldY <= 1048576;
}

size_t G2_StorageBytes(const struct G2_Map *maps, uint32_t count)
{
    size_t total = 0;
    uint32_t i;
    if (!maps || !count) return 0;
    for (i = 0; i < count; ++i)
    {
        size_t n;
        if (!G2_MapValid(&maps[i])) return 0;
        n = (size_t)maps[i].width * maps[i].height
            * (sizeof(uint16_t) + sizeof(struct G2_Edit));
        if (n > SIZE_MAX - total) return 0;
        total += n;
    }
    return total;
}

static uint32_t CrcByte(uint32_t crc, uint8_t value)
{
    unsigned i;
    crc ^= value;
    for (i = 0; i < 8; ++i)
        crc = (crc >> 1) ^ (UINT32_C(0xEDB88320) & (0u - (crc & 1u)));
    return crc;
}
static uint32_t CrcWord(uint32_t crc, uint32_t value)
{
    unsigned i;
    for (i = 0; i < 4; ++i) { crc = CrcByte(crc, (uint8_t)value); value >>= 8; }
    return crc;
}
static uint32_t Fingerprint(const struct G2_Map *m, uint32_t seed)
{
    uint32_t crc = UINT32_MAX;
    crc = CrcWord(crc, GENESIS2_VERSION);
    crc = CrcWord(crc, G2_CATALOG_VERSION);
    crc = CrcWord(crc, seed);
    crc = CrcWord(crc, m->mapId);
    crc = CrcWord(crc, m->layoutId);
    crc = CrcWord(crc, (uint32_t)m->width | ((uint32_t)m->height << 16));
    crc = CrcWord(crc, (uint32_t)m->worldX);
    crc = CrcWord(crc, (uint32_t)m->worldY);
    crc = CrcWord(crc, m->domain);
    return ~CrcWord(crc, m->assetHash);
}

int G2_Begin(struct G2_World *w, const struct G2_Map *maps, uint32_t count,
             struct G2_Entry *entries, void *storage, size_t bytes, uint32_t seed)
{
    size_t needed = G2_StorageBytes(maps, count), offset = 0;
    uint32_t i;
    if (!w) return 0;
    memset(w, 0, sizeof(*w));
    if (!needed || !entries || !storage || bytes < needed
        || (uintptr_t)storage % sizeof(uint16_t)) return 0;
    w->maps = maps; w->entries = entries; w->seed = seed;
    w->version = GENESIS2_VERSION; w->catalogVersion = G2_CATALOG_VERSION;
    w->mapCount = count;
    for (i = 0; i < count; ++i)
    {
        struct G2_Entry *e = &entries[i];
        memset(e, 0, sizeof(*e));
        e->tiles = (uint16_t *)((uint8_t *)storage + offset);
        e->edits = (struct G2_Edit *)(e->tiles + (size_t)maps[i].width * maps[i].height);
        e->fingerprint = Fingerprint(&maps[i], seed);
        e->phase = G2_QUEUED;
        offset += (size_t)maps[i].width * maps[i].height
            * (sizeof(uint16_t) + sizeof(struct G2_Edit));
    }
    return 1;
}

uint16_t G2_RenderCell(const struct G2_Map *m, uint32_t seed, uint32_t k)
{
    uint32_t x, y;
    uint16_t old = m->native[k], tile = old & G2_ID_MASK, target = tile;
    int32_t wx, wy;
    uint32_t climateSeed = seed ^ m->domain;
    uint32_t wet, hot, vegetation, dry, track, along, across, a, b, center;
    if (!(m->editable[k / 8u] & (1u << (k % 8u)))) return old;
    if ((old & G2_COLLISION_MASK) || (tile != G2_GRASS && tile != G2_TALL)) return old;
    x = k % m->width; y = k / m->width;
    wx = m->worldX + (int32_t)x; wy = m->worldY + (int32_t)y;
    /* Global coordinate fields, independent streams. Wide wavelengths allow
     * homogeneous routes; continuous local fields create gradual ecotones. */
    wet = G2_Climate(climateSeed ^ UINT32_C(0x48554D49), wx, wy, 7);
    hot = G2_Climate(climateSeed ^ UINT32_C(0x54454D50), wx, wy, 8);
    vegetation = G2_Climate(climateSeed ^ UINT32_C(0x56454745), wx, wy, 4);
    dry = hot > wet ? hot - wet : 0;
    /* Different route widths/clearings, while native authored obstacles,
     * directional movement, HM gates and all protected cells stay exact. */
    along = m->width >= m->height ? x : y;
    across = m->width >= m->height ? y : x;
    a = 4u + G2_Hash(seed ^ m->mapId, (int32_t)(across / 16u), (int32_t)(along / 8u)) % 7u;
    b = 4u + G2_Hash(seed ^ m->mapId, (int32_t)(across / 16u), (int32_t)(along / 8u + 1u)) % 7u;
    center = (a * (8u - along % 8u) + b * (along % 8u)) / 8u;
    track = across % 16u;
    if (tile == G2_TALL)
    {
        if (track == center || track == center + 1u || vegetation + wet < 180u)
            target = G2_GRASS;
        /* Keep original encounter patches in wet regions and always retain
         * a deterministic portion in dry ones, away from paths. */
        else if (dry > 48u && vegetation < dry)
            target = G2_SAND;
    }
    else if (dry > 48u && vegetation < dry + 32u)
        target = G2_SAND;
    return (uint16_t)((old & (uint16_t)~G2_ID_MASK) | target);
}

static int CellValid(const struct G2_Map *m, uint32_t k, uint16_t value)
{
    uint16_t old = m->native[k];
    uint16_t id = value & G2_ID_MASK;
    if (value == old) return 1;
    /* Exact movement-equivalence proof for this catalog: only passable
     * normal floors/encounter grass, identical collision and elevation.
     * Water, bridges, ledges, bike, scripted terrain never enter the set. */
    return (m->editable[k / 8u] & (1u << (k % 8u)))
        && !(old & G2_COLLISION_MASK)
        && ((old ^ value) & (uint16_t)~G2_ID_MASK) == 0
        && ((old & G2_ID_MASK) == G2_GRASS || (old & G2_ID_MASK) == G2_TALL)
        && (id == G2_GRASS || id == G2_TALL || id == G2_SAND);
}

uint32_t G2_Tick(struct G2_World *w, uint32_t budget)
{
    uint32_t used = 0, credits = 0, limit;
    if (!w || !w->maps || !w->entries || !budget) return 0;
    if (budget > UINT32_MAX / 16u) budget = UINT32_MAX / 16u;
    limit = budget * 16u;
    while (credits < limit && w->current < w->mapCount)
    {
        const struct G2_Map *m = &w->maps[w->current];
        struct G2_Entry *e = &w->entries[w->current];
        uint32_t cells = (uint32_t)m->width * m->height, k = e->cursor;
        uint32_t cost;
        if (e->phase == G2_QUEUED) { e->phase = G2_RENDER; e->cursor = k = 0; }
        /* Weighted deterministic budget: protected copies are much cheaper
         * than climate sampling. Never waste an entire frame on 32 copies.
         * One unit = 16 credits; native measurements calibrate the weights. */
        cost = e->phase == G2_RENDER
            ? ((m->editable[k / 8u] & (1u << (k % 8u))) ? 16u : 4u)
            : e->phase == G2_VALIDATE ? 8u : 4u;
        if (cost > limit - credits) break;
        if (e->phase == G2_RENDER)
        {
            e->tiles[k] = G2_RenderCell(m, w->seed, k);
            if (e->tiles[k] != m->native[k])
            {
                struct G2_Edit *edit = &e->edits[e->changed++];
                edit->sourceIndex = (uint16_t)k;
                edit->liveIndex = (uint16_t)((k / m->width + 7u)
                    * (m->width + G2_MAP_OFFSET_W) + k % m->width + 7u);
            }
            if (++e->cursor == cells)
            { e->phase = G2_VALIDATE; e->cursor = 0; e->crc = UINT32_MAX; }
        }
        else if (e->phase == G2_VALIDATE)
        {
            uint16_t value = e->tiles[k];
            if (!CellValid(m, k, value))
            { e->phase = G2_FAILED; ++w->failed; ++w->current; }
            else
            {
                e->crc = CrcByte(CrcByte(e->crc, (uint8_t)value), (uint8_t)(value >> 8));
                if (++e->cursor == cells)
                { e->crc = ~e->crc; e->phase = G2_READY; ++w->current; }
            }
        }
        else ++w->current;
        credits += cost;
    }
    used = (credits + 15u) / 16u;
    ++w->ticks; w->workUnits += used;
    if (used > w->maxTickUnits) w->maxTickUnits = used;
    return used;
}

int G2_Complete(const struct G2_World *w)
{ return w && w->maps && w->current == w->mapCount; }

const struct G2_Entry *G2_FindReady(const struct G2_World *w, uint16_t mapId,
                                  uint16_t layoutId, uint16_t width, uint16_t height)
{
    uint32_t i;
    if (!w || !w->maps || w->version != GENESIS2_VERSION
        || w->catalogVersion != G2_CATALOG_VERSION) return NULL;
    for (i = 0; i < w->mapCount; ++i)
    {
        const struct G2_Map *m = &w->maps[i];
        if (m->mapId == mapId && m->layoutId == layoutId && m->width == width && m->height == height
            && w->entries[i].phase == G2_READY) return &w->entries[i];
    }
    return NULL;
}
