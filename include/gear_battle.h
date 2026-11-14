#ifndef GUARD_GEAR_BATTLE_H
#define GUARD_GEAR_BATTLE_H

#include "global.h"
#include "gear_data.h"

// Máximo de Pokémon en batalla (jugador + oponentes)
#define GEAR_BATTLE_MAX_MONS 6

// Datos de gear en batalla (memoria efímera, se limpia al salir del combate)
struct GearBattleMonData
{
    u16 itemId;           // Ítem del gear equipado (0 = ninguno)
    u8 phase;             // Fase actual del ciclo (GEAR_PHASE_*)
    u8 turnsRemaining;    // Turnos que quedan en la fase actual
    u8 overloadCount;     // Cuántas veces se ha sobrecargado este combate
    bool8 active;         // Si el gear está activo en este combate
};

// Estado global de gears en batalla
struct GearBattleState
{
    struct GearBattleMonData mons[GEAR_BATTLE_MAX_MONS];
    bool8 initialized;
};

extern struct GearBattleState gGearBattleState;

// Inicializar datos de gear al empezar un combate
void GearBattle_Init(void);

// Limpiar datos al terminar el combate
void GearBattle_Cleanup(void);

// Obtener los datos de gear de un Pokémon en batalla (por índice en gParties)
struct GearBattleMonData *GearBattle_GetMonData(u8 battlerIdx);

// Avanzar el ciclo de gear de un Pokémon (llamar al inicio de cada turno)
void GearBattle_AdvanceTurn(u8 battlerIdx);

// Obtener el bonus de daño (en partes por mil) para un ataque de un tipo dado
// Retorna 0 si no hay bonus aplicable
u16 GearBattle_GetDamageBonus(u8 battlerIdx, u16 moveType, bool8 isPhysical);

// Verificar si un gear cambia la efectividad de tipo (Cetro Siniestro / Visor Cósmico)
// Retorna el multiplicador de efectividad (100 = normal, 200 = ×2, etc.) o 0 si no aplica
u16 GearBattle_GetEffectivenessMultiplier(u8 battlerIdx, u16 targetType1, u16 targetType2);

// Verificar si un gear cura PS por turno (Permafrost)
// Retorna la fracción (16 = 1/16) o 0 si no aplica
u8 GearBattle_GetHealFraction(u8 battlerIdx, bool8 isIceType);

// Verificar si un gear es consumible (Teseracto / Ancla)
bool8 GearBattle_IsConsumable(u8 battlerIdx);

// Marcar un gear consumible como usado (se consume al activar)
void GearBattle_MarkConsumed(u8 battlerIdx);

#endif // GUARD_GEAR_BATTLE_H