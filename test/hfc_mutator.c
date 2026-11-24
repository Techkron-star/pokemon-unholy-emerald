#include "global.h"
#include "hfc_mutator.h"
#include "money.h"
#include "random.h"
#include "test/test.h"

static void SeedForRoll(u8 wanted)
{
    for (u32 seed = 0; ; seed++)
    {
        rng_value_t rng = LocalRandomSeed(seed);
        if (LocalRandom32(&rng) % 100 == wanted)
        {
            // Use the local state so VBlank cannot consume the first roll.
            gRngValue = LocalRandomSeed(seed);
            return;
        }
    }
}

TEST("HFC Mutator charges once and preserves the Pokemon on failure")
{
    struct Pokemon mon;
    enum PokemonBadge mutation;
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 100, 0, OTID_STRUCT_PRESET(0), 10);
    struct Pokemon original = mon;
    SetMoney(&money, 12000);

    SeedForRoll(35);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_BASIC, FALSE, &money, &mutation), HFC_MUTATOR_ROLL_FAILED);
    EXPECT_EQ(GetMoney(&money), 0);
    EXPECT_EQ(mutation, 0);
    EXPECT_EQ(memcmp(&mon, &original, sizeof(mon)), 0);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_BASIC, FALSE, &money, &mutation), HFC_MUTATOR_INSUFFICIENT_FUNDS);
    EXPECT_EQ(memcmp(&mon, &original, sizeof(mon)), 0);
}

TEST("HFC Mutator applies all three plan rates and level penalties")
{
    static const u32 sPrices[] = {12000, 30000, 60000};
    static const u8 sSuccess[] = {35, 60, 85};
    static const u8 sExpectedLevels[] = {95, 90, 85};
    for (enum HfcMutatorPlan plan = 0; plan < HFC_MUTATOR_PLAN_COUNT; plan++)
    {
        struct Pokemon mon;
        enum PokemonBadge mutation;
        u32 money;
        CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 100, 0, OTID_STRUCT_PRESET(0), 10);
        u16 hp = 0;
        SetMonData(&mon, MON_DATA_HP, &hp);
        SetMoney(&money, sPrices[plan] * 2);

        SeedForRoll(sSuccess[plan]);
        EXPECT_EQ(HfcMutator_Attempt(&mon, plan, FALSE, &money, &mutation), HFC_MUTATOR_ROLL_FAILED);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), 100);
        EXPECT_EQ(GetMoney(&money), sPrices[plan]);

        SeedForRoll(sSuccess[plan] - 1);
        EXPECT_EQ(HfcMutator_Attempt(&mon, plan, FALSE, &money, &mutation), HFC_MUTATOR_APPLIED);
        EXPECT(mutation != 0 && (mutation & (mutation - 1)) == 0);
        EXPECT_NE(mutation, POKEMON_BADGE_OMEGA);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIAL_BADGES), mutation);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), sExpectedLevels[plan]);
        EXPECT_EQ(GetLevelFromMonExp(&mon), sExpectedLevels[plan]);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HP), 0);
        EXPECT_EQ(GetMoney(&money), 0);
    }
}

TEST("HFC Mutator protects eggs and Omega and gates rerolls")
{
    struct Pokemon mon;
    enum PokemonBadge mutation;
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 60, 0, OTID_STRUCT_PRESET(0), 10);
    SetMoney(&money, 60000);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_PLAN_COUNT, TRUE, &money, &mutation), HFC_MUTATOR_INVALID_PLAN);

    u16 badges = POKEMON_BADGE_ALPHA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_BASIC, FALSE, &money, &mutation), HFC_MUTATOR_NEEDS_SUPER_LEAGUE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIAL_BADGES), POKEMON_BADGE_ALPHA);

    badges = POKEMON_BADGE_OMEGA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_BASIC, TRUE, &money, &mutation), HFC_MUTATOR_OMEGA);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIAL_BADGES), POKEMON_BADGE_OMEGA);

    u8 isEgg = TRUE;
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_BASIC, TRUE, &money, &mutation), HFC_MUTATOR_EGG);
    EXPECT_EQ(GetMoney(&money), 60000);
}

TEST("HFC Mutator replaces Ascended without stacking a duplicate or leaving illegal EVs")
{
    struct Pokemon mon;
    enum PokemonBadge mutation;
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 1, 0, OTID_STRUCT_PRESET(0), 10);
    u16 badges = POKEMON_BADGE_ASCENDED | POKEMON_BADGE_ALPHA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    CalculateMonStats(&mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), 252);
    SetMoney(&money, 60000);

    SeedForRoll(0);
    EXPECT_EQ(HfcMutator_Attempt(&mon, HFC_MUTATOR_INTENSIVE, TRUE, &money, &mutation), HFC_MUTATOR_APPLIED);
    EXPECT(mutation != POKEMON_BADGE_ALPHA && mutation != POKEMON_BADGE_ASCENDED && mutation != POKEMON_BADGE_OMEGA);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIAL_BADGES), POKEMON_BADGE_ALPHA | mutation);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), 1);
    EXPECT_EQ(GetMoney(&money), 0);
    for (u8 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + stat), MAX_TOTAL_EVS / NUM_STATS);
}
