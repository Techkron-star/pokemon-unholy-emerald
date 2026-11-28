#include "genesis2/world.h"
#include "genesis2/assets.generated.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define COUNT (sizeof(sG2Maps) / sizeof(sG2Maps[0]))

static void finish(struct G2_World *w, unsigned budget)
{
    while (!G2_Complete(w)) assert(G2_Tick(w, budget) <= budget);
    assert(w->maxTickUnits <= budget);
}
int main(int argc, char **argv)
{
    const unsigned seeds = argc > 1 ? (unsigned)strtoul(argv[1], NULL, 10) : 1000u;
    const size_t bytes = G2_StorageBytes(sG2Maps, (uint32_t)COUNT);
    struct G2_World a, b;
    struct G2_Entry ea[COUNT], eb[COUNT];
    unsigned char *sa = malloc(bytes + 4), *sb = malloc(bytes + 4);
    unsigned seed, i, diverse = 0, modules = 0;
    uint32_t baseline[COUNT];
    assert(sa && sb && bytes);
    {
        struct G2_Map invalid = sG2Maps[0];
        invalid.width=65521; invalid.height=65522;
        assert(!G2_MapValid(&invalid) && !G2_StorageBytes(&invalid,1));
    }
    assert(!G2_Begin(&a, sG2Maps, (uint32_t)COUNT, ea, sa, bytes - 1, 1));
    assert(!G2_Begin(&a, sG2Maps, (uint32_t)COUNT, ea, sa + 1, bytes, 1));
    assert(!G2_Begin(&a, NULL, 0, ea, sa, bytes, 1));
    memset(sa + bytes, 0xA5, 4); memset(sb + bytes, 0xA5, 4);
    /* Corrupt a render result before validation: never publish a partial map. */
    assert(G2_Begin(&a, sG2Maps, (uint32_t)COUNT, ea, sa, bytes, 7));
    while (ea[0].phase != G2_VALIDATE) G2_Tick(&a, 1);
    ea[0].tiles[ea[0].cursor] ^= 0x0C00;
    finish(&a, 32);
    assert(a.failed == 1 && ea[0].phase == G2_FAILED);
    assert(!G2_FindReady(&a,sG2Maps[0].mapId,sG2Maps[0].layoutId,
                         sG2Maps[0].width,sG2Maps[0].height));
    for (seed = 0; seed < seeds; ++seed)
    {
        assert(G2_Begin(&a,sG2Maps,(uint32_t)COUNT,ea,sa,bytes,seed));
        finish(&a,32);
        assert(!a.failed);
        for (i = 0; i < COUNT; ++i)
        {
            const struct G2_Map *m = &sG2Maps[i];
            uint32_t k, n = (uint32_t)m->width * m->height;
            assert(G2_FindReady(&a,m->mapId,m->layoutId,m->width,m->height) == &ea[i]);
            assert(!G2_FindReady(&a,m->mapId,m->layoutId,m->width+1,m->height));
            modules += ea[i].plan.placementCount;
            if (!seed) baseline[i] = ea[i].crc;
            else if (ea[i].crc != baseline[i]) ++diverse;
            for (k = 0; k < ea[i].changed; ++k)
            {
                const struct G2_Edit *e = &ea[i].edits[k];
                assert(e->sourceIndex < n);
                assert(e->liveIndex == (e->sourceIndex / m->width + 7u)
                        * (m->width + G2_MAP_OFFSET_W) + e->sourceIndex % m->width + 7u);
                assert(ea[i].tiles[e->sourceIndex] != m->native[e->sourceIndex]);
            }
            for (k = 0; k < n; ++k)
            {
                uint16_t old = m->native[k], now = ea[i].tiles[k];
                assert((old & 0xF000u) == (now & 0xF000u));
                if ((old & ~G2_ID_MASK) != (now & ~G2_ID_MASK))
                    assert(G2_ModuleCellValid(m,&ea[i],k,now));
                if (!(m->editable[k/8] & (1u << (k%8)))) assert(now == old);
            }
            if (ea[i].encounterAnchor != G2_NO_NODE)
                assert(ea[i].tiles[ea[i].encounterAnchor] == m->native[ea[i].encounterAnchor]);
        }
        if (seed < 10)
        {
            /* Different scheduling and reverse visit order, identical bytes. */
            assert(G2_Begin(&b,sG2Maps,(uint32_t)COUNT,eb,sb,bytes,seed));
            finish(&b,257);
            for (i = 0; i < COUNT; ++i)
                assert(!memcmp(ea[i].tiles, eb[i].tiles,
                    (size_t)sG2Maps[i].width * sG2Maps[i].height * sizeof(uint16_t)));
            for (i = (unsigned)COUNT; i-- > 0;)
            {
                const struct G2_Map *m = &sG2Maps[i];
                assert(G2_FindReady(&b,m->mapId,m->layoutId,m->width,m->height));
                assert(ea[i].crc == eb[i].crc && ea[i].fingerprint == eb[i].fingerprint);
                assert(ea[i].plan.digest == eb[i].plan.digest);
                assert(ea[i].plan.placementCount == eb[i].plan.placementCount);
            }
        }
    }
    if (seeds > 1) assert(diverse);
    if (seeds >= 100) assert(modules);
    for (i = 0; i < 4; ++i) assert(sa[bytes+i] == 0xA5 && sb[bytes+i] == 0xA5);
    for (i = 0; i <= 12; ++i)
    {
        (void)G2_Climate(1,INT_MIN,INT_MAX,i);
        (void)G2_Climate(1,-1,-129,i);
    }
    /* Shared geographical point sampled through unrelated local origins. */
    assert(G2_Climate(77,-200+250,34+19,7) == G2_Climate(77,100-50,70-17,7));
    free(sa); free(sb);
    printf("PASS Genesis2: %u seeds x %zu native routes; %u complete modules; bounds, budgets, protection, determinism, failure\n",seeds,COUNT,modules);
    return 0;
}
