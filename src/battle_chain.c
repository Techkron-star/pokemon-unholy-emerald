#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_util.h"
#include "move.h"
#include "battle_stamina.h"
#include "battle_chain.h"

bool32 Chain_IsAvailable(enum BattlerId battler)
{
    if (!CHAIN_ENABLED)
        return FALSE;
    if (IsDoubleBattle())
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_SAFARI | BATTLE_TYPE_PALACE | BATTLE_TYPE_ARENA))
        return FALSE;
    return IsOnPlayerSide(battler);
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

void Chain_ExpandTurnOrder(void)
{
    enum BattlerId chainer = B_BATTLER_0;
    enum BattlerId rival = B_BATTLER_1;
    u8 actions[MAX_BATTLERS_COUNT];
    u8 battlers[MAX_BATTLERS_COUNT];
    u8 roles[MAX_BATTLERS_COUNT] = {0};

    gBattleStruct->numTurnActions = 0;
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));

    if (gBattleStruct->chainSecondPos[chainer] == 0 || !Chain_IsAvailable(chainer))
        return;

    // Ambos tienen que estar usando un movimiento; si no, la cadena se descarta.
    if (gActionsByTurnOrder[0] != B_ACTION_USE_MOVE || gActionsByTurnOrder[1] != B_ACTION_USE_MOVE)
    {
        gBattleStruct->chainSecondPos[chainer] = 0;
        return;
    }

    if (gBattlerByTurnOrder[0] == chainer)
    {
        // jugador, jugador (2º movimiento), rival, rival (turno extra)
        battlers[0] = chainer; battlers[1] = chainer; battlers[2] = rival; battlers[3] = rival;
        roles[1] = 1;
    }
    else
    {
        // rival, jugador, jugador (2º movimiento), rival (turno extra)
        battlers[0] = rival; battlers[1] = chainer; battlers[2] = chainer; battlers[3] = rival;
        roles[2] = 1;
    }

    for (u32 i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        actions[i] = B_ACTION_USE_MOVE;
        gActionsByTurnOrder[i] = actions[i];
        gBattlerByTurnOrder[i] = battlers[i];
        gBattleStruct->turnActionRole[i] = roles[i];
    }
    gBattleStruct->numTurnActions = MAX_BATTLERS_COUNT;
}

bool32 Chain_PrepareAction(enum BattlerId battler, u32 turnActionIndex)
{
    if (turnActionIndex >= MAX_BATTLERS_COUNT || gBattleStruct->turnActionRole[turnActionIndex] != 1)
        return FALSE;

    // Si el primer movimiento lo dejó agotado, el segundo se pierde (y el rival igual tiene su turno extra).
    if (Stamina_IsExhausted(battler))
        return TRUE;

    gBattleStruct->chosenMovePositions[battler] = gBattleStruct->chainSecondPos[battler] - 1;
    return FALSE;
}

void Chain_Reset(void)
{
    memset(gBattleStruct->chainFirstPos, 0, sizeof(gBattleStruct->chainFirstPos));
    memset(gBattleStruct->chainSecondPos, 0, sizeof(gBattleStruct->chainSecondPos));
    memset(gBattleStruct->turnActionRole, 0, sizeof(gBattleStruct->turnActionRole));
    gBattleStruct->numTurnActions = 0;
}
