#ifndef GUARD_HF_PC_GROUPS_H
#define GUARD_HF_PC_GROUPS_H

// Virtual folders reference the original PC slots; they never move or clone Pokémon.
#define HF_PC_GROUP_SPECIES 0
#define HF_PC_GROUP_INITIAL 1
#define HF_PC_GROUP_CAPACITY (TOTAL_BOXES_COUNT * IN_BOX_COUNT)
#define HF_PC_GROUP_COUNT HF_PC_GROUP_CAPACITY
#define HF_PC_FOLDER_NAME_LENGTH 12
#define HF_PC_FOLDER_COLORS 8
#define HF_PC_MANUAL_FOLDERS 8
#define HF_PC_MANUAL_MEMBERS 12

// Presentation metadata is independent of the underlying Pokémon slots.
// The caller persists custom names/colors using a versioned save extension.
struct HfPcFolderStyle
{
    u32 key;
    u8 mode;
    u8 color;
    u8 name[HF_PC_FOLDER_NAME_LENGTH + 1];
};

struct HfPcMonRef
{
    u8 box;
    u8 slot;
};

// Key is a species ID or the first character of the localized species name.
u32 HfPcGroup_GetKey(u8 mode, u8 box, u8 slot);
u16 HfPcGroup_Count(u8 mode, u32 key);
u16 HfPcGroup_List(u8 mode, u32 key, struct HfPcMonRef *out, u16 capacity);
bool8 HfPcGroup_IsMember(u8 mode, u32 key, u8 box, u8 slot);
// Manual folders store stable Pokemon identities, not box positions.
// Removing a folder does not release any Pokemon.
bool8 HfPcManual_Create(const u8 *name, u8 color, u8 *outId);
bool8 HfPcManual_Rename(u8 id, const u8 *name);
bool8 HfPcManual_SetColor(u8 id, u8 color);
bool8 HfPcManual_Delete(u8 id);
bool8 HfPcManual_Move(u8 from, u8 to);
bool8 HfPcManual_Add(u8 id, u8 box, u8 slot);
bool8 HfPcManual_Remove(u8 id, u8 box, u8 slot);
u16 HfPcManual_List(u8 id, struct HfPcMonRef *out, u16 capacity);
const u8 *HfPcManual_GetName(u8 id);
u8 HfPcManual_GetColor(u8 id);
u8 HfPcManual_GetCount(void);
u16 HfPcGroup_CollectKeys(u8 mode, u32 *keys, u16 capacity);

#endif
