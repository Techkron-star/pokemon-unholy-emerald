#include "genesis_free_test.h"

u16 GenesisFree_ChunkExtent(u16 remaining)
{
    if (remaining < GENESIS_MIN_DIM)
        return 0;
    if (remaining <= GENESIS_FREE_CHUNK)
        return remaining;
    if (remaining < GENESIS_FREE_CHUNK + GENESIS_MIN_DIM)
        return (u16)(remaining - GENESIS_MIN_DIM);
    return GENESIS_FREE_CHUNK;
}

gbool GenesisFree_Stitch(GenesisMap *m)
{
    u16 x, y, midX, midY;
    if (!m || !m->mask || m->w < GENESIS_MIN_DIM || m->h < GENESIS_MIN_DIM
        || m->w > GENESIS_MAX_W || m->h > GENESIS_MAX_H)
        return GFALSE;
    midX = (u16)(m->w / 2);
    midY = (u16)(m->h / 2);
    for (y = 0; y < m->h; y++)
        for (x = 0; x < m->w; x++)
            if (x == midX || x + 1 == midX || y == midY || y + 1 == midY)
                m->mask[(u32)y * m->w + x] = 1;
    return GenesisMap_JoinAllRegions(m);
}

u16 GenesisFree_RenderCell(const GenesisWorld *w, u16 x, u16 y,
                           u16 floor, u16 vegetation, u16 rock, u16 water,
                           gbool ocean, gbool cave)
{
    u32 k;
    u16 id;
    if (!w || x >= w->map.w || y >= w->map.h)
        return rock;
    k = (u32)y * w->map.w + x;
    id = w->map.cells[k] & 0x03FFu;
    /* Ocean mask denotes connected navigable water; inverse mask is island. */
    if (ocean)
        return w->map.mask[k] ? water : floor;
    if (w->map.mask[k])
        return !cave && id == w->profile.metatile_vegetation ? vegetation : floor;
    if (!cave && id == w->profile.metatile_water)
        return water;
    return rock;
}
