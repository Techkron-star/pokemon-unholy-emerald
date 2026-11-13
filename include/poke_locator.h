#ifndef GUARD_POKE_LOCATOR_H
#define GUARD_HFC_WATCH_H

#include "global.h"

// Número de Pokémon que puede mostrar el Poké Locator
#define POKE_LOCATOR_MAX_POKEMON 10

// Datos de un Pokémon disponible para encontrar
struct PokeLocatorEntry
{
    u16 species;           // Especie del Pokémon
    u8 level;              // Nivel del Pokémon
    u8 rarity;             // Rareza (1-10, menor es más raro)
    u8 iconSpriteId;       // ID del sprite del ícono
    bool8 isUnlocked;      // Si ya está desbloqueado (visto antes)
};

// Estado del Pokémon Locator
struct PokeLocatorState
{
    struct PokeLocatorEntry entries[POKE_LOCATOR_MAX_POKEMON]; // Lista de Pokémon disponibles
    u8 count;                          // Número de Pokémon en la lista
    u8 currentSelection;               // Selección actual del cursor
    u8 scrollOffset;                   // Desplazamiento de la lista
    bool8 isOpen;                      // Si el Poké Locator está abierto
};

// Prototipos de funciones
void CB2_InitPokeLocator(void);
void OpenPokeLocatorFromHfcWatch(void);
void ClosePokeLocator(void);
void PokeLocator_HandleInput(void);
void PokeLocator_UpdateDisplay(void);
void PokeLocator_StartBattleWithSelected(void);
void PokeLocator_LoadAreaPokemon(void);

#endif // GUARD_POKE_LOCATOR_H