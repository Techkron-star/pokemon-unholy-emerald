#include "genesis_api.h"
#include <string.h>
/* Reject overlapping or misaligned buffers before touching any memory. */
static gbool ranges_ok(const GenesisBuffers *b, u32 n)
{
    const void *ptr[6] = {b->mask,b->scratch,b->labels,b->parent,b->cells,b->queue};
    u32 size[6] = {n,n,2*n,n,2*n,2*n};
    u8 i,j;
    for(i=0;i<6;i++) {
        uintptr_t a=(uintptr_t)ptr[i];
        if(!a || a>UINTPTR_MAX-size[i])return 0;
        if((i==2||i==4||i==5)&&(a&1u))return 0;
        for(j=0;j<i;j++) {
            uintptr_t p=(uintptr_t)ptr[j];
            if(a<p+size[j]&&p<a+size[i])return 0;
        }
    }
    return 1;
}
static gbool valid(const GenesisConfig*c,const GenesisBuffers*b) {
    if(!c||!b||c->w<8||c->h<8||c->w>64||c->h>64||(unsigned)c->biome>=GENESIS_BIOME_COUNT||c->entrances>4||c->exits>4||c->generate_graph>1||(c->generate_graph&&(c->graph_nodes<5||c->graph_nodes>30))||GenesisPopulate_Requested(&c->spawn_cfg)>64)return 0;
    if(c->profile_override&&(!GenesisBiome_Validate(c->profile_override)||c->profile_override->id!=c->biome))return 0;
    return (gbool)(b->mask&&b->scratch&&b->labels&&b->parent&&b->cells&&b->queue&&b->capacity>=(u32)c->w*c->h&&ranges_ok(b,(u32)c->w*c->h));
}
static gbool density(const GenesisMap*m) {
    u32 i,n=(u32)m->w*m->h,walk=0;
    for(i=0;i<n;i++)walk+=m->mask[i];
    return (gbool)(walk*5>=n&&walk*5<=4*n);
}
static gbool prepare(GenesisWorld*w,u32 seed,u32 endseed,gbool emergency) {
    if(emergency)GenesisMap_Emergency(&w->map,seed);
    else GenesisBiome_ApplyMask(&w->profile,&w->map,seed);
    return (gbool)(GenesisMap_JoinAllRegions(&w->map)&&GenesisMap_PlaceEnds(&w->map,endseed,w->cfg.entrances,w->cfg.exits)&&density(&w->map));
}
gbool GenesisGenerate(GenesisWorld*w,const GenesisConfig*c,const GenesisBuffers*b) {
    GenesisCore core;
    u32 seed,endseed;
    u8 attempt;
    if(!w)return 0;
    w->status=GENESIS_STATUS_FAILED;
    w->version=GENESIS_VERSION;
    if(!valid(c,b))return 0;
    /* copy before clearing: supports caller passing world.cfg/profile */ {
        GenesisConfig copy=*c;
        GenesisBiomeProfile profile=c->profile_override?*c->profile_override:*GenesisBiome_Get(c->biome);
        gbool native=(gbool)(c->profile_override!=0);
        memset(w,0,sizeof(*w));
        w->cfg=copy;
        w->cfg.profile_override=0;
        w->profile=profile;
        w->native_profile=native;
    }
    w->version=GENESIS_VERSION;
    w->status=GENESIS_STATUS_FAILED;
    GenesisCore_Init(&core,w->cfg.master_seed);
    seed=GenesisCore_StreamSeed(&core,GENESIS_STREAM_TERRAIN);
    endseed=seed^0x1234abcdu;
    if(!GenesisMap_Init(&w->map,w->cfg.w,w->cfg.h,b->mask,b->scratch,b->labels,b->parent,b->cells,b->queue))return 0;
    for(attempt=0;attempt<4;attempt++) {
        u32 s=attempt<3?seed+(u32)attempt*0x9e3779b9u:seed;
        if(!prepare(w,s,endseed,(gbool)(attempt==3)))continue;
        /* Population failure never changes the generated terrain. */if(!GenesisPopulate_Generate(&w->spawns,&w->map,&w->cfg.spawn_cfg,&core))return 0;
        GenesisBiome_WriteCells(&w->profile,&w->map,s);
        if(w->cfg.generate_graph) {
            GenesisGraph_Generate(&w->graph,GenesisCore_StreamSeed(&core,GENESIS_STREAM_GRAPH),w->cfg.graph_nodes);
            if(!GenesisGraph_Validate(&w->graph))return 0;
        }
        w->status=attempt==3?GENESIS_STATUS_EMERGENCY:GENESIS_STATUS_OK;
        return 1;
    }
    return 0;
}
gbool Genesis_CommitToLayout(const GenesisMap*m,u16*dst,u32 cap,u16 stride,u16 height,u16 ox,u16 oy) {
    u16 y,x;
    if(!m||!m->cells||!dst||!stride||!height||!m->w||!m->h||(u32)stride*height>cap||(u32)ox+m->w>stride||(u32)oy+m->h>height)return 0;
    for(y=0;y<m->h;y++)for(x=0;x<m->w;x++)dst[(u32)(oy+y)*stride+ox+x]=m->cells[(u32)y*m->w+x];
    return 1;
}
