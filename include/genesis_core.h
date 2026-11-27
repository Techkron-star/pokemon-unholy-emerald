#ifndef GUARD_GENESIS_CORE_H
#define GUARD_GENESIS_CORE_H
#if defined(GENESIS_HOST) || defined(__GENESIS_HOST__)
#define GENESIS_BUILD_HOST 1
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32;
typedef int8_t s8; typedef int16_t s16; typedef int32_t s32;
typedef uint64_t u64; typedef int64_t s64;
#else
#include "global.h"
#endif
typedef u8 gbool;
#define GTRUE 1
#define GFALSE 0
#define GENESIS_VERSION 0x010100u
#define GENESIS_VERSION_MAJOR 1
#define GENESIS_VERSION_MINOR 1
#define GENESIS_VERSION_PATCH 0
#define GENESIS_MAX_W 64
#define GENESIS_MAX_H 64
#define GENESIS_MIN_DIM 8
#define GENESIS_MAX_CELLS (GENESIS_MAX_W * GENESIS_MAX_H)
enum { GENESIS_STREAM_TERRAIN, GENESIS_STREAM_ENCOUNTERS, GENESIS_STREAM_OBJECTS, GENESIS_STREAM_EVENTS, GENESIS_STREAM_BOSSES, GENESIS_STREAM_DECORATION, GENESIS_STREAM_SPAWNS, GENESIS_STREAM_GRAPH, GENESIS_STREAM_MAX };
typedef struct {u32 master_seed, stream_seed[GENESIS_STREAM_MAX];} GenesisCore;
typedef struct {u32 state;} GenesisRng;
void GenesisCore_Init(GenesisCore*,u32);
u32 GenesisCore_StreamSeed(const GenesisCore*,u8);
void GenesisCore_MixSeed(u32*,const char*);
void GenesisRng_Init(GenesisRng*,u32);
u32 GenesisRng_Next(GenesisRng*);
u32 GenesisRng_Range(GenesisRng*,u32);
u32 GenesisRng_Hash(u32,s32,s32);
u32 GenesisRng_Hash3(u32,s32,s32,s32);
#endif
