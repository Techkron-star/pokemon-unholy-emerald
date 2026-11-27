#ifndef GUARD_GENESIS_MAP_H
#define GUARD_GENESIS_MAP_H
#include "genesis_core.h"
#define GENESIS_MAP_MAX_W GENESIS_MAX_W
#define GENESIS_MAP_MAX_H GENESIS_MAX_H
#define GENESIS_MAP_MAX_ENDS 4
#define GENESIS_MAP_MAX_REPAIRS 64
#define GENESIS_MAP_PARENT_ROOT 254
#define GENESIS_MAP_PARENT_NONE 255
typedef struct {u16 w,h;u8 *mask,*scratch;u16 *labels;u8 *parent_dir;u16 *cells,*queue;u16 region_count,entrance_count,entrance_x[4],entrance_y[4],exit_count,exit_x[4],exit_y[4];} GenesisMap;
gbool GenesisMap_Init(GenesisMap*,u16,u16,u8*,u8*,u16*,u8*,u16*,u16*);
u16 GenesisMap_FloodFill(GenesisMap*);
gbool GenesisMap_ValidateConnectivity(GenesisMap*);
gbool GenesisMap_JoinAllRegions(GenesisMap*);
gbool GenesisMap_PlaceEnds(GenesisMap*,u32,u8,u8);
gbool GenesisMap_EnsureBorderWalkableEntrances(GenesisMap*);
void GenesisMap_Emergency(GenesisMap*,u32);
#endif
