#ifndef GUARD_MUTATION_TIERS_H
#define GUARD_MUTATION_TIERS_H

#include "pokemon.h"

// Gameplay power/availability tiers, NOT raw encounter rarity or egg quality.
// Keep ordered from weakest to strongest for a deterministic primary tier.
enum MutationTier
{
    MUTATION_TIER_NONE,
    MUTATION_TIER_C,
    MUTATION_TIER_B,
    MUTATION_TIER_A,
    MUTATION_TIER_S,
    MUTATION_TIER_OMEGA,
    MUTATION_TIER_UNKNOWN,
};

// Highest tier of a multi-mutated Pokémon is its displayed primary tier.
// Mutation stat bonuses continue to ADD independently; these tiers do not
// change their effect formulas or denominators.
enum MutationTier MutationTierFromBadge(enum PokemonBadge mutation);
enum MutationTier MutationTierFromMask(u16 badges);
bool8 MutationTierCanAppearInMarket(enum PokemonBadge mutation);
const u8 *MutationTierGetName(enum MutationTier tier);

#endif // GUARD_MUTATION_TIERS_H
