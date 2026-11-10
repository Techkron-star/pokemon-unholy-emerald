#include "global.h"
#include "battle.h"
#include "move.h"
#include "pokemon.h"
#include "gear.h"
#include "data/gears.h"

STATIC_ASSERT(GEAR_COUNT <= GEAR_MAX_COUNT, TooManyGears);

// El ciclo vive en gBattleStruct (no toca el formato de guardado). Se recuerda
// al cambiar un Pokémon durante un combate para evitar recargas gratis.

enum Gear Gear_GetEquipped(struct Pokemon *mon)
{
    u32 gear = GetMonData(mon, MON_DATA_GEAR);

    return gear < GEAR_COUNT ? gear : GEAR_NONE;
}

void Gear_SetEquipped(struct Pokemon *mon, enum Gear gear)
{
    u32 value = gear < GEAR_COUNT ? gear : GEAR_NONE;

    SetMonData(mon, MON_DATA_GEAR, &value);
}

static void StartCycle(enum BattlerId battler, const struct GearInfo *info)
{
    if (info->trigger == GEAR_TRIGGER_TIMED)
    {
        gBattleStruct->gearPhase[battler] = GEAR_PHASE_OVERLOAD;
        gBattleStruct->gearTimer[battler] = max(info->overloadTurns, 1);
    }
    else
    {
        gBattleStruct->gearPhase[battler] = GEAR_PHASE_ARMED;
        gBattleStruct->gearTimer[battler] = 0;
    }
}

// elapsed = finales de turno que ya cuentan como parte de la recarga.
static void StartRecharge(enum BattlerId battler, const struct GearInfo *info, u32 elapsed)
{
    s32 turns = max(info->cooldownTurns, 1) - elapsed;

    if (turns <= 0)
    {
        StartCycle(battler, info);
        return;
    }
    gBattleStruct->gearPhase[battler] = GEAR_PHASE_RECHARGING;
    gBattleStruct->gearTimer[battler] = turns;
}

// Conserva la fase al abandonar el campo (sin guardarla en la partida).
// Se indexa también por posición de combate: evita colisiones entre rivales
// de entrenadores distintos que tengan el mismo índice de equipo.
static void RememberCycle(enum BattlerId battler)
{
    u8 slot = gBattlerPartyIndexes[battler];

    if (slot >= PARTY_SIZE)
        return;
    gBattleStruct->gearPartyInitialized[battler][slot] = TRUE;
    gBattleStruct->gearPartyId[battler][slot] = gBattleStruct->gearId[battler];
    gBattleStruct->gearPartyPhase[battler][slot] = gBattleStruct->gearPhase[battler];
    gBattleStruct->gearPartyTimer[battler][slot] = gBattleStruct->gearTimer[battler];
}

// Inicializa el estado al entrar al campo y devuelve el Gear (NULL si no lleva ninguno).
static const struct GearInfo *Prepare(enum BattlerId battler)
{
    u8 slot = gBattlerPartyIndexes[battler];
    u8 owner = slot + 1;

    if (gBattleStruct->gearOwner[battler] != owner)
    {
        gBattleStruct->gearOwner[battler] = owner;
        gBattleStruct->gearId[battler] = Gear_GetEquipped(GetBattlerMon(battler));
        gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;

        if (slot < PARTY_SIZE
         && gBattleStruct->gearPartyInitialized[battler][slot]
         && gBattleStruct->gearPartyId[battler][slot] == gBattleStruct->gearId[battler])
        {
            gBattleStruct->gearPhase[battler] = gBattleStruct->gearPartyPhase[battler][slot];
            gBattleStruct->gearTimer[battler] = gBattleStruct->gearPartyTimer[battler][slot];
        }
        else
        {
            StartCycle(battler, &gGearsInfo[gBattleStruct->gearId[battler]]);
            RememberCycle(battler);
        }
    }
    return gBattleStruct->gearId[battler] == GEAR_NONE ? NULL : &gGearsInfo[gBattleStruct->gearId[battler]];
}

// Solo los movimientos de daño que cumplan los filtros del Gear lo activan.
static bool32 IsCompatible(const struct GearInfo *info, enum Move move, enum Type moveType)
{
    enum DamageCategory category = GetMoveCategory(move);

    if (category == DAMAGE_CATEGORY_STATUS)
        return FALSE;
    if (info->typeMask != 0 && !(info->typeMask & GEAR_TYPE(moveType)))
        return FALSE;
    if (info->categoryMask != 0 && !(info->categoryMask & (1 << category)))
        return FALSE;
    return TRUE;
}

static u32 CurrentPercent(enum BattlerId battler, const struct GearInfo *info, enum Move move, enum Type moveType)
{
    u32 percent = 0;

    if (IsCompatible(info, move, moveType))
        percent = gBattleStruct->gearPhase[battler] == GEAR_PHASE_RECHARGING ? info->residualPowerPercent : info->overloadPowerPercent;
    return percent != 0 ? percent : 100;
}

enum Gear Gear_GetBattlerGear(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return GEAR_NONE;
    Prepare(battler);
    return gBattleStruct->gearId[battler];
}

enum GearPhase Gear_GetPhase(enum BattlerId battler)
{
    Prepare(battler);
    return gBattleStruct->gearPhase[battler];
}

uq4_12_t Gear_GetPowerModifier(enum BattlerId battler, enum Move move, enum Type moveType)
{
    const struct GearInfo *info;
    u32 percent;

    if (!GEARS_ENABLED)
        return UQ_4_12(1.0);
    info = Prepare(battler);
    if (info == NULL)
        return UQ_4_12(1.0);

    // Si el movimiento ya se está ejecutando, vale lo que se fijó al empezar
    // (la sobrecarga ya se gastó y la fase actual es la de recarga).
    if (gBattleStruct->gearSnapshotMove[battler] == move)
        percent = gBattleStruct->gearSnapshotPercent[battler];
    else
        percent = CurrentPercent(battler, info, move, moveType);
    return (percent * 4096 + 50) / 100;
}

void Gear_OnMoveUsed(enum BattlerId battler, enum Move move, enum Type moveType)
{
    const struct GearInfo *info;

    if (!GEARS_ENABLED)
        return;
    info = Prepare(battler);
    if (info == NULL)
        return;

    gBattleStruct->gearSnapshotMove[battler] = move;
    gBattleStruct->gearSnapshotPercent[battler] = CurrentPercent(battler, info, move, moveType);

    if (info->trigger == GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE
     && gBattleStruct->gearPhase[battler] == GEAR_PHASE_ARMED
     && IsCompatible(info, move, moveType))
        StartRecharge(battler, info, 0); // el final de este mismo turno ya cuenta

    // Un U-turn/Volt Switch puede retirar al usuario antes del final del turno.
    RememberCycle(battler);
}

void Gear_EndOfTurn(enum BattlerId battler)
{
    const struct GearInfo *info;

    if (!GEARS_ENABLED)
        return;
    gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;
    info = Prepare(battler);
    if (info == NULL)
        return;

    switch (gBattleStruct->gearPhase[battler])
    {
    case GEAR_PHASE_OVERLOAD:
        if (--gBattleStruct->gearTimer[battler] == 0)
            StartRecharge(battler, info, 0); // la recarga comienza DESPUES del ultimo turno de sobrecarga
        break;
    case GEAR_PHASE_RECHARGING:
        if (--gBattleStruct->gearTimer[battler] == 0)
            StartCycle(battler, info);
        break;
    default:
        break;
    }
    RememberCycle(battler);
}
