#include "genesis2/world.h"
#include "genesis2/assets.generated.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int soft(const struct G2_Map *m,unsigned k)
{
    unsigned id=m->native[k]&1023u;
    return m->topology && (m->topology[k/8]&(1u<<(k%8)))
        && (m->editable[k/8]&(1u<<(k%8))) && !(m->native[k]&0xC00u)
        && (m->native[k]&0xF000u)==0x3000u && (id==1u || id==13u);
}
static int neighbor(const struct G2_Map *m,unsigned k,unsigned d)
{
    unsigned x=k%m->width,y=k/m->width;
    if(d==0 && x)return (int)k-1;
    if(d==1 && x+1<m->width)return (int)k+1;
    if(d==2 && y)return (int)k-m->width;
    if(d==3 && y+1<m->height)return (int)k+m->width;
    return -1;
}
static void gated_reach(const struct G2_Map *m,const uint16_t *tiles,unsigned start,
                        unsigned state,unsigned char *visited,unsigned *queue)
{
    unsigned n=(unsigned)m->width*m->height,head=0,tail=1;
    memset(visited,0,n);queue[0]=start;visited[start]=1;
    while(head<tail){
        unsigned k=queue[head++];
        for(unsigned d=0;d<4;++d){int j=neighbor(m,k,d);
            if(j<0 || visited[j] || (tiles[j]&0xC00u))continue;
            /* Arbitrary directed, capability-gated exterior. The generator
             * need not assume those edges are symmetric or currently open. */
            if(!soft(m,k) || !soft(m,(unsigned)j)){
                unsigned requirement=G2_Hash(0xD1CEu,(int32_t)k,j)&3u;
                if((requirement&state)!=requirement)continue;
            }
            assert(tail<n);visited[j]=1;queue[tail++]=(unsigned)j;
        }
    }
}
static unsigned verify(const struct G2_Map *m,const struct G2_Entry *e)
{
    unsigned n=(unsigned)m->width*m->height,regions=0,trees=0;
    unsigned *label=calloc(n,sizeof(*label)),*queue=malloc(n*sizeof(*queue));
    unsigned *owners=calloc(n,sizeof(*owners));
    unsigned char *seen=calloc(n,1);
    unsigned char *reference=calloc(n,1),*candidate=calloc(n,1);
    assert(label && queue && owners && seen && e->phase==G2_READY);
    for(unsigned k=0;k<n;++k)if(soft(m,k) && !label[k]){
        unsigned head=0,tail=1;queue[0]=k;label[k]=++regions;
        while(head<tail){
            unsigned p=queue[head++];
            for(unsigned d=0;d<4;++d){int q=neighbor(m,p,d);
                if(q>=0 && soft(m,(unsigned)q) && !label[q]){
                    assert(tail<n);label[q]=regions;queue[tail++]=(unsigned)q;
                }
            }
        }
    }
    if(!e->plan.fallback)assert(e->plan.regions==regions);
    for(unsigned region=1;region<=regions;++region){
        unsigned root=n,head=0,tail=0;
        for(unsigned k=0;k<n;++k)if(label[k]==region && !(e->tiles[k]&0xC00u)){root=k;break;}
        assert(root<n);queue[tail++]=root;seen[root]=1;
        while(head<tail){
            unsigned k=queue[head++];
            for(unsigned d=0;d<4;++d){int q=neighbor(m,k,d);
                if(q>=0 && label[q]==region && !seen[q] && !(e->tiles[q]&0xC00u)){
                    assert(tail<n);seen[q]=1;queue[tail++]=(unsigned)q;
                }
            }
        }
        for(unsigned k=0;k<n;++k)if(label[k]==region){
            int port=0;
            for(unsigned d=0;d<4;++d){int q=neighbor(m,k,d);
                if(q<0 || (!soft(m,(unsigned)q) && !(m->native[q]&0xC00u)))port=1;
            }
            if(!(e->tiles[k]&0xC00u))assert(seen[k]);
            if(port)assert(!(e->tiles[k]&0xC00u) && seen[k]);
            if(!e->plan.fallback){
                assert(((e->plan.flags[k]&G2_PORT)!=0)==port);
                assert(e->plan.region[k]==region);
                if(port){
                    unsigned p=k,steps=0;
                    for(;;){
                        unsigned parent=e->plan.parent[p];
                        assert(parent<n && label[parent]==region && (e->plan.flags[p]&G2_CORRIDOR));
                        assert(!(e->tiles[p]&0xC00u));
                        if(parent==p)break;
                        assert(++steps<=n);
                        int adjacent=0;
                        for(unsigned d=0;d<4;++d)if(neighbor(m,p,d)==(int)parent)adjacent=1;
                        assert(adjacent);p=parent;
                    }
                }
            }
        }
    }
    for(unsigned k=0;k<n;++k){
        unsigned owner=e->plan.owner?e->plan.owner[k]:0;
        if(owner){
            assert(owner<=e->plan.placementCount);++owners[owner-1];
            assert(soft(m,k) && (e->plan.flags[k]&G2_MODULE));
        }
        if((e->tiles[k]&0xC00u)!=(m->native[k]&0xC00u))assert(owner);
        if(!soft(m,k))assert((e->tiles[k]&0xFC00u)==(m->native[k]&0xFC00u));
    }
    for(unsigned i=0;i<e->plan.placementCount;++i){
        const struct G2_Placement *p=&e->plan.placements[i];
        const struct G2_Module *module=&m->catalog->modules[p->module];
        assert(owners[i]==(unsigned)module->width*module->height);
        for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
            unsigned k=(p->origin/m->width+y)*m->width+p->origin%m->width+x;
            assert(e->plan.owner[k]==i+1 && e->tiles[k]==module->cells[y*module->width+x]);
        }
        if(module->kind==G2_MODULE_TREE)++trees;
    }
    for(unsigned state=0;state<4;++state)for(unsigned a=0;a<8;++a){
        unsigned start=n*a/8;
        while(start<n && (soft(m,start) || (m->native[start]&0xC00u)))++start;
        if(start==n)continue;
        gated_reach(m,m->native,start,state,reference,queue);
        gated_reach(m,e->tiles,start,state,candidate,queue);
        for(unsigned k=0;k<n;++k)if(!soft(m,k))assert(reference[k]==candidate[k]);
    }
    free(label);free(queue);free(owners);free(seen);free(reference);free(candidate);return trees;
}
static void finish(struct G2_World *w)
{
    unsigned ticks=0;
    while(!G2_Complete(w)){assert(G2_Tick(w,17)<=17);assert(++ticks<200000);}
}
int main(void)
{
    const struct G2_Map *pilot=NULL;
    for(unsigned i=0;i<sizeof(sG2Maps)/sizeof(sG2Maps[0]);++i)
        if(G2_HasTopology(&sG2Maps[i]))pilot=&sG2Maps[i];
    assert(pilot && pilot->catalog->count==2);
    unsigned trees=0;
    for(unsigned seed=0;seed<1000;++seed){
        struct G2_World w;struct G2_Entry e;
        size_t bytes=G2_StorageBytes(pilot,1);void *storage=malloc(bytes);assert(storage);
        assert(G2_Begin(&w,pilot,1,&e,storage,bytes,seed));finish(&w);assert(!w.failed);
        trees+=verify(pilot,&e);free(storage);
    }
    assert(trees>0);
    /* Wider synthetic domain exercises heap/decrease-key and multiple full
     * modules. A native obstacle separates two components; portals stay open. */
    uint16_t native[256];uint8_t mask[32]={0};
    for(unsigned k=0;k<256;++k){
        unsigned x=k%16,y=k/16;native[k]=0x3001;
        if(x>0 && x<15 && y>0 && y<15 && x!=8)mask[k/8]|=(uint8_t)(1u<<(k%8));
        if(x==8)native[k]=0x3401;
    }
    struct G2_Map fixture={.mapId=999,.layoutId=999,.width=16,.height=16,
        .native=native,.editable=mask,.topology=mask,.catalog=pilot->catalog};
    unsigned repairSeed=0,found=0;
    for(unsigned seed=0;seed<100;++seed){
        struct G2_World w;struct G2_Entry e;size_t bytes=G2_StorageBytes(&fixture,1);
        void *storage=malloc(bytes);assert(storage);
        assert(G2_Begin(&w,&fixture,1,&e,storage,bytes,seed));finish(&w);
        assert(!w.failed);(void)verify(&fixture,&e);
        if(e.plan.placementCount){repairSeed=seed;found=1;}
        free(storage);
    }
    assert(found);
    /* A deliberately invalid tentative placement blocks an external port.
     * The navigation gate must remove that whole module and retry locally. */
    {
        struct G2_World w;struct G2_Entry e;size_t bytes=G2_StorageBytes(&fixture,1);
        void *storage=malloc(bytes);assert(storage);
        assert(G2_Begin(&w,&fixture,1,&e,storage,bytes,repairSeed));
        while(e.phase!=G2_PLAN_NAV_INIT)G2_Tick(&w,1);
        unsigned index=e.plan.placementCount-1;
        struct G2_Placement *p=&e.plan.placements[index];
        const struct G2_Module *old=&fixture.catalog->modules[p->module];
        for(unsigned y=0;y<old->height;++y)for(unsigned x=0;x<old->width;++x){
            unsigned k=(p->origin/16+y)*16+p->origin%16+x;
            e.tiles[k]=(e.plan.flags[k]&G2_CORRIDOR)?0x3001:G2_RenderCell(&fixture,repairSeed,k);
            e.plan.owner[k]=0;e.plan.flags[k]&=(uint8_t)~G2_MODULE;
        }
        unsigned tree=0;
        while(fixture.catalog->modules[tree].kind!=G2_MODULE_TREE)++tree;
        unsigned origin=256;
        for(unsigned y=1;y<14 && origin==256;++y)for(unsigned x=1;x<14;++x){
            int allowed=1,port=0;
            for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx){
                unsigned k=(y+dy)*16+x+dx;
                if(!soft(&fixture,k) || e.plan.owner[k])allowed=0;
                if(e.plan.flags[k]&G2_PORT)port=1;
            }
            if(allowed && port){origin=y*16+x;break;}
        }
        assert(origin<256);p->origin=(uint16_t)origin;p->module=(uint16_t)tree;
        for(unsigned y=0;y<2;++y)for(unsigned x=0;x<2;++x){
            unsigned k=(origin/16+y)*16+origin%16+x;assert(!e.plan.owner[k]);
            e.tiles[k]=fixture.catalog->modules[tree].cells[y*2+x];
            e.plan.owner[k]=(uint16_t)(index+1);e.plan.flags[k]|=G2_MODULE;
        }
        finish(&w);assert(!w.failed && e.plan.removedModules>0 && !e.plan.fallback);
        (void)verify(&fixture,&e);free(storage);
    }
    /* Remove one owned tile immediately before the footprint gate: rejection,
     * including the failed counter, rather than a three-quarter tree. */
    {
        struct G2_World w;struct G2_Entry e;size_t bytes=G2_StorageBytes(&fixture,1);
        void *storage=malloc(bytes);assert(storage);
        assert(G2_Begin(&w,&fixture,1,&e,storage,bytes,repairSeed));
        while(e.phase!=G2_PLAN_MODULE_CHECK)G2_Tick(&w,1);
        assert(e.plan.placementCount);
        unsigned k=e.plan.placements[e.plan.placementCount-1].origin;
        e.plan.owner[k]=0;e.plan.flags[k]&=(uint8_t)~G2_MODULE;e.tiles[k]=native[k];
        finish(&w);assert(e.phase==G2_FAILED && w.failed==1);
        assert(!G2_FindReady(&w,999,999,16,16));free(storage);
    }
    /* Failure in local planning has a bounded, complete baseline fallback. */
    {
        struct G2_World w;struct G2_Entry e;size_t bytes=G2_StorageBytes(&fixture,1);
        void *storage=malloc(bytes);assert(storage);
        assert(G2_Begin(&w,&fixture,1,&e,storage,bytes,22));w.work.capacity=0;
        finish(&w);assert(e.phase==G2_READY && e.plan.fallback && !e.plan.placementCount);
        for(unsigned k=0;k<256;++k)assert(e.tiles[k]==G2_RenderCell(&fixture,22,k));
        free(storage);
    }
    printf("PASS topology: 1000 native seeds, %u full trees; ports, directed-boundary substitution, parent paths, complete footprints, bounds, fallback\n",trees);
    return 0;
}
