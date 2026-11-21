#include "global.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"

TEST("Paladin Shield is an independently equipped, append-only Artifact")
{
    struct Pokemon mon;
    u32 heldItem = ITEM_LEFTOVERS;

    EXPECT_EQ(ITEM_GEAR_ESCUDO_PALADIN, ITEM_GEAR_ARMADURA_METEORICA + 1);
    EXPECT_EQ(GEAR_ESCUDO_PALADIN, GEAR_ARMADURA_METEORICA + 1);
    EXPECT_EQ(Gear_FromItem(ITEM_GEAR_ESCUDO_PALADIN), GEAR_ESCUDO_PALADIN);
    EXPECT_EQ(Gear_ToItem(GEAR_ESCUDO_PALADIN), ITEM_GEAR_ESCUDO_PALADIN);
    EXPECT_EQ(GetItemDisplayPocket(ITEM_GEAR_ESCUDO_PALADIN), POCKET_GEARS);
    EXPECT(gGearsInfo[GEAR_ESCUDO_PALADIN].iconGfx != NULL);
    EXPECT(gGearsInfo[GEAR_ESCUDO_PALADIN].iconPal != NULL);

    SetBagItemsPointers();
    ClearBag();
    CreateMonWithIVs(&mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(88), 15);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);
    EXPECT(AddBagItem(ITEM_GEAR_ESCUDO_PALADIN, 1));
    EXPECT_EQ(Gear_TryEquipFromBag(&mon, ITEM_GEAR_ESCUDO_PALADIN), GEAR_EQUIP_OK);
    EXPECT_EQ(Gear_GetEquipped(&mon), GEAR_ESCUDO_PALADIN);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
}

TEST("Paladin Shield accumulates pity and can block only once per full turn")
{
    struct BattleStruct *oldBattle = gBattleStruct;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    struct BattlePokemon oldDef = gBattleMons[B_BATTLER_0];
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];
    u8 oldCount = gBattlersCount;
    u32 oldType = gBattleTypeFlags;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattlersCount = 2;
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(89), 15);
    Gear_SetEquipped(mon, GEAR_ESCUDO_PALADIN);
    gBattleMons[B_BATTLER_0] = (struct BattlePokemon){.hp = 100, .maxHP = 100};

    EXPECT_EQ(Gear_GetPaladinChanceTenths(B_BATTLER_0), 125);
    for (u8 turn = 1; turn <= 5; turn++)
    {
        Gear_EndOfTurn(B_BATTLER_0);
        gBattleStruct->paladinTurnSerial++;
        EXPECT_EQ(Gear_GetPaladinChanceTenths(B_BATTLER_0), 125 + turn * 25);
    }
    for (u8 turn = 6; turn <= 8; turn++)
    {
        Gear_EndOfTurn(B_BATTLER_0);
        gBattleStruct->paladinTurnSerial++;
    }
    EXPECT_EQ(Gear_GetPaladinChanceTenths(B_BATTLER_0), 1000);
    EXPECT(Gear_TryPaladinBlock(B_BATTLER_0));
    EXPECT_EQ(Gear_GetPaladinChanceTenths(B_BATTLER_0), 0);
    EXPECT(!Gear_TryPaladinBlock(B_BATTLER_0));
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);

    Gear_EndOfTurn(B_BATTLER_0);
    gBattleStruct->paladinTurnSerial++;
    EXPECT_EQ(Gear_GetPaladinChanceTenths(B_BATTLER_0), 125);

    *mon = oldMon;
    gBattleMons[B_BATTLER_0] = oldDef;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattlersCount = oldCount;
    gBattleTypeFlags = oldType;
    Free(gBattleStruct);
    gBattleStruct = oldBattle;
}

TEST("Paladin Shield chain penalty affects only its blocked target")
{
    struct BattleStruct *oldBattle = gBattleStruct;
    u8 oldCount = gBattlersCount;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattlersCount = 4;
    gBattleStruct->paladinWeakenedChainTarget[B_BATTLER_1] = B_BATTLER_0 + 1;
    EXPECT_EQ(Gear_ApplyPaladinChainPenalty(B_BATTLER_1, B_BATTLER_0, 100), 75);
    EXPECT_EQ(Gear_ApplyPaladinChainPenalty(B_BATTLER_1, B_BATTLER_2, 100), 100);
    EXPECT_EQ(Gear_ApplyPaladinChainPenalty(B_BATTLER_3, B_BATTLER_0, 100), 100);

    Free(gBattleStruct);
    gBattleStruct = oldBattle;
    gBattlersCount = oldCount;
}
