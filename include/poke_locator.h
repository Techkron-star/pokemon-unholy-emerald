#ifndef GUARD_POKE_LOCATOR_H
#define GUARD_POKE_LOCATOR_H

#include "global.h"
#include "mutation.h"

#define POKE_LOCATOR_MAX_POKEMON 10
#define POKE_LOCATOR_MAX_LEVELS  5

struct PokeLocatorEntry
{
    u16 species;
    u8 level;
    u8 mutationMask;
    u8 stock;
};

struct PokeLocatorState
{
    struct PokeLocatorEntry entries[POKE_LOCATOR_MAX_POKEMON];
    u8 count;
    u8 currentSelection;
    u8 scrollOffset;
    u8 isOpen;

    // Filtros de búsqueda
    u8 selectedSpeciesIndex;      // Índice de la especie seleccionada en entries
    u32 selectedMutationMask;     // Mutación seleccionada (bitmask)
    bool8 shinyFilter;            // Filtro shiny activado
    u8 trackingAttempts;          // Contador de intentos de rastreo
    bool8 isTracking;             // Si está en modo rastreo activo
};

void CB2_InitPokeLocator(void);
void OpenPokeLocatorFromHfcWatch(void);
void ClosePokeLocator(void);
void PokeLocator_HandleInput(void);
void PokeLocator_UpdateDisplay(void);
void PokeLocator_StartBattleWithSelected(void);
void PokeLocator_LoadAreaPokemon(void);
void PokeLocator_SelectMutation(u32 mutationMask);
void PokeLocator_ToggleShinyFilter(void);
bool8 PokeLocator_TrackTarget(void);
u16 PokeLocator_CalculateEncounterOdds(u16 species, u32 mutationMask);

#endif // GUARD_POKE_LOCATOR_H