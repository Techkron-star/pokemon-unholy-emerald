#include "global.h"
#include "unholy_mon_ext.h"

bool32 UnholyMonExt_Validate(const void *record, u32 availableBytes)
{
    const struct UnholyMonExtHeader *header = record;
    u32 contentBytes;

    if (record == NULL || availableBytes < sizeof(*header))
        return FALSE;

    if (header->magic != UNHOLY_MON_EXT_MAGIC
     || header->version != UNHOLY_MON_EXT_VERSION)
        return FALSE;

    if (header->uidLow == 0 && header->uidHigh == 0)
        return FALSE;

    if (header->totalBytes < sizeof(*header)
     || header->totalBytes > availableBytes)
        return FALSE;

    contentBytes = header->totalBytes - sizeof(*header);

    // Never use unchecked entryCount as a loop bound or offset.
    if (header->entryCount > contentBytes / sizeof(struct UnholyMonExtEntry))
        return FALSE;
    if (contentBytes != (u32)header->entryCount * sizeof(struct UnholyMonExtEntry))
        return FALSE;

    return TRUE;
}
