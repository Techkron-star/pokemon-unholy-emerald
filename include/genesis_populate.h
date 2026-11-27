#ifndef GUARD_GENESIS_POPULATE_H
#define GUARD_GENESIS_POPULATE_H
#include "genesis_map.h"
#define GENESIS_SPAWN_MAX 64
typedef enum {GENESIS_SPAWN_ENCOUNTER,GENESIS_SPAWN_NPC,GENESIS_SPAWN_OBJECT,GENESIS_SPAWN_PORTAL,GENESIS_SPAWN_EVENT,GENESIS_SPAWN_BOSS,GENESIS_SPAWN_TYPE_COUNT} GenesisSpawnType;
typedef struct {u16 x,y;u8 type,subtype;u32 seed_hash;} GenesisSpawn;
typedef struct {GenesisSpawn entries[64];u16 count;} GenesisSpawnList;
typedef struct {u8 encounters,npcs,objects,portals,events,bosses;} GenesisSpawnConfig;
u16 GenesisPopulate_Requested(const GenesisSpawnConfig*);
gbool GenesisPopulate_Generate(GenesisSpawnList*,GenesisMap*,const GenesisSpawnConfig*,const GenesisCore*);
#endif
