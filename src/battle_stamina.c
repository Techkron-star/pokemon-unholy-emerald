#include "global.h"
#include "battle.h"
#include "move.h"
#include "pokemon.h"
#include "battle_stamina.h"

// La stamina vive en gBattleStruct, así que no toca el formato de guardado.
// Se (re)inicializa al máximo cada vez que un Pokémon entra al campo: se detecta
// comparando el índice de equipo guardado con el actual.

static u32 CalcMaxStamina(enum BattlerId battler)
{
    struct Pokemon *mon = GetBattlerMon(battler);
    u32 level = GetMonData(mon, MON_DATA_LEVEL);
    u32 hpEv = GetMonData(mon, MON_DATA_HP_EV);
    u32 hpIv = GetMonData(mon, MON_DATA_HP_IV);

    return STAMINA_MAX_BASE
         + level / STAMINA_MAX_LEVEL_DIVISOR
         + hpEv / STAMINA_MAX_EV_DIVISOR
         + hpIv / STAMINA_MAX_IV_DIVISOR;
}

static void EnsureInitialized(enum BattlerId battler)
{
    u8 owner = gBattlerPartyIndexes[battler] + 1;

    if (gBattleStruct->staminaOwner[battler] == owner)
        return;

    gBattleStruct->staminaOwner[battler] = owner;
    gBattleStruct->maxStamina[battler] = CalcMaxStamina(battler);
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

u32 Stamina_GetMoveCost(enum Move move)
{
    // Potencia efectiva = potencia * precisión (los que nunca fallan cuentan como 100%).
    u32 power = GetMovePower(move);
    u32 accuracy = GetMoveAccuracy(move);

    if (power < 2)
        power = 0; // movimientos de estado o de daño variable: solo pagan el costo base
    if (accuracy == 0)
        accuracy = 100;

    return STAMINA_COST_BASE + power * accuracy / 100 / STAMINA_COST_POWER_DIVISOR;
}

void Stamina_SpendForMove(enum BattlerId battler, enum Move move)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 cost = Stamina_GetMoveCost(move);

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
