#include "global.h"
#include "unholy_mon_ext.h"

// Serialized buffers may be byte-aligned when read from paged storage. Use
// memcpy rather than unaligned u32 loads (unsafe on GBA's ARM7TDMI).
bool32 UnholyMonExt_Validate(const void *record, u32 availableBytes)
{
    struct UnholyMonExtHeader header;
    struct UnholyMonExtEntry previous = {0};
    const u8 *data = record;
    u32 contentBytes;

    if (record == NULL || availableBytes < sizeof(header))
        return FALSE;

    memcpy(&header, data, sizeof(header));

    if (header.magic != UNHOLY_MON_EXT_MAGIC
     || header.version != UNHOLY_MON_EXT_VERSION)
        return FALSE;

    if (header.uidLow == 0 && header.uidHigh == 0)
        return FALSE;

    if (header.totalBytes < sizeof(header)
     || header.totalBytes > availableBytes)
        return FALSE;

    contentBytes = header.totalBytes - sizeof(header);
    if (header.entryCount > contentBytes / sizeof(struct UnholyMonExtEntry))
        return FALSE;
    if (contentBytes != (u32)header.entryCount * sizeof(struct UnholyMonExtEntry))
        return FALSE;

    // Canonically sorted entries allow linear validation, forbid duplicate
    // (domain,key) pairs, and leave future domains available.
    for (u32 i = 0; i < header.entryCount; i++)
    {
        struct UnholyMonExtEntry current;

        memcpy(&current, data + sizeof(header) + i * sizeof(current), sizeof(current));
        if (current.domain == 0 || current.key == 0)
            return FALSE;
        if (i != 0 && (current.domain < previous.domain
             || (current.domain == previous.domain && current.key <= previous.key)))
            return FALSE;
        previous = current;
    }

    return TRUE;
}
