#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "genesis_terrain.h"
static GenesisTerrainRules rules;
int main(void)
{
    static const u8 pos[8]={1,5,7,3,2,8,6,0};
    u8 grid[GENESIS_TERRAIN_PACKED_BYTES(9)];
    static u8 large[GENESIS_TERRAIN_PACKED_BYTES(10240)];
    u16 i,j;
    u32 checks=0;
    for(i=0;i<256;i++) {
        memset(grid,0,sizeof(grid));
        for(j=0;j<9;j++) GenesisTerrain_Set(grid,j,GENESIS_TERRAIN_GROUND);
        for(j=0;j<8;j++) if(i&(1u<<j)) GenesisTerrain_Set(grid,pos[j],GENESIS_TERRAIN_WALL|GENESIS_TERRAIN_GENERATED);
        assert(GenesisTerrain_Mask(grid,3,3,1,1,GENESIS_TERRAIN_WALL)==i);
        GenesisTerrain_Clear(&rules);
        assert(GenesisTerrain_Select(&rules,(u8)i,23,1)==23);
        GenesisTerrain_Observe(&rules,(u8)i,100);
        GenesisTerrain_Observe(&rules,(u8)i,100);
        GenesisTerrain_Observe(&rules,(u8)i,101);
        assert(GenesisTerrain_Select(&rules,(u8)i,23,0)==100);
        assert(GenesisTerrain_Select(&rules,(u8)(i^16),23,1)==100);
        assert(GenesisTerrain_Select(&rules,(u8)(i^1),23,1)==23);
        assert(GenesisTerrain_Select(&rules,(u8)(i^16),23,0)==23);
        checks+=6;
    }
    for(j=0;j<9;j++) GenesisTerrain_Set(grid,j,GENESIS_TERRAIN_WALL);
    assert(GenesisTerrain_Mask(grid,3,3,0,0,GENESIS_TERRAIN_WALL)==(2|4|32));
    assert(GenesisTerrain_Mask(grid,3,3,3,0,GENESIS_TERRAIN_WALL)==0);
    assert(GenesisTerrain_Mask(0,3,3,1,1,GENESIS_TERRAIN_WALL)==0);
    for(i=0;i<64;i++) for(j=0;j<1024;j++) {
        u16 cell=(u16)(i<<10|17);
        assert(GenesisTerrain_ReplaceId(cell,j)==(u16)(i<<10|j));
        assert(GenesisTerrain_ReplaceId(cell,65535)==cell);
        checks+=2;
    }
    assert(sizeof(large)==5120);
    for(i=0;i<16;i++) for(j=0;j<16;j++) {
        GenesisTerrain_Set(grid,0,(u8)i);
        GenesisTerrain_Set(grid,1,(u8)j);
        assert(GenesisTerrain_Get(grid,0)==i);
        assert(GenesisTerrain_Get(grid,1)==j);
        GenesisTerrain_Set(grid,0,(u8)(i^15u));
        assert(GenesisTerrain_Get(grid,1)==j);
        checks+=3;
    }
    for(i=0;i<10240;i++) GenesisTerrain_Set(large,i,(u8)((i*7u)&15u));
    for(i=0;i<10240;i++) {
        assert(GenesisTerrain_Get(large,i)==((i*7u)&15u));
        checks++;
    }
    printf("PASS: %u assertions plus edge cases; 256 neighborhood patterns, 65536 native cells, 10240 semantic cells\n", checks);
    return 0;
}
