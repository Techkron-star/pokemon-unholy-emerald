#include <assert.h>
#include <stdio.h>
#include "genesis_world_test.h"
int main(void)
{
    u16 grid[49];
    unsigned i;
    unsigned oldCount=0,newCount=0;
    u16 x,y;
    for (i=0;i<49;i++) grid[i]=0x3001;
    assert(GenesisWorldTest_CanBlock(grid,7,7,3,3,1,13));
    grid[24]=0x3C79;
    assert(!GenesisWorldTest_CanBlock(grid,7,7,3,3,1,13));
    assert(!GenesisWorldTest_CanBlock(grid,7,7,4,3,1,13));
    assert(!GenesisWorldTest_CanBlock(grid,7,7,0,3,1,13));
    grid[24]=0x3001;
    grid[17]=0x4001;
    assert(!GenesisWorldTest_CanBlock(grid,7,7,3,3,1,13));
    grid[17]=0x300D;
    assert(GenesisWorldTest_CanBlock(grid,7,7,3,3,1,13));
    grid[17]=0x3005;
    assert(!GenesisWorldTest_CanBlock(grid,7,7,3,3,1,13));
    assert(GenesisWorldTest_ProtectEdge(20,20,6,10,GENESIS_EDGE_WEST,7));
    assert(!GenesisWorldTest_ProtectEdge(20,20,7,10,GENESIS_EDGE_WEST,7));
    assert(!GenesisWorldTest_ProtectEdge(20,20,1,10,GENESIS_EDGE_NORTH,7));
    assert(GenesisWorldTest_ProtectEdge(20,20,0,10,GENESIS_EDGE_NORTH,7));
    assert(GenesisWorldTest_ProtectEdge(20,20,10,13,GENESIS_EDGE_SOUTH,7));
    assert(!GenesisWorldTest_ProtectEdge(20,20,10,12,GENESIS_EDGE_SOUTH,7));
    assert(GenesisWorldTest_ProtectEdge(20,20,13,10,GENESIS_EDGE_EAST,7));
    assert(!GenesisWorldTest_ProtectEdge(20,20,12,10,GENESIS_EDGE_EAST,7));
    assert(GenesisWorldTest_ProtectEdge(4,4,2,2,GENESIS_EDGE_ALL,7));
    assert(GenesisWorldTest_ProtectEdge(20,20,20,2,0,7));
    for(y=0;y<20;y++) for(x=0;x<50;x++) {
        oldCount += !GenesisWorldTest_ProtectEdge(50,20,x,y,GENESIS_EDGE_ALL,7);
        newCount += !GenesisWorldTest_ProtectEdge(50,20,x,y,GENESIS_EDGE_EAST|GENESIS_EDGE_WEST,7);
    }
    assert(oldCount==216 && newCount==648);
    printf("Genesis overlay checks passed; 50x20 horizontal route: %u -> %u eligible geometric cells\n",oldCount,newCount);
    return 0;
}
