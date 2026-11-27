#ifndef GUARD_GENESIS_GRAPH_H
#define GUARD_GENESIS_GRAPH_H
#include "genesis_core.h"
#define GENESIS_GRAPH_MAX_NODES 30
#define GENESIS_GRAPH_MAX_EDGES 4
typedef enum {GENESIS_NODE_ENTRANCE,GENESIS_NODE_COMBAT,GENESIS_NODE_EXPLORE,GENESIS_NODE_CAPTURE,GENESIS_NODE_TREASURE,GENESIS_NODE_REST,GENESIS_NODE_EVENT,GENESIS_NODE_ELITE,GENESIS_NODE_BOSS,GENESIS_NODE_EXIT,GENESIS_NODE_TYPE_COUNT} GenesisNodeType;
typedef struct {u8 type,depth,edges[4],edge_count;} GenesisGraphNode;
typedef struct {GenesisGraphNode nodes[30];u8 node_count,entrance,boss,exit_node;} GenesisGraph;
void GenesisGraph_Generate(GenesisGraph*,u32,u8);
gbool GenesisGraph_Validate(const GenesisGraph*);
#endif
