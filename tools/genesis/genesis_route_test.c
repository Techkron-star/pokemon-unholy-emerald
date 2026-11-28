#include "genesis_route.h"
#include <assert.h>
#include <stdio.h>
#define W 48u
#define H 40u
#define FLOOR 0x3001u
#define GRASS 0x300Du
#define WALL 0x3C7Cu
#define WATER 0x1170u
#define SPECIAL 0x3015u
static u16 original[W*H], first[W*H];
int main(void)
{
    u32 seed, k, changes=0, different=0;
    u16 x,y;
    for (y=0;y<H;++y)
        for(x=0;x<W;++x)
        {
            u16 tile=FLOOR;
            if (x==0 || y==0 || x==W-1 || y==H-1 || (x==23 && y!=20))
                tile=WALL;
            else if (y==20)
                tile=SPECIAL;
            else if (x>=30 && x<=34 && y>=5 && y<=15)
                tile=WATER;
            else if ((x>=6 && x<=17 && y>=5 && y<=15) ||
                     (x>=27 && x<=40 && y>=26 && y<=34))
                tile=GRASS;
            original[(u32)y*W+x]=tile;
        }
    for(seed=0;seed<1024;++seed)
    {
        u32 added=0,removed=0;
        for(y=0;y<H;++y)
            for(x=0;x<W;++x)
            {
                u16 old,next;
                k=(u32)y*W+x;
                old=original[k];
                next=GenesisRoute_PlanCell(original,W,H,x,y,FLOOR,GRASS,seed);
                assert(next==GenesisRoute_PlanCell(original,W,H,x,y,FLOOR,GRASS,seed));
                assert((old&0xFC00u)==(next&0xFC00u));
                if(old!=FLOOR && old!=GRASS) assert(next==old);
                if(next!=old) { ++changes; added+=next==GRASS; removed+=next==FLOOR; }
                if(seed==0)first[k]=next;
                else different+=first[k]!=next;
                if(y==20)assert(next==old);
            }
        assert(added+removed < W*H/4);
    }
    assert(changes>0 && different>0);
    for(k=0;k<W*H;++k)
        assert(GenesisRoute_PlanCell(original,W,H,k%W,k/W,FLOOR,0x3C0Du,7)==original[k]);
    assert(GenesisRoute_PlanCell(original,W,H,W,H,FLOOR,GRASS,1)==0);
    puts("1024 route seeds: determinism, unchanged collision/elevation, authored barriers and paths preserved");
    printf("vegetation changes: %lu; seed differences: %lu\n",(unsigned long)changes,(unsigned long)different);
    return 0;
}
