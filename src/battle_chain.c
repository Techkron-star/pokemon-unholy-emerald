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

// Rol 0: acción normal. Rol 1: segundo movimiento de una cadena. Rol 3: turno extra del rival,
// que usa otro movimiento distinto (nunca repite el mismo ataque).
#define CHAIN_ROLE_NORMAL       0
#define CHAIN_ROLE_SECOND       1
#define CHAIN_ROLE_RIVAL_EXTRA  3

static void AddTurnAction(u32 *n, u8 *battlers, u8 *roles, enum BattlerId battler, u8 role)
{
    battlers[*n] = battler;
    roles[*n] = role;
    (*n)++;
}

void Chain_ExpandTurnOrder(void)
{
    enum BattlerId player = B_BATTLER_0;
    enum BattlerId rival = B_BATTLER_1;
    u8 battlers[MAX_BATTLERS_COUNT] = {0};
    u8 roles[MAX_BATTLERS_COUNT] = {0};
    u32 n = 0;
    bool32 playerChains = gBattleStruct->chainSecondPos[player] != 0 && Chain_IsAvailable(player);
    bool32 rivalChains = gBattleStruct->chainSecondPos[rival] != 0 && Chain_IsAvailable(rival);

    gBattleStruct->numTurnActions = 0;
    gBattleStruct->chainTurnInterrupted = FALSE;
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));

    if (!playerChains && !rivalChains)
        return;

    // Ambos tienen que estar usando un movimiento; si no, la cadena se descarta.
    if (gActionsByTurnOrder[0] != B_ACTION_USE_MOVE || gActionsByTurnOrder[1] != B_ACTION_USE_MOVE)
    {
        gBattleStruct->chainSecondPos[player] = 0;
        gBattleStruct->chainSecondPos[rival] = 0;
        return;
    }

    // Orden: F1 [F2] S1 [S2]. Al que no encadena le toca un turno extra al final.
    enum BattlerId first = gBattlerByTurnOrder[0];
    enum BattlerId second = first == player ? rival : player;
    bool32 firstChains = first == player ? playerChains : rivalChains;
    bool32 secondChains = second == player ? playerChains : rivalChains;

    AddTurnAction(&n, battlers, roles, first, CHAIN_ROLE_NORMAL);
    if (firstChains)
        AddTurnAction(&n, battlers, roles, first, CHAIN_ROLE_SECOND);
    AddTurnAction(&n, battlers, roles, second, CHAIN_ROLE_NORMAL);
    if (secondChains)
        AddTurnAction(&n, battlers, roles, second, CHAIN_ROLE_SECOND);
    if (!firstChains && secondChains)
        AddTurnAction(&n, battlers, roles, first, first == rival ? CHAIN_ROLE_RIVAL_EXTRA : CHAIN_ROLE_NORMAL);
    if (firstChains && !secondChains)
        AddTurnAction(&n, battlers, roles, second, second == rival ? CHAIN_ROLE_RIVAL_EXTRA : CHAIN_ROLE_NORMAL);

    for (u32 i = 0; i < n; i++)
    {
        gActionsByTurnOrder[i] = B_ACTION_USE_MOVE;
        gBattlerByTurnOrder[i] = battlers[i];
        gBattleStruct->turnActionRole[i] = roles[i];
    }
    gBattleStruct->numTurnActions = n;
}

// Turno extra del rival: elige el movimiento más fuerte que no sea el ya usado ni el mismo nombre.
// Devuelve FALSE si no hay ninguno (la acción se salta).
static bool32 PickDifferentRivalMove(enum BattlerId battler)
{
    u8 used = gBattleStruct->chosenMovePositions[battler];
    enum Move usedMove = gBattleMons[battler].moves[used];
    u32 bestPower = 0;
    u32 bestPos = MAX_MON_MOVES;

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = gBattleMons[battler].moves[i];

        if (i == used || move == MOVE_NONE || move == usedMove || gBattleMons[battler].pp[i] == 0)
            continue;
        if (Stamina_GetMoveCost(battler, move) >= Stamina_GetCurrent(battler))
            continue;
        if (bestPos == MAX_MON_MOVES || GetMovePower(move) >= bestPower)
        {
            bestPower = GetMovePower(move);
            bestPos = i;
        }
    }

    if (bestPos == MAX_MON_MOVES)
        return FALSE;
    gBattleStruct->chosenMovePositions[battler] = bestPos;
    return TRUE;
}

bool32 Chain_PrepareAction(enum BattlerId battler, u32 turnActionIndex)
{
    if (gBattleStruct->chainTurnInterrupted)
        return TRUE;
    if (turnActionIndex >= MAX_BATTLERS_COUNT)
        return FALSE;

    if (gBattleStruct->turnActionRole[turnActionIndex] == CHAIN_ROLE_RIVAL_EXTRA)
        return !PickDifferentRivalMove(battler);

    if (gBattleStruct->turnActionRole[turnActionIndex] != CHAIN_ROLE_SECOND)
        return FALSE;

    // Si el primer movimiento lo dejó agotado, el segundo se pierde.
    if (Stamina_IsExhausted(battler))
        return TRUE;

    gBattleStruct->chosenMovePositions[battler] = gBattleStruct->chainSecondPos[battler] - 1;
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
    return gBattleStruct->numTurnActions != 0 && gBattleStruct->chainTurnInterrupted;
}

void Chain_Reset(void)
{
    memset(gBattleStruct->chainFirstPos, 0, sizeof(gBattleStruct->chainFirstPos));
    memset(gBattleStruct->chainSecondPos, 0, sizeof(gBattleStruct->chainSecondPos));
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));
    gBattleStruct->numTurnActions = 0;
    gBattleStruct->chainTurnInterrupted = FALSE;
}
