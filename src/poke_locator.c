#include "global.h"
#include "poke_locator.h"
#include "constants/items.h"
#include "constants/species.h"
#include "data.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include "start_menu.h"
#include "text.h"
#include "window.h"
#include "battle_setup.h"
#include "overworld.h"

static EWRAM_DATA struct PokeLocatorState sPokeLocator = {0};

static const u16 sRoute101Pokemon[] = {
    SPECIES_POOCHYENA,
    SPECIES_ZIGZAGOON,
    SPECIES_POOCHYENA,
    SPECIES_POOCHYENA,
    SPECIES_ZIGZAGOON,
};

static void CB2_InitPokeLocator(void)
{
    sPokeLocator.currentSelection = 0;
    sPokeLocator.scrollOffset = 0;
    sPokeLocator.isOpen = TRUE;
    sPokeLocator.count = 5;
    SetMainCallback2(CB2_InitPokeLocator);
}

void OpenPokeLocatorFromHfcWatch(void)
{
    SetMainCallback2(CB2_InitPokeLocator);
}

void ClosePokeLocator(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

void PokeLocator_HandleInput(void)
{
    if (JOY_NEW(DPAD_UP))
    {
        if (sPokeLocator.currentSelection > 0)
            sPokeLocator.currentSelection--;
        else
            sPokeLocator.currentSelection = sPokeLocator.count - 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sPokeLocator.currentSelection < sPokeLocator.count - 1)
            sPokeLocator.currentSelection++;
        else
            sPokeLocator.currentSelection = 0;
        PlaySE(SE_SELECT);
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (sPokeLocator.currentSelection < sPokeLocator.count)
        {
            PokeLocator_StartBattleWithSelected();
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        ClosePokeLocator();
        PlaySE(SE_SELECT);
    }
}

void PokeLocator_UpdateDisplay(void)
{
}

void PokeLocator_StartBattleWithSelected(void)
{
    u16 species = sPokeLocator.entries[sPokeLocator.currentSelection].species;
    u8 level = sPokeLocator.entries[sPokeLocator.currentSelection].level;
    CreateWildPokemon(species, level);
    CB2_InitWildBattle();
}

void PokeLocator_LoadAreaPokemon(void)
{
    sPokeLocator.count = 5;
}