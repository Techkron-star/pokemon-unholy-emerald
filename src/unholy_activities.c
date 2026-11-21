#include "global.h"
#include "unholy_activities.h"
#include "unholy_save.h"
#include "rtc.h"

#define ACTIVITY_MAGIC 0x55414331
#define ACTIVITY_VERSION 1
STATIC_ASSERT(sizeof(struct UnholyActivitySave) <= UNHOLY_SAVE4_PAYLOAD_BYTES, ActivitiesFitSave4);

struct UnholyActivitySave *UnholyActivities_Get(void)
{
    struct SaveBlock4 *block = UnholySave4_Get();
    struct UnholyActivitySave *save;
    if (block == NULL)
        return NULL;
    save = (struct UnholyActivitySave *)block->payload;
    if (save->magic == ACTIVITY_MAGIC && save->version == ACTIVITY_VERSION)
        return save;
    // Reserve the first record only on a blank extension; never erase unknown data.
    for (u32 i = 0; i < sizeof(*save); i++)
        if (block->payload[i] != 0)
            return NULL;
    memset(save, 0, sizeof(*save));
    save->magic = ACTIVITY_MAGIC;
    save->version = ACTIVITY_VERSION;
    UnholyActivities_Commit();
    return save;
}

void UnholyActivities_Commit(void)
{
    UnholySave4_Commit();
}

u32 UnholyActivities_Now(void)
{
    RtcCalcLocalTime();
    return max(0, gLocalTime.days) * 86400u + gLocalTime.hours * 3600u
         + gLocalTime.minutes * 60u + gLocalTime.seconds;
}

u32 UnholyActivities_Remaining(u32 deadline, u32 now)
{
    return deadline > now ? min(deadline - now, UNHOLY_RAID_COOLDOWN_SECONDS) : 0;
}

void UnholyActivities_RebaseClock(struct UnholyActivitySave *save, u32 now)
{
    if (now < save->lastClock)
    {
        for (u8 tier = 0; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
            for (u8 offer = 0; offer < UNHOLY_RAID_OFFER_COUNT; offer++)
                save->raidReadyAt[tier][offer] = now
                    + UnholyActivities_Remaining(save->raidReadyAt[tier][offer], save->lastClock);
    }
    save->lastClock = now;
}
