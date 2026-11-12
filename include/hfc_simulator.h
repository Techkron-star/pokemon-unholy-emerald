#ifndef GUARD_HFC_SIMULATOR_H
#define GUARD_HFC_SIMULATOR_H

#include "constants/pokemon.h"

struct Pokemon;

struct HfcSimulatorValues
{
    u8 ivs[NUM_STATS];
    u8 evs[NUM_STATS];
};

enum HfcSimulatorResult
{
    HFC_SIMULATOR_APPLIED,
    HFC_SIMULATOR_NO_CHANGES,
    HFC_SIMULATOR_INVALID_SPREAD,
    HFC_SIMULATOR_LOCKED,
    HFC_SIMULATOR_INSUFFICIENT_FUNDS,
};

void HfcSimulator_Open(void (*returnToWatch)(void));
void HfcSimulator_ReadValues(struct Pokemon *mon, struct HfcSimulatorValues *values);
enum HfcSimulatorResult HfcSimulator_Apply(struct Pokemon *mon, const struct HfcSimulatorValues *values, u32 *money);

#endif
