#include "global.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "item.h"
#include "test/test.h"
#include "constants/hold_effects.h"

// Tests for the HFC Watch tier logic. The hub window needs mGBA (see DISENO.md).

TEST("HFC Watch tier is the best watch owned")
{
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, FALSE, FALSE), HFC_WATCH_TIER_NONE);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, FALSE, FALSE), HFC_WATCH_TIER_POKE);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, TRUE, FALSE), HFC_WATCH_TIER_SUPER);
    EXPECT_EQ(HfcWatch_TierFromItems(TRUE, TRUE, TRUE), HFC_WATCH_TIER_HF);
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, FALSE, TRUE), HFC_WATCH_TIER_HF);
    EXPECT_EQ(HfcWatch_TierFromItems(FALSE, TRUE, FALSE), HFC_WATCH_TIER_SUPER);
}

TEST("HFC Watch lower tiers are the HF feature set with entries masked out")
{
    u16 poke = HfcWatch_GetTierMask(HFC_WATCH_TIER_POKE);
    u16 super = HfcWatch_GetTierMask(HFC_WATCH_TIER_SUPER);
    u16 hf = HfcWatch_GetTierMask(HFC_WATCH_TIER_HF);

    EXPECT_EQ(HfcWatch_GetTierMask(HFC_WATCH_TIER_NONE), 0);
    EXPECT_EQ(poke & hf, poke);
    EXPECT_EQ(super & hf, super);
    EXPECT_EQ(poke & super, poke);
    EXPECT_EQ(hf, (1 << HFC_WATCH_FEATURE_COUNT) - 1);
}

TEST("HFC Watch tiers add the features the design assigns")
{
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_HORIZON_SHOP));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_POKENAV));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_POKELOCATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_POKELOCATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_MUTATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_SIMULATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR));
    EXPECT(HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_NONE, HFC_WATCH_FEATURE_HORIZON_SHOP));
    EXPECT(!HfcWatch_TierHasFeature(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_COUNT));
}

TEST("HFC Watch opens the basic PokeNav at zero badges and lists working tools")
{
    u8 list[HFC_WATCH_FEATURE_COUNT];
    struct HfcWatchProgress start = {0};

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_NONE, start, list), 0);

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_POKE, start, list), 2);
    EXPECT_EQ(list[0], HFC_WATCH_FEATURE_HORIZON_SHOP);
    EXPECT_EQ(list[1], HFC_WATCH_FEATURE_POKENAV);

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_SUPER, start, list), 3);
    EXPECT_EQ(list[2], HFC_WATCH_FEATURE_POKELOCATOR);

    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_HF, start, list), 4);
    EXPECT_EQ(list[3], HFC_WATCH_FEATURE_ARCHIVE);
}

TEST("HFC Watch paid services require the HF tier and their own badge thresholds")
{
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_SIMULATOR, (struct HfcWatchProgress){.badges = 8}), HFC_WATCH_WRONG_TIER);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_MUTATOR, (struct HfcWatchProgress){.badges = 8}), HFC_WATCH_WRONG_TIER);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR, (struct HfcWatchProgress){.badges = 3}), HFC_WATCH_NEEDS_BADGES);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_SIMULATOR, (struct HfcWatchProgress){.badges = 4}), HFC_WATCH_READY);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR, (struct HfcWatchProgress){.badges = 7}), HFC_WATCH_NEEDS_BADGES);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_MUTATOR, (struct HfcWatchProgress){.badges = 8}), HFC_WATCH_READY);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_COUNT, (struct HfcWatchProgress){.badges = 8}), HFC_WATCH_WRONG_TIER);
}

TEST("HFC Watch preserves the crafting unlock when migrating it into the hub")
{
    u8 list[HFC_WATCH_FEATURE_COUNT];
    struct HfcWatchProgress progress = {.badges = 1};

    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_CRAFTING, progress), HFC_WATCH_NEEDS_UNLOCK);
    progress.craftingUnlocked = TRUE;
    EXPECT_EQ(HfcWatch_BuildMenu(HFC_WATCH_TIER_POKE, progress, list), 3);
    EXPECT_EQ(list[2], HFC_WATCH_FEATURE_CRAFTING);
}

TEST("HFC Watch gates Boxes, the Mutant Signal and the Daily Raid by tier and badges")
{
    struct HfcWatchProgress progress = {.badges = 1};
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_STORAGE, progress), HFC_WATCH_NEEDS_BADGES);

    progress.badges = 2;
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_POKE, HFC_WATCH_FEATURE_STORAGE, progress), HFC_WATCH_WRONG_TIER);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_SUPER, HFC_WATCH_FEATURE_STORAGE, progress), HFC_WATCH_READY);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_FREQUENCY, progress), HFC_WATCH_NEEDS_BADGES);

    progress.badges = 3;
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_FREQUENCY, progress), HFC_WATCH_READY);
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_RAID, progress), HFC_WATCH_NEEDS_BADGES);

    progress.badges = 4;
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_RAID, progress), HFC_WATCH_NEEDS_BADGES);
    progress.badges = 5;
    EXPECT_EQ(HfcWatch_GetAccess(HFC_WATCH_TIER_HF, HFC_WATCH_FEATURE_RAID, progress), HFC_WATCH_READY);
}

TEST("HFC Watch Raid unlocks four difficulties and varies capture odds to hundredths")
{
    EXPECT_EQ(HfcWatch_GetRaidCaptureChance(3186, 2314, HFC_RAID_STANDARD), 2356);
    for (enum HfcRaidDifficulty difficulty = 0; difficulty < HFC_RAID_DIFFICULTY_COUNT; difficulty++)
    {
        const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(difficulty);
        EXPECT_EQ(info->minBadges, 5 + difficulty);
        EXPECT(!HfcWatch_IsRaidDifficultyUnlocked(difficulty, info->minBadges - 1));
        EXPECT(HfcWatch_IsRaidDifficultyUnlocked(difficulty, info->minBadges));
        if (difficulty != HFC_RAID_STANDARD)
            EXPECT(info->minCaptureChance > HfcWatch_GetRaidDifficultyInfo(difficulty - 1)->maxCaptureChance);
        for (u16 day = 0; day < 30; day++)
        {
            u16 chance = HfcWatch_GetRaidCaptureChance(day, 2314, difficulty);
            EXPECT(chance >= info->minCaptureChance && chance <= info->maxCaptureChance);
            EXPECT_EQ(HfcWatch_GetRaidCaptureChance(day, 2314, difficulty), chance);
        }
        for (u8 i = 0; i < HFC_RAID_LOOT_COUNT; i++)
        {
            EXPECT(info->loot[i].item != ITEM_NONE);
            EXPECT(info->loot[i].quantity > 0);
        }
    }
    EXPECT(!HfcWatch_IsRaidDifficultyUnlocked(HFC_RAID_DIFFICULTY_COUNT, 8));
}

TEST("HFC Watch Raid has two pairs of opponents and a stronger boss")
{
    for (u16 day = 0; day < 30; day++)
    {
        enum Species species[5];
        for (u8 i = 0; i < ARRAY_COUNT(species); i++)
        {
            struct HfcRaidOpponent opponent = HfcWatch_GetRaidOpponent(day, 2314, HFC_RAID_MASTER, i / 2, i % 2);
            species[i] = opponent.species;
            for (u8 earlier = 0; earlier < i; earlier++)
                EXPECT(species[earlier] != species[i]);
            EXPECT_EQ(opponent.level, i == 4 ? 60 : (i < 2 ? 54 : 56));
        }
        EXPECT_EQ(HfcWatch_GetRaidOpponent(day, 2314, HFC_RAID_MASTER, HFC_RAID_BOSS, 0).species, species[4]);
        EXPECT_EQ(HfcWatch_GetRaidOpponent(day, 2314, HFC_RAID_MASTER, HFC_RAID_BOSS, 1).species, SPECIES_NONE);
    }
}

TEST("HFC Watch Raid boss always rolls exactly one non-Omega mutation")
{
    for (u32 roll = 0; roll < 32; roll++)
    {
        for (enum HfcRaidDifficulty difficulty = 0; difficulty < HFC_RAID_DIFFICULTY_COUNT; difficulty++)
        {
            u16 mutation = HfcWatch_GetRaidMutation(difficulty, roll);
            EXPECT(mutation != 0 && mutation != POKEMON_BADGE_OMEGA);
            EXPECT_EQ(mutation & (mutation - 1), 0);
        }
    }
    EXPECT_EQ(HfcWatch_GetRaidMutation(HFC_RAID_MASTER, 7), POKEMON_BADGE_ASCENDED);
}

TEST("HFC Watch Raid rotates twenty unique bosses per difficulty before repeating")
{
    enum Species seen[HFC_RAID_DIFFICULTY_COUNT * 20];
    u8 count = 0;

    for (enum HfcRaidDifficulty tier = HFC_RAID_STANDARD; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
    {
        const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(tier);
        for (u16 day = 0; day < 20; day++)
        {
            struct HfcRaidOpponent boss = HfcWatch_GetRaidOpponent(day, 2314, tier, HFC_RAID_BOSS, 0);
            EXPECT(boss.species != SPECIES_NONE && IsSpeciesEnabled(boss.species));
            EXPECT_EQ(HfcWatch_GetRaidOpponent(day + 20, 2314, tier, HFC_RAID_BOSS, 0).species, boss.species);
            for (u8 i = 0; i < count; i++)
                EXPECT(seen[i] != boss.species);
            seen[count++] = boss.species;

            enum Species opponents[5];
            for (u8 i = 0; i < ARRAY_COUNT(opponents); i++)
            {
                struct HfcRaidOpponent opponent = HfcWatch_GetRaidOpponent(day, 2314, tier, i / 2, i % 2);
                opponents[i] = opponent.species;
                EXPECT(opponents[i] != SPECIES_NONE);
                EXPECT_EQ(opponent.level, info->level + (i == 4 ? 6 : (i < 2 ? 0 : 2)));
                for (u8 earlier = 0; earlier < i; earlier++)
                    EXPECT(opponents[earlier] != opponents[i]);
                for (u8 move = 0; move < MAX_MON_MOVES; move++)
                    EXPECT(opponent.moves[move] != MOVE_NONE);
            }
            EXPECT_EQ(opponents[4], boss.species);
        }
    }
    EXPECT_EQ(count, ARRAY_COUNT(seen));
}

TEST("HFC Watch Raid equips stronger difficulties and uses at most one compatible Mega per battle")
{
    u16 gears[HFC_RAID_DIFFICULTY_COUNT] = {0};
    u16 megas[HFC_RAID_DIFFICULTY_COUNT] = {0};
    u16 items[HFC_RAID_DIFFICULTY_COUNT] = {0};
    u16 bossMegas[HFC_RAID_DIFFICULTY_COUNT] = {0};
    for (u16 day = 0; day < 100; day++)
    {
        for (enum HfcRaidDifficulty tier = 0; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
        {
            for (enum HfcRaidStage stage = 0; stage <= HFC_RAID_BOSS; stage++)
            {
                bool32 isBoss = stage == HFC_RAID_BOSS;
                u8 megaCount = 0;
                for (u8 slot = 0; slot < (isBoss ? 1 : 2); slot++)
                {
                    struct HfcRaidOpponent mon = HfcWatch_GetRaidOpponent(day, 2314, tier, stage, slot);
                    EXPECT_EQ(mon.mega, GetItemHoldEffect(mon.heldItem) == HOLD_EFFECT_MEGA_STONE);
                    if (mon.gear != GEAR_NONE)
                        gears[tier]++;
                    if (mon.heldItem != ITEM_NONE)
                        items[tier]++;
                    megaCount += mon.mega;
                }
                EXPECT(megaCount <= 1);
                if (isBoss)
                    bossMegas[tier] += megaCount;
                else
                    megas[tier] += megaCount;
            }
        }
    }
    EXPECT_EQ(items[HFC_RAID_STANDARD], 0);
    EXPECT(items[HFC_RAID_ADVANCED] > 0);
    EXPECT_EQ(gears[HFC_RAID_ADVANCED], 0);
    EXPECT(gears[HFC_RAID_EXPERT] > 0);
    EXPECT(gears[HFC_RAID_MASTER] > gears[HFC_RAID_EXPERT]);
    EXPECT_EQ(megas[HFC_RAID_ADVANCED], 0);
    EXPECT(megas[HFC_RAID_EXPERT] > 0);
    EXPECT(megas[HFC_RAID_MASTER] > megas[HFC_RAID_EXPERT]);
    EXPECT(bossMegas[HFC_RAID_MASTER] > 0);
}

TEST("HFC Watch Mutant Signal increases the relative bonus with badges")
{
    EXPECT_EQ(HfcWatch_GetFrequencyBoost(2), 0);
    EXPECT_EQ(HfcWatch_GetFrequencyBoost(3), 25);
    EXPECT_EQ(HfcWatch_GetFrequencyBoost(4), 30);
    EXPECT_EQ(HfcWatch_GetFrequencyBoost(8), 50);
    EXPECT_EQ(HfcWatch_GetFrequencyBoost(9), 50);
}
