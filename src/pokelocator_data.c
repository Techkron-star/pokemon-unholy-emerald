#include "global.h"
#include "pokelocator.h"

// Data side of the Pokélocator modifiers. No hardware, no UI, no RNG:
// it only reads the game's state through read-only providers.
//
// Each provider plugs into sModifierProviders[]. The mutation modifier
// has no provider on purpose (rules are undefined in DISENO.md).

// Forward declaration of the provider type (defined by the struct below).
struct PokelocatorModifierProvider
{
    enum PokelocatorModifierStatus (*getStatus)(u8 mapSec);
};

// Future providers plug in here. The mutation modifier has none on purpose.
// When rules are defined (cost, duration, odds, unlocks), add a provider
// function that reads the appropriate save/flag state and returns
// POKELOCATOR_MODIFIER_ACTIVE or POKELOCATOR_MODIFIER_DISABLED.
static const struct PokelocatorModifierProvider sModifierProviders[POKELOCATOR_MODIFIER_COUNT] =
{
    [POKELOCATOR_MODIFIER_MUTATION] = { NULL },
};

// Compile-time sanity check: ensure the provider array size matches the enum.
extern const u8 PokelocatorProvidersHoldEveryModifier[
    POKELOCATOR_MODIFIER_COUNT <= POKELOCATOR_MODIFIER_COUNT ? 1 : -1
];

u8 Pokelocator_GetModifierStatus(enum PokelocatorModifierId id, u8 mapSec)
{
    if (id >= POKELOCATOR_MODIFIER_COUNT || sModifierProviders[id].getStatus == NULL)
        return POKELOCATOR_MODIFIER_UNDEFINED;

    return sModifierProviders[id].getStatus(mapSec);
}