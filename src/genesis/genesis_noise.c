#include "genesis_noise.h"
static u32 smooth(u32 t) {
    u32 a=(u32)(((u64)t*t)>>16);
    u32 b=(u32)(((u64)a*t)>>16);
    return 3*a-2*b;
}
static u32 lerp(u32 a,u32 b,u32 t) {
    return (u32)(((u64)a*(65536u-t)+(u64)b*t)>>16);
}
/* Explicit floor and wrap of lattice coordinates; no signed right shift. */
static u32 axis(s32 x,u32 scale,u32*f) {
    s64 p=(s64)x*scale,q=p/65536,r=p%65536;
    if(r<0) {
        q--;
        r+=65536;
    }
    *f=(u32)r;
    return (u32)q;
}
static u32 hash_corner(u32 seed,u32 x,u32 y) {
    /* reduce to signed-representable coordinates without implementation-defined casts */return GenesisRng_Hash3(seed,(s32)(x&0x7fffffffu),(s32)(y&0x7fffffffu),(s32)((x>>31)|((y>>31)<<1)))&65535u;
}
s32 GenesisNoise_Value2D(u32 seed,s32 x,s32 y,u32 scale) {
    u32 fx,fy,a,b,t,u;
    if(scale>GENESIS_NOISE_SCALE_MAX)scale=GENESIS_NOISE_SCALE_MAX;
    a=axis(x,scale,&fx);
    b=axis(y,scale,&fy);
    t=smooth(fx);
    u=smooth(fy);
    return (s32)lerp(lerp(hash_corner(seed,a,b),hash_corner(seed,a+1,b),t),lerp(hash_corner(seed,a,b+1),hash_corner(seed,a+1,b+1),t),u);
}
s32 GenesisNoise_FBM(u32 s,s32 x,s32 y,u32 scale,u8 oct,u32 persistence,u32 lac) {
    u64 sum=0;
    u32 weight=65536,total=0;
    u8 i;
    if(oct>8)oct=8;
    if(persistence>65536)persistence=65536;
    if(lac>262144)lac=262144;
    if(scale>GENESIS_NOISE_SCALE_MAX)scale=GENESIS_NOISE_SCALE_MAX;
    for(i=0;i<oct&&weight;i++) {
        u64 next;
        sum+=(u64)(u32)GenesisNoise_Value2D(s+(u32)i*0x9e3779b9u,x,y,scale)*weight;
        total+=weight;
        weight=(u32)(((u64)weight*persistence)>>16);
        next=((u64)scale*lac)>>16;
        scale=next>GENESIS_NOISE_SCALE_MAX?GENESIS_NOISE_SCALE_MAX:(u32)next;
    }
    return total?(s32)(sum/total):32768;
}
