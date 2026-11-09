#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_util.h"
#include "battle_controllers.h"
#include "move.h"
#include "battle_stamina.h"
#include "battle_chain.h"
#include "task.h"

bool32 Chain_IsAvailable(enum BattlerId battler)
{
    if (!CHAIN_ENABLED)
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

    return move != MOVE_NONE && effect != EFFECT_PROTECT && effect != EFFECT_ENDURE;
}

u32 Chain_GetNumTurnActions(void)
{
    return gBattleStruct->numTurnActions != 0 ? gBattleStruct->numTurnActions : gBattlersCount;
}

void Chain_SelectOpponentMove(enum BattlerId battler, u8 firstMovePos)
{
    struct ChooseMoveStruct *moveInfo;

    if (!Chain_IsAvailable(battler)
     || firstMovePos >= MAX_MON_MOVES
     || !Chain_IsMoveChainable(gBattleMons[battler].moves[firstMovePos]))
        return;

    moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    gBattleStruct->chainSecondPos[battler] = 0;

    for (u8 movePos = 0; movePos < MAX_MON_MOVES; movePos++)
    {
        if (movePos == firstMovePos
         || moveInfo->moves[movePos] == moveInfo->moves[firstMovePos]
         || moveInfo->moves[movePos] == MOVE_NONE
         || moveInfo->currentPP[movePos] == 0
         || !Chain_IsMoveChainable(moveInfo->moves[movePos]))
            continue;

        gBattleStruct->chainSecondPos[battler] = movePos + 1;
        break;
    }
}

static void Task_ChainOpponentSignal(u8 taskId)
{
    enum BattlerId battler = gTasks[taskId].data[0];
    struct Sprite *sprite = &gSprites[gBattlerSpriteIds[battler]];

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
    u8 taskId = CreateTask(Task_ChainOpponentSignal, 0);

    gTasks[taskId].data[0] = battler;
    gTasks[taskId].data[1] = 0;
}

void Chain_ExpandTurnOrder(void)
{
    enum BattlerId chainer;
    enum BattlerId rival;
    u8 actions[MAX_BATTLERS_COUNT];
    u8 battlers[MAX_BATTLERS_COUNT];
    u8 roles[MAX_BATTLERS_COUNT] = {0};

    gBattleStruct->numTurnActions = 0;
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));

    if (gBattleStruct->chainSecondPos[B_BATTLER_0] != 0 && Chain_IsAvailable(B_BATTLER_0))
        chainer = B_BATTLER_0;
    else if (gBattleStruct->chainSecondPos[B_BATTLER_1] != 0 && Chain_IsAvailable(B_BATTLER_1))
        chainer = B_BATTLER_1;
    else
        return;

    rival = GetBattlerAtPosition(IsOnPlayerSide(chainer) ? B_POSITION_OPPONENT_LEFT : B_POSITION_PLAYER_LEFT);

    // Ambos tienen que estar usando un movimiento; si no, la cadena se descarta.
    if (gActionsByTurnOrder[0] != B_ACTION_USE_MOVE || gActionsByTurnOrder[1] != B_ACTION_USE_MOVE)
    {
        gBattleStruct->chainSecondPos[chainer] = 0;
        return;
    }

    // Conserva el orden normal, inserta el segundo movimiento junto al primero
    // y concede al rival un turno adicional al final.
    u32 action = 0;
    for (u32 i = 0; i < gBattlersCount; i++)
    {
        battlers[action] = gBattlerByTurnOrder[i];
        actions[action++] = B_ACTION_USE_MOVE;
        if (gBattlerByTurnOrder[i] == chainer)
        {
            battlers[action] = chainer;
            actions[action] = B_ACTION_USE_MOVE;
            roles[action++] = 1;
        }
    }
    battlers[action] = rival;
    actions[action++] = B_ACTION_USE_MOVE;

    for (u32 i = 0; i < action; i++)
    {
        gActionsByTurnOrder[i] = actions[i];
        gBattlerByTurnOrder[i] = battlers[i];
        gBattleStruct->turnActionRole[i] = roles[i];
    }
    gBattleStruct->numTurnActions = action;
}

bool32 Chain_PrepareAction(enum BattlerId battler, u32 turnActionIndex)
{
    if (turnActionIndex >= MAX_BATTLERS_COUNT || gBattleStruct->turnActionRole[turnActionIndex] != 1)
        return FALSE;

    // Si el primer movimiento lo dejó agotado, el segundo se pierde (y el rival igual tiene su turno extra).
    if (Stamina_IsExhausted(battler))
        return TRUE;

    gBattleStruct->chosenMovePositions[battler] = gBattleStruct->chainSecondPos[battler] - 1;
    if (!IsOnPlayerSide(battler))
        Chain_ShowOpponentSignal(battler);
    return FALSE;
}

void Chain_Reset(void)
{
    memset(gBattleStruct->chainFirstPos, 0, sizeof(gBattleStruct->chainFirstPos));
    memset(gBattleStruct->chainSecondPos, 0, sizeof(gBattleStruct->chainSecondPos));
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));
    gBattleStruct->numTurnActions = 0;
}
