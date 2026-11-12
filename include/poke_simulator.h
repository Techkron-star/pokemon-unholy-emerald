#ifndef GUARD_POKE_SIMULATOR_H
#define GUARD_POKE_SIMULATOR_H

#include "global.h"

// Costos del Poké Simulator
#define POKE_SIMULATOR_COST_PER_IV   300   // 300 pokedólares por punto de IV
#define POKE_SIMULATOR_COST_PER_EV    20   // 20 pokedólares por punto de EV

// Límites
#define POKE_SIMULATOR_MAX_IV       31
#define POKE_SIMULATOR_MAX_EV_STAT  252
#define POKE_SIMULATOR_MAX_EV_TOTAL 510

// Estadísticas del Pokémon
enum PokeSimulatorStat
{
    SIM_STAT_HP,
    SIM_STAT_ATK,
    SIM_STAT_DEF,
    SIM_STAT_SPEED,
    SIM_STAT_SPATK,
    SIM_STAT_SPDEF,
    SIM_STAT_COUNT,
};

// Estado de una estadística en el simulador
struct PokeSimulatorStatState
{
    u16 currentIV;    // IV actual (0-31)
    u16 currentEV;    // EV actual (0-252)
    u16 targetIV;     // IV seleccionado (0-31)
    u16 targetEV;     // EV seleccionado (0-252)
};

// Estado del Poké Simulator
struct PokeSimulatorState
{
    u8 partySlot;         // Índice del Pokémon en el equipo (0-5)
    u8 currentStat;       // Estadística seleccionada (0-5)
    u16 totalCost;        // Costo total acumulado
    u16 totalIVCost;      // Costo total de IVs
    u16 totalEVCost;      // Costo total de EVs
    bool8 isConfirming;   // Si está en pantalla de confirmación
    struct PokeSimulatorStatState stats[SIM_STAT_COUNT];
};

// Prototipos de funciones
void CB2_InitPokeSimulator(void);
void OpenPokeSimulatorFromHfcWatch(void);
void ClosePokeSimulator(void);
void PokeSimulator_HandleInput(void);
void PokeSimulator_UpdateCosts(void);
bool8 PokeSimulator_CanAfford(void);
void PokeSimulator_ApplyChanges(void);
u16 PokeSimulator_CalculateIVCost(u8 currentIV, u8 targetIV);
u16 PokeSimulator_CalculateEVCost(u8 currentEV, u8 targetEV);

#endif // GUARD_POKE_SIMULATOR_H