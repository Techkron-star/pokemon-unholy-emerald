#ifndef GUARD_HF_PC_GROUPS_H
#define GUARD_HF_PC_GROUPS_H

// Virtual folders reference the original PC slots; they never move or clone Pokémon.
#define HF_PC_GROUP_SPECIES 0
#define HF_PC_GROUP_INITIAL 1
#define HF_PC_GROUP_COUNT (TOTAL_BOXES_COUNT * IN_BOX_COUNT)

struct HfPcMonRef
{
    u8 box;
    u8 slot;
};

// Key is a species ID or the first character of the localized species name.
u32 HfPcGroup_GetKey(u8 mode, u8 box, u8 slot);
u16 HfPcGroup_Count(u8 mode, u32 key);
u16 HfPcGroup_List(u8 mode, u32 key, struct HfPcMonRef *out, u16 capacity);
u16 HfPcGroup_CollectKeys(u8 mode, u32 *keys, u16 capacity);

#endif
