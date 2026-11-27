#include "genesis_map.h"
static const s8 dx[]= {
    0,0,1,-1
},dy[]= {
    -1,1,0,0
};
static u32 index_at(const GenesisMap*m,u16 x,u16 y) {
    return (u32)y*m->w+x;
}
static gbool neighbor(const GenesisMap*m,u16 i,u8 d,u16*out) {
    s32 x=(s32)(i%m->w)+dx[d],y=(s32)(i/m->w)+dy[d];
    if(x<0||y<0||x>=m->w||y>=m->h)return 0;
    *out=(u16)((u32)y*m->w+(u32)x);
    return 1;
}
gbool GenesisMap_Init(GenesisMap*m,u16 w,u16 h,u8*mask,u8*scratch,u16*labels,u8*parent,u16*cells,u16*q) {
    if(!m||!mask||!scratch||!labels||!parent||!cells||!q||w<8||h<8||w>64||h>64)return 0;
    *m=(GenesisMap) {
        .w=w,.h=h,.mask=mask,.scratch=scratch,.labels=labels,.parent_dir=parent,.cells=cells,.queue=q
    };
    return 1;
}
u16 GenesisMap_FloodFill(GenesisMap*m) {
    u16 i,n=(u16)(m->w*m->h),label=0;
    for(i=0;i<n;i++)m->labels[i]=0;
    for(i=0;i<n;i++) {
        u16 head=0,tail=0;
        if(!m->mask[i]||m->labels[i])continue;
        label++;
        m->labels[i]=label;
        m->queue[tail++]=i;
        while(head<tail) {
            u16 a=m->queue[head++],b;
            u8 d;
            for(d=0;d<4;d++)if(neighbor(m,a,d,&b)&&m->mask[b]&&!m->labels[b]) {
                m->labels[b]=label;
                m->queue[tail++]=b;
            }
        }
    }
    return m->region_count=label;
}
gbool GenesisMap_ValidateConnectivity(GenesisMap*m) {
    return (gbool)(GenesisMap_FloodFill(m)==1);
}
/* Multi-source BFS from region 1: shortest cardinal corridor to any other region. */
static gbool join(GenesisMap*m) {
    u16 i,n=(u16)(m->w*m->h),head=0,tail=0,target=65535;
    for(i=0;i<n;i++) {
        m->parent_dir[i]=GENESIS_MAP_PARENT_NONE;
        if(m->labels[i]==1) {
            m->parent_dir[i]=GENESIS_MAP_PARENT_ROOT;
            m->queue[tail++]=i;
        }
    }while(head<tail&&target==65535) {
        u16 a=m->queue[head++],b;
        u8 d;
        for(d=0;d<4;d++)if(neighbor(m,a,d,&b)&&m->parent_dir[b]==GENESIS_MAP_PARENT_NONE) {
            m->parent_dir[b]=(u8)(d^1);
            m->queue[tail++]=b;
            if(m->labels[b]>1) {
                target=b;
                break;
            }
        }
    }
    if(target==65535)return 0;
    for(i=0;i<n;i++) {
        u8 d=m->parent_dir[target];
        u16 b;
        m->mask[target]=1;
        if(d==GENESIS_MAP_PARENT_ROOT)return 1;
        if(d>=4||!neighbor(m,target,d,&b))return 0;
        target=b;
    }
    return 0;
}
gbool GenesisMap_JoinAllRegions(GenesisMap*m) {
    u16 r=GenesisMap_FloodFill(m),i;
    if(!r)return 0;
    for(i=0;r>1&&i<GENESIS_MAP_MAX_REPAIRS;i++) {
        if(!join(m))return 0;
        r=GenesisMap_FloodFill(m);
    }
    return (gbool)(r==1);
}
static u16 dist(u16 a,u16 b) {
    return a>b?(u16)(a-b):(u16)(b-a);
}
static void connect_end(GenesisMap*m,u16 x,u16 y) {
    u16 a,b,bx=x,by=y,best=65535;
    for(b=0;b<m->h;b++)for(a=0;a<m->w;a++)if(m->mask[index_at(m,a,b)]) {
        u16 d=(u16)(dist(a,x)+dist(b,y));
        if(d<best) {
            best=d;
            bx=a;
            by=b;
        }
    }
    if(best==65535) {
        bx=(u16)(m->w/2);
        by=(u16)(m->h/2);
    }while(x!=bx||y!=by) {
        m->mask[index_at(m,x,y)]=1;
        if(x!=bx)x=(u16)(x+(x<bx?1:-1));
        else y=(u16)(y+(y<by?1:-1));
    }
    m->mask[index_at(m,x,y)]=1;
}
static void select_side(GenesisMap*m,GenesisRng*r,u16 y,u8 count,u16*xs,u16*ys) {
    u16 pool[GENESIS_MAX_W];
    u16 x,n=0,i;
    /* corners excluded: top/bottom never share endpoints. */for(x=1;x+1<m->w;x++)pool[n++]=x;
    for(i=0;i<count;i++) {
        u16 j=(u16)GenesisRng_Range(r,n);
        xs[i]=pool[j];
        ys[i]=y;
        pool[j]=pool[--n];
        connect_end(m,xs[i],y);
    }
}
gbool GenesisMap_PlaceEnds(GenesisMap*m,u32 seed,u8 en,u8 ex) {
    GenesisRng r;
    if(!m||en>4||ex>4||en>m->w-2||ex>m->w-2)return 0;
    m->entrance_count=m->exit_count=0;
    GenesisRng_Init(&r,seed);
    select_side(m,&r,0,en,m->entrance_x,m->entrance_y);
    select_side(m,&r,(u16)(m->h-1),ex,m->exit_x,m->exit_y);
    m->entrance_count=en;
    m->exit_count=ex;
    return GenesisMap_ValidateConnectivity(m);
}
gbool GenesisMap_EnsureBorderWalkableEntrances(GenesisMap*m) {
    u16 i;
    for(i=0;i<m->entrance_count;i++) {
        if(m->entrance_x[i]>=m->w||m->entrance_y[i]>=m->h)return 0;
        connect_end(m,m->entrance_x[i],m->entrance_y[i]);
    }
    for(i=0;i<m->exit_count;i++) {
        if(m->exit_x[i]>=m->w||m->exit_y[i]>=m->h)return 0;
        connect_end(m,m->exit_x[i],m->exit_y[i]);
    }
    return GenesisMap_ValidateConnectivity(m);
}
void GenesisMap_Emergency(GenesisMap*m,u32 seed) {
    u16 x,y,w=m->w,h=m->h;
    u16 left=(u16)(w/4),right=(u16)(w-left),mid=(u16)(left+GenesisRng_Hash(seed,0,0)%(right-left));
    for(y=0;y<h;y++)for(x=0;x<w;x++)m->mask[index_at(m,x,y)]=(u8)((x>=left&&x<right&&y>0&&y+1<h)||x==mid);
    m->entrance_count=m->exit_count=0;
}
