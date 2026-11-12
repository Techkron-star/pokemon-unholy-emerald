#ifndef GUARD_RAID_BATTLE_H
#define GUARD_RAID_BATTLE_H

#include "global.h"

#define RAID_POKEMON_COUNT 20
#define RAID_RIVAL_COUNT 5
#define RAID_MAX_GEARS 6

enum RaidDifficulty
{
    RAID_DIFFICULTY_EASY,
    RAID_DIFFICULTY_MEDIUM,
    RAID_DIFFICULTY_HARD,
    RAID_DIFFICULTY_EXPERT,
    RAID_DIFFICULTY_COUNT,
};

struct RaidRival
{
    u16 species;
    u8 level;
    u8 moves[4];
    u8 heldItem;
    bool8 hasMegaEvolution;
};

struct RaidBoss
{
    u16 species;
    u8 level;
    u8 moves[4];
    u32 mutationMask;
    u8 auraType;
};

struct RaidRaidData
{
    struct RaidRival rivals[RAID_RIVAL_COUNT];
    struct RaidBoss boss;
    u8 currentRivalIndex;
    u8 difficulty;
    u8 daySeed;
    u8 raidRoundsCleared;
    bool8 isActive;
    u8 rewardType;
    u16 captureRate;
};

struct RaidState
{
    struct RaidRaidData raid;
    u8 lastDayIndex;
    u8 raidsCompleted;
    u8 raidsWon;
    u16 gearsUsed;
    u16 megaEvolutionsUsed;
};

void CB2_InitRaidBattle(void);
void OpenRaidBattleFromHFCWatch(void);
void CloseRaidBattle(void);
void RaidBattle_HandleInput(void);
void RaidBattle_UpdateRotation(void);
bool8 RaidBattle_IsDifficultyUnlocked(u8 difficulty);
void RaidBattle_GenerateRaid(u8 difficulty);
void RaidBattle_ApplyMutationToBoss(void);
u16 RaidBattle_CalculateCaptureRate(void);
bool8 RaidBattle_CanUseMegaEvolution(void);

#endif // GUARD_RAID_BATTLE_H