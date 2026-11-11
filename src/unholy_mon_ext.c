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


bool32 UnholyMonExt_TryGetValue(const void *record, u32 availableBytes,
                               u16 domain, u16 key, s32 *outValue)
{
    struct UnholyMonExtHeader header;
    const u8 *data = record;
    u32 low, high;

    if (outValue == NULL || domain == 0 || key == 0
     || !UnholyMonExt_Validate(record, availableBytes))
        return FALSE;

    memcpy(&header, data, sizeof(header));
    low = 0;
    high = header.entryCount;

    // Domain-key entries are sorted; binary search is cheap even when a
    // future Pokémon has hundreds of additional stats or mutations.
    while (low < high)
    {
        struct UnholyMonExtEntry entry;
        u32 middle = low + (high - low) / 2;

        memcpy(&entry, data + sizeof(header) + middle * sizeof(entry), sizeof(entry));

        if (entry.domain < domain || (entry.domain == domain && entry.key < key))
            low = middle + 1;
        else if (entry.domain > domain || entry.key > key)
            high = middle;
        else
        {
            *outValue = entry.value;
            return TRUE;
        }
    }

    return FALSE;
}

bool32 UnholyMonExt_Init(void *record, u32 availableBytes, u32 uidLow, u32 uidHigh,
                         u16 gearId, u8 hiddenAbilitySlot)
{
    struct UnholyMonExtHeader header = {0};

    if (record == NULL || availableBytes < sizeof(header)
     || (uidLow == 0 && uidHigh == 0))
        return FALSE;

    header.magic = UNHOLY_MON_EXT_MAGIC;
    header.version = UNHOLY_MON_EXT_VERSION;
    header.totalBytes = sizeof(header);
    header.uidLow = uidLow;
    header.uidHigh = uidHigh;
    header.gearId = gearId;
    header.hiddenAbilitySlot = hiddenAbilitySlot;
    memcpy(record, &header, sizeof(header));
    return TRUE;
}

bool32 UnholyMonExt_TrySetValue(void *record, u32 availableBytes,
                               u16 domain, u16 key, s32 value)
{
    struct UnholyMonExtHeader header;
    struct UnholyMonExtEntry entry;
    u8 *data = record;
    u32 pos;

    if (domain == 0 || key == 0 || !UnholyMonExt_Validate(record, availableBytes))
        return FALSE;

    memcpy(&header, data, sizeof(header));

    // Find the first key >= requested (domain,key).
    for (pos = 0; pos < header.entryCount; pos++)
    {
        memcpy(&entry, data + sizeof(header) + pos * sizeof(entry), sizeof(entry));
        if (entry.domain > domain
         || (entry.domain == domain && entry.key >= key))
            break;
    }

    if (pos < header.entryCount && entry.domain == domain && entry.key == key)
    {
        entry.value = value;
        memcpy(data + sizeof(header) + pos * sizeof(entry), &entry, sizeof(entry));
        return TRUE;
    }

    // No partial writes on capacity failures, including the u16 length limit.
    if (availableBytes - header.totalBytes < sizeof(entry)
     || (u32)header.totalBytes + sizeof(entry) > 0xFFFFu
     || header.entryCount == 0xFFFFu)
        return FALSE;

    memmove(data + sizeof(header) + (pos + 1) * sizeof(entry),
            data + sizeof(header) + pos * sizeof(entry),
            (header.entryCount - pos) * sizeof(entry));
    entry.domain = domain;
    entry.key = key;
    entry.value = value;
    memcpy(data + sizeof(header) + pos * sizeof(entry), &entry, sizeof(entry));

    header.entryCount++;
    header.totalBytes += sizeof(entry);
    memcpy(data, &header, sizeof(header));
    return TRUE;
}

bool32 UnholyMonExt_TryRemoveValue(void *record, u32 availableBytes,
                                  u16 domain, u16 key)
{
    struct UnholyMonExtHeader header;
    struct UnholyMonExtEntry entry;
    u8 *data = record;

    if (domain == 0 || key == 0 || !UnholyMonExt_Validate(record, availableBytes))
        return FALSE;

    memcpy(&header, data, sizeof(header));

    for (u32 pos = 0; pos < header.entryCount; pos++)
    {
        memcpy(&entry, data + sizeof(header) + pos * sizeof(entry), sizeof(entry));
        if (entry.domain == domain && entry.key == key)
        {
            memmove(data + sizeof(header) + pos * sizeof(entry),
                    data + sizeof(header) + (pos + 1) * sizeof(entry),
                    (header.entryCount - pos - 1) * sizeof(entry));

            header.entryCount--;
            header.totalBytes -= sizeof(entry);
            // Make it harder to accidentally read stale, formerly-live data.
            memset(data + header.totalBytes, 0, sizeof(entry));
            memcpy(data, &header, sizeof(header));
            return TRUE;
        }
        if (entry.domain > domain
         || (entry.domain == domain && entry.key > key))
            return FALSE;
    }

    return FALSE;
}
