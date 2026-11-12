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

// --- Combate ---
// Gear del combatiente y fase actual del ciclo (se inicializa solo al entrar al campo).
enum Gear Gear_GetBattlerGear(enum BattlerId battler);
enum GearPhase Gear_GetPhase(enum BattlerId battler);
u8 Gear_GetTurnsRemaining(enum BattlerId battler);

// Multiplicador de potencia del movimiento (1.0 sin Gear o sin efecto). No gasta nada,
// así que la IA y los cálculos repetidos pueden llamarlo libremente.
uq4_12_t Gear_GetPowerModifier(enum BattlerId battler, enum Move move, enum Type moveType);

// Se llama cuando un movimiento empieza a ejecutarse: fija su bonificación y gasta la sobrecarga.
void Gear_OnMoveUsed(enum BattlerId battler, enum Move move, enum Type moveType);

// Se llama una vez por turno (dos movimientos encadenados = un turno): avanza el ciclo.
void Gear_EndOfTurn(enum BattlerId battler);

#endif // GUARD_GEAR_H
