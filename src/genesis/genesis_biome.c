#include "genesis_biome.h"
#include "genesis_ca.h"
#include "genesis_bsp.h"
#include "genesis_noise.h"
/* Logical default tiles, deliberately NOT a verified GBA tileset. */
#define PROFILE(id,name,fill,it,th,mn,mx,freq,oct,water,veg,rock) {id,name,fill,it,th,mn,mx,freq,oct,32768,131072,water,veg,rock,3,1,2,3,4,5,6,7}
static const GenesisBiomeProfile profiles[]= {
    PROFILE(GENESIS_BIOME_MEADOW,"meadow",44,3,4,5,9,8192,3,25,155,238),
    PROFILE(GENESIS_BIOME_FOREST,"forest",41,4,4,4,8,12288,4,20,125,220),
    PROFILE(GENESIS_BIOME_DESERT,"desert",43,3,4,6,10,4096,2,12,220,232),
    PROFILE(GENESIS_BIOME_CAVE,"cave",40,5,4,3,7,16384,4,0,230,210),
    PROFILE(GENESIS_BIOME_GLACIER,"glacier",44,4,5,5,10,8192,3,40,180,235),
    PROFILE(GENESIS_BIOME_VOLCANO,"volcano",43,4,4,3,8,16384,4,38,215,195),
    PROFILE(GENESIS_BIOME_RUINS,"ruins",35,3,4,6,12,8192,3,15,215,225),
    PROFILE(GENESIS_BIOME_ALIEN,"alien",41,4,4,4,8,12288,5,30,140,220)
};
const GenesisBiomeProfile*GenesisBiome_Get(GenesisBiomeId i) {
    return &profiles[(unsigned)i<GENESIS_BIOME_COUNT?(unsigned)i:0];
}
void GenesisBiome_CopyProfile(GenesisBiomeId i,GenesisBiomeProfile*p) {
    *p=*GenesisBiome_Get(i);
}
gbool GenesisBiome_Validate(const GenesisBiomeProfile*p) {
    return (gbool)(p&&(unsigned)p->id<GENESIS_BIOME_COUNT&&p->ca_fill_percent<=100&&p->ca_iterations<=16&&p->ca_neighbor_threshold<=8&&p->bsp_min_room&&p->bsp_max_room>=p->bsp_min_room&&p->noise_scale_fx&&p->noise_scale_fx<=GENESIS_NOISE_SCALE_MAX&&p->noise_octaves&&p->noise_octaves<=8&&p->noise_persistence_fx<=65536&&p->noise_lacunarity_fx>=65536&&p->noise_lacunarity_fx<=262144&&p->water_threshold<=p->rock_threshold&&p->elevation<=15&&p->metatile_ground<=1023&&p->metatile_water<=1023&&p->metatile_vegetation<=1023&&p->metatile_rock<=1023&&p->metatile_corridor<=1023&&p->metatile_entrance<=1023&&p->metatile_exit<=1023);
}
static u8 noise8(const GenesisBiomeProfile*p,u32 s,u16 x,u16 y) {
    return (u8)((u32)GenesisNoise_FBM(s^0x5eed5eedu,x,y,p->noise_scale_fx,p->noise_octaves,p->noise_persistence_fx,p->noise_lacunarity_fx)>>8);
}
void GenesisBiome_ApplyMask(const GenesisBiomeProfile*p,GenesisMap*m,u32 s) {
#ifdef GENESIS_BUILD_HOST
    GenesisBSP b;
#else
    static EWRAM_DATA GenesisBSP b = {
        0
    };
#endif
    u16 x,y;
    GenesisCA_Run(m->mask,m->scratch,m->w,m->h,s,p->ca_fill_percent,p->ca_iterations,p->ca_neighbor_threshold);
    for(y=0;y<m->h;y++)for(x=0;x<m->w;x++) {
        u8 n=noise8(p,s,x,y);
        u32 k=(u32)y*m->w+x;
        m->scratch[k]=n;
        if(n<p->water_threshold||n>p->rock_threshold)m->mask[k]=0;
    }/* BSP stamped after noise so mandatory rooms/corridors remain open. */GenesisBSP_Generate(&b,m->w,m->h,s^0xb5b5b5b5u,p->bsp_min_room,p->bsp_max_room);
    GenesisBSP_Stamp(&b,m->mask,m->w,m->h);
}
void GenesisBiome_WriteCells(const GenesisBiomeProfile*p,GenesisMap*m,u32 s) {
    u16 x,y,i,e=(u16)((u16)p->elevation<<12);
    for(y=0;y<m->h;y++)for(x=0;x<m->w;x++) {
        u32 k=(u32)y*m->w+x;
        u8 n=noise8(p,s,x,y);
        u16 id;
        if(!m->mask[k])id=(u16)((n<p->water_threshold?p->metatile_water:p->metatile_rock)|0x0c00);
        else if(n<p->water_threshold||n>p->rock_threshold)id=p->metatile_corridor;
        else id=n>p->vegetation_threshold?p->metatile_vegetation:p->metatile_ground;
        m->cells[k]=(u16)(id|e);
    }
    for(i=0;i<m->entrance_count;i++)m->cells[(u32)m->entrance_y[i]*m->w+m->entrance_x[i]]=(u16)(p->metatile_entrance|e);
    for(i=0;i<m->exit_count;i++)m->cells[(u32)m->exit_y[i]*m->w+m->exit_x[i]]=(u16)(p->metatile_exit|e);
}
