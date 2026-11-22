#include "global.h"
#include "battle.h"
#include "battle_chain.h"
#include "battle_stamina.h"
#include "battle_util.h"
#include "malloc.h"
#include "test/test.h"

TEST("Unholy stamina uses additive status costs and applies one regeneration per turn")
{
    const enum BattlerId battler = B_BATTLER_0;
    static const struct
    {
        u32 status;
        u32 regen;
    } cases[] = {
        {0, 18}, {STATUS1_BURN, 12}, {STATUS1_PARALYSIS, 15}, {STATUS1_FREEZE, 6},
        {STATUS1_POISON, 14}, {STATUS1_TOXIC_POISON, 12}, {1, 22},
    };
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    struct BattlePokemon previousMon = gBattleMons[battler];

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleStruct->staminaOwner[battler] = gBattlerPartyIndexes[battler] + 1;
    gBattleStruct->maxStamina[battler] = 100;
    gBattleMons[battler] = (struct BattlePokemon){0};
    gBattleStruct->stamina[battler] = 45;

    EXPECT_EQ(Stamina_GetMoveCost(battler, MOVE_EMBER), 18);
    EXPECT_EQ(Stamina_GetActionCost(battler, MOVE_SCRATCH, TRUE), 27);
    EXPECT(Chain_CanAfford(battler, MOVE_EMBER, MOVE_SCRATCH));
    gBattleStruct->stamina[battler]--;
    EXPECT(!Chain_CanAfford(battler, MOVE_EMBER, MOVE_SCRATCH));

    gBattleStruct->stamina[battler] = 70;
    gBattleMons[battler].status1 = STATUS1_PARALYSIS;
    gBattleMons[battler].volatiles.confusionTimer = 2;
    EXPECT_EQ(Stamina_GetMoveCost(battler, MOVE_EMBER), 20);
    EXPECT_EQ(Stamina_GetActionCost(battler, MOVE_SCRATCH, TRUE), 31);
    Stamina_SpendForMove(battler, MOVE_EMBER, FALSE);
    Stamina_SpendForMove(battler, MOVE_SCRATCH, TRUE);
    EXPECT_EQ(Stamina_GetCurrent(battler), 19);
    Stamina_EndOfTurn(battler);
    EXPECT_EQ(Stamina_GetCurrent(battler), 34);

    for (u32 i = 0; i < ARRAY_COUNT(cases); i++)
    {
        gBattleMons[battler].status1 = cases[i].status;
        gBattleStruct->stamina[battler] = 0;
        Stamina_EndOfTurn(battler);
        EXPECT_EQ(Stamina_GetCurrent(battler), cases[i].regen);
    }
    gBattleStruct->maxStamina[battler] = 41;
    gBattleStruct->stamina[battler] = 40;
    Stamina_EndOfTurn(battler);
    EXPECT_EQ(Stamina_GetCurrent(battler), 41);

    gBattleMons[battler] = previousMon;
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}

TEST("Unholy chains use two rounds with only selected follow-ups and cancel on changes")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    u32 previousBattleType = gBattleTypeFlags;
    u8 previousCount = gBattlersCount;
    u8 previousAction = gCurrentTurnActionNumber;
    struct BattlePokemon previousMons[2] = {gBattleMons[B_BATTLER_0], gBattleMons[B_BATTLER_1]};
    enum BattlerId previousOrder[MAX_BATTLERS_COUNT];
    u8 previousActions[MAX_BATTLERS_COUNT];

    memcpy(previousOrder, gBattlerByTurnOrder, sizeof(previousOrder));
    memcpy(previousActions, gActionsByTurnOrder, sizeof(previousActions));
    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gBattleTypeFlags = 0;
    gBattlersCount = 2;
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_NONE;

    for (enum BattlerId battler = B_BATTLER_0; battler <= B_BATTLER_1; battler++)
    {
        gBattleMons[battler] = (struct BattlePokemon){
            .moves = {MOVE_EMBER, battler == B_BATTLER_0 ? MOVE_SCRATCH : MOVE_QUICK_ATTACK},
            .pp = {10, 10}, .hp = 50, .maxHP = 50, .speed = 90,
        };
        gActionsByTurnOrder[battler] = B_ACTION_USE_MOVE;
        gBattlerByTurnOrder[battler] = battler;
        gBattleStruct->staminaOwner[battler] = gBattlerPartyIndexes[battler] + 1;
        gBattleStruct->maxStamina[battler] = 100;
        gBattleStruct->stamina[battler] = 100;
    }

    EXPECT(!Chain_IsMoveChainable(MOVE_CALM_MIND));
    EXPECT(!Chain_IsMoveChainable(MOVE_STRING_SHOT));
    EXPECT(!Chain_IsMoveChainable(MOVE_TOXIC));
    EXPECT(!Chain_IsMoveChainable(MOVE_RECOVER));
    EXPECT(!Chain_IsMoveChainable(MOVE_STEALTH_ROCK));
    EXPECT(!Chain_IsMoveChainable(MOVE_TRICK_ROOM));
    EXPECT(!Chain_IsMoveChainable(MOVE_TAILWIND));
    EXPECT(!Chain_IsMoveChainable(MOVE_SOLAR_BEAM));
    EXPECT(!Chain_IsMoveChainable(MOVE_FUTURE_SIGHT));
    EXPECT(Chain_IsMoveChainable(MOVE_SCRATCH));
    EXPECT(Chain_IsMoveChainable(MOVE_REFLECT));
    EXPECT(Chain_IsMoveChainable(MOVE_LIGHT_SCREEN));
    EXPECT(Chain_IsMoveChainable(MOVE_HEAL_BELL));
    EXPECT(Chain_IsPairAllowed(B_BATTLER_0, MOVE_REFLECT, MOVE_LIGHT_SCREEN));
    EXPECT(!Chain_IsPairAllowed(B_BATTLER_0, MOVE_REFLECT, MOVE_EMBER));
    EXPECT(!Chain_IsPairAllowed(B_BATTLER_0, MOVE_EMBER, MOVE_REFLECT));
    EXPECT(Chain_IsPairAllowed(B_BATTLER_0, MOVE_EMBER, MOVE_SCRATCH));
    gBattleStruct->chainSecondPos[B_BATTLER_0] = 2;
    Chain_ExpandTurnOrder();
    EXPECT_EQ(Chain_GetNumTurnActions(), 3);
    EXPECT_EQ(gBattlerByTurnOrder[1], B_BATTLER_1);
    EXPECT_EQ(gBattlerByTurnOrder[2], B_BATTLER_0);

    struct DamageContext damage = {
        .battlerAtk = B_BATTLER_0, .battlerDef = B_BATTLER_1,
        .move = MOVE_SONIC_BOOM, .moveType = TYPE_NORMAL,
        .updateFlags = TRUE, .useStoredTypeEffectiveness = TRUE,
    };
    struct SpecialStatus previousStatus = gSpecialStatuses[B_BATTLER_1];
    gSpecialStatuses[B_BATTLER_1].storedTypeEffectiveness = UQ_4_12(1.0);
    gCurrentTurnActionNumber = 0;
    EXPECT_EQ(CalculateMoveDamage(&damage), 20);
    gCurrentTurnActionNumber = 2;
    EXPECT_EQ(CalculateMoveDamage(&damage), 14);
    gSpecialStatuses[B_BATTLER_1] = previousStatus;

    gBattleStruct->chainSecondPos[B_BATTLER_1] = 2;
    Chain_ExpandTurnOrder();
    EXPECT_EQ(Chain_GetNumTurnActions(), 4);
    gCurrentTurnActionNumber = 0;
    Chain_RecordOpeningMove(B_BATTLER_0, MOVE_EMBER);
    gCurrentTurnActionNumber = 2;
    Chain_OrderFollowUps();
    EXPECT_EQ(gBattlerByTurnOrder[2], B_BATTLER_1);
    EXPECT_EQ(gBattlerByTurnOrder[3], B_BATTLER_0);
    EXPECT(Chain_PrepareAction(B_BATTLER_1)); // Its opening move never ran.

    gCurrentTurnActionNumber = 3;
    EXPECT(Chain_IsFollowingUp(B_BATTLER_0));
    gBattleStruct->shackleTurns[B_BATTLER_0] = 1;
    EXPECT(Chain_PrepareAction(B_BATTLER_0));
    EXPECT_EQ(gBattleMons[B_BATTLER_0].pp[1], 10);
    EXPECT_EQ(Stamina_GetCurrent(B_BATTLER_0), 100);
    gBattleStruct->shackleTurns[B_BATTLER_0] = 0;
    EXPECT(!Chain_PrepareAction(B_BATTLER_0));
    gBattleStruct->chosenMovePositions[B_BATTLER_0] = 0;
    Chain_InvalidateFollowUps();
    EXPECT(Chain_PrepareAction(B_BATTLER_0));

    // Team-wide support must reach the actual follow-up action. Previously
    // status moves could be selected but were unconditionally cancelled.
    gBattleMons[B_BATTLER_0].moves[0] = MOVE_REFLECT;
    gBattleMons[B_BATTLER_0].moves[1] = MOVE_LIGHT_SCREEN;
    gBattleStruct->chosenMovePositions[B_BATTLER_0] = 0;
    gBattleStruct->chainSecondPos[B_BATTLER_0] = 2;
    gBattleStruct->chainSecondPos[B_BATTLER_1] = 0;
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_NONE;
    gCurrentTurnActionNumber = 0;
    Chain_ExpandTurnOrder();
    EXPECT_EQ(Chain_GetNumTurnActions(), 3);
    Chain_RecordOpeningMove(B_BATTLER_0, MOVE_REFLECT);
    gCurrentTurnActionNumber = 2;
    EXPECT(Chain_IsFollowingUp(B_BATTLER_0));
    EXPECT(Chain_AllowsResolvedCategory(B_BATTLER_0, MOVE_LIGHT_SCREEN));
    EXPECT(!Chain_PrepareAction(B_BATTLER_0));

    gBattleMons[B_BATTLER_0] = previousMons[0];
    gBattleMons[B_BATTLER_1] = previousMons[1];
    gBattlersCount = previousCount;
    gBattleTypeFlags = previousBattleType;
    gCurrentTurnActionNumber = previousAction;
    memcpy(gBattlerByTurnOrder, previousOrder, sizeof(previousOrder));
    memcpy(gActionsByTurnOrder, previousActions, sizeof(previousActions));
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}

TEST("Exhausted Pokemon on either side cannot recover until the stamina threshold")
{
    struct BattleStruct *previousBattleStruct = gBattleStruct;
    struct BattlePokemon previousMons[2] = {gBattleMons[B_BATTLER_0], gBattleMons[B_BATTLER_1]};

    gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    for (enum BattlerId battler = B_BATTLER_0; battler <= B_BATTLER_1; battler++)
    {
        gBattleMons[battler] = (struct BattlePokemon){0};
        gBattleStruct->staminaOwner[battler] = gBattlerPartyIndexes[battler] + 1;
        gBattleStruct->maxStamina[battler] = 100;
        gBattleStruct->stamina[battler] = 5;

        // A final effort is allowed but must exhaust BOTH sides afterward.
        Stamina_SpendForMove(battler, MOVE_EMBER, FALSE);
        EXPECT_EQ(Stamina_GetCurrent(battler), 0);
        EXPECT(Stamina_IsExhausted(battler));

        Stamina_EndOfTurn(battler);
        EXPECT_EQ(Stamina_GetCurrent(battler), 18);
        EXPECT(Stamina_IsExhausted(battler));
        EXPECT_EQ(gBattleMons[battler].volatiles.rechargeTimer, 1);

        // Recovery is only possible once the configured threshold is met.
        gBattleMons[battler].volatiles.rechargeTimer = 0;
        Stamina_EndOfTurn(battler);
        EXPECT_EQ(Stamina_GetCurrent(battler), 36);
        EXPECT(!Stamina_IsExhausted(battler));

        // Even with a stale exhaustion flag, zero stamina must block actions.
        gBattleStruct->stamina[battler] = 0;
        gBattleStruct->staminaExhausted[battler] = FALSE;
        EXPECT(Stamina_IsExhausted(battler));
    }

    gBattleMons[B_BATTLER_0] = previousMons[0];
    gBattleMons[B_BATTLER_1] = previousMons[1];
    Free(gBattleStruct);
    gBattleStruct = previousBattleStruct;
}
