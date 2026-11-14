#ifndef GUARD_POKELOCATOR_H
#define GUARD_POKELOCATOR_H

#include "global.h"
#include "config/pokelocator.h"
#include "mutation.h"

// Identificadores de modificadores del Pokélocator.
// Cada modificador puede tener su propio proveedor de estado de solo lectura.
enum PokelocatorModifierId
{
    POKELOCATOR_MODIFIER_MUTATION,  // Modificador de probabilidad de mutación
    POKELOCATOR_MODIFIER_COUNT,
};

// Estado de un modificador del Pokélocator.
enum PokelocatorModifierStatus
{
    POKELOCATOR_MODIFIER_UNDEFINED,  // No hay proveedor definido (apagado)
    POKELOCATOR_MODIFIER_DISABLED,   // Proveedor existe pero está desactivado
    POKELOCATOR_MODIFIER_ACTIVE,     // Proveedor existe y está activado
};

// Obtener el estado de un modificador en un mapa/sector determinado.
// Devuelve UNDEFINED si no hay proveedor registrado para ese modificador.
u8 Pokelocator_GetModifierStatus(enum PokelocatorModifierId id, u8 mapSec);

// Verificar si el modificador de mutaciones está habilitado.
#define Pokelocator_IsMutationModifierEnabled() (POKELOCATOR_MUTATION_MODIFIER_ENABLED == TRUE)

#endif // GUARD_POKELOCATOR_H