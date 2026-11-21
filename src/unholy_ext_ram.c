#include "global.h"
#include "unholy_ext_ram.h"

// These buffers are transient. The canonical game data must remain in the
// normal save blocks; extended emulator RAM is never a save-file substitute.
EWRAM_DATA static u32 sUnholyExtRamOffset = 0;

bool8 UnholyExtRam_IsAvailable(void)
{
    volatile const u32 *registers = (volatile const u32 *)UNHOLY_EXT_RAM_BASE;

    // Keep this check read-only and side-effect free on standard GBA hardware.
    return registers[0] == UNHOLY_EXT_RAM_MAGIC
        && registers[1] == UNHOLY_EXT_RAM_VERSION
        && registers[2] == UNHOLY_EXT_RAM_MAX_SIZE;
}

u32 UnholyExtRam_Capacity(void)
{
    return UnholyExtRam_IsAvailable() ? UNHOLY_EXT_RAM_MAX_SIZE : 0;
}

void UnholyExtRam_ResetAllocator(void)
{
    sUnholyExtRamOffset = 0;
}

void *UnholyExtRam_Alloc(u32 size, u32 alignment)
{
    u32 start;

    if (!UnholyExtRam_IsAvailable() || size == 0)
        return NULL;

    if (alignment == 0 || (alignment & (alignment - 1)) != 0 || alignment > 4096)
        return NULL;

    if (sUnholyExtRamOffset > UNHOLY_EXT_RAM_MAX_SIZE - (alignment - 1))
        return NULL;

    start = (sUnholyExtRamOffset + alignment - 1) & ~(alignment - 1);
    if (start > UNHOLY_EXT_RAM_MAX_SIZE || size > UNHOLY_EXT_RAM_MAX_SIZE - start)
        return NULL;

    sUnholyExtRamOffset = start + size;
    return (void *)(UNHOLY_EXT_RAM_DATA + start);
}
