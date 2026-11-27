#include "genesis_api.h"
#include "genesis_noise.h"
#include "genesis_ca.h"
#include "genesis_bsp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifndef GENESIS_TEST_SEEDS
#define GENESIS_TEST_SEEDS 10000u
#endif
GENESIS_DECLARE_BUFFERS(a,64,64);
GENESIS_DECLARE_BUFFERS(b,64,64);
static GenesisWorld A,B;
static u32 cases,emergency,spawn_total;
static u32 counts[GENESIS_BIOME_COUNT],fallbacks[GENESIS_BIOME_COUNT];
static GenesisConfig cfg(u32 seed,GenesisBiomeId id,u16 w,u16 h) {
    GenesisConfig c= {
        0
    };
    c.master_seed=seed;
    c.biome=id;
    c.w=w;
    c.h=h;
    c.entrances=c.exits=1;
    c.spawn_cfg=(GenesisSpawnConfig) {
        8,2,3,1,1,1
    };
    c.generate_graph=1;
    c.graph_nodes=(u8)(5+seed%26);
    return c;
}
static void check_world(GenesisWorld*w,GenesisBuffers const*buf,GenesisConfig const*c) {
    u32 i,n=(u32)c->w*c->h,walk=0;
    u16 j,k,types[6]= {
        0
    };
    u8 used[GENESIS_MAX_CELLS]= {
        0
    };
    assert(w->status<=1);
    assert(w->map.region_count==1);
    assert(GenesisGraph_Validate(&w->graph));
    assert(w->graph.node_count==c->graph_nodes);
    assert(w->map.entrance_count==c->entrances&&w->map.exit_count==c->exits);
    for(i=0;i<n;i++) {
        walk+=buf->mask[i];
        assert(buf->mask[i]<=1);
        assert((buf->cells[i]&0x0c00u)==(buf->mask[i]?0u:0x0c00u));
        assert((buf->cells[i]>>12)==w->profile.elevation);
        assert(buf->labels[i]==(buf->mask[i]?1:0));
    }
    assert(walk*5>=n&&walk*5<=n*4);
    for(j=0;j<w->map.entrance_count;j++) {
        assert(w->map.entrance_x[j]<c->w&&w->map.entrance_y[j]==0);
        i=(u32)w->map.entrance_y[j]*c->w+w->map.entrance_x[j];
        assert(buf->mask[i]&&!used[i]);
        used[i]=1;
    }
    for(j=0;j<w->map.exit_count;j++) {
        assert(w->map.exit_x[j]<c->w&&w->map.exit_y[j]==c->h-1);
        i=(u32)w->map.exit_y[j]*c->w+w->map.exit_x[j];
        assert(buf->mask[i]&&!used[i]);
        used[i]=1;
    }
    assert(w->spawns.count==GenesisPopulate_Requested(&c->spawn_cfg));
    for(j=0;j<w->spawns.count;j++) {
        GenesisSpawn*s=&w->spawns.entries[j];
        assert(s->x<c->w&&s->y<c->h&&s->type<6);
        i=(u32)s->y*c->w+s->x;
        assert(buf->mask[i]&&!used[i]);
        used[i]=1;
        types[s->type]++;
    }
    assert(types[0]==c->spawn_cfg.encounters&&types[1]==c->spawn_cfg.npcs&&types[2]==c->spawn_cfg.objects&&types[3]==c->spawn_cfg.portals&&types[4]==c->spawn_cfg.events&&types[5]==c->spawn_cfg.bosses);
    assert(GenesisMap_ValidateConnectivity(&w->map));
    /* independent traversal with boss removed: exit must not be reachable. */ {
        u8 seen[30]= {
            0
        },q[30],head=0,tail=0;
        seen[w->graph.entrance]=1;
        q[tail++]=w->graph.entrance;
        while(head<tail) {
            u8 u=q[head++];
            for(k=0;k<w->graph.nodes[u].edge_count;k++) {
                u8 v=w->graph.nodes[u].edges[k];
                if(v!=w->graph.boss&&!seen[v]) {
                    seen[v]=1;
                    q[tail++]=v;
                }
            }
        }
        assert(!seen[w->graph.exit_node]);
    }
}
static void one(GenesisConfig c) {
    u32 n=(u32)c.w*c.h;
    u16 i;
    if(!GenesisGenerate(&A,&c,&a)) {
        fprintf(stderr,"FAIL seed=%u biome=%d %ux%u\n",c.master_seed,c.biome,c.w,c.h);
        abort();
    }
    check_world(&A,&a,&c);
    assert(GenesisGenerate(&B,&c,&b));
    assert(A.status==B.status);
    assert(memcmp(a.cells,b.cells,n*sizeof(u16))==0);
    assert(memcmp(a.mask,b.mask,n)==0);
    assert(memcmp(A.map.entrance_x,B.map.entrance_x,sizeof(A.map.entrance_x))==0);
    assert(memcmp(A.map.exit_x,B.map.exit_x,sizeof(A.map.exit_x))==0);
    assert(memcmp(&A.graph,&B.graph,sizeof(A.graph))==0);
    assert(A.spawns.count==B.spawns.count);
    for(i=0;i<A.spawns.count;i++) {
        GenesisSpawn p=A.spawns.entries[i],q=B.spawns.entries[i];
        assert(p.x==q.x&&p.y==q.y&&p.type==q.type&&p.subtype==q.subtype&&p.seed_hash==q.seed_hash);
    }
    cases++;
    spawn_total+=A.spawns.count;
    counts[c.biome]++;
    if(A.status==1) {
        emergency++;
        fallbacks[c.biome]++;
    }
}
static void regressions(void) {
    GenesisCore core;
    GenesisRng r;
    GenesisMap map;
    u16 i;
    u32 j;
    u8 seen[65536]= {
        0
    };
    unsigned unique=0;
    GenesisConfig c=cfg(0,GENESIS_BIOME_MEADOW,32,32);
    static u16 dst[47*46];
    GenesisBuffers bad=a;
    GenesisCore_Init(&core,0);
    {
        u32 enc=core.stream_seed[GENESIS_STREAM_ENCOUNTERS],boss=core.stream_seed[GENESIS_STREAM_BOSSES];
        GenesisCore_MixSeed(&core.stream_seed[GENESIS_STREAM_DECORATION],"changed");
        assert(enc==core.stream_seed[GENESIS_STREAM_ENCOUNTERS]&&boss==core.stream_seed[GENESIS_STREAM_BOSSES]);
    }
    GenesisRng_Init(&r,0);
    for(j=0;j<10000;j++)assert(GenesisRng_Range(&r,7)<7);
    for(j=0;j<1024;j++) {
        u32 v=(u32)GenesisNoise_FBM(0x12345678u,(s32)(j%32),(s32)(j/32),8192,3,32768,131072);
        assert(v<=65535);
        if(!seen[v]) {
            seen[v]=1;
            unique++;
        }
    }
    assert(unique>500);
    printf("Noise: %u distinct values in 1024 cells\n",unique);
    for(j=0;j<100;j++) {
        s32 v=GenesisNoise_FBM(j,(s32)0x80000000u,2147483647,0xffffffffu,255,0xffffffffu,0xffffffffu);
        assert(v>=0&&v<=65535);
    }
    memset(a.mask,0,64);
    GenesisCA_Step(a.mask,a.scratch,8,8,4);
    for(i=0;i<64;i++)assert(a.mask[i]==0);
    assert(GenesisMap_Init(&map,64,64,a.mask,a.scratch,a.labels,a.parent,a.cells,a.queue));
    for(i=0;i<4096;i++)a.mask[i]=(u8)(((i/64+i%64)&1)==0);
    assert(GenesisMap_FloodFill(&map)==2048);
    assert(!GenesisMap_JoinAllRegions(&map));
    GenesisMap_Emergency(&map,1);
    assert(GenesisMap_PlaceEnds(&map,2,4,4));
    assert(GenesisMap_ValidateConnectivity(&map));
    bad.capacity=1023;
    assert(!GenesisGenerate(&A,&c,&bad));
    bad=a;
    bad.scratch=bad.mask;
    assert(!GenesisGenerate(&A,&c,&bad));
    c.w=7;
    assert(!GenesisGenerate(&A,&c,&a));
    c=cfg(0,GENESIS_BIOME_MEADOW,32,32);
    c.spawn_cfg.bosses=0;
    one(c);
    c.spawn_cfg.bosses=3;
    one(c);
    c.spawn_cfg.bosses=65;
    assert(!GenesisGenerate(&A,&c,&a));
    c=cfg(0,GENESIS_BIOME_MEADOW,32,32);
    assert(GenesisGenerate(&A,&c,&a));
    for(i=0;i<47*46;i++)dst[i]=0xffff;
    assert(!Genesis_CommitToLayout(&A.map,dst,47*46-1,47,46,7,7));
    for(i=0;i<47*46;i++)assert(dst[i]==0xffff);
    assert(Genesis_CommitToLayout(&A.map,dst,47*46,47,46,7,7));
    for(i=0;i<47*46;i++) {
        u16 x=(u16)(i%47),y=(u16)(i/47);
        assert(dst[i]==(x>=7&&x<39&&y>=7&&y<39?a.cells[(u32)(y-7)*32+x-7]:0xffff));
    }
    assert(!Genesis_CommitToMap(&A));
    {
        GenesisGraph g;
        GenesisGraph_Generate(&g,0xec48c90eu,8);
        assert(GenesisGraph_Validate(&g));
        g.nodes[g.exit_node].edge_count=2;
        g.nodes[g.exit_node].edges[1]=g.entrance;
        assert(!GenesisGraph_Validate(&g));
        g.node_count=31;
        assert(!GenesisGraph_Validate(&g));
    }
    puts("Regression, padded layout and invalid input tests: PASS");
}
int main(void) {
    u8 id;
    u32 s;
    regressions();
    for(id=0;id<GENESIS_BIOME_COUNT;id++) {
        for(s=0;s<GENESIS_TEST_SEEDS;s++)one(cfg(s*2654435761u+(u32)id*0x9e3779b9u,(GenesisBiomeId)id,32,32));
        printf("%s: %u cases, %u emergency\n",GenesisBiome_Get((GenesisBiomeId)id)->name,counts[id],fallbacks[id]);
        fflush(stdout);
    }
    for(id=0;id<8;id++) {
        for(s=0;s<128;s++) {
            GenesisConfig c=cfg(s,(GenesisBiomeId)id,(u16)(8+s%57),(u16)(8+(s*13)%57));
            c.entrances=(u8)(s%5);
            c.exits=(u8)((s+1)%5);
            c.spawn_cfg=(GenesisSpawnConfig) {
                1,1,1,0,0,(u8)(s%3)
            };
            one(c);
        }
    }
    printf("PASS: %u cases, %u emergency, %u spawns, deterministic replay each case\n",cases,emergency,spawn_total);
    printf("sizeof: world=%zu map=%zu BSP=%zu graph=%zu (host ABI); buffers 32x32=%u bytes\n",sizeof(GenesisWorld),sizeof(GenesisMap),sizeof(GenesisBSP),sizeof(GenesisGraph),9u*32u*32u);
    return 0;
}
