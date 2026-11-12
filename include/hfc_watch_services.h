#ifndef GUARD_HFC_WATCH_SERVICES_H
#define GUARD_HFC_WATCH_SERVICES_H

#include "pokemon.h"

#define HFC_FREQUENCY_ENCOUNTERS 10

#define HFC_RAID_LOOT_COUNT 3
#define HFC_RAID_CHANCE_SCALE 10000

enum HfcRaidDifficulty
{
    HFC_RAID_STANDARD,
    HFC_RAID_ADVANCED,
    HFC_RAID_EXPERT,
    HFC_RAID_MASTER,
    HFC_RAID_DIFFICULTY_COUNT,
};

enum HfcRaidStage
{
    HFC_RAID_FIRST_DOUBLE,
    HFC_RAID_SECOND_DOUBLE,
    HFC_RAID_BOSS,
};

enum HfcRaidState
{
    HFC_RAID_IDLE,
    HFC_RAID_FIGHTING,
    HFC_RAID_NEXT_BATTLE,
    HFC_RAID_CHOOSE_REWARD,
    HFC_RAID_ENDED,
    HFC_RAID_LOOT_RECEIVED,
    HFC_RAID_CAPTURED,
    HFC_RAID_CAPTURE_FAILED,
};

enum HfcWatchRaidStatus
{
    HFC_RAID_READY,
    HFC_RAID_NOT_ON_ROUTE,
    HFC_RAID_USED_TODAY,
    HFC_RAID_NEEDS_PARTY,
    HFC_RAID_UNAVAILABLE,
};

enum HfcRaidRewardError
{
    HFC_RAID_REWARD_OK,
    HFC_RAID_REWARD_BAG_FULL,
    HFC_RAID_REWARD_STORAGE_FULL,
};

struct HfcRaidLoot
{
    enum Item item;
    u8 quantity;
};

struct HfcRaidDifficultyInfo
{
    const u8 *name;
    u8 minBadges;
    u8 level;
    u8 bossBoost;
    u16 minCaptureChance;
    u16 maxCaptureChance;
    struct HfcRaidLoot loot[HFC_RAID_LOOT_COUNT];
};

struct HfcRaidOpponent
{
    enum Species species;
    u8 level;
    const enum Move *moves;
};

const struct HfcRaidDifficultyInfo *HfcWatch_GetRaidDifficultyInfo(enum HfcRaidDifficulty difficulty);
bool32 HfcWatch_IsRaidDifficultyUnlocked(enum HfcRaidDifficulty difficulty, u8 badges);
struct HfcRaidOpponent HfcWatch_GetRaidOpponent(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty,
                                               enum HfcRaidStage stage, u8 slot);
u16 HfcWatch_GetRaidMutation(enum HfcRaidDifficulty difficulty, u32 roll);
u16 HfcWatch_GetRaidCaptureChance(u16 day, u32 trainerId, enum HfcRaidDifficulty difficulty);
enum HfcWatchRaidStatus HfcWatch_GetRaidStatus(void);
enum HfcRaidState HfcWatch_GetRaidState(void);
enum HfcRaidStage HfcWatch_GetRaidStage(void);
enum HfcRaidDifficulty HfcWatch_GetActiveRaidDifficulty(void);
u16 HfcWatch_GetActiveRaidCaptureChance(void);
bool32 HfcWatch_StartDailyRaid(enum HfcRaidDifficulty difficulty);
bool32 HfcWatch_ContinueRaid(void);
void HfcWatch_OnRaidBattleEnd(u8 outcome);
void HfcWatch_ResetRaid(void);
enum HfcRaidRewardError HfcWatch_ClaimRaidReward(bool32 capture);

u8 HfcWatch_GetFrequencyBoost(u8 badges);
bool32 HfcWatch_CanActivateFrequency(void);
bool32 HfcWatch_ActivateFrequency(void);
u8 HfcWatch_GetFrequencyRemaining(void);
u8 HfcWatch_ConsumeFrequencyEncounter(void);

void HfcWatch_RegisterMutation(struct BoxPokemon *mon);
u16 HfcWatch_ReadArchive(void);

#endif
