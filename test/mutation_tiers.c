#include "global.h"
#include "mutation_tiers.h"
#include "config/special_badges.h"
#include "pokemon.h"
#include "test/test.h"

TEST("Unholy mutation power tiers cover all nine badges")
{
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_ALPHA), MUTATION_TIER_C);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_HYPER), MUTATION_TIER_C);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_SUPER), MUTATION_TIER_B);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_OVERLOADED), MUTATION_TIER_B);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_ENRAGED), MUTATION_TIER_A);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_ANCESTRAL), MUTATION_TIER_A);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_TITAN), MUTATION_TIER_S);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_ASCENDED), MUTATION_TIER_S);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_OMEGA), MUTATION_TIER_OMEGA);
}

TEST("Unholy Horizon standard market accepts only mutation tiers C through A")
{
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_ALPHA));
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_HYPER));
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_SUPER));
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_OVERLOADED));
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_ENRAGED));
    EXPECT(MutationTierCanAppearInMarket(POKEMON_BADGE_ANCESTRAL));
    EXPECT(!MutationTierCanAppearInMarket(POKEMON_BADGE_TITAN));
    EXPECT(!MutationTierCanAppearInMarket(POKEMON_BADGE_ASCENDED));
    EXPECT(!MutationTierCanAppearInMarket(POKEMON_BADGE_OMEGA));
    EXPECT(!MutationTierCanAppearInMarket((enum PokemonBadge)0));
}

TEST("Unholy combined mutations use the highest tier as primary")
{
    u16 common = POKEMON_BADGE_ALPHA | POKEMON_BADGE_HYPER;
    u16 elite = POKEMON_BADGE_SUPER | POKEMON_BADGE_ANCESTRAL;
    u16 godly = POKEMON_BADGE_OMEGA | POKEMON_BADGE_ASCENDED | POKEMON_BADGE_TITAN;

    EXPECT_EQ(MutationTierFromMask(0), MUTATION_TIER_NONE);
    EXPECT_EQ(MutationTierFromMask(common), MUTATION_TIER_C);
    EXPECT_EQ(MutationTierFromMask(elite), MUTATION_TIER_A);
    EXPECT_EQ(MutationTierFromMask(godly), MUTATION_TIER_OMEGA);
    EXPECT_EQ(MutationTierFromMask(POKEMON_BADGES_ALL), MUTATION_TIER_OMEGA);
    EXPECT_EQ(MutationTierFromMask(0x8000), MUTATION_TIER_UNKNOWN);
}

TEST("Unholy special origin bonuses are relative and restricted to low tiers")
{
    // Per-roll odds = (100 + boost) / (denominator * 100).
    EXPECT_EQ(SPECIAL_BADGE_SPECIAL_ORIGIN_LOW_TIER_BOOST_PERCENT, 15);
    EXPECT_EQ(SPECIAL_BADGE_ALPHA_DENOMINATOR, 100);
    EXPECT_EQ(SPECIAL_BADGE_HYPER_DENOMINATOR, 250);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_ALPHA), MUTATION_TIER_C);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_HYPER), MUTATION_TIER_C);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_SUPER), MUTATION_TIER_B);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_OVERLOADED), MUTATION_TIER_B);
    EXPECT_EQ(MutationTierFromBadge(POKEMON_BADGE_OMEGA), MUTATION_TIER_OMEGA);
}

TEST("Unholy special origin never overwrites an existing event mutation")
{
    struct Pokemon mon;
    u16 badges = POKEMON_BADGE_ALPHA | POKEMON_BADGE_OMEGA;

    CreateRandomMon(&mon, SPECIES_TREECKO, 5);
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);

    // Horizon or a hand-authored gift must not be rerolled/overwritten.
    Pokemon_AssignSpecialOriginBadges(&mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIAL_BADGES), badges);
}
