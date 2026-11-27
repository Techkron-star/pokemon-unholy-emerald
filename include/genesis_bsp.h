#ifndef GUARD_GENESIS_BSP_H
#define GUARD_GENESIS_BSP_H
#include "genesis_core.h"
#define GENESIS_BSP_MAX_DEPTH 6
#define GENESIS_BSP_MAX_LEAVES 32
#define GENESIS_BSP_MAX_NODES 63
#define GENESIS_BSP_MAX_ROOMS 32
#define GENESIS_BSP_NONE 65535u
typedef struct {u16 x,y,w,h,parent;} GenesisBSPNode;
typedef struct {u16 x,y,w,h;} GenesisBSPRoom;
typedef struct {GenesisBSPNode nodes[GENESIS_BSP_MAX_NODES];GenesisBSPRoom rooms[GENESIS_BSP_MAX_ROOMS];u16 node_count,room_count;} GenesisBSP;
void GenesisBSP_Generate(GenesisBSP*,u16,u16,u32,u8,u8);
void GenesisBSP_Stamp(GenesisBSP*,u8*,u16,u16);
#endif
