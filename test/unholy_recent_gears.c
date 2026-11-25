#include "global.h"
#include "battle.h"
#include "battle_stamina.h"
#include "battle_util.h"
#include "gear.h"
#include "item.h"
#include "malloc.h"
#include "pokemon.h"
#include "test/test.h"

TEST("Fifteen approved Gear sprites map to unique, equipable items with compatible saved IDs")
{
    static const struct { enum Gear gear; enum Item item; } variants[] = {
#define RECENT_GEAR(id, gfx, path, name, description) {GEAR_##id, ITEM_GEAR_##id},
#include "constants/recent_gears.h"
#undef RECENT_GEAR
    };
    struct Pokemon mon;
    u32 heldItem = ITEM_LEFTOVERS;

    EXPECT_EQ(ARRAY_COUNT(variants), 15);
    EXPECT_EQ(variants[0].gear, GEAR_SHACKLE + 1);
    EXPECT_EQ(variants[0].item, ITEM_GEAR_SHACKLE + 1);
    EXPECT(GEAR_COUNT <= GEAR_MAX_COUNT);

    SetBagItemsPointers();
    ClearBag();
    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 40, 0, OTID_STRUCT_PRESET(123), MAX_PER_STAT_IVS);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &heldItem);

    for (u32 i = 0; i < ARRAY_COUNT(variants); i++)
    {
        enum Gear gear = variants[i].gear;
        enum Item item = variants[i].item;
        EXPECT_EQ(gear, GEAR_SHACKLE + 1 + i);
        EXPECT_EQ(item, ITEM_GEAR_SHACKLE + 1 + i);
        EXPECT_EQ(Gear_FromItem(item), gear);
        EXPECT_EQ(Gear_ToItem(gear), item);
        EXPECT(gGearsInfo[gear].name != NULL);
        EXPECT(gGearsInfo[gear].iconGfx != NULL);
        EXPECT(gGearsInfo[gear].iconPal != NULL);
        EXPECT_EQ(GetItemDisplayPocket(item), POCKET_GEARS);
        EXPECT(AddBagItem(item, 1));
        EXPECT_EQ(Gear_TryEquipFromBag(&mon, item), GEAR_EQUIP_OK);
        EXPECT_EQ(Gear_GetEquipped(&mon), gear);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), heldItem);
    }
}

TEST("Recent Gear event cooldown and periodic normalization keep distinct activation windows")
{
    struct BattleStruct *oldStruct = gBattleStruct;
    u32 oldFlags = gBattleTypeFlags;
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_TORCHIC, 40, 0, OTID_STRUCT_PRESET(1), 10);

    Gear_SetEquipped(mon, GEAR_BATERIA_RESERVA);
    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_BATERIA_RESERVA);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);
    EXPECT(Gear_ActivateEvent(B_BATTLER_0));
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 3);
    EXPECT(!Gear_ActivateEvent(B_BATTLER_0));
    Gear_EndOfTurn(B_BATTLER_0);
    Gear_EndOfTurn(B_BATTLER_0);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_ARMED);
    EXPECT(Gear_ActivateEvent(B_BATTLER_0));

    Gear_SetEquipped(mon, GEAR_NIHIL_ENGINE);
    gBattleStruct->gearOwner[B_BATTLER_0] = 0; // Simulate a new battle instance.
    PushHazardTypeToQueue(B_SIDE_OPPONENT, HAZARDS_STEALTH_ROCK);
    PushHazardTypeToQueue(B_SIDE_OPPONENT, HAZARDS_SPIKES);
    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_NIHIL_ENGINE);
    Gear_EndOfTurn(B_BATTLER_0);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(gBattleStruct->numHazards[B_SIDE_OPPONENT], 2);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(gBattleStruct->numHazards[B_SIDE_OPPONENT], 1);
    Gear_EndOfTurn(B_BATTLER_0);
    Gear_EndOfTurn(B_BATTLER_0);
    Gear_EndOfTurn(B_BATTLER_0);
    EXPECT_EQ(gBattleStruct->numHazards[B_SIDE_OPPONENT], 0);

    *mon = oldMon;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattleTypeFlags = oldFlags;
    Free(gBattleStruct);
    gBattleStruct = oldStruct;
}

TEST("Latest Gear batch keeps IDs append-only, equippable and displayable")
{
    static const struct { enum Gear gear; enum Item item; } gears[] = {
#define NEW_GEAR(id, gfx, name, description) {GEAR_##id, ITEM_GEAR_##id},
#include "constants/gear_batch8.h"
#undef NEW_GEAR
    };
    struct Pokemon mon;
    u32 ordinaryItem = ITEM_LEFTOVERS;

    EXPECT_EQ(ARRAY_COUNT(gears), 5);
    EXPECT_EQ(gears[0].gear, GEAR_MODULO_VECTORIAL + 1);
    EXPECT_EQ(gears[0].item, ITEM_GEAR_MODULO_VECTORIAL + 1);
    CreateMonWithIVs(&mon, SPECIES_MUDKIP, 35, 0, OTID_STRUCT_PRESET(14), 15);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &ordinaryItem);
    SetBagItemsPointers();
    ClearBag();

    for (u32 i = 0; i < ARRAY_COUNT(gears); i++)
    {
        EXPECT_EQ(gears[i].gear, GEAR_MODULO_VECTORIAL + 1 + i);
        EXPECT_EQ(gears[i].item, ITEM_GEAR_MODULO_VECTORIAL + 1 + i);
        EXPECT_EQ(Gear_FromItem(gears[i].item), gears[i].gear);
        EXPECT_EQ(Gear_ToItem(gears[i].gear), gears[i].item);
        EXPECT(gGearsInfo[gears[i].gear].iconGfx != NULL);
        EXPECT(gGearsInfo[gears[i].gear].iconPal != NULL);
        EXPECT_EQ(GetItemDisplayPocket(gears[i].item), POCKET_GEARS);
        EXPECT(AddBagItem(gears[i].item, 1));
        EXPECT_EQ(Gear_TryEquipFromBag(&mon, gears[i].item), GEAR_EQUIP_OK);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), ordinaryItem);
    }
}

TEST("Hydra Core heals indefinitely after taking damage below half HP")
{
    struct BattleStruct *oldStruct = gBattleStruct;
    struct BattlePokemon oldBattler = gBattleMons[B_BATTLER_0];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    u32 oldFlags = gBattleTypeFlags;
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_TREECKO, 35, 0, OTID_STRUCT_PRESET(99), 15);
    Gear_SetEquipped(mon, GEAR_NUCLEO_HYDRA);
    gBattleMons[B_BATTLER_0].maxHP = 100;
    gBattleMons[B_BATTLER_0].hp = 49;

    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_NUCLEO_HYDRA);
    Gear_OnDamaged(B_BATTLER_0, 1);
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_OVERLOAD);
    for (u8 turn = 0; turn < 20; turn++)
    {
        EXPECT_EQ(Gear_GetEndTurnHeal(B_BATTLER_0), 8);
        Gear_EndOfTurn(B_BATTLER_0);
        EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_OVERLOAD);
    }
    EXPECT_EQ(Gear_GetTurnsRemaining(B_BATTLER_0), 0);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);
    gBattleStruct->gearOwner[B_BATTLER_0] = 0; // Switched out and returned.
    EXPECT_EQ(Gear_GetPhase(B_BATTLER_0), GEAR_PHASE_OVERLOAD);
    EXPECT_EQ(Gear_GetEndTurnHeal(B_BATTLER_0), 8);
    gBattleMons[B_BATTLER_0].hp = 100;
    EXPECT_EQ(Gear_GetEndTurnHeal(B_BATTLER_0), 0);

    *mon = oldMon;
    gBattleMons[B_BATTLER_0] = oldBattler;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattleTypeFlags = oldFlags;
    Free(gBattleStruct);
    gBattleStruct = oldStruct;
}

TEST("Whetstone restores negative Attack to exactly +1 only after critical-hit recovery")
{
    struct BattleStruct *oldStruct = gBattleStruct;
    struct BattlePokemon oldBattler = gBattleMons[B_BATTLER_0];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    u32 oldFlags = gBattleTypeFlags;
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_SCEPTILE, 40, 0, OTID_STRUCT_PRESET(42), 15);
    Gear_SetEquipped(mon, GEAR_PIEDRA_AFILAR);
    gBattleMons[B_BATTLER_0].hp = 100;
    gBattleMons[B_BATTLER_0].maxHP = 100;
    gBattleMons[B_BATTLER_0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE - 3;

    EXPECT_EQ(Gear_GetBattlerGear(B_BATTLER_0), GEAR_PIEDRA_AFILAR);
    Gear_OnMoveUsed(B_BATTLER_0, MOVE_SLASH, TYPE_NORMAL);
    EXPECT_EQ(gBattleMons[B_BATTLER_0].statStages[STAT_ATK], DEFAULT_STAT_STAGE - 3);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 0);
    Gear_OnCriticalHit(B_BATTLER_0);
    EXPECT_EQ(gBattleMons[B_BATTLER_0].statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);
    Gear_OnCriticalHit(B_BATTLER_0);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 1);
    gBattleMons[B_BATTLER_0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE;
    Gear_OnCriticalHit(B_BATTLER_0);
    EXPECT_EQ(gBattleMons[B_BATTLER_0].statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    gBattleMons[B_BATTLER_0].statStages[STAT_ATK] = DEFAULT_STAT_STAGE - 6;
    Gear_OnCriticalHit(B_BATTLER_0);
    EXPECT_EQ(gBattleMons[B_BATTLER_0].statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    EXPECT_EQ(gBattleStruct->gearActivations[B_BATTLER_0], 2);

    *mon = oldMon;
    gBattleMons[B_BATTLER_0] = oldBattler;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattleTypeFlags = oldFlags;
    Free(gBattleStruct);
    gBattleStruct = oldStruct;
}
