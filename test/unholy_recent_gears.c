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
