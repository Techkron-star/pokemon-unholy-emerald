#ifndef GUARD_MUTATION_H
#define GUARD_MUTATION_H

#include "global.h"

// Tiers de mutación (según diseño.md §2.1)
enum MutationTier
{
    MUTATION_TIER_C,    // Alpha / Hyper
    MUTATION_TIER_B,    // Super / Overloaded
    MUTATION_TIER_A,    // Enraged / Ancestral
    MUTATION_TIER_S,    // Titan / Ascendido
    MUTATION_TIER_OMEGA, // Ω Omega
    MUTATION_TIER_COUNT,
};

// Máscaras de mutación (bit por mutación)
#define MUTATION_MASK_ALPHA       (1 << 0)
#define MUTATION_MASK_HYPER       (1 << 1)
#define MUTATION_MASK_SUPER       (1 << 2)
#define MUTATION_MASK_OVERLOADED  (1 << 3)
#define MUTATION_MASK_ENRAGED     (1 << 4)
#define MUTATION_MASK_ANCESTRAL   (1 << 5)
#define MUTATION_MASK_TITAN       (1 << 6)
#define MUTATION_MASK_ASCENDIDO   (1 << 7)
#define MUTATION_MASK_OMEGA       (1 << 8)

// Badges de mutación visuales (16×16)
#define MUTATION_BADGE_NONE    0
#define MUTATION_BADGE_HYPER   1
#define MUTATION_BADGE_TITAN   2
#define MUTATION_BADGE_OVERLOADED 3

// Funciones centrales de mutación (diseño.md §2.2)
u8 MutationTierFromBadge(u8 badge);
u8 MutationTierFromMask(u32 mask);
bool8 MutationTierCanAppearInMarket(u8 tier);
const u8 *MutationTierGetName(u8 tier);

// Obtener badge correspondiente a un tier
u8 MutationBadgeFromTier(u8 tier);

// Verificar si un Pokémon tiene alguna mutación activa
bool8 PokemonHasMutation(u32 mutationMask);

// Obtener el nombre de la mutación principal desde una máscara
const u8 *MutationGetPrimaryName(u32 mutationMask);

#endif // GUARD_MUTATION_H