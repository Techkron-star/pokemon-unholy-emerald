#include "global.h"
#include "battle.h"
#include "move.h"
#include "pokemon.h"
#include "battle_stamina.h"
#include "gear.h"

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

void Stamina_RestorePercent(enum BattlerId battler, u32 percent)
{
    if (!STAMINA_ENABLED)
        return;
    EnsureInitialized(battler);
    u32 maxStamina = gBattleStruct->maxStamina[battler];
    gBattleStruct->stamina[battler] = min(maxStamina,
        gBattleStruct->stamina[battler] + max(1, maxStamina * percent / 100));
}

bool32 Stamina_IsExhausted(enum BattlerId battler)
{
    EnsureInitialized(battler);
    return gBattleStruct->staminaExhausted[battler];
}

u32 Stamina_GetMoveCost(enum BattlerId battler, enum Move move)
{
    return Stamina_GetActionCost(battler, move, FALSE);
}

u32 Stamina_GetActionCost(enum BattlerId battler, enum Move move, bool32 followUp)
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
    u32 baseCost = (maxStamina * percent + 99) / 100;
    u32 costPercent = 100;
    if (followUp)
        costPercent = gBattleMons[battler].status1 & STATUS1_PARALYSIS
                    ? STAMINA_PARALYSIS_CHAIN_PERCENT : STAMINA_CHAIN_COST_PERCENT;
    if (gBattleMons[battler].volatiles.confusionTimer)
        costPercent += STAMINA_CONFUSION_COST_BONUS;

    u32 cost = (baseCost * costPercent + 99) / 100;
    if (Gear_GetBattlerGear(battler) == GEAR_MODULO_VECTORIAL && GetMoveEffect(move) == EFFECT_HIT_ESCAPE)
    {
        // Only actual pivot attacks, never generic quick attacks or switches.
        if (Gear_GetPhase(battler) == GEAR_PHASE_ARMED)
            cost = (cost * 75 + 99) / 100;
        else if (Gear_GetPhase(battler) == GEAR_PHASE_RECHARGING)
            cost = (cost * 92 + 99) / 100;
    }
    if (Gear_GetBattlerGear(battler) == GEAR_AMPLIFICADOR
     && IsSoundMove(move) && GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS
     && Gear_GetPhase(battler) == GEAR_PHASE_ARMED)
        cost = (cost * 110 + 99) / 100;
    return cost;
}

void Stamina_SpendForMove(enum BattlerId battler, enum Move move, bool32 followUp)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 cost = Stamina_GetActionCost(battler, move, followUp);

    if (Gear_GetBattlerGear(battler) == GEAR_MODULO_VECTORIAL && GetMoveEffect(move) == EFFECT_HIT_ESCAPE)
        Gear_ActivateEvent(battler);

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

u32 Stamina_GetRegenPercent(enum BattlerId battler)
{
    u32 status = gBattleMons[battler].status1;
    if (status & STATUS1_SLEEP)
        return STAMINA_SLEEP_REGEN_PERCENT;
    if (status & STATUS1_FREEZE)
        return STAMINA_FREEZE_REGEN_PERCENT;
    if (status & STATUS1_BURN)
        return STAMINA_BURN_REGEN_PERCENT;
    if (status & STATUS1_PARALYSIS)
        return STAMINA_PARALYSIS_REGEN_PERCENT;
    if (status & STATUS1_TOXIC_POISON)
        return STAMINA_TOXIC_REGEN_PERCENT;
    if (status & STATUS1_POISON)
        return STAMINA_POISON_REGEN_PERCENT;
    return STAMINA_REGEN_PERCENT;
}

void Stamina_EndOfTurn(enum BattlerId battler)
{
    if (!STAMINA_ENABLED)
        return;

    EnsureInitialized(battler);

    u32 max = gBattleStruct->maxStamina[battler];
    u32 regen = max * Stamina_GetRegenPercent(battler) / 100;
    u32 stamina = gBattleStruct->stamina[battler] + (regen > 0 ? regen : 1);

    if (Gear_GetBattlerGear(battler) == GEAR_BATERIA_RESERVA)
    {
        // Apply ordinary regeneration first. Crossing below half at the
        // end of a turn triggers the 20% surge only once per charge cycle.
        if (stamina * 2 < max && Gear_ActivateEvent(battler))
            stamina += max * 20 / 100;
        else if (Gear_GetPhase(battler) == GEAR_PHASE_RECHARGING)
            stamina += max * 4 / 100;
    }

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
