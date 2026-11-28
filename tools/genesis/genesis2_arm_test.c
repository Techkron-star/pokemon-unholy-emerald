/* Synthetic native-entry regression: executes the actual ROM's ARM code.
 * No owned saves, no user input or production save files are used. */
#include <mgba/core/core.h>
#include <mgba/core/timing.h>
#include <mgba/gba/core.h>
#include <mgba/internal/gba/gba.h>
#include "genesis2/world.h"
#include "genesis2/assets.generated.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct mCore *core;
static uint32_t call(uint32_t fn)
{
    const uint32_t stop=0x03007E00;
    uint32_t before=mTimingCurrentTime(&((struct GBA *)core->board)->timing);
    int32_t pc=0;
    unsigned steps=0;
    assert(core->writeRegister(core,"cpsr",0xFF));
    assert(core->writeRegister(core,"sp",0x03007C00));
    assert(core->writeRegister(core,"lr",stop|1));
    assert(core->writeRegister(core,"pc",fn&~1u));
    do {
        core->step(core);
        assert(core->readRegister(core,"pc",&pc));
        if (++steps>2000000) { fprintf(stderr,"function %08x timed out at %08x\n",fn,(unsigned)pc); abort(); }
    } while ((uint32_t)pc!=stop+2);
    return mTimingCurrentTime(&((struct GBA *)core->board)->timing)-before;
}
static uint32_t read32(uint32_t a) { return core->busRead32(core,a); }
static void write32(uint32_t a,uint32_t v) { core->busWrite32(core,a,v); }
static void native_grid(const struct G2_Map *m,uint32_t base)
{
    for(unsigned y=0;y<m->height;++y)for(unsigned x=0;x<m->width;++x)
        core->busWrite16(core,base+2*((y+7)*(m->width+15)+x+7),m->native[y*m->width+x]);
}
static uint32_t live_cell(const struct G2_Map *m,uint32_t base,unsigned k)
{return base+2*((k/m->width+7)*(m->width+15)+k%m->width+7);}
static void arguments(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e)
{
    assert(core->writeRegister(core,"r0",a));assert(core->writeRegister(core,"r1",b));
    assert(core->writeRegister(core,"r2",c));assert(core->writeRegister(core,"r3",d));
    write32(0x03007C00,e);
}
int main(int argc,char **argv)
{
    assert(argc==22);
    uint32_t s[20];
    for(unsigned i=0;i<20;++i)s[i]=(uint32_t)strtoul(argv[i+2],NULL,16);
    /* stats,start,tick,apply,commit,load,save1ptr,save2ptr,save3ptr,
     * save1,save2,save3,mapHeader,backup,backupData,mapGroups,rng */
    core=GBACoreCreate(); assert(core && core->init(core));
    mCoreInitConfig(core,NULL); core->opts.useBios=false; core->opts.skipBios=true;
    assert(mCoreLoadFile(core,argv[1])); core->reset(core);
    core->busWrite16(core,0x04000208,0);
    write32(s[6],s[9]); write32(s[7],s[10]); write32(s[8],s[11]);
    write32(s[16],getenv("G2_ARM_SEED")?(uint32_t)strtoul(getenv("G2_ARM_SEED"),NULL,0):0x12345678);
    call(s[1]);
    if(!((struct GBA *)core->board)->memory.gbax) {
        assert(read32(s[0]+32)==1);
        call(s[4]); call(s[5]);
        assert(read32(s[0]+32)==1);
        puts("PASS ARM runtime: unmarked ROM preserves native fallback without extended access");
        core->unloadROM(core); mCoreConfigDeinit(&core->config); core->deinit(core);
        return 0;
    }
    assert(read32(s[0]+32)==2);
    uint32_t maxTick=0,ticks=0;
    while(read32(s[0]+32)==2) {
        uint32_t cycles=call(s[2]);
        if(cycles>maxTick)maxTick=cycles;
        assert(++ticks<10000);
    }
    assert(read32(s[0]+32)==3 && read32(s[0]+4)==sizeof(sG2Maps)/sizeof(sG2Maps[0]));
    uint32_t seed=read32(s[0]);
    call(s[4]);
    call(s[5]); /* validated saved identity rebuilds exactly the same world */
    assert(read32(s[0])==seed && read32(s[0]+32)==2);
    while(read32(s[0]+32)==2) call(s[2]);
    uint32_t maxApply=0, changed=0,expectedHits=0,moduleTests=0;
    for(unsigned i=1;i<sizeof(sG2Maps)/sizeof(sG2Maps[0]);++i) {
        const struct G2_Map *m=&sG2Maps[i];
        struct G2_World hostWorld;struct G2_Entry expectedEntry;
        size_t hostBytes=G2_StorageBytes(m,1);void *hostStorage=malloc(hostBytes);assert(hostStorage);
        assert(G2_Begin(&hostWorld,m,1,&expectedEntry,hostStorage,hostBytes,seed));
        while(!G2_Complete(&hostWorld))G2_Tick(&hostWorld,257);
        assert(expectedEntry.phase==G2_READY);
        uint32_t group=read32(s[15]+4*(m->mapId>>8));
        uint32_t header=read32(group+4*(m->mapId&255));
        for(unsigned j=0;j<28;j+=4)write32(s[12]+j,read32(header+j));
        assert(core->busRead16(core,s[12]+0x12)==m->layoutId);
        core->busWrite8(core,s[9]+4,m->mapId>>8);
        core->busWrite8(core,s[9]+5,m->mapId&255);
        write32(s[13],m->width+15); write32(s[13]+4,m->height+14); write32(s[13]+8,s[14]);
        native_grid(m,s[14]);
        uint32_t nTicks=read32(s[0]+8), cycles=call(s[3]);
        ++expectedHits;
        if(cycles>maxApply)maxApply=cycles;
        assert(read32(s[0]+8)==nTicks);
        uint32_t editedLive=0;
        for(unsigned y=0;y<m->height;++y)for(unsigned x=0;x<m->width;++x) {
            uint16_t expected=expectedEntry.tiles[y*m->width+x];
            uint16_t got=core->busRead16(core,s[14]+2*((y+7)*(m->width+15)+x+7));
            assert(got==expected);
            if(got!=m->native[y*m->width+x]) {
                ++changed;
                editedLive=s[14]+2*((y+7)*(m->width+15)+x+7);
            }
        }
        /* A script change survives a second publication. */
        uint32_t live=editedLive?editedLive:s[14]+2*(7*(m->width+15)+7);
        core->busWrite16(core,live,0xFFFF);
        call(s[3]);++expectedHits; assert(core->busRead16(core,live)==0xFFFF);
        for(unsigned p=0;p<expectedEntry.plan.placementCount;++p){
            const struct G2_Placement *place=&expectedEntry.plan.placements[p];
            const struct G2_Module *module=&m->catalog->modules[place->module];
            if(module->kind!=G2_MODULE_TREE)continue;
            unsigned origin=place->origin,ox=origin%m->width,oy=origin/m->width;
            /* One live delta must prevent the entire four-cell tree. */
            native_grid(m,s[14]);core->busWrite16(core,live_cell(m,s[14],origin),0xFFFF);
            call(s[3]);++expectedHits;
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned k=(oy+y)*m->width+ox+x;
                assert(core->busRead16(core,live_cell(m,s[14],k))==(k==origin?0xFFFF:m->native[k]));
            }
            /* A saved player position inside the footprint leaves it native. */
            native_grid(m,s[14]);core->busWrite16(core,s[9],ox);core->busWrite16(core,s[9]+2,oy);
            call(s[3]);++expectedHits;
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned k=(oy+y)*m->width+ox+x;
                assert(core->busRead16(core,live_cell(m,s[14],k))==m->native[k]);
            }
            core->busWrite16(core,s[9],0);core->busWrite16(core,s[9]+2,0);
            native_grid(m,s[14]);call(s[3]);++expectedHits;
            const uint32_t view=0x0203F000;
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned j=y*module->width+x,k=(oy+y)*m->width+ox+x;
                core->busWrite16(core,view+j*2,j?expectedEntry.tiles[k]:0xFFFF);
            }
            arguments(ox+7,oy+7,view,module->width,module->height);call(s[17]);
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned k=(oy+y)*m->width+ox+x;uint16_t value=core->busRead16(core,view+(y*module->width+x)*2);
                int32_t restore;
                arguments(ox+x+7,oy+y+7,value,0,0);call(s[18]);assert(core->readRegister(core,"r0",&restore));
                if(restore)core->busWrite16(core,live_cell(m,s[14],k),value);
                assert(core->busRead16(core,live_cell(m,s[14],k))==(k==origin?0xFFFF:m->native[k]));
            }
            /* A later on-load tile change also retires the complete module. */
            native_grid(m,s[14]);call(s[3]);++expectedHits;
            core->busWrite16(core,live_cell(m,s[14],origin),0xFFFF);call(s[19]);
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned k=(oy+y)*m->width+ox+x;
                assert(core->busRead16(core,live_cell(m,s[14],k))==(k==origin?0xFFFF:m->native[k]));
            }
            /* Native mode must not reintroduce trees from its saved view. */
            uint16_t options=core->busRead16(core,s[10]+0x14);
            core->busWrite16(core,s[10]+0x14,(uint16_t)((options&0x1FFFu)|0xA000u));
            native_grid(m,s[14]);call(s[3]);++expectedHits;
            for(unsigned y=0;y<module->height;++y)for(unsigned x=0;x<module->width;++x){
                unsigned k=(oy+y)*m->width+ox+x;int32_t restore;
                arguments(ox+x+7,oy+y+7,expectedEntry.tiles[k],0,0);call(s[18]);
                assert(core->readRegister(core,"r0",&restore) && !restore);
                assert(core->busRead16(core,live_cell(m,s[14],k))==m->native[k]);
            }
            core->busWrite16(core,s[10]+0x14,options);
            ++moduleTests;
        }
        printf("map %u %ux%u apply %.3f ms\n",m->mapId,m->width,m->height,cycles*1000.0/16777216.0);
        free(hostStorage);
    }
    assert(changed && read32(s[0]+16)==expectedHits);
    if(seed==12)assert(moduleTests);
    assert(maxTick < 100000); /* <6 ms at native 16.78 MHz in this fixture */
    assert(maxApply < 100000);
    /* A damaged identity is rejected, never reinitialized or overwritten. */
    core->busWrite8(core,s[11]+4+12+1040+4,0xFF);
    call(s[5]);
    assert(read32(s[0]+32)==0);
    assert(core->busRead8(core,s[11]+4+12+1040+4)==0xFF);
    printf("PASS ARM runtime: seed %08x, %u ticks, max tick %.3f ms, max publish %.3f ms, %u changed cells, %u atomic-module fixtures.\n",seed,ticks,maxTick*1000.0/16777216.0,maxApply*1000.0/16777216.0,changed,moduleTests);
    core->unloadROM(core); mCoreConfigDeinit(&core->config); core->deinit(core);
    return 0;
}
