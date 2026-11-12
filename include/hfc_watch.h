#ifndef GUARD_HFC_WATCH_H
#define GUARD_HFC_WATCH_H

#include "global.h"

#define HFC_WATCH_OPTION_COUNT 9

enum HfcWatchOption
{
    HFC_WATCH_HORIZON,
    HFC_WATCH_POKENAV,
    HFC_WATCH_POKELOCATOR,
    HFC_WATCH_CRAFTING,
    HFC_WATCH_PC,
    HFC_WATCH_ANOMALY_ARCHIVE,
    HFC_WATCH_MUTANT_SIGNAL,
    HFC_WATCH_POKE_SIMULATOR,
    HFC_WATCH_RAID_BATTLE,
    HFC_WATCH_COUNT,
};

enum HfcWatchOptionState
{
    HFC_WATCH_LOCKED,
    HFC_WATCH_UNLOCKED,
    HFC_WATCH_ACTIVE,
};

struct HfcWatchMenuEntry
{
    enum HfcWatchOption option;
    enum HfcWatchOptionState state;
    const u8 *name;
    const u8 *description;
    u8 requiredBadge;
    u8 requiredFlag;
};

struct HfcWatchState
{
    u8 currentSelection;
    u8 isOpen;
    u8 optionsState[HFC_WATCH_COUNT];
};

void CB2_InitHfcWatch(void);
void OpenHfcWatchFromStartMenu(void);
void CloseHfcWatch(void);
void HfcWatch_HandleInput(void);
void HfcWatch_UpdateOptionStates(void);
bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option);
void HfcWatch_LaunchOption(enum HfcWatchOption option);

#endif // GUARD_HFC_WATCH_H