#include "global.h"
#include "battle.h"
#include "malloc.h"
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

TEST("Unholy Gear cooldown follows its party member across doubles positions")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    u32 previousBattleType = gBattleTypeFlags;
    u16 previousLeftSlot = gBattlerPartyIndexes[B_BATTLER_0];
    u16 previousRightSlot = gBattlerPartyIndexes[B_BATTLER_2];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon *partner = &gParties[B_TRAINER_PARTNER][0];
    struct Pokemon previousMon = *mon;
    struct Pokemon previousPartner = *partner;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = BATTLE_TYPE_DOUBLE;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    gBattlerPartyIndexes[B_BATTLER_2] = 1;
    CreateMonWithIVs(mon, SPECIES_ABSOL, 40, 0xAABBCCDD,
                     OTID_STRUCT_PRESET(0x10203040), MAX_PER_STAT_IVS);
    CreateMonWithIVs(partner, SPECIES_ABSOL, 40, 0x11223344,
                     OTID_STRUCT_PRESET(0x11203040), MAX_PER_STAT_IVS);
    Gear_SetEquipped(mon, GEAR_SKI_MASK);
    Gear_SetEquipped(partner, GEAR_SKI_MASK);

    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_OVERLOAD);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 1);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_SCRATCH, TYPE_NORMAL);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 0);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_CRUNCH, TYPE_DARK);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_RECHARGING);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 3);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_CRUNCH, TYPE_DARK);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);

    gBattlerPartyIndexes[B_BATTLER_0] = 1;
    gBattlerPartyIndexes[B_BATTLER_2] = 0;
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_2), GEAR_PHASE_RECHARGING);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_2), 3);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_2], 0);
    Gear_EndOfTurn(B_BATTLER_2);
    Gear_EndOfTurn(B_BATTLER_2);

    gBattlerPartyIndexes[B_BATTLER_2] = 1;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_RECHARGING);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 1);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_OVERLOAD);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 1);

    Gear_EndOfTurn(B_BATTLER_0);
    gBattleTypeFlags |= BATTLE_TYPE_MULTI;
    gBattlerPartyIndexes[B_BATTLER_2] = 0;
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_2), GEAR_PHASE_OVERLOAD);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_RECHARGING);

    *partner = previousPartner;
    *mon = previousMon;
    gBattlerPartyIndexes[B_BATTLER_0] = previousLeftSlot;
    gBattlerPartyIndexes[B_BATTLER_2] = previousRightSlot;
    gBattleTypeFlags = previousBattleType;
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}
