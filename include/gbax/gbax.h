/* SPDX-License-Identifier: MPL-2.0
 * GBAX SDK v1, freestanding ARM7TDMI guest header (C99).
 * Copy this header and gbax-abi.h into the game project. */
#ifndef GBAX_SDK_H
#define GBAX_SDK_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "gbax-abi.h"

#define GBAX_REG32(addr) (*(volatile uint32_t*)(uintptr_t)(addr))
#define GBAX_REG16(addr) (*(volatile uint16_t*)(uintptr_t)(addr))
typedef struct {
    uint32_t version, features, workSize, romSize, multiplier;
} GBAXInfo;
static inline bool GBAXDetect(GBAXInfo* info) {
    if(!info)return false;
    info->version=info->features=info->workSize=info->romSize=info->multiplier=0;
    if(GBAX_REG32(GBAX_ABI_BASE)!=GBAX_ABI_MAGIC ||
       GBAX_REG32(GBAX_ABI_BASE+4)!=GBAX_ABI_SIGNATURE)return false;
    uint32_t version=GBAX_REG32(GBAX_ABI_BASE+8);
    if((version>>16)!=(GBAX_ABI_VERSION>>16))return false;
    info->version=version;
    info->features=GBAX_REG32(GBAX_ABI_BASE+12);
    info->workSize=GBAX_REG32(GBAX_ABI_BASE+16);
    info->romSize=GBAX_REG32(GBAX_ABI_BASE+20);
    info->multiplier=GBAX_REG32(GBAX_ABI_BASE+24);
    if(!(info->features&GBAX_CAP_RAM64) || info->workSize!=GBAX_ABI_WORK_SIZE ||
       info->romSize>GBAX_ABI_ROM_MAX) {
        info->features=0;return false;
    }
    return true;
}
static inline bool GBAXHas(const GBAXInfo* info,uint32_t capabilities) {
    return info && info->version && (info->features&capabilities)==capabilities;
}
static inline bool GBAXRange(uint32_t base,uint32_t size,uint32_t address,size_t bytes) {
    return bytes!=0 && address>=base && address-base<size &&
        bytes<=(size_t)(size-(address-base));
}
/* One root arena per game. Reserve subarenas for long-lived systems.
 * A rewind invalidates all allocations after the checkpoint; no implicit
 * clearing, no thread/IRQ safety, no general-purpose free. */
typedef struct {uint32_t begin,end,cursor;} GBAXArena;
static inline bool GBAXArenaInit(GBAXArena* a,const GBAXInfo* info) {
    if(!a)return false;
    a->begin=a->end=a->cursor=0;
    if(!GBAXHas(info,GBAX_CAP_RAM64))return false;
    a->begin=a->cursor=GBAX_ABI_WORK_BASE;
    a->end=GBAX_ABI_WORK_BASE+GBAX_ABI_WORK_SIZE;
    return true;
}
static inline void* GBAXAlloc(GBAXArena* a,size_t size,unsigned alignment) {
    if(!a || a->begin<GBAX_ABI_WORK_BASE || a->end>GBAX_ABI_STAGE_BASE ||
       a->cursor<a->begin || a->cursor>a->end || !size ||
       !alignment || alignment>65536 || (alignment&(alignment-1)))return NULL;
    uint32_t start=(a->cursor+alignment-1)&~(uint32_t)(alignment-1);
    if(start>a->end || size>(size_t)(a->end-start))return NULL;
    a->cursor=start+(uint32_t)size;
    return (void*)(uintptr_t)start;
}
static inline bool GBAXReserve(GBAXArena* parent,GBAXArena* child,size_t size,unsigned alignment) {
    if(!child || child==parent)return false;
    void* p=GBAXAlloc(parent,size,alignment);
    if(!p)return false;
    child->begin=child->cursor=(uint32_t)(uintptr_t)p;
    child->end=child->begin+(uint32_t)size;
    return true;
}
static inline uint32_t GBAXCheckpoint(const GBAXArena* a) {return a?a->cursor:0;}
static inline bool GBAXRewind(GBAXArena* a,uint32_t mark) {
    if(!a || mark<a->begin || mark>a->cursor)return false;
    a->cursor=mark;return true;
}
static inline size_t GBAXRemaining(const GBAXArena* a) {
    return a && a->cursor<=a->end?a->end-a->cursor:0;
}
static inline const void* GBAXAsset(const GBAXInfo* info,uint32_t offset,size_t size) {
    if(!GBAXHas(info,GBAX_CAP_ROM128) || offset>=info->romSize ||
       !size || size>info->romSize-offset)return NULL;
    return (const void*)(uintptr_t)(GBAX_ABI_ROM_BASE+offset);
}
/* Caller owns DMA3 (no other DMA3 users or interrupt handlers may use it).
 * Only aligned, non-overlapping copies, max 65536 words per transfer.
 * RAM destination includes work and staging; use staging only deliberately.
 * Native source/destination limited to physically backed EWRAM/IWRAM.
 * ROM128 source is allowed, ROM destination is never allowed. */
static inline bool GBAXRamRange(uint32_t a,size_t n) {
    return GBAXRange(GBAX_ABI_WORK_BASE,GBAX_ABI_WORK_SIZE+GBAX_ABI_STAGE_SIZE,a,n) ||
           GBAXRange(0x02000000u,0x40000u,a,n) ||
           GBAXRange(0x03000000u,0x8000u,a,n);
}
static inline bool GBAXCopy32(const GBAXInfo* info,void* dest,const void* src,uint32_t words) {
    if(!GBAXHas(info,GBAX_CAP_DMA3) || !words || words>65536)return false;
    uint32_t d=(uint32_t)(uintptr_t)dest,s=(uint32_t)(uintptr_t)src,n=words*4;
    if((d|s)&3u || !GBAXRamRange(d,n))return false;
    if(!GBAXRamRange(s,n) && !(GBAXHas(info,GBAX_CAP_ROM128) &&
        GBAXRange(GBAX_ABI_ROM_BASE,info->romSize,s,n)))return false;
    if(s<d+n && d<s+n)return false;
    /* Keep native DMA3 from being reprogrammed between setup and completion. */
    uint16_t ime=GBAX_REG16(0x04000208u);
    GBAX_REG16(0x04000208u)=0;
    if(GBAX_REG16(0x040000deu)&0x8000u) {
        GBAX_REG16(0x04000208u)=ime;return false;
    }
    GBAX_REG32(0x040000d4u)=s;
    GBAX_REG32(0x040000d8u)=d;
    GBAX_REG16(0x040000dcu)=(uint16_t)words;
    GBAX_REG16(0x040000deu)=0x8400u;
    /* Immediate DMA blocks the emulated CPU until completion. */
    while(GBAX_REG16(0x040000deu)&0x8000u) {}
    GBAX_REG16(0x04000208u)=ime;
    return true;
}
enum { GBAX_SDK_UNAVAILABLE=0, GBAX_SDK_READY=1, GBAX_SDK_LOADED=2,
       GBAX_SDK_EMPTY=3, GBAX_SDK_COMMITTED=4 };
static inline uint32_t GBAXSaveStatus(const GBAXInfo* info) {
    if(!GBAXHas(info,GBAX_CAP_SAVX))return GBAX_SDK_UNAVAILABLE;
    return GBAX_REG32(GBAX_ABI_SAVX_BASE+4);
}
static inline uint32_t GBAXLoadSave(const GBAXInfo* info) {
    if(!GBAXHas(info,GBAX_CAP_SAVX))return GBAX_SDK_UNAVAILABLE;
    GBAX_REG32(GBAX_ABI_SAVX_BASE+8)=1;
    return GBAXSaveStatus(info);
}
static inline uint32_t GBAXCommitSave(const GBAXInfo* info) {
    if(!GBAXHas(info,GBAX_CAP_SAVX))return GBAX_SDK_UNAVAILABLE;
    GBAX_REG32(GBAX_ABI_SAVX_BASE+8)=2;
    return GBAXSaveStatus(info);
}
static inline void* GBAXSaveBuffer(const GBAXInfo* info,uint32_t offset,size_t bytes) {
    if(!GBAXHas(info,GBAX_CAP_SAVX) || offset>=GBAX_ABI_STAGE_SIZE ||
       !bytes || bytes>GBAX_ABI_STAGE_SIZE-offset)return NULL;
    uint32_t status=GBAXSaveStatus(info);
    if(status!=GBAX_SDK_EMPTY && status!=GBAX_SDK_LOADED &&
       status!=GBAX_SDK_COMMITTED)return NULL;
    return (void*)(uintptr_t)(GBAX_ABI_STAGE_BASE+offset);
}
#endif
