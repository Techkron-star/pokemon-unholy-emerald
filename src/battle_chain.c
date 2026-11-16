#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_util.h"
#include "battle_controllers.h"
#include "move.h"
#include "item.h"
#include "battle_stamina.h"
#include "battle_chain.h"
#include "gear.h"
#include "task.h"

bool32 Chain_IsAvailable(enum BattlerId battler)
{
    if (!CHAIN_ENABLED || gBattleStruct->shackleTurns[battler] != 0)
        return FALSE;
    if (IsDoubleBattle())
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_SAFARI | BATTLE_TYPE_PALACE | BATTLE_TYPE_ARENA))
        return FALSE;
    return TRUE;
}

bool32 Chain_IsMoveChainable(enum Move move)
{
    enum BattleMoveEffects effect = GetMoveEffect(move);

    return move != MOVE_NONE && move != MOVE_STRUGGLE
        && GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS
        && !gBattleMoveEffects[effect].twoTurnEffect
        && effect != EFFECT_FUTURE_SIGHT;
}

bool32 Chain_IsPairAllowed(enum BattlerId battler, enum Move first, enum Move second)
{
    if (first == second || !Chain_IsMoveChainable(first) || !Chain_IsMoveChainable(second))
        return FALSE;
    if (IsHoldEffectChoice(GetBattlerHoldEffect(battler)) || GetBattlerAbility(battler) == ABILITY_GORILLA_TACTICS)
        return FALSE;

    return GetMoveCategory(first) != DAMAGE_CATEGORY_SPECIAL
        || GetMoveCategory(second) != DAMAGE_CATEGORY_SPECIAL
        || Gear_GetBattlerGear(battler) != GEAR_STORM_STAFF;
}

bool32 Chain_CanAfford(enum BattlerId battler, enum Move first, enum Move second)
{
    return !STAMINA_ENABLED || Stamina_GetMoveCost(battler, first) + Stamina_GetActionCost(battler, second, TRUE)
                           <= Stamina_GetCurrent(battler);
}

u32 Chain_GetNumTurnActions(void)
{
    return gBattleStruct->numTurnActions != 0 ? gBattleStruct->numTurnActions : gBattlersCount;
}

void Chain_SelectOpponentMove(enum BattlerId battler, u8 firstMovePos)
{
    struct ChooseMoveStruct *moveInfo;

    // The AI must make a tactical choice, not automatically chain every turn
    // simply because its moveset contains two different moves.
    gBattleStruct->chainSecondPos[battler] = 0;
    if (!Chain_IsAvailable(battler)
     || firstMovePos >= MAX_MON_MOVES
     || !Chain_IsMoveChainable(gBattleMons[battler].moves[firstMovePos])
     || Stamina_IsExhausted(battler))
        return;

    // In a trainer battle the AI already simulated the selected attack.
    // If even its minimum expected damage is lethal, a chained move would
    // only waste PP/Stamina (and will be cancelled by the KO anyway).
    if ((gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     && gAiLogicData != NULL && gAiBattleData != NULL)
    {
        enum BattlerId target = gAiBattleData->chosenTarget[battler];

        if (target < gBattlersCount && IsBattlerAlive(target)
         && gAiLogicData->simulatedDmg[battler][target][firstMovePos].minimum >= gBattleMons[target].hp)
            return;
    }

    moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    u32 limitations = CheckMoveLimitations(battler, 0, MOVE_LIMITATIONS_ALL);

    for (u8 movePos = 0; movePos < MAX_MON_MOVES; movePos++)
    {
        if ((limitations & (1u << movePos)) || moveInfo->currentPP[movePos] == 0
         || !Chain_IsPairAllowed(battler, moveInfo->moves[firstMovePos], moveInfo->moves[movePos])
         || !Chain_CanAfford(battler, moveInfo->moves[firstMovePos], moveInfo->moves[movePos]))
            continue;

        // Avoid follow-ups that the AI already knows cannot damage the foe.
        if ((gBattleTypeFlags & BATTLE_TYPE_TRAINER)
         && gAiLogicData != NULL && gAiBattleData != NULL)
        {
            enum BattlerId target = gAiBattleData->chosenTarget[battler];

            if (target < gBattlersCount && GetMovePower(moveInfo->moves[movePos]) != 0
             && gAiLogicData->simulatedDmg[battler][target][movePos].maximum == 0)
                continue;
        }

        gBattleStruct->chainSecondPos[battler] = movePos + 1;
        break;
    }
}

static void Task_ChainOpponentSignal(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].data[0];
    u8 spriteId = gTasks[taskId].data[2];
    struct Sprite *sprite;

    // A faint, switch or scene transition can destroy the battler sprite
    // before this visual task's 12-frame animation finishes.
    if (spriteId >= MAX_SPRITES || gBattlerSpriteIds[battler] != spriteId
     || !gSprites[spriteId].inUse)
    {
        DestroyTask(taskId);
        return;
    }

    sprite = &gSprites[spriteId];
    if (++gTasks[taskId].data[1] % 2 == 0)
        sprite->invisible = !sprite->invisible;

    if (gTasks[taskId].data[1] >= 12)
    {
        sprite->invisible = FALSE;
        DestroyTask(taskId);
    }
}

static void Chain_ShowOpponentSignal(enum BattlerId battler)
{
    u8 taskId;
    u32 i;

    if (gBattlerSpriteIds[battler] >= MAX_SPRITES
     || !gSprites[gBattlerSpriteIds[battler]].inUse)
        return;

    // CreateTask returns task ID zero on exhaustion, which can already be
    // occupied. Check capacity first to avoid corrupting another task.
    for (i = 0; i < NUM_TASKS; i++)
        if (!gTasks[i].isActive)
            break;
    if (i == NUM_TASKS)
        return;

    taskId = CreateTask(Task_ChainOpponentSignal, 0);
    gTasks[taskId].data[0] = battler;
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = gBattlerSpriteIds[battler];
}

enum ChainActionRole
{
    CHAIN_ROLE_OPENING,
    CHAIN_ROLE_FOLLOW_UP,
};

static bool32 HasValidSelection(enum BattlerId battler)
{
    u8 first = gBattleStruct->chosenMovePositions[battler];
    u8 second = gBattleStruct->chainSecondPos[battler] - 1;
    if (!Chain_IsAvailable(battler) || first >= MAX_MON_MOVES || second >= MAX_MON_MOVES)
        return FALSE;

    enum Move firstMove = gBattleMons[battler].moves[first];
    enum Move secondMove = gBattleMons[battler].moves[second];
    return Chain_IsPairAllowed(battler, firstMove, secondMove) && Chain_CanAfford(battler, firstMove, secondMove);
}

void Chain_ExpandTurnOrder(void)
{
    gBattleStruct->numTurnActions = 0;
    gBattleStruct->chainTurnInterrupted = FALSE;
    gBattleStruct->chainFollowUpsInvalid = FALSE;
    gBattleStruct->chainSpecialMask = 0;
    gBattleStruct->chainOpeningMask = 0;
    memset(gBattleStruct->turnActionRole, CHAIN_ROLE_OPENING, sizeof(gBattleStruct->turnActionRole));

    if (!CHAIN_ENABLED || gBattlersCount != 2
     || gActionsByTurnOrder[0] != B_ACTION_USE_MOVE || gActionsByTurnOrder[1] != B_ACTION_USE_MOVE)
        return;

    u32 nextAction = gBattlersCount;
    for (u32 action = 0; action < gBattlersCount; action++)
    {
        enum BattlerId battler = gBattlerByTurnOrder[action];
        if (HasValidSelection(battler))
        {
            gBattlerByTurnOrder[nextAction] = battler;
            gActionsByTurnOrder[nextAction] = B_ACTION_USE_MOVE;
            gBattleStruct->turnActionRole[nextAction++] = CHAIN_ROLE_FOLLOW_UP;
        }
    }

    if (nextAction > gBattlersCount)
        gBattleStruct->numTurnActions = nextAction;
}

void Chain_OrderFollowUps(void)
{
    if (gCurrentTurnActionNumber != gBattlersCount || gBattleStruct->numTurnActions != MAX_BATTLERS_COUNT
     || gBattleStruct->chainTurnInterrupted || gBattleStruct->chainFollowUpsInvalid)
        return;

    enum BattlerId first = gBattlerByTurnOrder[2];
    enum BattlerId second = gBattlerByTurnOrder[3];
    struct BattleCalcValues calc = {.battlerAtk = first, .battlerDef = second};
    calc.abilities[first] = GetBattlerAbility(first);
    calc.abilities[second] = GetBattlerAbility(second);
    calc.holdEffects[first] = GetBattlerHoldEffect(first);
    calc.holdEffects[second] = GetBattlerHoldEffect(second);

    enum Move firstMove = gBattleMons[first].moves[gBattleStruct->chainSecondPos[first] - 1];
    enum Move secondMove = gBattleMons[second].moves[gBattleStruct->chainSecondPos[second] - 1];
    if (GetWhichBattlerFasterArgs(&calc, FALSE,
         GetBattlerTotalSpeedStat(first, calc.abilities[first], calc.holdEffects[first]),
         GetBattlerTotalSpeedStat(second, calc.abilities[second], calc.holdEffects[second]),
         GetBattleMovePriority(first, calc.abilities[first], firstMove),
         GetBattleMovePriority(second, calc.abilities[second], secondMove)) < 0)
    {
        gBattlerByTurnOrder[2] = second;
        gBattlerByTurnOrder[3] = first;
    }
}

bool32 Chain_IsFollowingUp(enum BattlerId battler)
{
    u32 action = gCurrentTurnActionNumber;
    return gBattleStruct->numTurnActions != 0 && action < gBattleStruct->numTurnActions
        && gBattlerByTurnOrder[action] == battler
        && gBattleStruct->turnActionRole[action] == CHAIN_ROLE_FOLLOW_UP;
}

void Chain_RecordOpeningMove(enum BattlerId battler, enum Move move)
{
    if (gBattleStruct->numTurnActions == 0 || gCurrentTurnActionNumber >= gBattlersCount
     || gBattlerByTurnOrder[gCurrentTurnActionNumber] != battler || IsBattleMoveStatus(move))
        return;

    gBattleStruct->chainOpeningMask |= 1u << battler;
    if (IsBattleMoveSpecial(move))
        gBattleStruct->chainSpecialMask |= 1u << battler;
}

bool32 Chain_AllowsResolvedCategory(enum BattlerId battler, enum Move move)
{
    if (!Chain_IsFollowingUp(battler))
        return TRUE;
    if (IsBattleMoveStatus(move))
        return FALSE;

    return !IsBattleMoveSpecial(move) || !(gBattleStruct->chainSpecialMask & (1u << battler))
        || Gear_GetBattlerGear(battler) != GEAR_STORM_STAFF;
}

void Chain_InvalidateFollowUps(void)
{
    if (gBattleStruct->numTurnActions != 0)
        gBattleStruct->chainFollowUpsInvalid = TRUE;
}

bool32 Chain_PrepareAction(enum BattlerId battler)
{
    if (gBattleStruct->chainTurnInterrupted)
        return TRUE;
    if (!Chain_IsFollowingUp(battler))
        return FALSE;

    if (gBattleStruct->chainFollowUpsInvalid || !Chain_IsAvailable(battler)
     || !(gBattleStruct->chainOpeningMask & (1u << battler))
     || gBattleMons[battler].volatiles.multipleTurns
     || gBattleMons[battler].volatiles.rechargeTimer
     || (STAMINA_ENABLED && Stamina_IsExhausted(battler)))
        return TRUE;

    u8 first = gBattleStruct->chosenMovePositions[battler];
    u8 second = gBattleStruct->chainSecondPos[battler] - 1;
    if (first >= MAX_MON_MOVES || second >= MAX_MON_MOVES)
        return TRUE;

    enum Move move = gBattleMons[battler].moves[second];
    if (!Chain_IsPairAllowed(battler, gBattleMons[battler].moves[first], move)
     || (CheckMoveLimitations(battler, 0, MOVE_LIMITATIONS_ALL) & (1u << second))
     || (STAMINA_ENABLED && Stamina_GetActionCost(battler, move, TRUE) > Stamina_GetCurrent(battler)))
        return TRUE;

    gBattleStruct->chosenMovePositions[battler] = second;
    if (!IsOnPlayerSide(battler))
        Chain_ShowOpponentSignal(battler);
    return FALSE;
}

void Chain_MarkKnockout(void)
{
    // Solo alteramos turnos expandidos; el comportamiento original no cambia.
    if (gBattleStruct->numTurnActions != 0)
        gBattleStruct->chainTurnInterrupted = TRUE;
}

bool32 Chain_WasInterrupted(void)
{
    if (gBattleStruct->numTurnActions == 0)
        return FALSE;

    // HP can reach zero before SetValuesOnFaint marks the knockout.
    // Do not let any queued chain action target an empty side in that gap.
    if (!IsBattlerAlive(B_BATTLER_0) || !IsBattlerAlive(B_BATTLER_1))
        gBattleStruct->chainTurnInterrupted = TRUE;

    return gBattleStruct->chainTurnInterrupted;
}

void Chain_Reset(void)
{
    gBattleStruct->chainSpecialMask = 0;
    gBattleStruct->chainOpeningMask = 0;
    gBattleStruct->chainFollowUpsInvalid = FALSE;
    memset(gBattleStruct->chainFirstPos, 0, sizeof(gBattleStruct->chainFirstPos));
    memset(gBattleStruct->chainSecondPos, 0, sizeof(gBattleStruct->chainSecondPos));
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));
    gBattleStruct->numTurnActions = 0;
    gBattleStruct->chainTurnInterrupted = FALSE;
}
