#ifndef GUARD_GEAR_H
#define GUARD_GEAR_H

#include "constants/gears.h"
#include "constants/items.h"
#include "config/gears.h"
#include "fpmath.h"

// Cómo se activa la sobrecarga de un Gear.
enum GearTrigger
{
    // Queda "armada" y se gasta en el primer movimiento de daño compatible.
    GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE,
    // Se activa sola y dura overloadTurns turnos, use o no un movimiento.
    GEAR_TRIGGER_TIMED,
    GEAR_TRIGGER_PASSIVE,
    // Consumed by a specific event (hazard, status, accuracy, Stamina...)
    // rather than by the first damaging move of any type.
    GEAR_TRIGGER_EVENT,
};

// Fase del ciclo de un Gear durante el combate.
enum GearPhase
{
    GEAR_PHASE_ARMED,       // lista; la sobrecarga espera al primer movimiento compatible
    GEAR_PHASE_OVERLOAD,    // sobrecarga activa (solo GEAR_TRIGGER_TIMED)
    GEAR_PHASE_RECHARGING,  // recargando; aplica la bonificación residual
};

// Máscaras de categoría para GearInfo.categoryMask (0 = cualquiera de daño).
#define GEAR_CATEGORY_PHYSICAL  (1 << DAMAGE_CATEGORY_PHYSICAL)
#define GEAR_CATEGORY_SPECIAL   (1 << DAMAGE_CATEGORY_SPECIAL)

// Máscara de tipo para GearInfo.typeMask: GEAR_TYPE(TYPE_DARK) | GEAR_TYPE(TYPE_GHOST)...
#define GEAR_TYPE(type)         (1u << (type))

struct GearInfo
{
    const u8 *name;
    const u8 *description;
    const u32 *iconGfx;         // PNG 16x32 -> .4bpp: arriba "lista/sobrecarga", abajo "recargando"
    const u16 *iconPal;         // paleta de 16 colores (.gbapal)
    u8 trigger;                 // enum GearTrigger
    u8 overloadTurns;           // solo TIMED: turnos de sobrecarga (mínimo 1)
    u8 cooldownTurns;           // final de turnos que dura la recarga (mínimo 1)
    u32 typeMask;               // tipos de movimiento compatibles (0 = cualquiera)
    u8 categoryMask;            // categorías compatibles (0 = física y especial)
    u16 overloadPowerPercent;   // multiplicador de potencia en sobrecarga (150 = x1.5; 0 = sin efecto)
    u16 residualPowerPercent;   // multiplicador de potencia mientras recarga (0 = sin efecto)
};

extern const struct GearInfo gGearsInfo[GEAR_COUNT];

// Textos visibles en mochila, resumen, crafting y equipo. Usar estos getters
// en vez de acceder directamente a gGearsInfo[gear].name/description.
const u8 *Gear_GetLocalizedName(enum Gear gear);
const u8 *Gear_GetLocalizedDescription(enum Gear gear);

// --- Guardado: el Gear vive en el Pokémon (slot aparte del objeto equipado) ---
struct Pokemon;
enum Gear Gear_GetEquipped(struct Pokemon *mon);
void Gear_SetEquipped(struct Pokemon *mon, enum Gear gear); // GEAR_NONE = quitar

// Crafted Gear items belong to a separate Pokémon slot, never MON_DATA_HELD_ITEM.
enum GearEquipResult
{
    GEAR_EQUIP_OK,
    GEAR_EQUIP_ALREADY,
    GEAR_EQUIP_NOT_GEAR,
    GEAR_EQUIP_NOT_OWNED,
    GEAR_EQUIP_BAG_FULL,
};
enum Gear Gear_FromItem(enum Item item);
enum Item Gear_ToItem(enum Gear gear);
enum GearEquipResult Gear_TryEquipFromBag(struct Pokemon *mon, enum Item item);

// Gear stat stages obey the same inversion/doubling as battle stat changes.
// Use ApplySelf only for changes originating from the wearer's own Gear;
// effects inflicted on an opponent still use the native TryStatChange pipeline.
s8 Gear_ModifyStatStageByAbility(enum Ability ability, s8 stages);
bool32 Gear_ApplySelfStatChange(enum BattlerId battler, enum Stat stat, s8 stages);

// --- Combate ---
// Gear del combatiente y fase actual del ciclo (se inicializa solo al entrar al campo).
enum Gear Gear_GetBattlerGear(enum BattlerId battler);
enum GearPhase Gear_GetPhase(enum BattlerId battler);
u8 Gear_GetTurnsRemaining(enum BattlerId battler);
bool32 Gear_ActivateEvent(enum BattlerId battler);

struct AdditionalEffect;
bool32 Gear_StormStaffRepeatsMove(enum BattlerId battler, enum Move move);
bool32 Gear_SuppressesAdditionalEffect(enum BattlerId battler, enum Move move, const struct AdditionalEffect *effect);
// Extra hits are resolved as normal strikes but never generate further echoes.
u8 Gear_TryStartExtraStrikes(enum BattlerId battler, enum Move move, enum BattlerId target, bool32 singleTarget);
u32 Gear_AdjustResolvedDamage(enum BattlerId attacker, enum BattlerId defender, enum Move move, u32 damage, bool32 spread);
enum BattlerId Gear_GetReboundTarget(enum BattlerId attacker, enum BattlerId originalTarget);
void Gear_OnExtraStrike(enum BattlerId attacker, enum BattlerId defender, enum Move move);
void Gear_OnFireMoveHit(enum BattlerId attacker, enum BattlerId defender, enum Move move);
bool32 Gear_WeatherBonusesSuppressed(enum BattlerId battler);
void Gear_OnScreenSet(enum BattlerId battler, u16 *duration);
void Gear_OnProtectExtended(enum BattlerId battler);
// Returns a recipient battler when a move genuinely imposed control; no proc on misses.
enum BattlerId Gear_TryWarBannerSupport(enum BattlerId attacker, enum BattlerId defender);

// Multiplicador de potencia del movimiento (1.0 sin Gear o sin efecto). No gasta nada,
// así que la IA y los cálculos repetidos pueden llamarlo libremente.
uq4_12_t Gear_GetPowerModifier(enum BattlerId battler, enum Move move, enum Type moveType);

// Se llama cuando un movimiento empieza a ejecutarse: fija su bonificación y gasta la sobrecarga.
void Gear_OnMoveUsed(enum BattlerId battler, enum Move move, enum Type moveType);
// HP/reactive Gears are resolved after real damage; preview must not consume them.
void Gear_OnDamaged(enum BattlerId battler, u32 damage);
// Only actual, direct, single-turn/single-strike lethal move damage can activate it.
bool32 Gear_TryMeteorArmorSurvive(enum BattlerId defender, enum BattlerId attacker,
                                 enum Move move, s32 damage, u16 *remainingHp);
// Consumes the pending impact after the HP update and applies its ash backlash.
bool32 Gear_ResolveMeteorArmorAsh(enum BattlerId defender, enum BattlerId attacker);
// One successful block per complete turn, one roll per hostile action (not per hit).
// Chance is in tenths of a percent; 1000 means guaranteed after eight dry turns.
u16 Gear_GetPaladinChanceTenths(enum BattlerId battler);
bool32 Gear_TryPaladinBlock(enum BattlerId defender);
u32 Gear_ApplyPaladinChainPenalty(enum BattlerId attacker, enum BattlerId defender, u32 damage);
void Gear_OnProtectSuccess(enum BattlerId battler);
// Returns the end-turn heal amount (no direct HP mutation; caller owns battle script).
u32 Gear_GetEndTurnHeal(enum BattlerId battler);

// Se llama una vez por turno (dos movimientos encadenados = un turno): avanza el ciclo.
void Gear_EndOfTurn(enum BattlerId battler);

#endif // GUARD_GEAR_H
