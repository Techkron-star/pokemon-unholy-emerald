#ifndef GUARD_UNHOLY_EXT_RAM_H
#define GUARD_UNHOLY_EXT_RAM_H

#include "global.h"

// Optional Unholy Extended emulator protocol v1. Region 0x01 is unmapped
// on real GBA hardware. Always probe before dereferencing the data window.
#define UNHOLY_EXT_RAM_BASE     0x01000000u
#define UNHOLY_EXT_RAM_DATA     0x01001000u
#define UNHOLY_EXT_RAM_MAGIC    0x58454855u // UHEX, little-endian
#define UNHOLY_EXT_RAM_VERSION  1u
#define UNHOLY_EXT_RAM_MAX_SIZE (4u * 1024u * 1024u - 0x1000u)

bool8 UnholyExtRam_IsAvailable(void);
u32 UnholyExtRam_Capacity(void);
void *UnholyExtRam_Alloc(u32 size, u32 alignment);
void UnholyExtRam_ResetAllocator(void);

#endif // GUARD_UNHOLY_EXT_RAM_H
