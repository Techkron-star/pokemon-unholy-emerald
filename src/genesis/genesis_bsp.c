#include "genesis_bsp.h"
void GenesisBSP_Generate(GenesisBSP*b,u16 w,u16 h,u32 s,u8 mn,u8 mx) {
    GenesisRng r;
    u8 depth[GENESIS_BSP_MAX_NODES];
    u16 i,j;
    if(!b)return;
    b->node_count=b->room_count=0;
    if(w<8||h<8||w>64||h>64||!mn||mx<mn)return;
    GenesisRng_Init(&r,s);
    b->nodes[0]=(GenesisBSPNode) {
        0,0,w,h,GENESIS_BSP_NONE
    };
    b->node_count=1;
    depth[0]=0;
    for(i=0;i<b->node_count&&b->node_count+2<=GENESIS_BSP_MAX_NODES;i++) {
        GenesisBSPNode n=b->nodes[i],a=n,c=n;
        u16 min=(u16)(mn+2),cut,total;
        gbool hor,cv=(gbool)(n.w>=2*min),ch=(gbool)(n.h>=2*min);
        if(depth[i]>=6||(!cv&&!ch))continue;
        hor=(gbool)(ch&&(!cv||n.h>n.w||(n.h==n.w&&GenesisRng_Range(&r,2))));
        total=hor?n.h:n.w;
        cut=(u16)(min+GenesisRng_Range(&r,(u32)(total-2*min+1)));
        a.parent=c.parent=i;
        if(hor) {
            a.h=cut;
            c.y=(u16)(n.y+cut);
            c.h=(u16)(n.h-cut);
        }else {
            a.w=cut;
            c.x=(u16)(n.x+cut);
            c.w=(u16)(n.w-cut);
        }
        j=b->node_count;
        depth[j]=depth[j+1]=(u8)(depth[i]+1);
        b->nodes[j]=a;
        b->nodes[j+1]=c;
        b->node_count=(u16)(j+2);
    }
    for(i=0;i<b->node_count&&b->room_count<32;i++) {
        GenesisBSPNode n=b->nodes[i];
        GenesisBSPRoom rm;
        u16 rw,rh;
        for(j=(u16)(i+1);j<b->node_count;j++)if(b->nodes[j].parent==i)break;
        if(j<b->node_count||n.w<mn||n.h<mn)continue;
        rw=(u16)(n.w>=mn+2?n.w-2:n.w);
        rh=(u16)(n.h>=mn+2?n.h-2:n.h);
        if(rw>mx)rw=mx;
        if(rh>mx)rh=mx;
        rm.w=(u16)(mn+GenesisRng_Range(&r,(u32)(rw-mn+1)));
        rm.h=(u16)(mn+GenesisRng_Range(&r,(u32)(rh-mn+1)));
        {
            u16 pad_x=(u16)(n.w>=rm.w+2),pad_y=(u16)(n.h>=rm.h+2);
            rm.x=(u16)(n.x+pad_x+GenesisRng_Range(&r,(u32)(n.w-rm.w-2*pad_x+1)));
            rm.y=(u16)(n.y+pad_y+GenesisRng_Range(&r,(u32)(n.h-rm.h-2*pad_y+1)));
        }
        b->rooms[b->room_count++]=rm;
    }
}
static void line(u8*m,u16 w,u16 h,s32 x,s32 y,s32 a,s32 b) {
    while(x!=a||y!=b) {
        if(x>=0&&x<w&&y>=0&&y<h)m[(u32)y*w+(u32)x]=1;
        if(x!=a)x+=x<a?1:-1;
        else y+=y<b?1:-1;
    }
    if(x>=0&&x<w&&y>=0&&y<h)m[(u32)y*w+(u32)x]=1;
}
void GenesisBSP_Stamp(GenesisBSP*b,u8*m,u16 w,u16 h) {
    u16 i,x,y;
    if(!b||!m||!w||!h||w>64||h>64||b->room_count>32)return;
    for(i=0;i<b->room_count;i++) {
        GenesisBSPRoom r=b->rooms[i];
        for(y=r.y;y<h&&(u32)y<(u32)r.y+r.h;y++)for(x=r.x;x<w&&(u32)x<(u32)r.x+r.w;x++)m[(u32)y*w+x]=1;
        if(i) {
            GenesisBSPRoom p=b->rooms[i-1];
            if(p.x<w&&p.y<h&&r.x<w&&r.y<h&&p.w<=w-p.x&&p.h<=h-p.y&&r.w<=w-r.x&&r.h<=h-r.y)line(m,w,h,p.x+p.w/2,p.y+p.h/2,r.x+r.w/2,r.y+r.h/2);
        }
    }
}
