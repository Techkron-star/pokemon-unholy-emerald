#include <assert.h>
#include <stdio.h>
#include "genesis_world_test.h"
int main(void)
{
    u16 grid[49];
    unsigned i;
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
    puts("Genesis global overlay topology checks passed");
    return 0;
}
