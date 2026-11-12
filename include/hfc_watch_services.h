#ifndef GUARD_HFC_WATCH_SERVICES_H
#define GUARD_HFC_WATCH_SERVICES_H

#include "pokemon.h"

#define HFC_FREQUENCY_ENCOUNTERS 10

struct HfcWatchRaid
{
    enum Species species;
    u8 level;
    u16 mutation;
    const enum Move *moves;
};

enum HfcWatchRaidStatus
{
    HFC_RAID_READY,
    HFC_RAID_NOT_ON_ROUTE,
    HFC_RAID_USED_TODAY,
    HFC_RAID_NEEDS_PARTY,
    HFC_RAID_UNAVAILABLE,
};

struct HfcWatchRaid HfcWatch_GetDailyRaid(u16 day, u32 trainerId, u8 badges);
enum HfcWatchRaidStatus HfcWatch_GetRaidStatus(void);
bool32 HfcWatch_StartDailyRaid(void);

u8 HfcWatch_GetFrequencyBoost(u8 badges);
bool32 HfcWatch_CanActivateFrequency(void);
bool32 HfcWatch_ActivateFrequency(void);
u8 HfcWatch_GetFrequencyRemaining(void);
u8 HfcWatch_ConsumeFrequencyEncounter(void);

void HfcWatch_RegisterMutation(struct BoxPokemon *mon);
u16 HfcWatch_ReadArchive(void);

#endif
