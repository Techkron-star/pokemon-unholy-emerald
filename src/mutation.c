#include "mutation.h"
#include "strings.h"

// Tiers de mutación en orden de potencia
static const u8 sMutationTierNames[MUTATION_TIER_COUNT][12] = {
    "C - Alpha",
    "B - Super",
    "A - Enraged",
    "S - Titan",
    "Ω Omega",
};

// Mapeo de badge a tier
static const u8 sBadgeToTier[] = {
    MUTATION_TIER_C,   // MUTATION_BADGE_NONE (no debería usarse)
    MUTATION_TIER_C,   // MUTATION_BADGE_HYPER -> Tier C
    MUTATION_TIER_S,   // MUTATION_BADGE_TITAN -> Tier S
    MUTATION_TIER_B,   // MUTATION_BADGE_OVERLOADED -> Tier B
};

// Mapeo de tier a badge
static const u8 sTierToBadge[] = {
    MUTATION_BADGE_HYPER,      // MUTATION_TIER_C -> Hyper
    MUTATION_BADGE_OVERLOADED, // MUTATION_TIER_B -> Overloaded
    MUTATION_BADGE_HYPER,      // MUTATION_TIER_A -> Hyper (badge disponible más cercano)
    MUTATION_BADGE_TITAN,      // MUTATION_TIER_S -> Titan
    MUTATION_BADGE_TITAN,      // MUTATION_TIER_OMEGA -> Titan (badge disponible más cercano)
};

// Obtener tier desde badge
u8 MutationTierFromBadge(u8 badge)
{
    if (badge < ARRAY_COUNT(sBadgeToTier))
        return sBadgeToTier[badge];
    return MUTATION_TIER_C;
}

// Obtener tier desde máscara de mutación
u8 MutationTierFromMask(u32 mask)
{
    // Omega tiene prioridad máxima
    if (mask & MUTATION_MASK_OMEGA)
        return MUTATION_TIER_OMEGA;

    // Titan/Ascendido (Tier S)
    if (mask & MUTATION_MASK_TITAN || mask & MUTATION_MASK_ASCENDIDO)
        return MUTATION_TIER_S;

    // Enraged/Ancestral (Tier A)
    if (mask & MUTATION_MASK_ENRAGED || mask & MUTATION_MASK_ANCESTRAL)
        return MUTATION_TIER_A;

    // Super/Overloaded (Tier B)
    if (mask & MUTATION_MASK_SUPER || mask & MUTATION_MASK_OVERLOADED)
        return MUTATION_TIER_B;

    // Alpha/Hyper (Tier C)
    if (mask & MUTATION_MASK_ALPHA || mask & MUTATION_MASK_HYPER)
        return MUTATION_TIER_C;

    return MUTATION_TIER_C;
}

// Verificar si un tier puede aparecer en el mercado Horizon
bool8 MutationTierCanAppearInMarket(u8 tier)
{
    // Según diseño.md §2.4: límite tier A en mercado Horizon
    // Los tiers S y Ω no pueden filtrarse desde stock antiguo
    if (tier == MUTATION_TIER_S || tier == MUTATION_TIER_OMEGA)
        return FALSE;

    return TRUE;
}

// Obtener nombre legible del tier
const u8 *MutationTierGetName(u8 tier)
{
    if (tier < MUTATION_TIER_COUNT)
        return sMutationTierNames[tier];
    return sMutationTierNames[MUTATION_TIER_C];
}

// Obtener badge correspondiente a un tier
u8 MutationBadgeFromTier(u8 tier)
{
    if (tier < MUTATION_TIER_COUNT)
        return sTierToBadge[tier];
    return MUTATION_BADGE_HYPER;
}

// Verificar si un Pokémon tiene alguna mutación activa
bool8 PokemonHasMutation(u32 mutationMask)
{
    return mutationMask != 0;
}

// Obtener el nombre de la mutación principal desde una máscara
const u8 *MutationGetPrimaryName(u32 mask)
{
    if (mask & MUTATION_MASK_OMEGA)
        return gText_MutationNameOmega;
    if (mask & MUTATION_MASK_TITAN || mask & MUTATION_MASK_ASCENDIDO)
        return gText_MutationNameTitan;
    if (mask & MUTATION_MASK_ENRAGED || mask & MUTATION_MASK_ANCESTRAL)
        return gText_MutationNameEnraged;
    if (mask & MUTATION_MASK_SUPER || mask & MUTATION_MASK_OVERLOADED)
        return gText_MutationNameOverloaded;
    if (mask & MUTATION_MASK_ALPHA || mask & MUTATION_MASK_HYPER)
        return gText_MutationNameHyper;

    return gText_MutationNameNone;
}