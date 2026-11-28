#include "genesis2/world.h"

#define G2_DONE_NODE UINT16_C(0xFFFE)

int G2_HasTopology(const struct G2_Map *m)
{
    return m && m->topology && m->catalog && m->catalog->modules && m->catalog->count;
}

static int Soft(const struct G2_Map *m, uint32_t k)
{
    uint16_t v = m->native[k], id = v & G2_ID_MASK;
    return (m->topology[k / 8u] & (1u << (k % 8u)))
        && (m->editable[k / 8u] & (1u << (k % 8u)))
        && !(v & G2_COLLISION_MASK) && (v & 0xF000u) == 0x3000u
        && (id == G2_GRASS || id == G2_TALL);
}

static uint16_t Neighbor(const struct G2_Map *m, uint32_t k, unsigned d)
{
    uint32_t x = k % m->width, y = k / m->width;
    if (d == 0 && x) return (uint16_t)(k - 1u);
    if (d == 1 && x + 1u < m->width) return (uint16_t)(k + 1u);
    if (d == 2 && y) return (uint16_t)(k - m->width);
    if (d == 3 && y + 1u < m->height) return (uint16_t)(k + m->width);
    return G2_NO_NODE;
}

static int Less(const struct G2_Work *s, uint16_t a, uint16_t b)
{
    return s->distance[a] < s->distance[b]
        || (s->distance[a] == s->distance[b] && a < b);
}

static void Swap(struct G2_Work *s, uint32_t a, uint32_t b)
{
    uint16_t v = s->heap[a];
    s->heap[a] = s->heap[b]; s->heap[b] = v;
    s->position[s->heap[a]] = (uint16_t)a;
    s->position[s->heap[b]] = (uint16_t)b;
}

static int Push(struct G2_Work *s, uint16_t node)
{
    uint32_t p = s->position[node];
    if (p == G2_DONE_NODE) return 1;
    if (p == G2_NO_NODE)
    {
        if (s->heapCount >= s->capacity) return 0;
        p = s->heapCount++;
        s->heap[p] = node; s->position[node] = (uint16_t)p;
    }
    while (p)
    {
        uint32_t parent = (p - 1u) / 2u;
        if (!Less(s, s->heap[p], s->heap[parent])) break;
        Swap(s, p, parent); p = parent;
    }
    return 1;
}

static uint16_t Pop(struct G2_Work *s)
{
    uint16_t node = s->heap[0];
    uint32_t p = 0;
    s->position[node] = G2_DONE_NODE;
    if (!--s->heapCount) return node;
    s->heap[0] = s->heap[s->heapCount]; s->position[s->heap[0]] = 0;
    for (;;)
    {
        uint32_t left = p * 2u + 1u, best = left;
        if (left >= s->heapCount) break;
        if (left + 1u < s->heapCount && Less(s, s->heap[left + 1u], s->heap[left])) ++best;
        if (!Less(s, s->heap[best], s->heap[p])) break;
        Swap(s, p, best); p = best;
    }
    return node;
}

static void Fallback(struct G2_Entry *e, struct G2_Work *s)
{
    e->plan.fallback = 1; e->plan.placementCount = 0;
    s->cursor = 0; e->phase = G2_PLAN_FALLBACK;
}

static int ModuleValid(const struct G2_Module *m)
{
    uint32_t i, n = (uint32_t)m->width * m->height;
    if (!m->width || !m->height || n < 4 || n > G2_MODULE_MAX_CELLS
        || (m->kind != G2_MODULE_TREE && m->kind != G2_MODULE_CLEARING)) return 0;
    for (i = 0; i < n; ++i)
    {
        uint16_t v = m->cells[i];
        if ((v & 0xF000u) != 0x3000u || (v & G2_ID_MASK) >= 512u) return 0;
        if (m->kind == G2_MODULE_TREE && (v & G2_COLLISION_MASK) != 0x0400u) return 0;
        if (m->kind == G2_MODULE_CLEARING && (v & (G2_ID_MASK | G2_COLLISION_MASK)) != G2_GRASS) return 0;
    }
    return 1;
}

static int CanPlace(const struct G2_Map *m, const struct G2_Entry *e,
                    uint32_t origin, const struct G2_Module *module)
{
    uint32_t x = origin % m->width, y = origin / m->width, dx, dy;
    if (!ModuleValid(module) || x + module->width > m->width
        || y + module->height > m->height) return 0;
    for (dy = 0; dy < module->height; ++dy)
        for (dx = 0; dx < module->width; ++dx)
        {
            uint32_t k = (y + dy) * m->width + x + dx;
            if (!(e->plan.flags[k] & G2_SOFT) || e->plan.owner[k] || k == e->encounterAnchor) return 0;
            /* This catalog has a grass base. A dry/sandy biome needs its own
             * reviewed art, not a green-backed tree stamped onto sand. */
            if ((e->tiles[k] & G2_ID_MASK) != G2_GRASS && (e->tiles[k] & G2_ID_MASK) != G2_TALL) return 0;
            if (module->kind == G2_MODULE_TREE
                && (e->plan.flags[k] & (G2_PORT | G2_CORRIDOR))) return 0;
        }
    /* Separate generated footprints by one metatile. Native complete forest
     * art outside the editable footprint is never erased or reconstructed. */
    for (dy = y ? y - 1u : 0; dy <= y + module->height && dy < m->height; ++dy)
        for (dx = x ? x - 1u : 0; dx <= x + module->width && dx < m->width; ++dx)
            if (e->plan.owner[dy * m->width + dx]) return 0;
    return 1;
}

int G2_ModuleCellValid(const struct G2_Map *m, const struct G2_Entry *e,
                       uint32_t k, uint16_t value)
{
    uint32_t owner, x, y, ox, oy;
    const struct G2_Placement *p;
    const struct G2_Module *module;
    if (!G2_HasTopology(m) || !e->plan.owner || !e->plan.owner[k]) return 0;
    owner = e->plan.owner[k] - 1u;
    if (owner >= e->plan.placementCount) return 0;
    p = &e->plan.placements[owner];
    if (p->module >= m->catalog->count || p->origin >= (uint32_t)m->width * m->height) return 0;
    module = &m->catalog->modules[p->module];
    x = k % m->width; y = k / m->width;
    ox = p->origin % m->width; oy = p->origin / m->width;
    return ModuleValid(module) && Soft(m,k) && x >= ox && y >= oy
        && x - ox < module->width && y - oy < module->height
        && value == module->cells[(y - oy) * module->width + x - ox]
        && (module->kind != G2_MODULE_TREE || !(e->plan.flags[k] & (G2_PORT | G2_CORRIDOR)));
}

/* Every call performs one cell, one heap pop/4 relaxations, one parent step,
 * or one complete <=36-cell module. No unbounded flood or retry loop. */
void G2_PlanStep(struct G2_World *w, struct G2_Entry *e, const struct G2_Map *m)
{
    struct G2_Work *s = &w->work;
    struct G2_Plan *p = &e->plan;
    uint32_t n = (uint32_t)m->width * m->height, k = s->cursor;
    unsigned d;
    if (e->phase == G2_PLAN_INIT)
    {
        uint8_t flags = 0;
        if (Soft(m,k))
        {
            flags = G2_SOFT; ++p->softCells;
            for (d = 0; d < 4; ++d)
            {
                uint16_t j = Neighbor(m,k,d);
                if (j == G2_NO_NODE || (!Soft(m,j) && !(m->native[j] & G2_COLLISION_MASK)))
                    flags |= G2_PORT;
            }
        }
        p->flags[k] = flags; p->region[k] = 0; p->parent[k] = G2_NO_NODE; p->owner[k] = 0;
        s->distance[k] = UINT32_MAX; s->position[k] = G2_NO_NODE;
        if (++s->cursor == n)
        {
            s->scan = 0; s->head = s->tail = 0; s->floodRegion = 0;
            e->phase = G2_PLAN_FLOOD;
        }
    }
    else if (e->phase == G2_PLAN_FLOOD)
    {
        if (s->head < s->tail)
        {
            uint32_t hash;
            k = s->queue[s->head++];
            hash = G2_Hash(w->seed ^ m->mapId ^ UINT32_C(0x524F4F54), (int32_t)k, 0);
            if (((p->flags[k] & G2_PORT) && !s->rootIsPort)
                || (((p->flags[k] & G2_PORT) != 0) == s->rootIsPort && hash < s->rootHash))
            { s->root = (uint16_t)k; s->rootHash = hash; s->rootIsPort = (p->flags[k] & G2_PORT) != 0; }
            for (d = 0; d < 4; ++d)
            {
                uint16_t j = Neighbor(m,k,d);
                if (j != G2_NO_NODE && (p->flags[j] & G2_SOFT) && !p->region[j])
                {
                    if (s->tail >= s->capacity) { Fallback(e,s); return; }
                    p->region[j] = s->floodRegion; s->queue[s->tail++] = j;
                }
            }
        }
        else if (s->scan < n)
        {
            if (s->floodRegion) s->roots[s->floodRegion - 1u] = s->root;
            k = s->scan++;
            if ((p->flags[k] & G2_SOFT) && !p->region[k])
            {
                if (p->regions >= n) { Fallback(e,s); return; }
                s->floodRegion = ++p->regions;
                s->root = (uint16_t)k; s->rootHash = UINT32_MAX; s->rootIsPort = 0;
                s->head = 0; s->tail = 1; s->queue[0] = (uint16_t)k; p->region[k] = s->floodRegion;
            }
        }
        else
        {
            if (s->floodRegion) s->roots[s->floodRegion - 1u] = s->root;
            s->cursor = 0; s->heapCount = 0; e->phase = G2_PLAN_ROOTS;
        }
    }
    else if (e->phase == G2_PLAN_ROOTS)
    {
        if (k < p->regions)
        {
            uint16_t root = s->roots[k];
            s->distance[root] = 0; p->parent[root] = root;
            if (!Push(s,root)) { Fallback(e,s); return; }
            ++s->cursor;
        }
        else e->phase = G2_PLAN_PATHS;
    }
    else if (e->phase == G2_PLAN_PATHS)
    {
        if (s->heapCount)
        {
            k = Pop(s);
            for (d = 0; d < 4; ++d)
            {
                uint16_t j = Neighbor(m,k,d);
                uint32_t cost, next, hash;
                if (j == G2_NO_NODE || !(p->flags[j] & G2_SOFT) || p->region[k] != p->region[j]) continue;
                hash = G2_Hash(w->seed ^ m->mapId ^ UINT32_C(0x50415448), j % m->width, j / m->width);
                /* Three bounded spatial grammars: clearings, winding woodland
                 * and alternating lanes. Geometry remains inside each domain. */
                cost = p->grammar == 0 ? 8u + (hash & 3u)
                     : p->grammar == 1 ? 3u + (hash & 31u)
                     : 5u + (hash & 7u) + (((j / m->width) / 3u) % 2u) * 12u;
                next = s->distance[k] + cost;
                if (next < s->distance[j])
                {
                    s->distance[j] = next; p->parent[j] = (uint16_t)k;
                    if (!Push(s,j)) { Fallback(e,s); return; }
                }
            }
        }
        else { s->scan = 0; s->trace = G2_NO_NODE; e->phase = G2_PLAN_TRACE; }
    }
    else if (e->phase == G2_PLAN_TRACE)
    {
        if (s->trace != G2_NO_NODE)
        {
            uint16_t parent;
            k = s->trace;
            if (p->flags[k] & G2_CORRIDOR) { s->trace = G2_NO_NODE; return; }
            p->flags[k] |= G2_CORRIDOR;
            parent = p->parent[k];
            if (parent == G2_NO_NODE || ++s->traceSteps > n) { Fallback(e,s); return; }
            s->trace = parent == k ? G2_NO_NODE : parent;
        }
        else if (s->scan < n)
        {
            k = s->scan++;
            if (p->flags[k] & G2_PORT) { s->trace = (uint16_t)k; s->traceSteps = 0; }
        }
        else { s->cursor = 0; e->phase = G2_PLAN_PAINT; }
    }
    else if (e->phase == G2_PLAN_PAINT)
    {
        /* A root with no external ports is still a reserved reachable clearing. */
        if ((p->flags[k] & G2_SOFT) && p->parent[k] == k) p->flags[k] |= G2_CORRIDOR;
        if (p->flags[k] & G2_CORRIDOR)
        {
            if (k != e->encounterAnchor && (e->tiles[k] & G2_ID_MASK) != G2_SAND)
                e->tiles[k] = (uint16_t)((m->native[k] & 0xFC00u) | G2_GRASS);
            ++p->corridorCells;
        }
        if (++s->cursor == n) { s->cursor = 0; e->phase = G2_PLAN_COMPOSE; }
    }
    else if (e->phase == G2_PLAN_COMPOSE)
    {
        uint32_t hash = G2_Hash(w->seed ^ m->mapId ^ UINT32_C(0x4D4F4455), (int32_t)k, 0);
        uint32_t threshold = p->grammar == 0 ? 64u : p->grammar == 1 ? 220u : 150u;
        const struct G2_Module *module = NULL;
        uint16_t moduleIndex = 0;
        /* At most one scan of a bounded validated catalog, never WFC/backtracking. */
        uint32_t j;
        for (j = 0; j < m->catalog->count; ++j)
        {
            uint16_t index = (uint16_t)((hash + j) % m->catalog->count);
            if (CanPlace(m,e,k,&m->catalog->modules[index]))
            { module = &m->catalog->modules[index]; moduleIndex = index; break; }
        }
        if (module && (hash & 255u) < threshold)
        {
            uint32_t dx, dy;
            uint16_t owner;
            if (p->placementCount >= p->placementCapacity) { Fallback(e,s); return; }
            p->placements[p->placementCount] = (struct G2_Placement){(uint16_t)k,moduleIndex};
            owner = ++p->placementCount;
            for (dy = 0; dy < module->height; ++dy)
                for (dx = 0; dx < module->width; ++dx)
                {
                    uint32_t target = (k / m->width + dy) * m->width + k % m->width + dx;
                    e->tiles[target] = module->cells[dy * module->width + dx];
                    p->owner[target] = owner; p->flags[target] |= G2_MODULE;
                }
        }
        if (++s->cursor == n) { s->cursor = 0; e->phase = G2_PLAN_NAV_INIT; }
    }
    else if (e->phase == G2_PLAN_NAV_INIT)
    {
        s->seen[k] = 0;
        if (++s->cursor == n)
        { s->scan = 0; s->head = s->tail = 0; s->cursor = 0; e->phase = G2_PLAN_NAV; }
    }
    else if (e->phase == G2_PLAN_NAV)
    {
        if (s->head < s->tail)
        {
            k = s->queue[s->head++];
            for (d = 0; d < 4; ++d)
            {
                uint16_t j = Neighbor(m,k,d);
                if (j != G2_NO_NODE && p->region[j] == p->region[k]
                    && !s->seen[j] && !(e->tiles[j] & G2_COLLISION_MASK))
                {
                    if (s->tail >= s->capacity) { Fallback(e,s); return; }
                    s->seen[j] = 1; s->queue[s->tail++] = j;
                }
            }
        }
        else if (s->scan < p->regions)
        {
            uint16_t root = s->roots[s->scan++];
            s->head = 0; s->tail = 1; s->queue[0] = root; s->seen[root] = 1;
        }
        else { s->cursor = 0; e->phase = G2_PLAN_NAV_CHECK; }
    }
    else if (e->phase == G2_PLAN_NAV_CHECK)
    {
        if ((p->flags[k] & G2_SOFT)
            && (((p->flags[k] & G2_PORT) && (e->tiles[k] & G2_COLLISION_MASK))
                || (!(e->tiles[k] & G2_COLLISION_MASK) && !s->seen[k])))
        {
            if (!p->placementCount) { Fallback(e,s); return; }
            s->cursor = 0; e->phase = G2_PLAN_REPAIR;
        }
        else if (++s->cursor == n)
        { s->cursor = 0; e->phase = G2_PLAN_MODULE_CHECK; }
    }
    else if (e->phase == G2_PLAN_MODULE_CHECK)
    {
        if (k < p->placementCount)
        {
            const struct G2_Placement *place = &p->placements[k];
            const struct G2_Module *module;
            uint32_t dx,dy;
            if (place->module >= m->catalog->count) { e->phase = G2_FAILED; return; }
            module = &m->catalog->modules[place->module];
            if (!ModuleValid(module) || place->origin % m->width + module->width > m->width
                || place->origin / m->width + module->height > m->height)
            { e->phase = G2_FAILED; return; }
            for (dy = 0; dy < module->height; ++dy)
                for (dx = 0; dx < module->width; ++dx)
                {
                    uint32_t target = (place->origin / m->width + dy) * m->width + place->origin % m->width + dx;
                    if (p->owner[target] != k + 1u || !(p->flags[target] & G2_MODULE)
                        || !G2_ModuleCellValid(m,e,target,e->tiles[target]))
                    { e->phase = G2_FAILED; return; }
                }
            ++s->cursor;
        }
        else
        { e->phase = G2_VALIDATE; e->cursor = 0; e->crc = UINT32_MAX; e->changed = 0; p->digest = UINT32_C(2166136261); }
    }
    else if (e->phase == G2_PLAN_REPAIR)
    {
        const struct G2_Placement *place = &p->placements[p->placementCount - 1u];
        const struct G2_Module *module = &m->catalog->modules[place->module];
        uint32_t target = (place->origin / m->width + k / module->width) * m->width
                          + place->origin % m->width + k % module->width;
        e->tiles[target] = G2_RenderCell(m,w->seed,target);
        if ((p->flags[target] & G2_CORRIDOR) && (e->tiles[target] & G2_ID_MASK) != G2_SAND)
            e->tiles[target] = (uint16_t)((m->native[target] & 0xFC00u) | G2_GRASS);
        p->owner[target] = 0; p->flags[target] &= (uint8_t)~G2_MODULE;
        if (++s->cursor == (uint32_t)module->width * module->height)
        { --p->placementCount; ++p->removedModules; s->cursor = 0; e->phase = G2_PLAN_NAV_INIT; }
    }
    else if (e->phase == G2_PLAN_FALLBACK)
    {
        e->tiles[k] = G2_RenderCell(m,w->seed,k);
        if (k == e->encounterAnchor) e->tiles[k] = m->native[k];
        p->flags[k] = 0; p->owner[k] = 0; p->region[k] = 0; p->parent[k] = G2_NO_NODE;
        if (++s->cursor == n)
        { e->phase = G2_VALIDATE; e->cursor = 0; e->crc = UINT32_MAX; e->changed = 0; p->digest = UINT32_C(2166136261); }
    }
}
