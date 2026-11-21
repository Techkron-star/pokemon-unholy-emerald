#ifndef GUARD_UNHOLY_ACTIVITIES_H
#define GUARD_UNHOLY_ACTIVITIES_H

#include "hfc_watch_services.h"

#define UNHOLY_RAID_OFFER_COUNT 20
#define UNHOLY_RAID_COOLDOWN_SECONDS (5 * 60)

enum UnholyBountyState
{
    BOUNTY_NONE,
    BOUNTY_ACTIVE,
    BOUNTY_REWARD_READY,
};

struct UnholyActivitySave
{
    u32 magic;
    u16 version;
    u16 reserved;
    u32 lastClock;
    u32 raidReadyAt[HFC_RAID_DIFFICULTY_COUNT][UNHOLY_RAID_OFFER_COUNT];
    u8 raidRemainder[HFC_RAID_DIFFICULTY_COUNT][HFC_RAID_LOOT_COUNT];
    u8 bountyState;
    u8 bountyRank;
    u8 bountyRoute;
    u8 reserved2;
    u32 bountyMoney;
    u32 bountySeed;
};

struct UnholyActivitySave *UnholyActivities_Get(void);
void UnholyActivities_Commit(void);
u32 UnholyActivities_Now(void);
u32 UnholyActivities_Remaining(u32 deadline, u32 now);
void UnholyActivities_RebaseClock(struct UnholyActivitySave *save, u32 now);

#endif
