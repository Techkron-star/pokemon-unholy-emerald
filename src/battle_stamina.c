#include "global.h"
#include "battle.h"
#include "move.h"
#include "pokemon.h"
#include "battle_stamina.h"

// La stamina vive en gBattleStruct, así que no toca el formato de guardado.
// Se (re)inicializa al máximo cada vez que un Pokémon entra al campo: se detecta
// comparando el índice de equipo guardado con el actual.

u32 Stamina_CalcMaxForMon(struct Pokemon *mon)
{
    if (mon == NULL)
        return 0;
    u32 level = GetMonData(mon, MON_DATA_LEVEL);
    u32 hpEv = GetMonData(mon, MON_DATA_HP_EV);
    u32 hpIv = GetMonData(mon, MON_DATA_HP_IV);

    u32 raw = STAMINA_MAX_BASE
            + level / STAMINA_MAX_LEVEL_DIVISOR
            + hpEv / STAMINA_MAX_EV_DIVISOR
            + hpIv / STAMINA_MAX_IV_DIVISOR;
    s32 bonus = Pokemon_GetMutationStaminaBonusPercent(mon);

    return (raw * (100 + bonus) + 50) / 100;
}

static void EnsureInitialized(enum BattlerId battler)
{
    u8 owner = gBattlerPartyIndexes[battler] + 1;

    if (gBattleStruct->staminaOwner[battler] == owner)
        return;

    gBattleStruct->staminaOwner[battler] = owner;
    gBattleStruct->maxStamina[battler] = Stamina_CalcMaxForMon(GetBattlerMon(battler));
    gBattleStruct->stamina[battler] = gBattleStruct->maxStamina[battler];
    gBattleStruct->staminaExhausted[battler] = FALSE;
}

u32 Stamina_GetCurrent(enum BattlerId battler)
{
    EnsureInitialized(battler);
    return gBattleStruct->stamina[battler];
}

u32 Stamina_GetMax(enum BattlerId battler)
{
    EnsureInitialized(battler);
    return gBattleStruct->maxStamina[battler];
}

bool32 Stamina_IsExhausted(enum BattlerId battler)
{
    EnsureInitialized(battler);
    return gBattleStruct->staminaExhausted[battler];
}

u32 Stamina_GetMoveCost(enum BattlerId battler, enum Move move)
{
    // Every battler pays the same share of their own capacity. This keeps
    // cheap moves and status moves relevant even at the level cap.
    u32 maxStamina = Stamina_GetMax(battler);
    u32 power = GetMovePower(move);
    u32 accuracy = GetMoveAccuracy(move);
    u32 percent;

    if (power < 2)
        power = 0; // status / variable-damage moves: only the base percentage
    if (accuracy == 0 || accuracy > 100)
        accuracy = 100; // never-miss and nonstandard accuracy

    percent = STAMINA_COST_BASE_PERCENT
            + (power * accuracy / 100) / STAMINA_COST_POWER_DIVISOR;
    // Ceiling prevents tiny fractional costs from disappearing at low levels.
    return (maxStamina * percent + 99) / 100;
}

void Stamina_SpendForMove(enum BattlerId battler, enum Move move)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 cost = Stamina_GetMoveCost(battler, move);

    if (cost >= gBattleStruct->stamina[battler])
    {
        // El último esfuerzo sale igual, pero deja al Pokémon agotado.
        gBattleStruct->stamina[battler] = 0;
        gBattleStruct->staminaExhausted[battler] = TRUE;
    }
    else
    {
        gBattleStruct->stamina[battler] -= cost;
    }
}

void Stamina_RestoreOnKnockout(enum BattlerId battler, u32 numKos)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 max = gBattleStruct->maxStamina[battler];
    u32 stamina = gBattleStruct->stamina[battler] + max * STAMINA_KO_RESTORE_PERCENT / 100 * numKos;

    gBattleStruct->stamina[battler] = stamina > max ? max : stamina;
    // Si estaba agotado y ya supera el umbral, el chequeo de fin de turno lo recuperará.
}

void Stamina_EndOfTurn(enum BattlerId battler)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 max = gBattleStruct->maxStamina[battler];
    u32 regen = max * STAMINA_REGEN_PERCENT / 100;
    u32 stamina = gBattleStruct->stamina[battler] + (regen > 0 ? regen : 1);

    if (stamina > max)
        stamina = max;
    gBattleStruct->stamina[battler] = stamina;

    if (!gBattleStruct->staminaExhausted[battler])
        return;

    if (stamina * 100 >= max * STAMINA_RECOVER_PERCENT)
    {
        gBattleStruct->staminaExhausted[battler] = FALSE;
        return;
    }

    // Sigue agotado: pierde el próximo turno reutilizando el mecanismo de recarga
    // (el mismo que usa Hyper Beam), sin poder usar Struggle.
    gBattleMons[battler].volatiles.rechargeTimer = 1;
    gLockedMoves[battler] = gLastMoves[battler] != MOVE_NONE ? gLastMoves[battler] : gBattleMons[battler].moves[0];
}
