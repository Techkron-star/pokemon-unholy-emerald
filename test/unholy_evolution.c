#include "global.h"
#include "unholy_evolution.h"
#include "battle.h"
#include "caps.h"
#include "event_data.h"
#include "gear.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "random.h"
#include "test/test.h"
#include "constants/items.h"

struct EvolutionBackup
{
    struct SaveBlock1 save;
    struct Pokemon party[PARTY_SIZE];
    u8 count;
    bool8 battle;
};

static struct EvolutionBackup *Begin(enum Species species, u8 level)
{
    struct EvolutionBackup *old = Alloc(sizeof(*old));
    old->save = *gSaveBlock1Ptr;
    memcpy(old->party, gParties[B_TRAINER_PLAYER], sizeof(old->party));
    old->count = gPartiesCount[B_TRAINER_PLAYER];
    old->battle = gMain.inBattle;
    gMain.inBattle = FALSE;
    ClearBag();
    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], species, level, 123, OTID_STRUCT_PRESET(456), 12);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    return old;
}

static void End(struct EvolutionBackup *old)
{
    *gSaveBlock1Ptr = old->save;
    memcpy(gParties[B_TRAINER_PLAYER], old->party, sizeof(old->party));
    gPartiesCount[B_TRAINER_PLAYER] = old->count;
    gMain.inBattle = old->battle;
    Free(old);
}

static struct UnholyEvolutionOption Find(enum Species target)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    struct UnholyEvolutionOption out = {.status = UNHOLY_EVO_INVALID};
    for (u16 i = 0; i < UnholyEvolution_Count(species); i++)
    {
        UnholyEvolution_GetOption(mon, species, i, 0, &out);
        if (out.target == target) return out;
    }
    return (struct UnholyEvolutionOption){.status = UNHOLY_EVO_INVALID};
}

TEST("Evolution QoL can trigger a reached level at the badge cap without extra EXP")
{
    struct EvolutionBackup *old = Begin(SPECIES_TREECKO, 15);
    FlagClear(FLAG_BADGE01_GET);
    EXPECT_EQ(GetCurrentLevelCap(), 15);
    struct UnholyEvolutionOption option = Find(SPECIES_GROVYLE);
    EXPECT_EQ(option.status, UNHOLY_EVO_LEVEL);
    EXPECT_EQ(option.minimumLevel, 16);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u8 level = 16;
    SetMonData(mon, MON_DATA_LEVEL, &level);
    u16 mutations = POKEMON_BADGE_ALPHA | POKEMON_BADGE_OMEGA;
    SetMonData(mon, MON_DATA_SPECIAL_BADGES, &mutations);
    Gear_SetEquipped(mon, GEAR_PERMAFROST_FRAGMENT);
    option = Find(SPECIES_GROVYLE);
    EXPECT_EQ(option.status, UNHOLY_EVO_READY);
    struct Pokemon before = *mon;
    rng_value_t rng = gRngValue;
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT_EQ(memcmp(mon, &before, sizeof(before)), 0); // The native scene changes the species later.
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_CHANGED);
    End(old);
}

TEST("Evolution QoL chooses Gloom branches and consumes only the confirmed stone")
{
    struct EvolutionBackup *old = Begin(SPECIES_GLOOM, 25);
    EXPECT(AddBagItem(ITEM_LEAF_STONE, 2));
    EXPECT(AddBagItem(ITEM_SUN_STONE, 1));
    EXPECT_EQ(UnholyEvolution_Count(SPECIES_GLOOM), 2);
    struct UnholyEvolutionOption vileplume = Find(SPECIES_VILEPLUME);
    struct UnholyEvolutionOption bellossom = Find(SPECIES_BELLOSSOM);
    EXPECT_EQ(vileplume.status, UNHOLY_EVO_READY);
    EXPECT_EQ(bellossom.status, UNHOLY_EVO_READY);
    struct UnholyEvolutionPlan plan;
    rng_value_t rng = gRngValue;
    EXPECT_EQ(UnholyEvolution_Prepare(0, bellossom.entry, &plan), UNHOLY_EVO_READY);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUN_STONE), 1); // Review / cancel is free.
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT_EQ(plan.option.target, SPECIES_BELLOSSOM);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUN_STONE), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LEAF_STONE), 2);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_CHANGED);
    End(old);
}

TEST("Evolution QoL deduplicates trade and Linking Cord recipes and never evolves free")
{
    struct EvolutionBackup *old = Begin(SPECIES_KADABRA, 20);
    EXPECT_EQ(UnholyEvolution_Count(SPECIES_KADABRA), 1);
    struct UnholyEvolutionOption option = Find(SPECIES_ALAKAZAM);
    EXPECT_EQ(option.status, UNHOLY_EVO_ITEM);
    EXPECT(AddBagItem(ITEM_LINKING_CORD, 2));
    option = Find(SPECIES_ALAKAZAM);
    EXPECT_EQ(option.status, UNHOLY_EVO_READY);
    EXPECT_EQ(option.costCount, 1);
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LINKING_CORD), 1);
    End(old);
}

TEST("Evolution QoL accepts trade items in the bag and preserves an unrelated held item")
{
    struct EvolutionBackup *old = Begin(SPECIES_ONIX, 25);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u16 held = ITEM_LEFTOVERS;
    SetMonData(mon, MON_DATA_HELD_ITEM, &held);
    struct UnholyEvolutionOption option = Find(SPECIES_STEELIX);
    EXPECT_EQ(option.status, UNHOLY_EVO_ITEM);
    EXPECT(AddBagItem(ITEM_METAL_COAT, 1));
    option = Find(SPECIES_STEELIX);
    EXPECT_EQ(option.status, UNHOLY_EVO_READY);
    EXPECT_EQ(option.costCount, 1);
    EXPECT_EQ(option.costs[0].item, ITEM_METAL_COAT);
    EXPECT(!option.costs[0].held);
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_METAL_COAT), 0);
    End(old);
}

TEST("Evolution QoL accepts an equipped trade item without requiring a second copy")
{
    struct EvolutionBackup *old = Begin(SPECIES_ONIX, 25);
    u16 held = ITEM_METAL_COAT;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM, &held);
    struct UnholyEvolutionOption option = Find(SPECIES_STEELIX);
    EXPECT_EQ(option.status, UNHOLY_EVO_READY);
    EXPECT(option.costs[0].held);
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_NONE);
    End(old);
}

TEST("Evolution QoL revalidates the Pokemon and bag before consuming resources")
{
    struct EvolutionBackup *old = Begin(SPECIES_GLOOM, 25);
    EXPECT(AddBagItem(ITEM_SUN_STONE, 1));
    struct UnholyEvolutionOption option = Find(SPECIES_BELLOSSOM);
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    u32 personality = 789;
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_PERSONALITY, &personality);
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_CHANGED);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUN_STONE), 1);
    gParties[B_TRAINER_PLAYER][0] = plan.before;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_READY);
    EXPECT(RemoveBagItem(ITEM_SUN_STONE, 1));
    EXPECT_EQ(UnholyEvolution_Consume(&plan), UNHOLY_EVO_ITEM);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SUN_STONE), 0);
    End(old);
}

TEST("Evolution QoL retains friendship, Everstone and egg restrictions and previews future stages")
{
    struct EvolutionBackup *old = Begin(SPECIES_PICHU, 5);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u8 friendship = 0;
    SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    struct UnholyEvolutionOption option = Find(SPECIES_PIKACHU);
    EXPECT_EQ(option.status, UNHOLY_EVO_CONDITION);
    friendship = 255;
    SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
    option = Find(SPECIES_PIKACHU);
    EXPECT_EQ(option.status, UNHOLY_EVO_READY);
    u16 stone = ITEM_EVERSTONE;
    SetMonData(mon, MON_DATA_HELD_ITEM, &stone);
    option = Find(SPECIES_PIKACHU);
    EXPECT_EQ(option.status, UNHOLY_EVO_EVERSTONE);
    struct UnholyEvolutionPlan plan;
    EXPECT_EQ(UnholyEvolution_Prepare(0, option.entry, &plan), UNHOLY_EVO_EVERSTONE);
    EXPECT_NE(UnholyEvolution_Consume(&plan), UNHOLY_EVO_READY);
    EXPECT(UnholyEvolution_GetOption(mon, SPECIES_PIKACHU, 0, 0, &option));
    EXPECT_EQ(option.status, UNHOLY_EVO_PREVIEW);
    u8 egg = TRUE;
    SetMonData(mon, MON_DATA_IS_EGG, &egg);
    option = Find(SPECIES_PIKACHU);
    EXPECT_EQ(option.status, UNHOLY_EVO_INVALID);
    EXPECT_EQ(UnholyEvolution_Count(SPECIES_NONE), 0);
    EXPECT_EQ(UnholyEvolution_Count(SPECIES_VENUSAUR), 0);
    End(old);
}
