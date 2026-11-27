#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "genesis_free_test.h"
GENESIS_DECLARE_BUFFERS(buffers,32,32);
static GenesisWorld world;
int main(void)
{
    u16 n;
    u32 s, count=0, stitched=0;
    for(n=8;n<=240;n++) {
        u16 rem=n;
        while(rem) {
            u16 k=GenesisFree_ChunkExtent(rem);
            assert(k>=8 && k<=32 && k<=rem);
            rem=(u16)(rem-k);
        }
    }
    assert(GenesisFree_ChunkExtent(7)==0);
    for(s=0;s<1024;s++) {
        GenesisConfig cfg={0};
        u16 x,y;
        cfg.master_seed=s*2654435761u;
        cfg.w=(u16)(8+s%25); cfg.h=(u16)(8+(s/25)%25);
        cfg.biome=(GenesisBiomeId)(s%8);
        assert(GenesisGenerate(&world,&cfg,&buffers));
        assert(GenesisFree_Stitch(&world.map));
        assert(GenesisMap_ValidateConnectivity(&world.map));
        for(y=0;y<cfg.h;y++) for(x=0;x<cfg.w;x++) {
            u32 k=(u32)y*cfg.w+x;
            u16 midX=cfg.w/2,midY=cfg.h/2;
            u16 land=GenesisFree_RenderCell(&world,x,y,1,13,0xC7C,0x1170,0,0);
            u16 sea=GenesisFree_RenderCell(&world,x,y,1,13,0xC7C,0x1170,1,0);
            if(x==midX || x+1==midX || y==midY || y+1==midY) {
                assert(world.map.mask[k]); stitched++;
            }
            if(world.map.mask[k]) { assert(land==1||land==13); assert(sea==0x1170); }
            else { assert(land==0xC7C||land==0x1170); assert(sea==1); }
            count++;
        }
    }
    printf("PASS: 233 axis partitions, 1024 connected chunks, %u terrain cells, %u seam cells\n",count,stitched);
    return 0;
}
