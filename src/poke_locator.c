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

// Estado del Pokémon Locator
static EWRAM_DATA struct PokeLocatorState sPokeLocator = {0};

// Lista de Pokémon disponibles en Ruta 101 (área inicial)
static const struct PokeLocatorEntry sRoute101Pokemon[] = {
    {SPECIES_POOCHYENA, 5, 8, 0, FALSE},
    {SPECIES_ZIGZAGOON, 5, 8, 1, FALSE},
    {SPECIES_POOCHYENA, 6, 6, 2, FALSE},
    {SPECIES_POOCHYENA, 7, 4, 3, FALSE},
    {SPECIES_ZIGZAGOON, 6, 4, 4, FALSE},
};

// Función auxiliar para obtener el nombre de una especie
static const u8 *GetSpeciesName(u16 species)
{
    switch (species)
    {
        case SPECIES_POOCHYENA:   return gText_Poochyena;
        case SPECIES_ZIGZAGOON:   return gText_Zigzagoon;
        default:                  return gText_Poochyena;
    }
}

// Inicializar el Pokémon Locator
static void CB2_InitPokeLocator(void)
{
    sPokeLocator.currentSelection = 0;
    sPokeLocator.scrollOffset = 0;
    sPokeLocator.isOpen = TRUE;

    // Cargar Pokémon del área actual (Ruta 101 por defecto)
    sPokeLocator.count = 5;
    for (u8 i = 0; i < sPokeLocator.count; i++)
    {
        sPokeLocator.entries[i] = sRoute101Pokemon[i];
    }

    SetMainCallback2(CB2_InitPokeLocator);
}

// Abrir Poké Locator desde HFC Watch
void OpenPokeLocatorFromHfcWatch(void)
{
    SetMainCallback2(CB2_InitPokeLocator);
}

// Cerrar Poké Locator de forma segura
void ClosePokeLocator(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

// Manejo de entrada de usuario en Poké Locator
void PokeLocator_HandleInput(void)
{
    // Navegación arriba/abajo entre Pokémon
    if (JOY_NEW(DPAD_UP))
    {
        if (sPokeLocator.currentSelection > 0)
            sPokeLocator.currentSelection--;
        else
            sPokeLocator.currentSelection = sPokeLocator.count - 1;

        // Ajustar scroll si es necesario
        if (sPokeLocator.currentSelection < sPokeLocator.scrollOffset)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection;
        else if (sPokeLocator.currentSelection >= sPokeLocator.scrollOffset + 4)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection - 3;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sPokeLocator.currentSelection < sPokeLocator.count - 1)
            sPokeLocator.currentSelection++;
        else
            sPokeLocator.currentSelection = 0;

        // Ajustar scroll si es necesario
        if (sPokeLocator.currentSelection < sPokeLocator.scrollOffset)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection;
        else if (sPokeLocator.currentSelection >= sPokeLocator.scrollOffset + 4)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection - 3;
        PlaySE(SE_SELECT);
    }

    // A para iniciar combate con el Pokémon seleccionado
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

// Actualizar pantalla del Poké Locator
void PokeLocator_UpdateDisplay(void)
{
    // La visualización se maneja en el menú HFC Watch
}

// Iniciar batalla con el Pokémon seleccionado
void PokeLocator_StartBattleWithSelected(void)
{
    u16 species = sPokeLocator.entries[sPokeLocator.currentSelection].species;
    u8 level = sPokeLocator.entries[sPokeLocator.currentSelection].level;

    // Crear el Pokémon salvaje
    CreateWildPokemon(species, level);

    // Iniciar batalla salvaje
    CB2_InitWildBattle();
}

// Cargar Pokémon del área actual
void PokeLocator_LoadAreaPokemon(void)
{
    // Por ahora usamos Ruta 101 por defecto
    // TODO: Detectar el mapa actual automáticamente
}