#include "genesis_api.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
GENESIS_DECLARE_BUFFERS(a,32,32);
GENESIS_DECLARE_BUFFERS(b,32,32);
static GenesisWorld wa,wb;
int main(void){GenesisConfig c={0};GenesisBiomeProfile p;GenesisCore core;GenesisSpawnList first,second;GenesisBuffers bad=a;u16 i;
 c.w=c.h=32;c.master_seed=0x12345678;c.biome=GENESIS_BIOME_MEADOW;c.entrances=c.exits=1;c.generate_graph=1;c.graph_nodes=20;
 assert(GenesisGenerate(&wa,&c,&a));c.spawn_cfg=(GenesisSpawnConfig){8,2,3,1,1,1};assert(GenesisGenerate(&wb,&c,&b));assert(!memcmp(a.cells,b.cells,1024*sizeof(u16)));assert(!memcmp(&wa.graph,&wb.graph,sizeof(wa.graph)));
 GenesisCore_Init(&core,c.master_seed);assert(GenesisPopulate_Generate(&first,&wa.map,&c.spawn_cfg,&core));GenesisCore_MixSeed(&core.stream_seed[GENESIS_STREAM_DECORATION],"unused-deco");assert(GenesisPopulate_Generate(&second,&wa.map,&c.spawn_cfg,&core));assert(first.count==second.count);for(i=0;i<first.count;i++){assert(first.entries[i].x==second.entries[i].x&&first.entries[i].y==second.entries[i].y&&first.entries[i].seed_hash==second.entries[i].seed_hash);}
 bad.scratch=bad.mask+1;assert(!GenesisGenerate(&wa,&c,&bad));GenesisBiome_CopyProfile(c.biome,&p);p.ca_fill_percent=100;p.ca_neighbor_threshold=0;p.water_threshold=0;p.rock_threshold=255;c.profile_override=&p;assert(GenesisGenerate(&wa,&c,&a));assert(wa.status==GENESIS_STATUS_EMERGENCY);assert(GenesisMap_ValidateConnectivity(&wa.map));puts("Isolation, buffer overlap, forced structured fallback: PASS");return 0;}
