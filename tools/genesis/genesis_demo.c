#include "genesis_api.h"
#include <stdio.h>
#include <stdlib.h>
GENESIS_DECLARE_BUFFERS(buffers,32,32);
static GenesisWorld world;
int main(int argc,char**argv) {
    GenesisConfig c= {
        0
    };
    u16 x,y,i;
    unsigned long biome=argc>2?strtoul(argv[2],0,0):0;
    if(biome>=GENESIS_BIOME_COUNT)return 2;
    c.master_seed=argc>1?(u32)strtoul(argv[1],0,0):0x12345678u;
    c.biome=(GenesisBiomeId)biome;
    c.w=c.h=32;
    c.entrances=c.exits=1;
    c.spawn_cfg=(GenesisSpawnConfig) {
        12,3,4,1,2,1
    };
    c.generate_graph=1;
    c.graph_nodes=20;
    if(!GenesisGenerate(&world,&c,&buffers))return 1;
    printf("Genesis v1.1 | %s | seed 0x%08X | status %u | %u spawns\n",world.profile.name,c.master_seed,world.status,world.spawns.count);
    for(y=0;y<c.h;y++) {
        for(x=0;x<c.w;x++) {
            u16 k=(u16)((u32)y*c.w+x),id=(u16)(buffers.cells[k]&1023u);
            char ch=buffers.mask[k]?(id==3?'"':'.'):(id==2?'~':'#');
            for(i=0;i<world.spawns.count;i++)if(world.spawns.entries[i].x==x&&world.spawns.entries[i].y==y)ch="enopvB"[world.spawns.entries[i].type];
            for(i=0;i<world.map.entrance_count;i++)if(world.map.entrance_x[i]==x&&world.map.entrance_y[i]==y)ch='E';
            for(i=0;i<world.map.exit_count;i++)if(world.map.exit_x[i]==x&&world.map.exit_y[i]==y)ch='X';
            putchar(ch);
        }
        putchar('\n');
    }
    puts("Graph: node type -> neighbors");
    for(i=0;i<world.graph.node_count;i++) {
        u8 j;
        printf("%02u type %u ->",i,world.graph.nodes[i].type);
        for(j=0;j<world.graph.nodes[i].edge_count;j++)printf(" %02u",world.graph.nodes[i].edges[j]);
        putchar('\n');
    }
    return 0;
}
