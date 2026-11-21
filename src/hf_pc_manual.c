#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "unholy_save.h"
#include "hf_pc_groups.h"

// Dedicated, guarded SaveBlock5 reservation. Never initialize over unknown data.
// Wallet reserves the final 48 bytes; this region is at the start of the payload.
#define HF_PC_MANUAL_MAGIC 0x48465043u // HFPC
#define HF_PC_MANUAL_OFFSET 0

struct HfPcIdentity
{
    u32 personality;
    u32 otId;
};

struct HfPcManualFolder
{
    u8 name[HF_PC_FOLDER_NAME_LENGTH + 1];
    u8 color;
    u8 count;
    u8 reserved;
    struct HfPcIdentity members[HF_PC_MANUAL_MEMBERS];
};

struct HfPcManualData
{
    u32 magic;
    u8 count;
    u8 version;
    u16 reserved;
    struct HfPcManualFolder folders[HF_PC_MANUAL_FOLDERS];
};

STATIC_ASSERT(sizeof(struct HfPcManualData) + HF_PC_MANUAL_OFFSET
              <= UNHOLY_SAVE5_PAYLOAD_BYTES - 48, hf_pc_manual_must_not_overlap_wallet);

static struct HfPcManualData *HfPcManual_Data(void)
{
    struct SaveBlock5 *save = UnholySave5_Get();
    struct HfPcManualData *data;
    const u8 *bytes;
    u32 i;
    if (save == NULL)
        return NULL;
    data = (struct HfPcManualData *)&save->payload[HF_PC_MANUAL_OFFSET];
    if (data->magic != HF_PC_MANUAL_MAGIC)
    {
        bytes = (const u8 *)data;
        for (i = 0; i < sizeof(*data); i++)
            if (bytes[i] != 0 && bytes[i] != 0xFF)
                return NULL;
        memset(data, 0, sizeof(*data));
        data->magic = HF_PC_MANUAL_MAGIC;
        data->version = 1;
        UnholySave5_Commit();
    }
    if (data->version != 1 || data->count > HF_PC_MANUAL_FOLDERS)
        return NULL;
    return data;
}

static bool8 HfPcManual_Identity(u8 box, u8 slot, struct HfPcIdentity *id)
{
    if (box >= TOTAL_BOXES_COUNT || slot >= IN_BOX_COUNT
     || GetBoxMonDataAt(box, slot, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    id->personality = GetBoxMonDataAt(box, slot, MON_DATA_PERSONALITY);
    id->otId = GetBoxMonDataAt(box, slot, MON_DATA_OT_ID);
    return TRUE;
}

static bool8 HfPcManual_Matches(const struct HfPcIdentity *a, const struct HfPcIdentity *b)
{
    return a->personality == b->personality && a->otId == b->otId;
}

u8 HfPcManual_GetCount(void)
{
    struct HfPcManualData *data = HfPcManual_Data();
    return data == NULL ? 0 : data->count;
}

const u8 *HfPcManual_GetName(u8 id)
{
    struct HfPcManualData *data = HfPcManual_Data();
    return data == NULL || id >= data->count ? NULL : data->folders[id].name;
}

u8 HfPcManual_GetColor(u8 id)
{
    struct HfPcManualData *data = HfPcManual_Data();
    return data == NULL || id >= data->count ? 0 : data->folders[id].color;
}

bool8 HfPcManual_Create(const u8 *name, u8 color, u8 *outId)
{
    struct HfPcManualData *data = HfPcManual_Data();
    struct HfPcManualFolder *folder;
    if (data == NULL || name == NULL || name[0] == EOS
     || color >= HF_PC_FOLDER_COLORS || data->count >= HF_PC_MANUAL_FOLDERS)
        return FALSE;
    if (StringLength(name) > HF_PC_FOLDER_NAME_LENGTH)
        return FALSE;
    if (outId != NULL)
        *outId = data->count;
    folder = &data->folders[data->count++];
    memset(folder, 0, sizeof(*folder));
    StringCopy(folder->name, name);
    folder->color = color;
    return UnholySave5_Commit();
}

bool8 HfPcManual_Rename(u8 id, const u8 *name)
{
    struct HfPcManualData *data = HfPcManual_Data();
    if (data == NULL || id >= data->count || name == NULL || name[0] == EOS
     || StringLength(name) > HF_PC_FOLDER_NAME_LENGTH)
        return FALSE;
    StringCopy(data->folders[id].name, name);
    return UnholySave5_Commit();
}

bool8 HfPcManual_SetColor(u8 id, u8 color)
{
    struct HfPcManualData *data = HfPcManual_Data();
    if (data == NULL || id >= data->count || color >= HF_PC_FOLDER_COLORS)
        return FALSE;
    data->folders[id].color = color;
    return UnholySave5_Commit();
}

bool8 HfPcManual_Delete(u8 id)
{
    struct HfPcManualData *data = HfPcManual_Data();
    u8 i;
    if (data == NULL || id >= data->count)
        return FALSE;
    for (i = id; i + 1 < data->count; i++)
        data->folders[i] = data->folders[i + 1];
    memset(&data->folders[--data->count], 0, sizeof(data->folders[0]));
    return UnholySave5_Commit();
}

bool8 HfPcManual_Move(u8 from, u8 to)
{
    struct HfPcManualData *data = HfPcManual_Data();
    struct HfPcManualFolder temp;
    u8 i;
    if (data == NULL || from >= data->count || to >= data->count)
        return FALSE;
    if (from == to)
        return TRUE;
    temp = data->folders[from];
    if (from < to)
        for (i = from; i < to; i++)
            data->folders[i] = data->folders[i + 1];
    else
        for (i = from; i > to; i--)
            data->folders[i] = data->folders[i - 1];
    data->folders[to] = temp;
    return UnholySave5_Commit();
}

bool8 HfPcManual_Add(u8 id, u8 box, u8 slot)
{
    struct HfPcManualData *data = HfPcManual_Data();
    struct HfPcIdentity identity;
    struct HfPcManualFolder *folder;
    u8 i;
    if (data == NULL || id >= data->count || !HfPcManual_Identity(box, slot, &identity))
        return FALSE;
    folder = &data->folders[id];
    for (i = 0; i < folder->count; i++)
        if (HfPcManual_Matches(&folder->members[i], &identity))
            return TRUE;
    if (folder->count >= HF_PC_MANUAL_MEMBERS)
        return FALSE;
    folder->members[folder->count++] = identity;
    return UnholySave5_Commit();
}

bool8 HfPcManual_Remove(u8 id, u8 box, u8 slot)
{
    struct HfPcManualData *data = HfPcManual_Data();
    struct HfPcIdentity identity;
    struct HfPcManualFolder *folder;
    u8 i, j;
    if (data == NULL || id >= data->count || !HfPcManual_Identity(box, slot, &identity))
        return FALSE;
    folder = &data->folders[id];
    for (i = 0; i < folder->count; i++)
        if (HfPcManual_Matches(&folder->members[i], &identity))
        {
            for (j = i; j + 1 < folder->count; j++)
                folder->members[j] = folder->members[j + 1];
            memset(&folder->members[--folder->count], 0, sizeof(identity));
            return UnholySave5_Commit();
        }
    return FALSE;
}

u16 HfPcManual_List(u8 id, struct HfPcMonRef *out, u16 capacity)
{
    struct HfPcManualData *data = HfPcManual_Data();
    u16 count = 0;
    u8 i, box, slot;
    struct HfPcIdentity identity;
    if (data == NULL || id >= data->count || out == NULL)
        return 0;
    for (i = 0; i < data->folders[id].count; i++)
        for (box = 0; box < TOTAL_BOXES_COUNT; box++)
            for (slot = 0; slot < IN_BOX_COUNT; slot++)
                if (HfPcManual_Identity(box, slot, &identity)
                 && HfPcManual_Matches(&data->folders[id].members[i], &identity))
                {
                    if (count < capacity)
                    {
                        out[count].box = box;
                        out[count].slot = slot;
                    }
                    count++;
                    goto next_member;
                }
next_member:
    return count;
}
