#include "global.h"
#include "poke_simulator.h"
#include "constants/items.h"
#include "constants/money.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include "text.h"
#include "window.h"

static EWRAM_DATA struct PokeSimulatorState sPokeSimulator = {0};

static void CB2_InitPokeSimulator(void)
{
    sPokeSimulator.partySlot = 0;
    sPokeSimulator.currentStat = SIM_STAT_HP;
    sPokeSimulator.totalCost = 0;
    sPokeSimulator.totalIVCost = 0;
    sPokeSimulator.totalEVCost = 0;
    sPokeSimulator.isConfirming = FALSE;
    SetMainCallback2(CB2_InitPokeSimulator);
}

void OpenPokeSimulatorFromHfcWatch(void)
{
    SetMainCallback2(CB2_InitPokeSimulator);
}

void ClosePokeSimulator(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

void PokeSimulator_HandleInput(void)
{
    if (JOY_NEW(DPAD_UP))
    {
        if (sPokeSimulator.currentStat > 0)
            sPokeSimulator.currentStat--;
        else
            sPokeSimulator.currentStat = SIM_STAT_COUNT - 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sPokeSimulator.currentStat < SIM_STAT_COUNT - 1)
            sPokeSimulator.currentStat++;
        else
            sPokeSimulator.currentStat = 0;
        PlaySE(SE_SELECT);
    }

    if (JOY_NEW(DPAD_LEFT))
    {
        if (sPokeSimulator.stats[sPokeSimulator.currentStat].targetIV > 0)
        {
            sPokeSimulator.stats[sPokeSimulator.currentStat].targetIV--;
            PokeSimulator_UpdateCosts();
            PlaySE(SE_SELECT);
        }
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (sPokeSimulator.stats[sPokeSimulator.currentStat].targetIV < POKE_SIMULATOR_MAX_IV)
        {
            sPokeSimulator.stats[sPokeSimulator.currentStat].targetIV++;
            PokeSimulator_UpdateCosts();
            PlaySE(SE_SELECT);
        }
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (PokeSimulator_CanAfford())
        {
            sPokeSimulator.isConfirming = TRUE;
        }
        else
        {
            PlaySE(SE_FAILURE);
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        if (sPokeSimulator.isConfirming)
        {
            sPokeSimulator.isConfirming = FALSE;
        }
        else
        {
            ClosePokeSimulator();
        }
        PlaySE(SE_SELECT);
    }
}

void PokeSimulator_UpdateCosts(void)
{
    u8 i;
    sPokeSimulator.totalIVCost = 0;
    sPokeSimulator.totalEVCost = 0;

    for (i = 0; i < SIM_STAT_COUNT; i++)
    {
        sPokeSimulator.totalIVCost += PokeSimulator_CalculateIVCost(
            sPokeSimulator.stats[i].currentIV,
            sPokeSimulator.stats[i].targetIV
        );
        sPokeSimulator.totalEVCost += PokeSimulator_CalculateEVCost(
            sPokeSimulator.stats[i].currentEV,
            sPokeSimulator.stats[i].targetEV
        );
    }

    sPokeSimulator.totalCost = sPokeSimulator.totalIVCost + sPokeSimulator.totalEVCost;
}

bool8 PokeSimulator_CanAfford(void)
{
    return TRUE;
}

void PokeSimulator_ApplyChanges(void)
{
    u8 i;
    struct Pokemon *mon = &gParties[0][sPokeSimulator.partySlot];

    for (i = 0; i < SIM_STAT_COUNT; i++)
    {
        // TODO: Aplicar IV y EV al Pokémon
    }

    sPokeSimulator.isConfirming = FALSE;
}

u16 PokeSimulator_CalculateIVCost(u8 currentIV, u8 targetIV)
{
    u16 diff = (targetIV > currentIV) ? (targetIV - currentIV) : (currentIV - targetIV);
    return diff * POKE_SIMULATOR_COST_PER_IV;
}

u16 PokeSimulator_CalculateEVCost(u8 currentEV, u8 targetEV)
{
    u16 diff = (targetEV > currentEV) ? (targetEV - currentEV) : (currentEV - targetEV);
    return diff * POKE_SIMULATOR_COST_PER_EV;
}