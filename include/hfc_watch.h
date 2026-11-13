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

void CB2_InitHfcWatch(void);
void OpenHfcWatchFromStartMenu(void);
void CloseHfcWatch(void);
void HfcWatch_HandleInput(void);

#endif // GUARD_HFC_WATCH_H