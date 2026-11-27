#include "genesis_core.h"
static u32 mix(u32 x) {
    x^=x>>16;
    x*=0x7feb352du;
    x^=x>>15;
    x*=0x846ca68bu;
    return x^(x>>16);
}
void GenesisCore_Init(GenesisCore*c,u32 s) {
    u8 i;
    c->master_seed=s;
    for(i=0;i<GENESIS_STREAM_MAX;i++)c->stream_seed[i]=mix(s^((u32)(i+1)*0x9e3779b9u));
}
u32 GenesisCore_StreamSeed(const GenesisCore*c,u8 i) {
    return i<GENESIS_STREAM_MAX?c->stream_seed[i]:0;
}
void GenesisCore_MixSeed(u32*s,const char*t) {
    u32 h=2166136261u;
    while(*t) {
        h^=(u8)*t++;
        h*=16777619u;
    }
    *s=mix(*s^h);
}
void GenesisRng_Init(GenesisRng*r,u32 s) {
    r->state=s?s:0x9e3779b9u;
}
u32 GenesisRng_Next(GenesisRng*r) {
    u32 x=r->state;
    x^=x<<13;
    x^=x>>17;
    x^=x<<5;
    return r->state=x;
}
/* xorshift visits 1..UINT32_MAX, not zero. Rejection uses that domain. */
u32 GenesisRng_Range(GenesisRng*r,u32 n) {
    u32 x,limit;
    unsigned i;
    if(!n)return 0;
    limit=0xffffffffu-(0xffffffffu%n);
    for(i=0;i<64;i++) {
        x=GenesisRng_Next(r);
        if(x<=limit)return (x-1)%n;
    }
    return (x-1)%n;
}
u32 GenesisRng_Hash(u32 s,s32 x,s32 y) {
    return mix(s^((u32)x*0x85ebca6bu)^((u32)y*0xc2b2ae35u));
}
u32 GenesisRng_Hash3(u32 s,s32 x,s32 y,s32 z) {
    return mix(s^((u32)x*0x85ebca6bu)^((u32)y*0xc2b2ae35u)^((u32)z*0x27d4eb2fu));
}
