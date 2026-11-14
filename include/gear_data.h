#ifndef GUARD_GEAR_DATA_H
#define GUARD_GEAR_DATA_H

#include "global.h"
#include "constants/items.h"

// Nombres de ítems de gear (definidos en data/text/gear_items.inc)
extern const u8 gText_ItemVisitorCosmico[];
extern const u8 gText_ItemCorazonTitan[];
extern const u8 gText_ItemSkiMask[];
extern const u8 gText_ItemCetroSiniestro[];
extern const u8 gText_ItemImpulsoCosmico[];
extern const u8 gText_ItemPermafrostFragment[];
extern const u8 gText_ItemTesseracto[];
extern const u8 gText_ItemAnclaRealidad[];
extern const u8 gText_ItemColmilloRayquaza[];
extern const u8 gText_ItemSangreNormal[];
extern const u8 gText_ItemSangreLucha[];
extern const u8 gText_ItemSangreVolador[];
extern const u8 gText_ItemSangreVeneno[];
extern const u8 gText_ItemSangreTierra[];
extern const u8 gText_ItemSangreRoca[];
extern const u8 gText_ItemSangreBicho[];
extern const u8 gText_ItemSangreFantasma[];
extern const u8 gText_ItemSangreAcero[];
extern const u8 gText_ItemSangreFuego[];
extern const u8 gText_ItemSangreAgua[];
extern const u8 gText_ItemSangrePlanta[];
extern const u8 gText_ItemSangreElectrico[];
extern const u8 gText_ItemSangrePsiquico[];
extern const u8 gText_ItemSangreHielo[];
extern const u8 gText_ItemSangreDragon[];
extern const u8 gText_ItemSangreSiniestro[];
extern const u8 gText_ItemSangreHada[];

// Número de turnos de ciclo de un Gear: 1 sobrecargado + 3 residuales
#define GEAR_CYCLE_TURNS          4
#define GEAR_OVERLOAD_TURN        0
#define GEAR_RESIDUAL_TURN_1      1
#define GEAR_RESIDUAL_TURN_2      2
#define GEAR_RESIDUAL_TURN_3      3

// Tipo de efecto del Gear
enum GearEffectType
{
    GEAR_EFFECT_NONE,
    GEAR_EFFECT_TYPE_BOOST,        // Bonus de daño por tipo (sobrecarga + residual)
    GEAR_EFFECT_EFFECTIVENESS,     // Cambia efectividad de tipo (siempre activo)
    GEAR_EFFECT_STAT_BOOST,        // Bonus de estadísticas (sobrecarga + residual)
    GEAR_EFFECT_STATUS_CHANCE,     // Modifica probabilidad de estado (Hielo)
    GEAR_EFFECT_HEAL_PER_TURN,     // Cura por turno (tipo Hielo)
    GEAR_EFFECT_CONSUMABLE,        // Consumible único (Teseracto, Ancla)
    GEAR_EFFECT_STAT_BOOST_ON_KO,  // Bonus de estadísticas tras KO (Colmillo de Rayquaza)
};

// Marcas especiales para typeBoost cuando no es un tipo de Pokémon
#define GEAR_TYPE_PHYSICAL 0xFFFE   // Afecta a ataques físicos (Corazón de Titán)
#define GEAR_TYPE_SPECIAL  0xFFFD   // Afecta a ataques especiales (Impulso Cósmico)

// Fase actual del Gear en un Pokémon
enum GearPhase
{
    GEAR_PHASE_READY,     // Puede sobrecargarse
    GEAR_PHASE_OVERLOAD,  // Turno de sobrecarga activa
    GEAR_PHASE_RESIDUAL,  // Turnos de efecto residual
    GEAR_PHASE_COOLDOWN,  // En recarga, sin efecto
};

// Datos de un Gear equipado en un Pokémon (en batalla, memoria efímera)
struct GearBattleData
{
    u16 itemId;           // Ítem del Gear equipado
    u8 phase;             // Fase actual del ciclo
    u8 turnsRemaining;    // Turnos que quedan en la fase actual
    u8 overloadCount;     // Cuántas veces se ha sobrecargado este combate
};

// Configuración estática de cada Gear
struct GearTemplate
{
    u16 itemId;
    const u8 *name;           // Nombre para depuración/UI
    enum GearEffectType effectType;
    u16 typeBoost;            // Tipo que se beneficia (para TYPE_BOOST y STAT_BOOST_ON_KO)
    u16 overloadPercent;      // Bonus durante sobrecarga (en partes por mil)
    u16 residualPercent;      // Bonus durante residual (en partes por mil)
    u8 effectivenessType1;    // Tipo 1 para efectividad ×2 (para EFFECTIVENESS)
    u8 effectivenessType2;    // Tipo 2 para efectividad ×2
    u8 effectivenessType3;    // Tipo 3 para efectividad ×2
    u8 effectivenessType4;    // Tipo 4 para efectividad ×2
    bool8 consumable;         // Si se consume al usar (Teseracto, Ancla)
    bool8 alwaysActive;       // Si el efecto es siempre activo (Cetro Siniestro)
    u8 healFraction;          // 1/16 de cura por turno (Permafrost)
};

// Número total de gears definidos
#define GEAR_TEMPLATE_COUNT 27

extern const struct GearTemplate gGearTemplates[GEAR_TEMPLATE_COUNT];

// Obtener la plantilla de un Gear por su ID de ítem
const struct GearTemplate *Gear_GetTemplate(u16 itemId);

// Verificar si un ítem es un Gear
bool8 Gear_IsGearItem(u16 itemId);

// Verificar si un ítem es un Gear Sangre
bool8 Gear_IsBloodGear(u16 itemId);

// Obtener el tipo de Pokémon asociado a un Gear Sangre (para el primer ataque)
u16 Gear_GetBloodGearType(u16 itemId);

#endif // GUARD_GEAR_DATA_H