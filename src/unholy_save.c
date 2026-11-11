#include "global.h"
#include "unholy_save.h"

// This extension is persisted automatically by the original SaveBlock3
// per-sector copy logic. No extra sector, bank, or emulator API is involved.
// The original SaveBlock3 data remain at their exact existing offsets.

void UnholySave4_Reset(void)
{
    struct SaveBlock4 *save4 = &gSaveBlock3Ptr->unholySave4;

    memset(save4, 0, sizeof(*save4));
    save4->magic = UNHOLY_SAVE4_MAGIC;
    save4->version = UNHOLY_SAVE4_VERSION;
}

struct SaveBlock4 *UnholySave4_Get(void)
{
    struct SaveBlock4 *save4 = &gSaveBlock3Ptr->unholySave4;

    if (save4->magic != UNHOLY_SAVE4_MAGIC)
        UnholySave4_Reset(); // Old saves contain no extension.

    // Never silently erase a newer format when an older build opens it.
    if (save4->version != UNHOLY_SAVE4_VERSION)
        return NULL;

    return save4;
}
