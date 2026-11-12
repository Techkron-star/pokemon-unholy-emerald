#include "global.h"
#include "battle.h"
#include "battle_util.h"
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

TEST("Unholy Gear Cosmic effects use the correct slot and combat modifier")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    u16 previousSlot = gBattlerPartyIndexes[B_BATTLER_0];
    u16 previousTargetSlot = gBattlerPartyIndexes[B_BATTLER_1];
    u32 previousBattleType = gBattleTypeFlags;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon previousMon = *mon;
    enum Type previousTypes[3];
    for (u8 i = 0; i < ARRAY_COUNT(previousTypes); i++)
        previousTypes[i] = gBattleMons[B_BATTLER_1].types[i];

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    gBattlerPartyIndexes[B_BATTLER_1] = 0;
    CreateMonWithIVs(mon, SPECIES_GENGAR, 40, 0, OTID_STRUCT_PRESET(1), 10);
    struct DamageContext ctx = {
        .battlerAtk = B_BATTLER_0, .battlerDef = B_BATTLER_1,
        .move = MOVE_SHADOW_BALL, .moveType = TYPE_GHOST,
    };
    gBattleMons[B_BATTLER_1].types[0] = TYPE_NORMAL;
    gBattleMons[B_BATTLER_1].types[1] = TYPE_FAIRY;
    gBattleMons[B_BATTLER_1].types[2] = TYPE_MYSTERY;

    Gear_SetEquipped(mon, Gear_FromItem(ITEM_GEAR_VISOR_COSMICO));
    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_VISOR_COSMICO);
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_SHADOW_BALL, TYPE_GHOST), UQ_4_12(1.0));
    EXPECT_EQ(CalcTypeEffectivenessMultiplier(&ctx), UQ_4_12(4.0));
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(CalcTypeEffectivenessMultiplier(&ctx), UQ_4_12(4.0));

    Gear_SetEquipped(mon, Gear_FromItem(ITEM_GEAR_IMPULSO_COSMICO));
    gBattleStruct->gearOwner[B_BATTLER_0] = 0;
    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_IMPULSO_COSMICO);
    EXPECT_EQ(CalcTypeEffectivenessMultiplier(&ctx), UQ_4_12(0.0));
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_SHADOW_BALL, TYPE_GHOST), UQ_4_12(1.2));
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_SHADOW_CLAW, TYPE_GHOST), UQ_4_12(1.0));
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_SHADOW_BALL, TYPE_GHOST), UQ_4_12(1.1));

    *mon = previousMon;
    for (u8 i = 0; i < ARRAY_COUNT(previousTypes); i++)
        gBattleMons[B_BATTLER_1].types[i] = previousTypes[i];
    gBattlerPartyIndexes[B_BATTLER_0] = previousSlot;
    gBattlerPartyIndexes[B_BATTLER_1] = previousTargetSlot;
    gBattleTypeFlags = previousBattleType;
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}
