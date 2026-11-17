#include "global.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"

TEST("Meteor Armor is an append-only Gear that fits the independent slot")
{
    struct Pokemon mon;
    u32 heldItem = ITEM_LEFTOVERS;

    EXPECT_EQ(ITEM_GEAR_ARMADURA_METEORICA, ITEM_GEAR_CASCO_HOPLITA + 1);
    EXPECT_EQ(GEAR_ARMADURA_METEORICA, GEAR_CASCO_HOPLITA + 1);
    EXPECT_EQ(Gear_FromItem(ITEM_GEAR_ARMADURA_METEORICA), GEAR_ARMADURA_METEORICA);
    EXPECT_EQ(Gear_ToItem(GEAR_ARMADURA_METEORICA), ITEM_GEAR_ARMADURA_METEORICA);
    EXPECT_EQ(GetItemDisplayPocket(ITEM_GEAR_ARMADURA_METEORICA), POCKET_GEARS);
    EXPECT(gGearsInfo[GEAR_ARMADURA_METEORICA].iconGfx != NULL);
    EXPECT(gGearsInfo[GEAR_ARMADURA_METEORICA].iconPal != NULL);

    SetBagItemsPointers();
    ClearBag();
    CreateMonWithIVs(&mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(14), 15);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);
    EXPECT(AddBagItem(ITEM_GEAR_ARMADURA_METEORICA, 1));
    EXPECT_EQ(Gear_TryEquipFromBag(&mon, ITEM_GEAR_ARMADURA_METEORICA), GEAR_EQUIP_OK);
    EXPECT_EQ(Gear_GetEquipped(&mon), GEAR_ARMADURA_METEORICA);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
}

TEST("Meteor Armor requires a single-turn lethal hit and scales its overkill")
{
    struct BattleStruct *oldBattle = gBattleStruct;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    struct BattlePokemon oldDef = gBattleMons[B_BATTLER_0];
    struct BattlePokemon oldAtk = gBattleMons[B_BATTLER_1];
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];
    u8 oldCount = gBattlersCount;
    u32 oldType = gBattleTypeFlags;
    struct SpecialStatus oldAttackerStatus = gSpecialStatuses[B_BATTLER_1];
    u16 remainingHp = 0;
    struct GearCycle *cycle;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlersCount = 2;
    gSpecialStatuses[B_BATTLER_1] = (struct SpecialStatus){0};
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(21), 15);
    Gear_SetEquipped(mon, GEAR_ARMADURA_METEORICA);
    gBattleMons[B_BATTLER_0] = (struct BattlePokemon){ .hp = 100, .maxHP = 200 };
    gBattleMons[B_BATTLER_1] = (struct BattlePokemon){ .hp = 100, .maxHP = 100 };

    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_ARMADURA_METEORICA);
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SCRATCH, 99, &remainingHp));
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_STRING_SHOT, 200, &remainingHp));
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SOLAR_BEAM, 200, &remainingHp));
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_FURY_SWIPES, 200, &remainingHp));
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_FUTURE_SIGHT, 200, &remainingHp));

    EXPECT(Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SCRATCH, 110, &remainingHp));
    EXPECT_EQ(remainingHp, 60);
    EXPECT_EQ(gBattleStruct->meteorArmorPending[B_BATTLER_0], 1);
    EXPECT(!Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SCRATCH, 200, &remainingHp));
    for (u8 turn = 0; turn < 6; turn++)
        Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_RECHARGING);

    cycle = &gBattleStruct->gearCycles[B_TRAINER_PLAYER][0];
    cycle->activations = 0;
    cycle->phase = GEAR_PHASE_ARMED;
    EXPECT(Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SCRATCH, 170, &remainingHp));
    EXPECT_EQ(gBattleStruct->meteorArmorPending[B_BATTLER_0], 2);
    cycle->activations = 0;
    cycle->phase = GEAR_PHASE_ARMED;
    EXPECT(Gear_TryMeteorArmorSurvive(B_BATTLER_0, B_BATTLER_1, MOVE_SCRATCH, 270, &remainingHp));
    EXPECT_EQ(gBattleStruct->meteorArmorPending[B_BATTLER_0], 3);

    *mon = oldMon;
    gBattleMons[B_BATTLER_0] = oldDef;
    gBattleMons[B_BATTLER_1] = oldAtk;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattlersCount = oldCount;
    gBattleTypeFlags = oldType;
    gSpecialStatuses[B_BATTLER_1] = oldAttackerStatus;
    Free(gBattleStruct);
    gBattleStruct = oldBattle;
}
