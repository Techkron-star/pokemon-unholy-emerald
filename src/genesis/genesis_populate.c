#include "genesis_populate.h"
#include <string.h>
u16 GenesisPopulate_Requested(const GenesisSpawnConfig*c) {
    return (u16)((u16)c->encounters+c->npcs+c->objects+c->portals+c->events+c->bosses);
}
static void push(GenesisSpawnList*out,GenesisMap*m,u16 i,u8 type,u32 seed) {
    GenesisSpawn*p=&out->entries[out->count++];
    p->x=(u16)(i%m->w);
    p->y=(u16)(i/m->w);
    p->type=type;
    p->subtype=0;
    p->seed_hash=GenesisRng_Hash3(seed,p->x,p->y,type);
    m->parent_dir[i]=1;
}
static u16 distance(u16 a,u16 b) {
    return a>b?(u16)(a-b):(u16)(b-a);
}
/* Manhattan distance is a placement heuristic only; not a path distance. */
static gbool bosses(GenesisSpawnList*out,GenesisMap*m,u8 count,u32 s) {
    u8 j;
    u16 i,n=(u16)(m->w*m->h);
    u16 x=m->entrance_count?m->entrance_x[0]:(u16)(m->w/2),y=m->entrance_count?m->entrance_y[0]:(u16)(m->h/2);
    for(j=0;j<count;j++) {
        u16 best=65535,dmax=0;
        u32 best_hash=0;
        for(i=0;i<n;i++)if(m->mask[i]&&!m->parent_dir[i]) {
            u16 d=(u16)(distance((u16)(i%m->w),x)+distance((u16)(i/m->w),y));
            u32 h=GenesisRng_Hash(s,i,j);
            if(best==65535||d>dmax||(d==dmax&&h>best_hash)) {
                best=i;
                dmax=d;
                best_hash=h;
            }
        }
        if(best==65535)return 0;
        push(out,m,best,GENESIS_SPAWN_BOSS,s);
    }
    return 1;
}
/* Exact placement, without replacement; queue reused after flood fill. */
static gbool place(GenesisSpawnList*out,GenesisMap*m,u8 count,u8 type,u32 seed) {
    GenesisRng r;
    u16 i,n=(u16)(m->w*m->h),k=0;
    u8 j;
    for(i=0;i<n;i++)if(m->mask[i]&&!m->parent_dir[i])m->queue[k++]=i;
    if(k<count)return 0;
    GenesisRng_Init(&r,seed);
    for(j=0;j<count;j++) {
        u16 at=(u16)GenesisRng_Range(&r,k),idx=m->queue[at];
        m->queue[at]=m->queue[--k];
        push(out,m,idx,type,seed);
    }
    return 1;
}
static u32 tagged(const GenesisCore*c,u8 stream,const char*t) {
    u32 s=GenesisCore_StreamSeed(c,stream);
    GenesisCore_MixSeed(&s,t);
    return s;
}
gbool GenesisPopulate_Generate(GenesisSpawnList*out,GenesisMap*m,const GenesisSpawnConfig*c,const GenesisCore*core) {
    u16 i,n,free_count=0;
    if(!out||!m||!c||!core)return 0;
    memset(out,0,sizeof(*out));
    if(GenesisPopulate_Requested(c)>64||m->region_count!=1||m->entrance_count>4||m->exit_count>4)return 0;
    n=(u16)(m->w*m->h);
    for(i=0;i<n;i++)m->parent_dir[i]=0;
    for(i=0;i<m->entrance_count;i++) {
        if(m->entrance_x[i]>=m->w||m->entrance_y[i]>=m->h)return 0;
        m->parent_dir[(u32)m->entrance_y[i]*m->w+m->entrance_x[i]]=1;
    }
    for(i=0;i<m->exit_count;i++) {
        if(m->exit_x[i]>=m->w||m->exit_y[i]>=m->h)return 0;
        m->parent_dir[(u32)m->exit_y[i]*m->w+m->exit_x[i]]=1;
    }
    for(i=0;i<n;i++)if(m->mask[i]&&!m->parent_dir[i])free_count++;
    if(free_count<GenesisPopulate_Requested(c))return 0;
    return (gbool)(bosses(out,m,c->bosses,tagged(core,GENESIS_STREAM_BOSSES,"boss"))&&place(out,m,c->encounters,GENESIS_SPAWN_ENCOUNTER,tagged(core,GENESIS_STREAM_ENCOUNTERS,"enc"))&&place(out,m,c->npcs,GENESIS_SPAWN_NPC,tagged(core,GENESIS_STREAM_OBJECTS,"npc"))&&place(out,m,c->objects,GENESIS_SPAWN_OBJECT,tagged(core,GENESIS_STREAM_OBJECTS,"obj"))&&place(out,m,c->portals,GENESIS_SPAWN_PORTAL,tagged(core,GENESIS_STREAM_SPAWNS,"portal"))&&place(out,m,c->events,GENESIS_SPAWN_EVENT,tagged(core,GENESIS_STREAM_EVENTS,"evt")));
}
