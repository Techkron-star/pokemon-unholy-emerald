#include "genesis_api.h"
#ifndef GENESIS_BUILD_HOST
#include "fieldmap.h"
#include "global.fieldmap.h"
#include "tilesets.h"
#include "constants/metatile_labels.h"
#endif
gbool Genesis_CommitToMap(const GenesisWorld*w) {
#ifdef GENESIS_BUILD_HOST
    (void)w;
    return 0;
#else
    const struct MapLayout *layout=gMapHeader.mapLayout;
    if(!w||w->status>GENESIS_STATUS_EMERGENCY||!w->native_profile||!layout||!gBackupMapLayout.map)return 0;
    if(layout->width!=w->map.w||layout->height!=w->map.h||gBackupMapLayout.width!=layout->width+MAP_OFFSET_W||gBackupMapLayout.height!=layout->height+MAP_OFFSET_H)return 0;
    if((u32)gBackupMapLayout.width*(u32)gBackupMapLayout.height>MAX_MAP_DATA_SIZE)return 0;
    return Genesis_CommitToLayout(&w->map,gBackupMapLayout.map,MAX_MAP_DATA_SIZE,(u16)gBackupMapLayout.width,(u16)gBackupMapLayout.height,MAP_OFFSET,MAP_OFFSET);
#endif
}
gbool Genesis_GetGeneralProfile(GenesisBiomeId biome, GenesisBiomeProfile *p) {
#ifdef GENESIS_BUILD_HOST
    (void)biome;
    (void)p;
    return 0;
#else
    const struct MapLayout *layout=gMapHeader.mapLayout;
    if(!p||!layout||layout->primaryTileset!=&gTileset_General||layout->isFrlg||(biome!=GENESIS_BIOME_MEADOW&&biome!=GENESIS_BIOME_FOREST))return 0;
    GenesisBiome_CopyProfile(biome,p);
    p->metatile_ground=METATILE_General_Grass;
    p->metatile_water=METATILE_General_CalmWater;
    p->metatile_vegetation=METATILE_General_TallGrass;
    p->metatile_rock=METATILE_General_RockWall_RockBase;
    p->metatile_corridor=p->metatile_entrance=p->metatile_exit=METATILE_General_Grass;
    return 1;
#endif
}
