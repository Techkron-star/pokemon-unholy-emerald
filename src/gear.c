#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "move.h"
#include "pokemon.h"
#include "gear.h"
#include "item.h"
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

// Explicit mapping: item IDs and Gear IDs are independently append-only.
enum Gear Gear_FromItem(enum Item item)
{
    switch (item)
    {
    case ITEM_GEAR_VISOR_COSMICO: return GEAR_VISOR_COSMICO;
    case ITEM_GEAR_CORAZON_TITAN: return GEAR_CORAZON_TITAN;
    case ITEM_GEAR_SKI_MASK: return GEAR_SKI_MASK;
    case ITEM_GEAR_CETRO_SINIESTRO: return GEAR_CETRO_SINIESTRO;
    case ITEM_GEAR_IMPULSO_COSMICO: return GEAR_IMPULSO_COSMICO;
    case ITEM_GEAR_PERMAFROST_FRAGMENT: return GEAR_PERMAFROST_FRAGMENT;
    default: return GEAR_NONE;
    }
}

enum Item Gear_ToItem(enum Gear gear)
{
    switch (gear)
    {
    case GEAR_VISOR_COSMICO: return ITEM_GEAR_VISOR_COSMICO;
    case GEAR_CORAZON_TITAN: return ITEM_GEAR_CORAZON_TITAN;
    case GEAR_SKI_MASK: return ITEM_GEAR_SKI_MASK;
    case GEAR_CETRO_SINIESTRO: return ITEM_GEAR_CETRO_SINIESTRO;
    case GEAR_IMPULSO_COSMICO: return ITEM_GEAR_IMPULSO_COSMICO;
    case GEAR_PERMAFROST_FRAGMENT: return ITEM_GEAR_PERMAFROST_FRAGMENT;
    default: return ITEM_NONE;
    }
}

enum GearEquipResult Gear_TryEquipFromBag(struct Pokemon *mon, enum Item item)
{
    enum Gear requested = Gear_FromItem(item);
    enum Gear current;
    enum Item previousItem;

    if (mon == NULL || requested == GEAR_NONE)
        return GEAR_EQUIP_NOT_GEAR;
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return GEAR_EQUIP_NOT_GEAR;
    current = Gear_GetEquipped(mon);
    if (current == requested)
        return GEAR_EQUIP_ALREADY;
    if (!CheckBagHasItem(item, 1))
        return GEAR_EQUIP_NOT_OWNED;

    previousItem = Gear_ToItem(current);
    // No Pokémon mutation until the inventory exchange is complete.
    if (!RemoveBagItem(item, 1))
        return GEAR_EQUIP_NOT_OWNED;

    if (current != GEAR_NONE)
    {
        if (previousItem == ITEM_NONE || !AddBagItem(previousItem, 1))
        {
            AddBagItem(item, 1); // Restore the incoming Gear on failure.
            return GEAR_EQUIP_BAG_FULL;
        }
    }

    Gear_SetEquipped(mon, requested);
    return GEAR_EQUIP_OK;
}

static void StartCycle(struct GearCycle *cycle, const struct GearInfo *info)
{
    if (info->trigger == GEAR_TRIGGER_TIMED)
    {
        cycle->phase = GEAR_PHASE_OVERLOAD;
        cycle->turns = max(info->overloadTurns, 1);
    }
    else
    {
        cycle->phase = GEAR_PHASE_ARMED;
        cycle->turns = 0;
    }
}

static void StartRecharge(struct GearCycle *cycle, const struct GearInfo *info)
{
    cycle->phase = GEAR_PHASE_RECHARGING;
    cycle->turns = max(info->cooldownTurns, 1);
}

static struct GearCycle *Prepare(enum BattlerId battler)
{
    u32 slot = gBattlerPartyIndexes[battler];

    if (slot >= PARTY_SIZE)
    {
        gBattleStruct->gearOwner[battler] = 0;
        return NULL;
    }

    // Dos puestos del mismo entrenador comparten equipo. Entrenadores
    // distintos pueden tener el mismo índice de equipo sin compartir el ciclo.
    enum BattleTrainer trainer = GetBattlerTrainer(battler);
    u8 owner = trainer * PARTY_SIZE + slot + 1;
    struct GearCycle *cycle = &gBattleStruct->gearCycles[trainer][slot];
    if (gBattleStruct->gearOwner[battler] != owner)
    {
        enum Gear gear = Gear_GetEquipped(&GetTrainerParty(trainer)[slot]);
        gBattleStruct->gearOwner[battler] = owner;
        gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;

        if (cycle->gear != gear)
        {
            cycle->gear = gear;
            StartCycle(cycle, &gGearsInfo[gear]);
        }
    }
    return cycle->gear == GEAR_NONE ? NULL : cycle;
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

static u32 CurrentPercent(const struct GearCycle *cycle, enum Move move, enum Type moveType)
{
    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    u32 percent = 0;

    if (IsCompatible(info, move, moveType))
        percent = cycle->phase == GEAR_PHASE_RECHARGING ? info->residualPowerPercent : info->overloadPowerPercent;
    return percent != 0 ? percent : 100;
}

enum Gear Gear_GetBattlerGear(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return GEAR_NONE;
    const struct GearCycle *cycle = Prepare(battler);
    return cycle == NULL ? GEAR_NONE : cycle->gear;
}

enum GearPhase Gear_GetPhase(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return GEAR_PHASE_ARMED;

    const struct GearCycle *cycle = Prepare(battler);
    return cycle == NULL ? GEAR_PHASE_ARMED : cycle->phase;
}

uq4_12_t Gear_GetPowerModifier(enum BattlerId battler, enum Move move, enum Type moveType)
{
    const struct GearCycle *cycle;
    u32 percent;

    if (!GEARS_ENABLED)
        return UQ_4_12(1.0);
    cycle = Prepare(battler);
    if (cycle == NULL)
        return UQ_4_12(1.0);

    // Si el movimiento ya se está ejecutando, vale lo que se fijó al empezar
    // (la sobrecarga ya se gastó y la fase actual es la de recarga).
    if (gBattleStruct->gearSnapshotMove[battler] == move)
        percent = gBattleStruct->gearSnapshotPercent[battler];
    else
        percent = CurrentPercent(cycle, move, moveType);
    return (percent * 4096 + 50) / 100;
}

void Gear_OnMoveUsed(enum BattlerId battler, enum Move move, enum Type moveType)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED)
        return;
    cycle = Prepare(battler);
    if (cycle == NULL)
        return;

    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    gBattleStruct->gearSnapshotMove[battler] = move;
    gBattleStruct->gearSnapshotPercent[battler] = CurrentPercent(cycle, move, moveType);

    if (info->trigger == GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE
     && cycle->phase == GEAR_PHASE_ARMED
     && IsCompatible(info, move, moveType))
        StartRecharge(cycle, info); // el final de este mismo turno ya cuenta
}

void Gear_EndOfTurn(enum BattlerId battler)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED)
        return;
    gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;
    cycle = Prepare(battler);
    if (cycle == NULL)
        return;

    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    switch (cycle->phase)
    {
    case GEAR_PHASE_OVERLOAD:
        if (--cycle->turns == 0)
            StartRecharge(cycle, info);
        break;
    case GEAR_PHASE_RECHARGING:
        if (--cycle->turns == 0)
            StartCycle(cycle, info);
        break;
    default:
        break;
    }
}
