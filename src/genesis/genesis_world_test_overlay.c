#include "genesis_world_test.h"

gbool GenesisWorldTest_CanBlock(const u16 *grid, u16 stride, u16 height,
                               u16 x, u16 y, u16 floorA, u16 floorB)
{
    s32 dx, dy;
    u16 center, elevation;
    if (!grid || x == 0 || y == 0 || (u32)x + 1 >= stride || (u32)y + 1 >= height)
        return GFALSE;
    center = grid[(u32)y * stride + x];
    elevation = center & 0xF000u;
    for (dy = -1; dy <= 1; dy++)
    {
        for (dx = -1; dx <= 1; dx++)
        {
            u16 entry = grid[(u32)((s32)y + dy) * stride + (u32)((s32)x + dx)];
            u16 id = entry & 0x03FFu;
            if ((entry & 0x0C00u) || (entry & 0xF000u) != elevation
                || (id != floorA && id != floorB))
                return GFALSE;
        }
    }
    return GTRUE;
}
