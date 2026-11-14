#ifndef GUARD_CONFIG_POKELOCATOR_H
#define GUARD_CONFIG_POKELOCATOR_H

// Mutation-spawn modifier shown by the Pokélocator.
//
// DISENO.md (section 7) lists the cost, duration, odds and unlock rules of this
// modifier as undefined, so it stays OFF. Browsing only queries this state.
// Explicit Search creates a normal encounter; it does not enable this modifier.
// Do not set this to TRUE until a read-only provider exists in
// sModifierProviders[] (src/pokelocator_data.c) and those rules are written down.
#define POKELOCATOR_MUTATION_MODIFIER_ENABLED FALSE

#endif // GUARD_CONFIG_POKELOCATOR_H