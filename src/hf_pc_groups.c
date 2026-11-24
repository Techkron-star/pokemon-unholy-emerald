#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "hf_pc_groups.h"

// Virtual views are recalculated from live storage. A Pokémon may appear in
// multiple views without consuming extra save bytes or changing box offsets.
u32 HfPcGroup_GetKey(u8 mode, u8 box, u8 slot)
{
    u32 species;
    const u8 *name;

    if (box >= TOTAL_BOXES_COUNT || slot >= IN_BOX_COUNT)
        return 0;
    species = GetBoxMonDataAt(box, slot, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return 0;
    if (mode == HF_PC_GROUP_SPECIES)
        return species;
    if (mode != HF_PC_GROUP_INITIAL)
        return 0;
    name = GetSpeciesName(species);
    return name[0];
}

u16 HfPcGroup_List(u8 mode, u32 key, struct HfPcMonRef *out, u16 capacity)
{
    u16 count = 0;
    u8 box, slot;

    if (key == 0 || out == NULL)
        return 0;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            if (HfPcGroup_GetKey(mode, box, slot) == key)
            {
                if (count < capacity)
                {
                    out[count].box = box;
                    out[count].slot = slot;
                }
                count++;
            }
        }
    }
    return count;
}

u16 HfPcGroup_Count(u8 mode, u32 key)
{
    u16 count = 0;
    u8 box, slot;

    if (key == 0)
        return 0;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
            if (HfPcGroup_GetKey(mode, box, slot) == key)
                count++;
    return count;
}

u16 HfPcGroup_CollectKeys(u8 mode, u32 *keys, u16 capacity)
{
    u16 count = 0, i;
    u8 box, slot;
    u32 key;
    bool8 found;

    if (keys == NULL || capacity == 0)
        return 0;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            key = HfPcGroup_GetKey(mode, box, slot);
            if (key == 0)
                continue;
            found = FALSE;
            for (i = 0; i < count; i++)
                if (keys[i] == key)
                {
                    found = TRUE;
                    break;
                }
            if (!found && count < capacity)
                keys[count++] = key;
        }
    }
    return count;
}

// Resolve membership without copying or relocating boxed Pokémon.
bool8 HfPcGroup_IsMember(u8 mode, u32 key, u8 box, u8 slot)
{
    return key != 0 && HfPcGroup_GetKey(mode, box, slot) == key;
}
