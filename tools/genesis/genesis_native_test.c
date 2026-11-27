/* Compile non-host code against a small ABI-shaped mock; not a ROM build. */
#include "genesis_api.h"
#include "fieldmap.h"
#include "tilesets.h"
#include <assert.h>
#include <stdio.h>
GENESIS_DECLARE_BUFFERS(bufs,32,32);
static GenesisWorld world;
static u16 grid[MAX_MAP_DATA_SIZE];
const struct Tileset gTileset_General={0};
static const struct Tileset wrong={0};
static struct MapLayout layout={.width=32,.height=32,.primaryTileset=&gTileset_General};
struct MapHeader gMapHeader={&layout};
struct BackupMapLayout gBackupMapLayout={47,46,grid};
int main(void)
{
    GenesisConfig c={0}; GenesisBiomeProfile p; u32 i;
    c.w=c.h=32;c.master_seed=0x12345678u;c.biome=GENESIS_BIOME_MEADOW;c.entrances=c.exits=1;
    assert(GenesisGenerate(&world,&c,&bufs));
    assert(!Genesis_CommitToMap(&world));
    assert(!Genesis_GetGeneralProfile(GENESIS_BIOME_ALIEN,&p));
    assert(Genesis_GetGeneralProfile(c.biome,&p));
    c.profile_override=&p;
    assert(GenesisGenerate(&world,&c,&bufs));
    for(i=0;i<MAX_MAP_DATA_SIZE;i++)grid[i]=0xa55a;
    gBackupMapLayout.width=46;
    assert(!Genesis_CommitToMap(&world));
    for(i=0;i<MAX_MAP_DATA_SIZE;i++)assert(grid[i]==0xa55a);
    gBackupMapLayout.width=47;
    assert(Genesis_CommitToMap(&world));
    for(i=0;i<MAX_MAP_DATA_SIZE;i++){
        u32 x=i%47,y=i/47;
        assert(grid[i]==(x>=7&&x<39&&y>=7&&y<39?bufs.cells[(y-7)*32+x-7]:0xa55a));
    }
    layout.primaryTileset=&wrong;
    assert(!Genesis_GetGeneralProfile(c.biome,&p));
    puts("Native adapter mock: PASS (General IDs, bounds, border, placeholder rejection)");
    return 0;
}
