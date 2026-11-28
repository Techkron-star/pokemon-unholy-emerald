#include <assert.h>
#include <stdio.h>
#include "genesis_route_first.h"

int main(void)
{
    u32 seed, x, y, entry, changed = 0;
    u32 totals[GENESIS_TERRAIN_COUNT] = {0};
    u8 p;
    for (entry = 0; entry < 65536u; ++entry)
    {
        u16 next = GenesisRoute_ReplaceGrass((u16)entry, 0x00D, 0x001, 0x03FF, 0x0C00);
        assert((next & 0xFC00u) == (entry & 0xFC00u));
        if ((entry & 0x03FFu) != 0x00Du || (entry & 0x0C00u))
            assert(next == entry);
        else
            assert((next & 0x03FFu) == 1u);
    }
    for (seed = 0; seed < 128u; ++seed)
        for (y = 0; y < 64u; ++y)
            for (x = 0; x < 64u; ++x)
            {
                u32 a = GenesisRoute_Noise(seed, x, y);
                u32 b = GenesisRoute_Noise(seed, x + 1u, y);
                u32 c = GenesisRoute_Noise(seed, x, y + 1u);
                gbool clear = GenesisRoute_Clear(seed, x, y, 64, 32, 8);
                assert(a <= 255u && b <= 255u && c <= 255u);
                assert(a <= b + 33u && b <= a + 33u);
                assert(a <= c + 33u && c <= a + 33u);
                assert(clear == GenesisRoute_Clear(seed, x, y, 64, 32, 8));
                assert(!GenesisRoute_Clear(seed, x, y, 64, 32, 2));
                changed += clear;
                for (p = 0; p < GENESIS_TERRAIN_COUNT; ++p)
                {
                    u8 need = GenesisRoute_Intent(seed, x, y, 64, 32, p);
                    assert(need == 0 || need == 3 || need == 6);
                    totals[p] += need != 0;
                }
            }
    assert(totals[GENESIS_TERRAIN_NATIVE] == 0);
    assert(totals[GENESIS_TERRAIN_FOREST] < totals[GENESIS_TERRAIN_BALANCED]);
    assert(totals[GENESIS_TERRAIN_PLAINS] > totals[GENESIS_TERRAIN_BALANCED]);
    assert(totals[GENESIS_TERRAIN_BALANCED] == changed);
    assert(GenesisRoute_Profile(7) == GENESIS_TERRAIN_BALANCED);
    assert(changed > 0 && changed < 128u * 64u * 64u);
    printf("Route-first: 65536 metatile contracts, 524288 deterministic samples, %u cleared\n", changed);
    return 0;
}
