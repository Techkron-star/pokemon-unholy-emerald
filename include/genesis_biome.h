#ifndef GUARD_GENESIS_BIOME_H
#define GUARD_GENESIS_BIOME_H
#include "genesis_map.h"
typedef enum {GENESIS_BIOME_MEADOW,GENESIS_BIOME_FOREST,GENESIS_BIOME_DESERT,GENESIS_BIOME_CAVE,GENESIS_BIOME_GLACIER,GENESIS_BIOME_VOLCANO,GENESIS_BIOME_RUINS,GENESIS_BIOME_ALIEN,GENESIS_BIOME_COUNT} GenesisBiomeId;
typedef struct {GenesisBiomeId id;const char*name;u8 ca_fill_percent,ca_iterations,ca_neighbor_threshold,bsp_min_room,bsp_max_room;u32 noise_scale_fx;u8 noise_octaves;u32 noise_persistence_fx,noise_lacunarity_fx;u8 water_threshold,vegetation_threshold,rock_threshold,elevation;u16 metatile_ground,metatile_water,metatile_vegetation,metatile_rock,metatile_corridor,metatile_entrance,metatile_exit;} GenesisBiomeProfile;
const GenesisBiomeProfile*GenesisBiome_Get(GenesisBiomeId);
void GenesisBiome_CopyProfile(GenesisBiomeId,GenesisBiomeProfile*);
gbool GenesisBiome_Validate(const GenesisBiomeProfile*);
void GenesisBiome_ApplyMask(const GenesisBiomeProfile*,GenesisMap*,u32);
void GenesisBiome_WriteCells(const GenesisBiomeProfile*,GenesisMap*,u32);
#endif
