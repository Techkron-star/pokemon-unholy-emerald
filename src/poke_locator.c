#include "global.h"
#include "poke_locator.h"
#include "random.h"
#include "pokemon.h"
#include "battle_setup.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/species.h"
#include "data.h"
#include "menu.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "text.h"
#include "window.h"

// Estado del Pokémon Locator
static EWRAM_DATA struct PokeLocatorState sPokeLocator = {0};

// Lista de Pokémon disponibles en Ruta 101 (área inicial)
static const struct PokeLocatorEntry sRoute101Pokemon[] = {
    {SPECIES_POOCHYENA,  5, MUTATION_MASK_ALPHA,  0},
    {SPECIES_ZIGZAGOON,  5, MUTATION_MASK_ALPHA,  1},
    {SPECIES_POOCHYENA,  6, MUTATION_MASK_HYPER,  2},
    {SPECIES_POOCHYENA,  7, 0,                    3},
    {SPECIES_ZIGZAGOON,  6, 0,                    4},
};

// Inicializar el Pokémon Locator
static void CB2_InitPokeLocator(void)
{
    sPokeLocator.currentSelection = 0;
    sPokeLocator.scrollOffset = 0;
    sPokeLocator.isOpen = TRUE;
    sPokeLocator.count = 5;
    sPokeLocator.selectedSpeciesIndex = 0;
    sPokeLocator.selectedMutationMask = 0;
    sPokeLocator.shinyFilter = FALSE;
    sPokeLocator.trackingAttempts = 0;
    sPokeLocator.isTracking = FALSE;

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
    if (sPokeLocator.isTracking)
    {
        // En modo rastreo: solo B para cancelar
        if (JOY_NEW(B_BUTTON))
        {
            sPokeLocator.isTracking = FALSE;
            sPokeLocator.trackingAttempts = 0;
            PlaySE(SE_SELECT);
        }
        return;
    }

    if (JOY_NEW(DPAD_UP))
    {
        if (sPokeLocator.currentSelection > 0)
            sPokeLocator.currentSelection--;
        else
            sPokeLocator.currentSelection = sPokeLocator.count - 1;

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

        if (sPokeLocator.currentSelection < sPokeLocator.scrollOffset)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection;
        else if (sPokeLocator.currentSelection >= sPokeLocator.scrollOffset + 4)
            sPokeLocator.scrollOffset = sPokeLocator.currentSelection - 3;

        PlaySE(SE_SELECT);
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (sPokeLocator.currentSelection < sPokeLocator.count)
        {
            sPokeLocator.selectedSpeciesIndex = sPokeLocator.currentSelection;
            PokeLocator_StartBattleWithSelected();
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        ClosePokeLocator();
        PlaySE(SE_SELECT);
    }
}

// Actualizar pantalla del Poké Locator (llamado cada frame)
void PokeLocator_UpdateDisplay(void)
{
    // La visualización se maneja en el menú HFC Watch
}

// Iniciar batalla con el Pokémon seleccionado
void PokeLocator_StartBattleWithSelected(void)
{
    u16 species = sPokeLocator.entries[sPokeLocator.selectedSpeciesIndex].species;
    u8 level = sPokeLocator.entries[sPokeLocator.selectedSpeciesIndex].level;

    // Configurar el Pokémon salvaje en el oponente A
    SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES, &species);
    SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL, &level);

    // Iniciar batalla salvaje
    BattleSetup_StartWildBattle();
}

// Cargar Pokémon del área actual
void PokeLocator_LoadAreaPokemon(void)
{
    sPokeLocator.count = 5;
    sPokeLocator.currentSelection = 0;
    sPokeLocator.scrollOffset = 0;
    sPokeLocator.selectedSpeciesIndex = 0;
    sPokeLocator.selectedMutationMask = 0;
    sPokeLocator.shinyFilter = FALSE;
    sPokeLocator.trackingAttempts = 0;
    sPokeLocator.isTracking = FALSE;

    for (u8 i = 0; i < sPokeLocator.count; i++)
    {
        sPokeLocator.entries[i] = sRoute101Pokemon[i];
    }
}

// Seleccionar una máscara de mutación para el rastreo
void PokeLocator_SelectMutation(u32 mutationMask)
{
    sPokeLocator.selectedMutationMask = mutationMask;
}

// Activar/desactivar filtro shiny
void PokeLocator_ToggleShinyFilter(void)
{
    sPokeLocator.shinyFilter = !sPokeLocator.shinyFilter;
    PlaySE(SE_SELECT);
}

// Ejecutar un intento de rastreo RNG
bool8 PokeLocator_TrackTarget(void)
{
    if (!sPokeLocator.isTracking)
        return FALSE;

    sPokeLocator.trackingAttempts++;

    // Tiro RNG simple para determinar si se encuentra el objetivo
    // Probabilidad base: 1% por intento (ajustable según mutación y shiny)
    u16 roll = (u16)(Random() % 100);

    // Ajustar probabilidad según mutación (mutaciones raras son más difíciles)
    u8 rarityModifier = 0;
    if (sPokeLocator.selectedMutationMask & MUTATION_MASK_OMEGA)
        rarityModifier = 95;   // Omega: 0.05% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_TITAN)
        rarityModifier = 80;   // Titan: 1% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_ASCENDIDO)
        rarityModifier = 70;   // Ascendido: 2% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_ANCESTRAL)
        rarityModifier = 60;   // Ancestral: 3% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_ENRAGED)
        rarityModifier = 50;   // Enraged: 4% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_OVERLOADED)
        rarityModifier = 40;   // Overloaded: 5% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_SUPER)
        rarityModifier = 30;   // Super: 6% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_HYPER)
        rarityModifier = 20;   // Hyper: 7% base
    else if (sPokeLocator.selectedMutationMask & MUTATION_MASK_ALPHA)
        rarityModifier = 10;   // Alpha: 8% base

    // Filtro shiny reduce la probabilidad adicional
    if (sPokeLocator.shinyFilter)
        rarityModifier += 90;  // Shiny: muy raro

    u16 targetOdds = 100 - rarityModifier;

    if (roll < targetOdds)
    {
        // ¡Éxito! Se encontró el objetivo, iniciar batalla
        sPokeLocator.isTracking = FALSE;
        PokeLocator_StartBattleWithSelected();
        return TRUE;
    }

    return FALSE; // Búsqueda falló, intentar de nuevo
}

// Calcular probabilidades de encuentro (en permille, es decir, 0-1000)
u16 PokeLocator_CalculateEncounterOdds(u16 species, u32 mutationMask)
{
    u16 baseOdds = 100;  // 10% base por especie

    // Ajustar según mutación
    if (mutationMask & MUTATION_MASK_OMEGA)
        baseOdds = 5;      // 0.5%
    else if (mutationMask & MUTATION_MASK_TITAN)
        baseOdds = 10;     // 1%
    else if (mutationMask & MUTATION_MASK_ASCENDIDO)
        baseOdds = 20;     // 2%
    else if (mutationMask & MUTATION_MASK_ANCESTRAL)
        baseOdds = 30;     // 3%
    else if (mutationMask & MUTATION_MASK_ENRAGED)
        baseOdds = 40;     // 4%
    else if (mutationMask & MUTATION_MASK_OVERLOADED)
        baseOdds = 50;     // 5%
    else if (mutationMask & MUTATION_MASK_SUPER)
        baseOdds = 60;     // 6%
    else if (mutationMask & MUTATION_MASK_HYPER)
        baseOdds = 70;     // 7%
    else if (mutationMask & MUTATION_MASK_ALPHA)
        baseOdds = 80;     // 8%

    // Shiny reduce drásticamente
    if (sPokeLocator.shinyFilter)
        baseOdds = baseOdds / 100;  // ~1/8192 aprox

    return baseOdds;
}