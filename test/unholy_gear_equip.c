#include "global.h"
#include "battle.h"
#include "battle_chain.h"
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

TEST("Unholy Blood Gear catalog covers all 18 types and equips from the Gears pocket")
{
    static const struct
    {
        enum Gear gear;
        enum Item item;
        enum Type type;
    } variants[] =
    {
#define GEAR_BLOOD_VARIANT(type, name, moveTypeName, bagIcon) { GEAR_BLOOD_##type, ITEM_GEAR_BLOOD_##type, TYPE_##type },
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
    };
    struct Pokemon mon;
    u32 mask = 0;
    u32 heldItem = ITEM_LEFTOVERS;

    SetBagItemsPointers();
    ClearBag();
    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 40, 0, OTID_STRUCT_PRESET(1), MAX_PER_STAT_IVS);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);

    EXPECT_EQ(ARRAY_COUNT(variants), 18);
    for (u32 i = 0; i < ARRAY_COUNT(variants); i++)
    {
        enum Gear gear = variants[i].gear;
        enum Item item = variants[i].item;
        u32 type = GEAR_TYPE(variants[i].type);

        EXPECT_EQ(mask & type, 0);
        mask |= type;
        EXPECT_EQ(gGearsInfo[gear].typeMask, type);
        EXPECT_EQ(gGearsInfo[gear].trigger, GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE);
        EXPECT(gGearsInfo[gear].iconGfx != NULL);
        EXPECT(gGearsInfo[gear].iconPal != NULL);
        EXPECT_EQ(Gear_FromItem(item), gear);
        EXPECT_EQ(Gear_ToItem(gear), item);
        EXPECT_EQ(GetItemDisplayPocket(item), POCKET_GEARS);
        EXPECT(AddBagItem(item, 1));
        EXPECT_EQ(Gear_TryEquipFromBag(&mon, item), GEAR_EQUIP_OK);
        EXPECT_EQ(Gear_GetEquipped(&mon), gear);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
        EXPECT_EQ(CountTotalItemQuantityInBag(item), 0);
        if (i > 0)
            EXPECT_EQ(CountTotalItemQuantityInBag(variants[i - 1].item), 1);
    }
    EXPECT_EQ(mask, ((GEAR_TYPE(TYPE_FAIRY) << 1) - 1) & ~GEAR_TYPE(TYPE_NONE) & ~GEAR_TYPE(TYPE_MYSTERY));
}

TEST("Unholy Blood Gear overload is spent once and recharges once per turn")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    u32 previousBattleType = gBattleTypeFlags;
    u16 previousSlot = gBattlerPartyIndexes[B_BATTLER_0];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon previousMon = *mon;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_TORCHIC, 40, 0, OTID_STRUCT_PRESET(1), 10);
    Gear_SetEquipped(mon, GEAR_BLOOD_FIRE);

    Gear_OnMoveUsed(B_BATTLER_0, MOVE_SUNNY_DAY, TYPE_FIRE);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_SCRATCH, TYPE_NORMAL);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_EMBER, TYPE_FIRE), UQ_4_12(1.4));
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);

    Gear_OnMoveUsed(B_BATTLER_0, MOVE_EMBER, TYPE_FIRE);
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_EMBER, TYPE_FIRE), UQ_4_12(1.4));
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_EMBER, TYPE_FIRE), UQ_4_12(1.4));
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 3);

    Gear_OnMoveUsed(B_BATTLER_0, MOVE_INCINERATE, TYPE_FIRE);
    EXPECT_EQ(Gear_GetPowerModifier(B_BATTLER_0, MOVE_INCINERATE, TYPE_FIRE), UQ_4_12(1.12));
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 2);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 1);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);

    *mon = previousMon;
    gBattlerPartyIndexes[B_BATTLER_0] = previousSlot;
    gBattleTypeFlags = previousBattleType;
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}

TEST("Unholy Storm Staff permits mixed chains and rejects two resolved special moves")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon previousMon = *mon;
    u16 previousSlot = gBattlerPartyIndexes[B_BATTLER_0];
    u8 previousCount = gBattlersCount;
    u8 previousAction = gCurrentTurnActionNumber;
    u8 previousOrder0 = gBattlerByTurnOrder[0];
    u8 previousOrder2 = gBattlerByTurnOrder[2];

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattlersCount = 2;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_ABRA, 40, 0, OTID_STRUCT_PRESET(1), 10);
    EXPECT_EQ(Gear_FromItem(ITEM_GEAR_STORM_STAFF), GEAR_STORM_STAFF);
    EXPECT_EQ(GetItemDisplayPocket(ITEM_GEAR_STORM_STAFF), POCKET_GEARS);
    EXPECT(Chain_IsPairAllowed(B_BATTLER_0, MOVE_EMBER, MOVE_WATER_GUN));
    Gear_SetEquipped(mon, GEAR_STORM_STAFF);
    gBattleStruct->gearOwner[B_BATTLER_0] = 0;
    EXPECT(!Chain_IsPairAllowed(B_BATTLER_0, MOVE_EMBER, MOVE_WATER_GUN));
    EXPECT(Chain_IsPairAllowed(B_BATTLER_0, MOVE_EMBER, MOVE_SCRATCH));
    EXPECT(!Chain_IsPairAllowed(B_BATTLER_0, MOVE_CALM_MIND, MOVE_EMBER));
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_EMBER, TYPE_FIRE);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);

    gBattleStruct->numTurnActions = 3;
    gBattleStruct->turnActionRole[2] = 1;
    gBattlerByTurnOrder[0] = B_BATTLER_0;
    gBattlerByTurnOrder[2] = B_BATTLER_0;
    gCurrentTurnActionNumber = 0;
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_SPECIAL;
    EXPECT(Gear_StormStaffRepeatsMove(B_BATTLER_0, MOVE_EMBER));
    EXPECT(!Gear_StormStaffRepeatsMove(B_BATTLER_0, MOVE_WATER_SHURIKEN));
    EXPECT(!Gear_StormStaffRepeatsMove(B_BATTLER_0, MOVE_PSYWAVE));
    EXPECT(!Gear_StormStaffRepeatsMove(B_BATTLER_0, MOVE_FUTURE_SIGHT));
    EXPECT(Chain_AllowsResolvedCategory(B_BATTLER_0, MOVE_EMBER));
    Chain_RecordOpeningMove(B_BATTLER_0, MOVE_EMBER);
    gCurrentTurnActionNumber = 2;
    EXPECT(!Chain_AllowsResolvedCategory(B_BATTLER_0, MOVE_SCRATCH));
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_PHYSICAL;
    EXPECT(Chain_AllowsResolvedCategory(B_BATTLER_0, MOVE_SCRATCH));

    *mon = previousMon;
    gBattlerPartyIndexes[B_BATTLER_0] = previousSlot;
    gBattlersCount = previousCount;
    gCurrentTurnActionNumber = previousAction;
    gBattlerByTurnOrder[0] = previousOrder0;
    gBattlerByTurnOrder[2] = previousOrder2;
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
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
