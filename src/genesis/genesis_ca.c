#include "genesis_ca.h"
static gbool valid(u16 w,u16 h) {
    return w&&h&&w<=GENESIS_MAX_W&&h<=GENESIS_MAX_H;
}
void GenesisCA_Init(u8*m,u16 w,u16 h,u32 s,u8 fill) {
    GenesisRng r;
    u32 i;
    if(!m||!valid(w,h))return;
    GenesisRng_Init(&r,s);
    if(fill>100)fill=100;
    for(i=0;i<(u32)w*h;i++)m[i]=(u8)(GenesisRng_Range(&r,100)<fill);
}
void GenesisCA_Step(u8*m,u8*t,u16 w,u16 h,u8 threshold) {
    s32 x,y,dx,dy;
    u32 i;
    if(!m||!t||m==t||!valid(w,h))return;
    if(threshold>8)threshold=8;
    for(y=0;y<h;y++)for(x=0;x<w;x++) {
        u8 n=0;
        for(dy=-1;dy<=1;dy++)for(dx=-1;dx<=1;dx++) {
            s32 a=x+dx,b=y+dy;
            if((dx||dy)&&a>=0&&a<w&&b>=0&&b<h&&m[(u32)b*w+(u32)a])n++;
        }
        t[(u32)y*w+(u32)x]=(u8)(n>=threshold);
    }
    for(i=0;i<(u32)w*h;i++)m[i]=t[i];
}
void GenesisCA_Run(u8*m,u8*t,u16 w,u16 h,u32 s,u8 fill,u8 it,u8 th) {
    u8 i;
    GenesisCA_Init(m,w,h,s,fill);
    if(it>16)it=16;
    for(i=0;i<it;i++)GenesisCA_Step(m,t,w,h,th);
}
