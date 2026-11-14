#include "global.h"
#include "gear_battle.h"
#include "gear_data.h"
#include "constants/items.h"
#include "constants/types.h"
#include "battle.h"

// Estado de gears en batalla (memoria efímera)
static EWRAM_DATA struct GearBattleState sGearBattleState = {0};

struct GearBattleState *gGearBattleState = &sGearBattleState;

// Inicializar datos de gear al empezar un combate
void GearBattle_Init(void)
{
    u8 i;
    sGearBattleState.initialized = TRUE;

    for (i = 0; i < GEAR_BATTLE_MAX_MONS; i++)
    {
        sGearBattleState.mons[i].itemId = ITEM_NONE;
        sGearBattleState.mons[i].phase = GEAR_PHASE_READY;
        sGearBattleState.mons[i].turnsRemaining = 0;
        sGearBattleState.mons[i].overloadCount = 0;
        sGearBattleState.mons[i].active = FALSE;
    }
}

// Limpiar datos al terminar el combate
void GearBattle_Cleanup(void)
{
    u8 i;
    sGearBattleState.initialized = FALSE;

    for (i = 0; i < GEAR_BATTLE_MAX_MONS; i++)
    {
        sGearBattleState.mons[i].itemId = ITEM_NONE;
        sGearBattleState.mons[i].phase = GEAR_PHASE_READY;
        sGearBattleState.mons[i].turnsRemaining = 0;
        sGearBattleState.mons[i].overloadCount = 0;
        sGearBattleState.mons[i].active = FALSE;
    }
}

// Obtener los datos de gear de un Pokémon en batalla
struct GearBattleMonData *GearBattle_GetMonData(u8 battlerIdx)
{
    if (battlerIdx >= GEAR_BATTLE_MAX_MONS)
        return NULL;

    return &sGearBattleState.mons[battlerIdx];
}

// Avanzar el ciclo de gear de un Pokémon (llamar al inicio de cada turno)
void GearBattle_AdvanceTurn(u8 battlerIdx)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);
    const struct GearTemplate *tpl;

    if (data == NULL || !data->active || data->itemId == ITEM_NONE)
        return;

    tpl = Gear_GetTemplate(data->itemId);
    if (tpl == NULL)
        return;

    // Ciclo de gear:
    // READY -> (atacar) -> OVERLOAD (1 turno) -> RESIDUAL (3 turnos) -> READY

    switch (data->phase)
    {
    case GEAR_PHASE_READY:
        // Al atacar, pasar a sobrecarga
        data->phase = GEAR_PHASE_OVERLOAD;
        data->turnsRemaining = 1;
        data->overloadCount++;
        break;

    case GEAR_PHASE_OVERLOAD:
        // Sobrecarga terminó, pasar a residual (3 turnos)
        data->phase = GEAR_PHASE_RESIDUAL;
        data->turnsRemaining = 3;
        break;

    case GEAR_PHASE_RESIDUAL:
        data->turnsRemaining--;
        if (data->turnsRemaining == 0)
        {
            // Residual terminó, volver a ready
            data->phase = GEAR_PHASE_READY;
            data->turnsRemaining = 0;
        }
        break;

    case GEAR_PHASE_COOLDOWN:
        // No debería ocurrir en este diseño simplificado
        data->phase = GEAR_PHASE_READY;
        data->turnsRemaining = 0;
        break;
    }
}

// Obtener el bonus de daño (en partes por mil) para un ataque
u16 GearBattle_GetDamageBonus(u8 battlerIdx, u16 moveType, bool8 isPhysical)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);
    const struct GearTemplate *tpl;

    if (data == NULL || !data->active || data->itemId == ITEM_NONE)
        return 0;

    tpl = Gear_GetTemplate(data->itemId);
    if (tpl == NULL)
        return 0;

    // Solo gears con efecto TYPE_BOOST otorgan bonus de daño
    if (tpl->effectType != GEAR_EFFECT_TYPE_BOOST)
        return 0;

    // Verificar que el tipo del movimiento coincida con el tipo del gear
    // GEAR_TYPE_PHYSICAL / GEAR_TYPE_SPECIAL son marcas especiales
    if (tpl->typeBoost == GEAR_TYPE_PHYSICAL)
    {
        if (!isPhysical)
            return 0;
    }
    else if (tpl->typeBoost == GEAR_TYPE_SPECIAL)
    {
        if (isPhysical)
            return 0;
    }
    else if (tpl->typeBoost != moveType)
    {
        return 0;
    }

    // Retornar bonus según la fase actual
    switch (data->phase)
    {
    case GEAR_PHASE_OVERLOAD:
        return tpl->overloadPercent;
    case GEAR_PHASE_RESIDUAL:
        return tpl->residualPercent;
    default:
        return 0;
    }
}

// Obtener el multiplicador de efectividad de tipo
u16 GearBattle_GetEffectivenessMultiplier(u8 battlerIdx, u16 targetType1, u16 targetType2)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);
    const struct GearTemplate *tpl;
    u16 multiplier = 100; // normal

    if (data == NULL || !data->active || data->itemId == ITEM_NONE)
        return 100;

    tpl = Gear_GetTemplate(data->itemId);
    if (tpl == NULL || tpl->effectType != GEAR_EFFECT_EFFECTIVENESS)
        return 100;

    // Los gears de efectividad son siempre activos (no dependen de fase)
    if (tpl->effectivenessType1 != TYPE_NONE && (targetType1 == tpl->effectivenessType1 || targetType2 == tpl->effectivenessType1))
        multiplier = 200;
    if (tpl->effectivenessType2 != TYPE_NONE && (targetType1 == tpl->effectivenessType2 || targetType2 == tpl->effectivenessType2))
        multiplier = 200;
    if (tpl->effectivenessType3 != TYPE_NONE && (targetType1 == tpl->effectivenessType3 || targetType2 == tpl->effectivenessType3))
        multiplier = 200;
    if (tpl->effectivenessType4 != TYPE_NONE && (targetType1 == tpl->effectivenessType4 || targetType2 == tpl->effectivenessType4))
        multiplier = 200;

    return multiplier;
}

// Obtener la fracción de curación por turno (Permafrost)
u8 GearBattle_GetHealFraction(u8 battlerIdx, bool8 isIceType)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);
    const struct GearTemplate *tpl;

    if (data == NULL || !data->active || data->itemId == ITEM_NONE)
        return 0;

    tpl = Gear_GetTemplate(data->itemId);
    if (tpl == NULL || !(tpl->effectType & GEAR_EFFECT_HEAL_PER_TURN))
        return 0;

    // Solo curan si el Pokémon es tipo Hielo
    if (!isIceType)
        return 0;

    return tpl->healFraction;
}

// Verificar si un gear es consumible
bool8 GearBattle_IsConsumable(u8 battlerIdx)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);
    const struct GearTemplate *tpl;

    if (data == NULL || !data->active || data->itemId == ITEM_NONE)
        return FALSE;

    tpl = Gear_GetTemplate(data->itemId);
    if (tpl == NULL)
        return FALSE;

    return tpl->consumable;
}

// Marcar un gear consumible como usado
void GearBattle_MarkConsumed(u8 battlerIdx)
{
    struct GearBattleMonData *data = GearBattle_GetMonData(battlerIdx);

    if (data == NULL || !data->active)
        return;

    data->itemId = ITEM_NONE;
    data->active = FALSE;
    data->phase = GEAR_PHASE_READY;
    data->turnsRemaining = 0;
    data->overloadCount = 0;
}