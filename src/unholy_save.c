#include "global.h"
#include "unholy_save.h"
#include "pokemon_storage_system.h"

// The standard SaveBlock3 sidecar is not included in the sector checksum.
// This CRC32 protects the two logical extension envelopes independently.
// It does NOT replace the flash save's two-slot recovery protocol.
static u32 UnholySave_Crc32Update(u32 crc, const u8 *data, u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        crc ^= data[i];
        for (u8 bit = 0; bit < 8; bit++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320u;
            else
                crc >>= 1;
        }
    }
    return crc;
}

static u32 UnholySave_CalculateCrc(const u16 *version, u32 payloadBytes)
{
    // Layout is [magic:4][version:2][flags:2][crc32:4][payload:N].
    // Hash the header's version/flags followed by the full payload;
    // never include the CRC field itself.
    const u8 *bytes = (const u8 *)version;
    u32 crc = UnholySave_Crc32Update(0xFFFFFFFFu, bytes, 4);
    return ~UnholySave_Crc32Update(crc, bytes + 8, payloadBytes);
}

static bool32 UnholySave_IsBlank(const void *ptr, u32 size)
{
    const u8 *bytes = ptr;
    bool32 allZero = TRUE;
    bool32 allErased = TRUE;

    for (u32 i = 0; i < size; i++)
    {
        if (bytes[i] != 0)
            allZero = FALSE;
        if (bytes[i] != 0xFF)
            allErased = FALSE;
        if (!allZero && !allErased)
            return FALSE;
    }
    return TRUE;
}

bool32 UnholySave4_Validate(const struct SaveBlock4 *save4)
{
    return save4 != NULL
        && save4->magic == UNHOLY_SAVE4_MAGIC
        && save4->version == UNHOLY_SAVE4_VERSION
        && save4->crc32 == UnholySave_CalculateCrc(&save4->version,
                                                   sizeof(save4->payload));
}

bool32 UnholySave5_Validate(const struct SaveBlock5 *save5)
{
    return save5 != NULL
        && save5->magic == UNHOLY_SAVE5_MAGIC
        && save5->version == UNHOLY_SAVE5_VERSION
        && save5->crc32 == UnholySave_CalculateCrc(&save5->version,
                                                   sizeof(save5->payload));
}

bool32 UnholySave4_Commit(void)
{
    struct SaveBlock4 *save4 = &gSaveBlock3Ptr->unholySave4;

    if (save4->magic != UNHOLY_SAVE4_MAGIC
     || save4->version != UNHOLY_SAVE4_VERSION)
        return FALSE;

    save4->crc32 = UnholySave_CalculateCrc(&save4->version, sizeof(save4->payload));
    return TRUE;
}

bool32 UnholySave5_Commit(void)
{
    struct SaveBlock5 *save5 = &gPokemonStoragePtr->unholySave5;

    if (save5->magic != UNHOLY_SAVE5_MAGIC
     || save5->version != UNHOLY_SAVE5_VERSION)
        return FALSE;

    save5->crc32 = UnholySave_CalculateCrc(&save5->version, sizeof(save5->payload));
    return TRUE;
}

void UnholySave4_Reset(void)
{
    struct SaveBlock4 *save4 = &gSaveBlock3Ptr->unholySave4;

    memset(save4, 0, sizeof(*save4));
    save4->magic = UNHOLY_SAVE4_MAGIC;
    save4->version = UNHOLY_SAVE4_VERSION;
    UnholySave4_Commit();
}

void UnholySave5_Reset(void)
{
    struct SaveBlock5 *save5 = &gPokemonStoragePtr->unholySave5;

    memset(save5, 0, sizeof(*save5));
    save5->magic = UNHOLY_SAVE5_MAGIC;
    save5->version = UNHOLY_SAVE5_VERSION;
    UnholySave5_Commit();
}

struct SaveBlock4 *UnholySave4_Get(void)
{
    struct SaveBlock4 *save4 = &gSaveBlock3Ptr->unholySave4;

    if (save4->magic != UNHOLY_SAVE4_MAGIC)
    {
        // Existing saves have an empty/erased extension. Unknown bytes are
        // not silently discarded: they may belong to another save format.
        if (!UnholySave_IsBlank(save4, sizeof(*save4)))
            return NULL;
        UnholySave4_Reset();
    }

    // A newer version or corrupted extension must never be erased just
    // because an older build attempted to read it.
    return UnholySave4_Validate(save4) ? save4 : NULL;
}

struct SaveBlock5 *UnholySave5_Get(void)
{
    struct SaveBlock5 *save5 = &gPokemonStoragePtr->unholySave5;

    if (save5->magic != UNHOLY_SAVE5_MAGIC)
    {
        if (!UnholySave_IsBlank(save5, sizeof(*save5)))
            return NULL;
        UnholySave5_Reset();
    }

    return UnholySave5_Validate(save5) ? save5 : NULL;
}
