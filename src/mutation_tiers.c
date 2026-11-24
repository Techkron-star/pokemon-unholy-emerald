#include "global.h"
#include "mutation_tiers.h"

// Classification is centralized to keep shop, egg roulette, evolution and
// future UI/balance changes consistent. Unknown mutations fail closed.
enum MutationTier MutationTierFromBadge(enum PokemonBadge mutation)
{
    switch (mutation)
    {
    case POKEMON_BADGE_ALPHA:
    case POKEMON_BADGE_HYPER:
        return MUTATION_TIER_C;
    case POKEMON_BADGE_SUPER:
    case POKEMON_BADGE_OVERLOADED:
        return MUTATION_TIER_B;
    case POKEMON_BADGE_ENRAGED:
    case POKEMON_BADGE_ANCESTRAL:
        return MUTATION_TIER_A;
    case POKEMON_BADGE_TITAN:
    case POKEMON_BADGE_ASCENDED:
        return MUTATION_TIER_S;
    case POKEMON_BADGE_OMEGA:
        return MUTATION_TIER_OMEGA;
    default:
        return MUTATION_TIER_UNKNOWN;
    }
}

enum MutationTier MutationTierFromMask(u16 badges)
{
    static const enum PokemonBadge sBadges[] =
    {
        POKEMON_BADGE_ALPHA, POKEMON_BADGE_SUPER,
        POKEMON_BADGE_ENRAGED, POKEMON_BADGE_OMEGA,
        POKEMON_BADGE_HYPER, POKEMON_BADGE_OVERLOADED,
        POKEMON_BADGE_TITAN, POKEMON_BADGE_ANCESTRAL,
        POKEMON_BADGE_ASCENDED,
    };
    enum MutationTier primary = MUTATION_TIER_NONE;

    if (badges & ~POKEMON_BADGES_ALL)
        return MUTATION_TIER_UNKNOWN;
    for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
    {
        if (badges & sBadges[i])
        {
            enum MutationTier candidate = MutationTierFromBadge(sBadges[i]);
            if (candidate > primary)
                primary = candidate;
        }
    }
    return primary;
}

bool8 MutationTierCanAppearInMarket(enum PokemonBadge mutation)
{
    enum MutationTier tier = MutationTierFromBadge(mutation);
    return tier >= MUTATION_TIER_C && tier <= MUTATION_TIER_A;
}

const u8 *MutationTierGetName(enum MutationTier tier)
{
    static const u8 sNone[] = _("NORMAL");
    static const u8 sC[] = _("C");
    static const u8 sB[] = _("B");
    static const u8 sA[] = _("A");
    static const u8 sS[] = _("S");
    static const u8 sOmega[] = _("OMEGA");
    static const u8 sUnknown[] = _("UNKNOWN");

    switch (tier)
    {
    case MUTATION_TIER_NONE: return sNone;
    case MUTATION_TIER_C: return sC;
    case MUTATION_TIER_B: return sB;
    case MUTATION_TIER_A: return sA;
    case MUTATION_TIER_S: return sS;
    case MUTATION_TIER_OMEGA: return sOmega;
    default: return sUnknown;
    }
}
