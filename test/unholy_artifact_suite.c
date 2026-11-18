#include "global.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "malloc.h"
#include "move.h"
#include "pokemon.h"
#include "test/test.h"

TEST("Artifact batch retains every earlier save ID and independently maps eleven items")
{
    enum Gear gear;
    enum Item item;

    EXPECT_EQ(GEAR_ESTANDARTE_GUERRA, GEAR_ESCUDO_PALADIN + 1);
    EXPECT_EQ(ITEM_GEAR_ESTANDARTE_GUERRA, ITEM_GEAR_ESCUDO_PALADIN + 1);
    for (gear = GEAR_ESTANDARTE_GUERRA; gear <= GEAR_ABONO; gear++)
    {
        item = Gear_ToItem(gear);
        EXPECT_NE(item, ITEM_NONE);
        EXPECT_EQ(Gear_FromItem(item), gear);
        EXPECT_EQ(GetItemDisplayPocket(item), POCKET_GEARS);
        EXPECT(gGearsInfo[gear].iconGfx != NULL);
        EXPECT(gGearsInfo[gear].iconPal != NULL);
    }
}

TEST("Silo echoes weaken by strike and never include rays, pins or status moves")
{
    struct BattleStruct *oldBattle = gBattleStruct;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    struct BattlePokemon oldDef = gBattleMons[B_BATTLER_1];
    struct SpecialStatus oldStatus = gSpecialStatuses[B_BATTLER_0];
    u8 oldCount = gBattlersCount;
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];
    u32 oldType = gBattleTypeFlags;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattlersCount = 2;
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(99), 15);
    Gear_SetEquipped(mon, GEAR_RED_SILOS);
    gBattleMons[B_BATTLER_1] = (struct BattlePokemon){.hp = 100, .maxHP = 100};

    EXPECT_EQ(Gear_TryStartExtraStrikes(B_BATTLER_0, MOVE_ICE_BEAM, B_BATTLER_1, TRUE), 0);
    EXPECT_EQ(Gear_TryStartExtraStrikes(B_BATTLER_0, MOVE_BULLET_SEED, B_BATTLER_1, TRUE), 0);
    EXPECT_EQ(Gear_TryStartExtraStrikes(B_BATTLER_0, MOVE_SHADOW_BALL, B_BATTLER_1, FALSE), 0);
    u8 replicas = Gear_TryStartExtraStrikes(B_BATTLER_0, MOVE_SHADOW_BALL, B_BATTLER_1, TRUE);
    EXPECT(replicas >= 1 && replicas <= 4);

    gBattleStruct->artifactReplicaKind[B_BATTLER_0] = 1;
    gBattleStruct->artifactReplicaTotal[B_BATTLER_0] = 4;
    gSpecialStatuses[B_BATTLER_0].multiHitOn = TRUE;
    for (u8 index = 1; index <= 4; index++)
    {
        gMultiHitCounter = 5 - index;
        EXPECT_EQ(Gear_AdjustResolvedDamage(B_BATTLER_0, B_BATTLER_1, MOVE_SHADOW_BALL, 100, FALSE),
                  index <= 2 ? 15 : 10);
    }

    *mon = oldMon;
    gBattleMons[B_BATTLER_1] = oldDef;
    gSpecialStatuses[B_BATTLER_0] = oldStatus;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattlersCount = oldCount;
    gBattleTypeFlags = oldType;
    Free(gBattleStruct);
    gBattleStruct = oldBattle;
}

TEST("War banner only activates after a real negative control stat change")
{
    struct BattleStruct *oldBattle = gBattleStruct;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct Pokemon oldMon = *mon;
    struct BattlePokemon oldAtk = gBattleMons[B_BATTLER_0];
    struct BattlePokemon oldDef = gBattleMons[B_BATTLER_1];
    u8 oldCount = gBattlersCount;
    u16 oldIndex = gBattlerPartyIndexes[B_BATTLER_0];
    u32 oldType = gBattleTypeFlags;

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattlersCount = 2;
    gBattleTypeFlags = 0;
    gBattlerPartyIndexes[B_BATTLER_0] = 0;
    CreateMonWithIVs(mon, SPECIES_SKARMORY, 40, 0, OTID_STRUCT_PRESET(100), 15);
    Gear_SetEquipped(mon, GEAR_ESTANDARTE_GUERRA);
    gBattleMons[B_BATTLER_0] = (struct BattlePokemon){.hp = 60, .maxHP = 100};
    gBattleMons[B_BATTLER_1] = (struct BattlePokemon){.hp = 100, .maxHP = 100};
    gBattleMons[B_BATTLER_1].statStages[STAT_SPEED] = DEFAULT_STAT_STAGE;

    Gear_OnMoveUsed(B_BATTLER_0, MOVE_STRING_SHOT, TYPE_BUG);
    EXPECT_EQ(Gear_TryWarBannerSupport(B_BATTLER_0, B_BATTLER_1), MAX_BATTLERS_COUNT);
    gBattleMons[B_BATTLER_1].statStages[STAT_SPEED]--;
    EXPECT_EQ(Gear_TryWarBannerSupport(B_BATTLER_0, B_BATTLER_1), B_BATTLER_0);
    EXPECT_EQ(Gear_TryWarBannerSupport(B_BATTLER_0, B_BATTLER_1), MAX_BATTLERS_COUNT);

    *mon = oldMon;
    gBattleMons[B_BATTLER_0] = oldAtk;
    gBattleMons[B_BATTLER_1] = oldDef;
    gBattlerPartyIndexes[B_BATTLER_0] = oldIndex;
    gBattlersCount = oldCount;
    gBattleTypeFlags = oldType;
    Free(gBattleStruct);
    gBattleStruct = oldBattle;
}
