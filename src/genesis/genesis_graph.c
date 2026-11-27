#include "genesis_graph.h"
#include <string.h>
static gbool has(const GenesisGraph*g,u8 a,u8 b) {
    u8 i;
    for(i=0;i<g->nodes[a].edge_count;i++)if(g->nodes[a].edges[i]==b)return 1;
    return 0;
}
static void edge(GenesisGraph*g,u8 a,u8 b) {
    GenesisGraphNode*x=&g->nodes[a],*y=&g->nodes[b];
    if(a==b||x->edge_count>=4||y->edge_count>=4||has(g,a,b))return;
    x->edges[x->edge_count++]=b;
    y->edges[y->edge_count++]=a;
}
void GenesisGraph_Generate(GenesisGraph*g,u32 s,u8 n) {
    GenesisRng r;
    u8 i,spine,previous;
    if(!g)return;
    memset(g,0,sizeof(*g));
    if(n<5)n=5;
    if(n>30)n=30;
    GenesisRng_Init(&r,s);
    g->node_count=n;
    g->entrance=0;
    g->boss=(u8)(n-2);
    g->exit_node=(u8)(n-1);
    g->nodes[0].type=GENESIS_NODE_ENTRANCE;
    spine=(u8)(2+(n-5)/5);
    previous=0;
    for(i=1;i<=spine;i++) {
        g->nodes[i].type=i==spine?GENESIS_NODE_ELITE:GENESIS_NODE_COMBAT;
        g->nodes[i].depth=i;
        edge(g,previous,i);
        previous=i;
    }
    g->nodes[g->boss].type=GENESIS_NODE_BOSS;
    g->nodes[g->boss].depth=(u8)(spine+1);
    g->nodes[g->exit_node].type=GENESIS_NODE_EXIT;
    g->nodes[g->exit_node].depth=(u8)(spine+2);
    edge(g,previous,g->boss);
    edge(g,g->boss,g->exit_node);
    for(i=(u8)(spine+1);i<g->boss;i++) {
        u8 j,pool[30],k=0,a;
        for(j=0;j<i;j++)if(g->nodes[j].edge_count<4&&g->nodes[j].depth+1<g->nodes[g->boss].depth)pool[k++]=j;
        if(!k)return;
        a=pool[GenesisRng_Range(&r,k)];
        g->nodes[i].type=(u8)(GENESIS_NODE_COMBAT+GenesisRng_Range(&r,7));
        g->nodes[i].depth=(u8)(g->nodes[a].depth+1);
        edge(g,a,i);
    }
    for(i=0;i<8;i++) {
        u8 a=(u8)GenesisRng_Range(&r,g->boss),b=(u8)GenesisRng_Range(&r,g->boss);
        int d=(int)g->nodes[a].depth-g->nodes[b].depth;
        if(d>=-1&&d<=1)edge(g,a,b);
    }
}
gbool GenesisGraph_Validate(const GenesisGraph*g) {
    u8 i,j,k,count=0,head=0,tail=0,q[30],visited[30]= {
        0
    };
    if(!g||g->node_count<5||g->node_count>30||g->entrance>=g->node_count||g->boss>=g->node_count||g->exit_node>=g->node_count||g->entrance==g->boss||g->entrance==g->exit_node||g->boss==g->exit_node)return 0;
    if(g->nodes[g->entrance].type!=GENESIS_NODE_ENTRANCE||g->nodes[g->boss].type!=GENESIS_NODE_BOSS||g->nodes[g->exit_node].type!=GENESIS_NODE_EXIT)return 0;
    for(i=0;i<g->node_count;i++) {
        const GenesisGraphNode*n=&g->nodes[i];
        if(n->type>=GENESIS_NODE_TYPE_COUNT||!n->edge_count||n->edge_count>4)return 0;
        if((n->type==GENESIS_NODE_ENTRANCE&&i!=g->entrance)||(n->type==GENESIS_NODE_BOSS&&i!=g->boss)||(n->type==GENESIS_NODE_EXIT&&i!=g->exit_node))return 0;
        for(j=0;j<n->edge_count;j++) {
            u8 b=n->edges[j];
            if(b>=g->node_count||b==i||g->nodes[b].edge_count>4||!has(g,b,i))return 0;
            for(k=0;k<j;k++)if(n->edges[k]==b)return 0;
        }
    }
    if(g->nodes[g->entrance].depth!=0||g->nodes[g->exit_node].depth!=g->nodes[g->boss].depth+1)return 0;
    for(i=0;i<g->node_count;i++)if(i!=g->exit_node&&i!=g->boss&&g->nodes[i].depth>=g->nodes[g->boss].depth)return 0;
    if(g->nodes[g->exit_node].edge_count!=1||g->nodes[g->exit_node].edges[0]!=g->boss)return 0;
    visited[g->entrance]=1;
    q[tail++]=g->entrance;
    while(head<tail) {
        u8 a=q[head++];
        count++;
        for(j=0;j<g->nodes[a].edge_count;j++) {
            u8 b=g->nodes[a].edges[j];
            if(!visited[b]) {
                visited[b]=1;
                q[tail++]=b;
            }
        }
    }
    return (gbool)(count==g->node_count);
}
