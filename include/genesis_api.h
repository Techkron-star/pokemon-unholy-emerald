#ifndef GUARD_GENESIS_API_H
#define GUARD_GENESIS_API_H
#include "genesis_biome.h"
#include "genesis_graph.h"
#include "genesis_populate.h"
#define GENESIS_STATUS_OK 0
#define GENESIS_STATUS_EMERGENCY 1
#define GENESIS_STATUS_FAILED 2
typedef struct {u32 master_seed;u16 w,h;GenesisBiomeId biome;u8 entrances,exits;GenesisSpawnConfig spawn_cfg;gbool generate_graph;u8 graph_nodes;/* explicit, validated tileset profile; NULL = host-only defaults */const GenesisBiomeProfile *profile_override;} GenesisConfig;
typedef struct {u8 *mask,*scratch;u16 *labels;u8 *parent;u16 *cells,*queue;u32 capacity;} GenesisBuffers;
typedef struct {GenesisMap map;GenesisBiomeProfile profile;GenesisSpawnList spawns;GenesisGraph graph;GenesisConfig cfg;u32 version;u8 status;gbool native_profile;} GenesisWorld;
#ifdef GENESIS_BUILD_HOST
#define GENESIS_BUFFER_STORAGE static
#else
#define GENESIS_BUFFER_STORAGE static EWRAM_DATA
#endif
#define GENESIS_DECLARE_BUFFERS(name,w,h) \
 GENESIS_BUFFER_STORAGE u8 name##_mask[(w)*(h)] = {0}; \
 GENESIS_BUFFER_STORAGE u8 name##_scratch[(w)*(h)] = {0}; \
 GENESIS_BUFFER_STORAGE u16 name##_labels[(w)*(h)] = {0}; \
 GENESIS_BUFFER_STORAGE u8 name##_parent[(w)*(h)] = {0}; \
 GENESIS_BUFFER_STORAGE u16 name##_cells[(w)*(h)] = {0}; \
 GENESIS_BUFFER_STORAGE u16 name##_queue[(w)*(h)] = {0}; \
 static const GenesisBuffers name={name##_mask,name##_scratch,name##_labels,name##_parent,name##_cells,name##_queue,(u32)((w)*(h))}
gbool GenesisGenerate(GenesisWorld*,const GenesisConfig*,const GenesisBuffers*);
/* Pure helper, padded buffer layout matches Emerald. No partial writes on error. */
gbool Genesis_CommitToLayout(const GenesisMap*,u16*,u32,u16,u16,u16,u16);
/* Must be called after active layout initialized and before drawing; returns false on host. */
gbool Genesis_CommitToMap(const GenesisWorld*);
/* Validated General tileset mapping; only meadow/forest. */
gbool Genesis_GetGeneralProfile(GenesisBiomeId,GenesisBiomeProfile*);
#endif
