#include "global.h"
#include "pokemon.h"
#include "gear.h"
#include "item.h"
#include "test/test.h"

TEST("Crafted Gears equip independently of held items and return old Gear to bag")
{
    struct Pokemon mon;
    u32 heldItem = ITEM_LEFTOVERS;

    SetBagItemsPointers();
    ClearBag();
    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 40, 0xAABBCCDD,
                     OTID_STRUCT_PRESET(0x10203040), MAX_PER_STAT_IVS);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);

    EXPECT_EQ(Gear_FromItem(ITEM_GEAR_SKI_MASK), GEAR_SKI_MASK);
    EXPECT_EQ(Gear_ToItem(GEAR_SKI_MASK), ITEM_GEAR_SKI_MASK);
    EXPECT_EQ(Gear_FromItem(ITEM_LEFTOVERS), GEAR_NONE);
    EXPECT_EQ(Gear_ToItem(GEAR_NONE), ITEM_NONE);

    EXPECT_EQ(AddBagItem(ITEM_GEAR_SKI_MASK, 1), TRUE);
    EXPECT_EQ(Gear_TryEquipFromBag(&mon, ITEM_GEAR_SKI_MASK), GEAR_EQUIP_OK);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_GEAR), GEAR_SKI_MASK);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
    EXPECT_EQ(CheckBagHasItem(ITEM_GEAR_SKI_MASK, 1), FALSE);
    EXPECT_EQ(Gear_TryEquipFromBag(&mon, ITEM_GEAR_SKI_MASK), GEAR_EQUIP_ALREADY);

    EXPECT_EQ(AddBagItem(ITEM_GEAR_PERMAFROST_FRAGMENT, 1), TRUE);
    EXPECT_EQ(Gear_TryEquipFromBag(&mon, ITEM_GEAR_PERMAFROST_FRAGMENT), GEAR_EQUIP_OK);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_GEAR), GEAR_PERMAFROST_FRAGMENT);
    EXPECT_EQ(CheckBagHasItem(ITEM_GEAR_SKI_MASK, 1), TRUE);
    EXPECT_EQ(CheckBagHasItem(ITEM_GEAR_PERMAFROST_FRAGMENT, 1), FALSE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
}
