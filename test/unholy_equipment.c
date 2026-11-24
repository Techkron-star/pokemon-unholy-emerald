#include "global.h"
#include "unholy_equipment.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "test/test.h"

struct EquipmentBackup
{
    struct SaveBlock1 save;
    struct Pokemon party[PARTY_SIZE];
    u8 count;
    bool8 battle;
};

static struct EquipmentBackup *Begin(void)
{
    struct EquipmentBackup *old = Alloc(sizeof(*old));
    old->save = *gSaveBlock1Ptr;
    memcpy(old->party, gParties[B_TRAINER_PLAYER], sizeof(old->party));
    old->count = gPartiesCount[B_TRAINER_PLAYER];
    old->battle = gMain.inBattle;
    gMain.inBattle = FALSE;
    ClearBag();
    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_TREECKO, 5, 123, OTID_STRUCT_PRESET(456), 12);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    u16 held = ITEM_LEFTOVERS;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM, &held);
    Gear_SetEquipped(&gParties[B_TRAINER_PLAYER][0], GEAR_PERMAFROST_FRAGMENT);
    return old;
}

static void End(struct EquipmentBackup *old)
{
    *gSaveBlock1Ptr = old->save;
    memcpy(gParties[B_TRAINER_PLAYER], old->party, sizeof(old->party));
    gPartiesCount[B_TRAINER_PLAYER] = old->count;
    gMain.inBattle = old->battle;
    Free(old);
}

TEST("Equipment QoL reviews and swaps held items without touching Gear")
{
    struct EquipmentBackup *old = Begin();
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    EXPECT(AddBagItem(ITEM_METAL_COAT, 1));
    struct UnholyEquipmentPlan plan;
    EXPECT_EQ(UnholyEquipment_Prepare(0, FALSE, ITEM_METAL_COAT, &plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_METAL_COAT), 1);
    EXPECT_EQ(UnholyEquipment_Apply(&plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), ITEM_METAL_COAT);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LEFTOVERS), 1);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_METAL_COAT), 0);
    EXPECT_EQ(Gear_GetEquipped(mon), GEAR_PERMAFROST_FRAGMENT);
    EXPECT_EQ(UnholyEquipment_Apply(&plan), UNHOLY_EQUIP_CHANGED);
    End(old);
}

TEST("Equipment QoL swaps and unequips Gear through the inventory service")
{
    struct EquipmentBackup *old = Begin();
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    EXPECT(AddBagItem(ITEM_GEAR_SKI_MASK, 1));
    struct UnholyEquipmentPlan plan;
    EXPECT_EQ(UnholyEquipment_Prepare(0, TRUE, ITEM_GEAR_SKI_MASK, &plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(UnholyEquipment_Apply(&plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(Gear_GetEquipped(mon), GEAR_SKI_MASK);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_GEAR_PERMAFROST_FRAGMENT), 1);
    EXPECT_EQ(UnholyEquipment_Prepare(0, TRUE, ITEM_NONE, &plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(UnholyEquipment_Apply(&plan), UNHOLY_EQUIP_OK);
    EXPECT_EQ(Gear_GetEquipped(mon), GEAR_NONE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_GEAR_SKI_MASK), 1);
    End(old);
}

TEST("Equipment QoL revalidates resources and blocks cross-slot, mail and egg edits")
{
    struct EquipmentBackup *old = Begin();
    struct UnholyEquipmentPlan plan;
    EXPECT(AddBagItem(ITEM_METAL_COAT, 1));
    EXPECT_EQ(UnholyEquipment_Prepare(0, FALSE, ITEM_METAL_COAT, &plan), UNHOLY_EQUIP_OK);
    EXPECT(RemoveBagItem(ITEM_METAL_COAT, 1));
    EXPECT_EQ(UnholyEquipment_Apply(&plan), UNHOLY_EQUIP_MISSING);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(UnholyEquipment_Prepare(0, FALSE, ITEM_GEAR_SKI_MASK, &plan), UNHOLY_EQUIP_INVALID);
    EXPECT_EQ(UnholyEquipment_Prepare(0, TRUE, ITEM_METAL_COAT, &plan), UNHOLY_EQUIP_INVALID);
    u16 held = ITEM_ORANGE_MAIL;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM, &held);
    EXPECT_EQ(UnholyEquipment_Prepare(0, FALSE, ITEM_NONE, &plan), UNHOLY_EQUIP_MAIL);
    u8 egg = TRUE;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, &egg);
    EXPECT_EQ(UnholyEquipment_Prepare(0, TRUE, ITEM_NONE, &plan), UNHOLY_EQUIP_INVALID);
    End(old);
}

TEST("Equipment QoL lists owned items once and keeps Gear in a separate bounded catalog")
{
    struct EquipmentBackup *old = Begin();
    EXPECT(AddBagItem(ITEM_METAL_COAT, 3));
    EXPECT(AddBagItem(ITEM_GEAR_SKI_MASK, 1));
    u16 items[3] = {0xDEAD, 0xBEEF, 0xCAFE};
    EXPECT_EQ(UnholyEquipment_List(FALSE, items, 1), 2);
    EXPECT_EQ(items[0], ITEM_NONE);
    EXPECT_EQ(items[1], 0xBEEF);
    EXPECT_EQ(UnholyEquipment_List(FALSE, items, ARRAY_COUNT(items)), 2);
    EXPECT_EQ(items[1], ITEM_METAL_COAT);
    EXPECT_EQ(UnholyEquipment_List(TRUE, items, ARRAY_COUNT(items)), 2);
    EXPECT_EQ(items[1], ITEM_GEAR_SKI_MASK);
    End(old);
}
