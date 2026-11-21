#include "global.h"
#include "unholy_activities.h"
#include "unholy_bounty.h"
#include "unholy_save.h"
#include "event_data.h"
#include "hfc_watch.h"
#include "item.h"
#include "money.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"

TEST("Unholy activities: badges increase raid variety without changing difficulty")
{
    static const u8 levels[] = {34, 40, 47, 54};
    static const u8 counts[] = {0, 0, 0, 0, 0, 12, 14, 16, 18};
    for (u8 badges = 0; badges <= 8; badges++)
    {
        EXPECT_EQ(HfcWatch_GetRaidOfferCount(badges, FALSE), counts[badges]);
        for (enum HfcRaidDifficulty tier = 0; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
        {
            EXPECT_EQ(HfcWatch_IsRaidDifficultyUnlocked(tier, badges), badges >= 5);
            EXPECT_EQ(HfcWatch_GetRaidDifficultyInfo(tier)->level, levels[tier]);
        }
    }
    EXPECT_EQ(HfcWatch_GetRaidOfferCount(8, TRUE), 20);
    EXPECT_EQ(HfcWatch_GetRaidOfferCount(255, FALSE), 18);
}

TEST("Unholy activities: resource bonuses match the fifth badge eighth badge and League milestones")
{
    EXPECT_EQ(HfcWatch_GetRaidResourceBonus(4, FALSE), 0);
    EXPECT_EQ(HfcWatch_GetRaidResourceBonus(5, FALSE), 20);
    EXPECT_EQ(HfcWatch_GetRaidResourceBonus(7, FALSE), 20);
    EXPECT_EQ(HfcWatch_GetRaidResourceBonus(8, FALSE), 28);
    EXPECT_EQ(HfcWatch_GetRaidResourceBonus(8, TRUE), 35);
}

TEST("Unholy activities: small raid rewards accumulate the exact percentage over repeated claims")
{
    static const u8 bonuses[] = {20, 28, 35};
    for (u8 b = 0; b < ARRAY_COUNT(bonuses); b++)
    {
        for (u8 base = 1; base <= 5; base++)
        {
            u8 remainder = 0;
            u32 total = 0;
            for (u8 i = 0; i < 100; i++)
                total += HfcWatch_ScaleRaidResource(base, bonuses[b], &remainder);
            EXPECT_EQ(total, base * (100 + bonuses[b]));
            EXPECT_EQ(remainder, 0);
        }
    }
}

TEST("Unholy activities: cooldowns expire independently and a clock rollback preserves remaining time")
{
    struct UnholyActivitySave save = {0};
    save.lastClock = 1000;
    save.raidReadyAt[HFC_RAID_STANDARD][0] = 1200;
    save.raidReadyAt[HFC_RAID_STANDARD][1] = 900;
    save.raidReadyAt[HFC_RAID_MASTER][0] = 1250;
    EXPECT_EQ(UnholyActivities_Remaining(1200, 1199), 1);
    EXPECT_EQ(UnholyActivities_Remaining(1200, 1200), 0);
    UnholyActivities_RebaseClock(&save, 100);
    EXPECT_EQ(UnholyActivities_Remaining(save.raidReadyAt[HFC_RAID_STANDARD][0], 100), 200);
    EXPECT_EQ(UnholyActivities_Remaining(save.raidReadyAt[HFC_RAID_STANDARD][1], 100), 0);
    EXPECT_EQ(UnholyActivities_Remaining(save.raidReadyAt[HFC_RAID_MASTER][0], 100), 250);
    EXPECT_EQ(save.lastClock, 100);
    EXPECT_EQ(UnholyActivities_Remaining(99999999, 100), UNHOLY_RAID_COOLDOWN_SECONDS);
}

TEST("Unholy activities: activity records persist inside the existing save CRC envelope")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    struct UnholyActivitySave *save;
    UnholySave4_Reset();
    save = UnholyActivities_Get();
    EXPECT(save != NULL);
    save->raidReadyAt[3][19] = 12345;
    save->bountyState = BOUNTY_ACTIVE;
    save->bountyMoney = 54321;
    UnholyActivities_Commit();
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    save = UnholyActivities_Get();
    EXPECT_EQ(save->raidReadyAt[3][19], 12345);
    EXPECT_EQ(save->bountyMoney, 54321);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: unknown activity versions are not overwritten")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    struct UnholyActivitySave *save;
    UnholySave4_Reset();
    save = UnholyActivities_Get();
    save->version = 255;
    UnholyActivities_Commit();
    EXPECT(UnholyActivities_Get() == NULL);
    EXPECT_EQ(save->version, 255);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: all bounty ranks have profitable bounded rewards and badge gates")
{
    static const u32 fees[] = {500, 2000, 6000, 15000, 40000};
    static const u32 minimums[] = {1200, 4000, 12000, 30000, 80000};
    static const u32 maximums[] = {1800, 6000, 18000, 45000, 120000};
    for (u8 rank = 0; rank < UNHOLY_BOUNTY_RANK_COUNT; rank++)
    {
        EXPECT_EQ(UnholyBounty_GetFee(rank), fees[rank]);
        EXPECT_EQ(UnholyBounty_GetRequiredBadges(rank), 2 * rank);
        for (u32 seed = 0; seed < 100000; seed += 7919)
        {
            u32 money = UnholyBounty_GetReward(rank, seed);
            EXPECT(money >= minimums[rank] && money <= maximums[rank]);
            EXPECT(money > fees[rank]);
        }
    }
    EXPECT_EQ(UnholyBounty_GetFee(255), 0);
    EXPECT_EQ(UnholyBounty_GetRequiredBadges(255), 255);
}

TEST("Unholy activities: a contract charges once survives defeat pays once and removes its marker")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u32 won;
    u16 potions = CountTotalItemQuantityInBag(ITEM_POTION);
    u16 shards = CountTotalItemQuantityInBag(ITEM_RED_SHARD);
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 5000);
    EXPECT_EQ(UnholyBounty_Purchase(0, 123), BOUNTY_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4500);
    EXPECT(UnholyBounty_GetMapSection() != MAPSEC_NONE);
    EXPECT_EQ(UnholyBounty_Purchase(0, 999), BOUNTY_BUSY);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4500);
    UnholyBounty_OnBattleEnd(B_OUTCOME_LOST);
    EXPECT_EQ(UnholyActivities_Get()->bountyState, BOUNTY_ACTIVE);
    EXPECT(UnholyBounty_GetMapSection() != MAPSEC_NONE);
    EXPECT_EQ(UnholyBounty_Claim(), BOUNTY_UNAVAILABLE);
    UnholyBounty_OnBattleEnd(B_OUTCOME_WON);
    EXPECT_EQ(UnholyActivities_Get()->bountyState, BOUNTY_REWARD_READY);
    EXPECT_EQ(UnholyBounty_GetMapSection(), MAPSEC_NONE);
    won = UnholyBounty_GetReward(0, 123);
    EXPECT_EQ(UnholyBounty_Claim(), BOUNTY_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4500 + won);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), potions + 2);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_RED_SHARD), shards + 1);
    EXPECT_EQ(UnholyBounty_Claim(), BOUNTY_UNAVAILABLE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4500 + won);
    RemoveBagItem(ITEM_POTION, 2);
    RemoveBagItem(ITEM_RED_SHARD, 1);
    SetMoney(&gSaveBlock1Ptr->money, money);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: an unaffordable contract does not create a target or take money")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 499);
    EXPECT_EQ(UnholyBounty_Purchase(0, 123), BOUNTY_NO_MONEY);
    EXPECT_EQ(UnholyActivities_Get()->bountyState, BOUNTY_NONE);
    EXPECT_EQ(UnholyBounty_GetMapSection(), MAPSEC_NONE);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 499);
    SetMoney(&gSaveBlock1Ptr->money, money);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: a full resource pocket rolls back earlier loot and preserves the contract reward")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    struct BagPocket *pocket = &gBagPockets[GetItemPocket(ITEM_RED_SHARD)];
    u16 capacity = pocket->capacity;
    u16 potions = CountTotalItemQuantityInBag(ITEM_POTION);
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 5000);
    EXPECT_EQ(UnholyBounty_Purchase(0, 123), BOUNTY_OK);
    UnholyBounty_OnBattleEnd(B_OUTCOME_WON);
    pocket->capacity = 0;
    EXPECT_EQ(UnholyBounty_Claim(), BOUNTY_BAG_FULL);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), potions);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4500);
    EXPECT_EQ(UnholyActivities_Get()->bountyState, BOUNTY_REWARD_READY);
    pocket->capacity = capacity;
    EXPECT_EQ(UnholyBounty_Claim(), BOUNTY_OK);
    RemoveBagItem(ITEM_POTION, 2);
    RemoveBagItem(ITEM_RED_SHARD, 1);
    SetMoney(&gSaveBlock1Ptr->money, money);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: cooldown reads isolate each boss and difficulty in persistent storage")
{
    struct SaveBlock4 backup = gSaveBlock3Ptr->unholySave4;
    struct UnholyActivitySave *save;
    u32 now;
    u32 remaining;
    UnholySave4_Reset();
    save = UnholyActivities_Get();
    now = UnholyActivities_Now();
    save->lastClock = now;
    save->raidReadyAt[HFC_RAID_STANDARD][0] = now + 45;
    UnholyActivities_Commit();
    remaining = HfcWatch_GetRaidCooldown(HFC_RAID_STANDARD, 0);
    EXPECT(remaining > 0 && remaining <= 45);
    EXPECT_EQ(HfcWatch_GetRaidCooldown(HFC_RAID_STANDARD, 1), 0);
    EXPECT_EQ(HfcWatch_GetRaidCooldown(HFC_RAID_MASTER, 0), 0);
    EXPECT_EQ(HfcWatch_GetRaidCooldown(HFC_RAID_DIFFICULTY_COUNT, 0), UNHOLY_RAID_COOLDOWN_SECONDS);
    gSaveBlock3Ptr->unholySave4 = backup;
}

TEST("Unholy activities: raid offers are distinct stable and expand after badges and League")
{
    u8 oldBadges = 0;
    bool32 oldChampion = FlagGet(FLAG_IS_CHAMPION);
    enum Species bosses[UNHOLY_RAID_OFFER_COUNT];
    for (u8 i = 0; i < 8; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i)) oldBadges |= 1 << i;
        if (i < 5) FlagSet(FLAG_BADGE01_GET + i);
        else FlagClear(FLAG_BADGE01_GET + i);
    }
    FlagClear(FLAG_IS_CHAMPION);
    for (enum HfcRaidDifficulty tier = 0; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
    {
        for (u8 i = 0; i < 12; i++)
        {
            struct HfcRaidOpponent boss = HfcWatch_GetRaidOfferOpponent(tier, i, HFC_RAID_BOSS, 0);
            bosses[i] = boss.species;
            EXPECT(boss.species != SPECIES_NONE);
            EXPECT_EQ(boss.level, HfcWatch_GetRaidDifficultyInfo(tier)->level + 6);
            EXPECT_EQ(boss.species, HfcWatch_GetRaidOfferOpponent(tier, i, HFC_RAID_BOSS, 0).species);
            for (u8 j = 0; j < i; j++) EXPECT(bosses[i] != bosses[j]);
        }
        EXPECT_EQ(HfcWatch_GetRaidOfferOpponent(tier, 12, HFC_RAID_BOSS, 0).species, SPECIES_NONE);
    }
    for (u8 i = 0; i < 8; i++) FlagSet(FLAG_BADGE01_GET + i);
    EXPECT(HfcWatch_GetRaidOfferOpponent(HFC_RAID_STANDARD, 17, HFC_RAID_BOSS, 0).species != SPECIES_NONE);
    EXPECT_EQ(HfcWatch_GetRaidOfferOpponent(HFC_RAID_STANDARD, 18, HFC_RAID_BOSS, 0).species, SPECIES_NONE);
    FlagSet(FLAG_IS_CHAMPION);
    EXPECT(HfcWatch_GetRaidOfferOpponent(HFC_RAID_STANDARD, 19, HFC_RAID_BOSS, 0).species != SPECIES_NONE);
    for (u8 i = 0; i < 8; i++)
    {
        if (oldBadges & (1 << i)) FlagSet(FLAG_BADGE01_GET + i);
        else FlagClear(FLAG_BADGE01_GET + i);
    }
    if (!oldChampion) FlagClear(FLAG_IS_CHAMPION);
}
