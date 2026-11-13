#include "global.h"
#include "hfc_watch.h"

static EWRAM_DATA u8 sHfcWatchState = 0;

static void CB2_InitHfcWatch(void)
{
    sHfcWatchState = 0;
    SetMainCallback2(CB2_InitHfcWatch);
}

void OpenHfcWatchFromStartMenu(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

void CloseHfcWatch(void)
{
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}

void HfcWatch_HandleInput(void)
{
}

void HfcWatch_UpdateOptionStates(void)
{
}

bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option)
{
    return TRUE;
}

void HfcWatch_LaunchOption(enum HfcWatchOption option)
{
}

void HfcWatch_DrawCards(void)
{
}

void HfcWatch_DrawCard(u8 index, u8 x, u8 y)
{
}