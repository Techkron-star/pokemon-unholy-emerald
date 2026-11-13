#ifndef GUARD_POKE_LOCATOR_H
#define GUARD_POKE_LOCATOR_H

#include "global.h"

#define POKE_LOCATOR_MAX_POKEMON 10

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
    u8 isOpen;
};

void CB2_InitPokeLocator(void);
void OpenPokeLocatorFromHfcWatch(void);
void ClosePokeLocator(void);
void PokeLocator_HandleInput(void);
void PokeLocator_UpdateDisplay(void);
void PokeLocator_StartBattleWithSelected(void);
void PokeLocator_LoadAreaPokemon(void);

#endif // GUARD_POKE_LOCATOR_H