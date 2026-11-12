#ifndef GUARD_POKE_SIMULATOR_H
#define GUARD_POKE_SIMULATOR_H

#include "global.h"

#define POKE_SIMULATOR_COST_PER_IV   300
#define POKE_SIMULATOR_COST_PER_EV    20

#define POKE_SIMULATOR_MAX_IV       31
#define POKE_SIMULATOR_MAX_EV_STAT  252
#define POKE_SIMULATOR_MAX_EV_TOTAL 510

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

struct PokeSimulatorStatState
{
    u16 currentIV;
    u16 currentEV;
    u16 targetIV;
    u16 targetEV;
};

struct PokeSimulatorState
{
    u8 partySlot;
    u8 currentStat;
    u16 totalCost;
    u16 totalIVCost;
    u16 totalEVCost;
    bool8 isConfirming;
    struct PokeSimulatorStatState stats[SIM_STAT_COUNT];
};

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