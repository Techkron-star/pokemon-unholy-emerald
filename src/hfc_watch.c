#include "global.h"
#include "hfc_watch.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include "text.h"
#include "window.h"

// Estado del menú HFC Watch
static EWRAM_DATA struct HfcWatchState sHfcWatchState = {0};

// Nombres de las opciones del menú
static const u8 *sHfcWatchOptionNames[HFC_WATCH_COUNT] = {
    gText_HfcWatch_Horizon,
    gText_HfcWatch_PokeNav,
    gText_HfcWatch_PokeLocator,
    gText_HfcWatch_Crafting,
    gText_HfcWatch_PC,
    gText_HfcWatch_AnomalyArchive,
    gText_HfcWatch_MutantSignal,
    gText_HfcWatch_PokeSimulator,
    gText_HfcWatch_RaidBattle,
};

// Descripciones de las opciones
static const u8 *sHfcWatchOptionDescriptions[HFC_WATCH_COUNT] = {
    gText_HfcWatch_HorizonDesc,
    gText_HfcWatch_PokeNavDesc,
    gText_HfcWatch_PokeLocatorDesc,
    gText_HfcWatch_CraftingDesc,
    gText_HfcWatch_PCDesc,
    gText_HfcWatch_AnomalyArchiveDesc,
    gText_HfcWatch_MutantSignalDesc,
    gText_HfcWatch_PokeSimulatorDesc,
    gText_HfcWatch_RaidBattleDesc,
};

// Banderas requeridas para cada opción (0 = sin requisito especial)
static const u16 sHfcWatchRequiredFlags[HFC_WATCH_COUNT] = {
    FLAG_HORIZON_UNLOCKED,   // HORIZON
    FLAG_POKENAV_UNLOCKED,   // POKENAV
    FLAG_POKELOCATOR_UNLOCKED, // POKELOCATOR
    FLAG_CRAFTING_UNLOCKED,  // CRAFTING
    FLAG_PC_UNLOCKED,        // PC
    FLAG_ANOMALY_ARCHIVE_UNLOCKED, // ANOMALY_ARCHIVE
    FLAG_MUTANT_SIGNAL_UNLOCKED, // MUTANT_SIGNAL
    FLAG_POKE_SIMULATOR_UNLOCKED, // POKE_SIMULATOR
    FLAG_RAID_BATTLE_UNLOCKED, // RAID_BATTLE
};

// Inicializar la pantalla de HFC Watch
static void CB2_InitHfcWatch(void)
{
    // Inicializar estado
    sHfcWatchState.currentSelection = 0;
    sHfcWatchState.isOpen = TRUE;

    // Actualizar estados de las opciones
    HfcWatch_UpdateOptionStates();

    // Crear ventanas de UI (pestañas, paneles, lista de opciones)
    // TODO: Implementar creación de ventanas con CreateWindowFromRect

    SetMainCallback2(CB2_InitHfcWatch);
}

// Abrir HFC Watch desde el menú START
void OpenHfcWatchFromStartMenu(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

// Cerrar HFC Watch de forma segura
void CloseHfcWatch(void)
{
    // Liberar ventanas de UI creadas
    // TODO: Destruir ventanas

    // Reconstruir el campo y el menú START
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}

// Manejo de entrada de usuario en HFC Watch
void HfcWatch_HandleInput(void)
{
    // Navegación arriba/abajo entre opciones
    if (JOY_NEW(DPAD_UP))
    {
        if (sHfcWatchState.currentSelection > 0)
            sHfcWatchState.currentSelection--;
        else
            sHfcWatchState.currentSelection = HFC_WATCH_COUNT - 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sHfcWatchState.currentSelection < HFC_WATCH_COUNT - 1)
            sHfcWatchState.currentSelection++;
        else
            sHfcWatchState.currentSelection = 0;
        PlaySE(SE_SELECT);
    }

    // A para seleccionar opción
    if (JOY_NEW(A_BUTTON))
    {
        if (HfcWatch_IsOptionUnlocked(sHfcWatchState.currentSelection))
        {
            HfcWatch_LaunchOption(sHfcWatchState.currentSelection);
        }
        else
        {
            PlaySE(SE_FAILURE); // Sonido de acción no disponible
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        CloseHfcWatch();
        PlaySE(SE_SELECT);
    }
}

// Actualizar estados de las opciones (desbloqueadas/bloqueadas)
void HfcWatch_UpdateOptionStates(void)
{
    u8 i;
    for (i = 0; i < HFC_WATCH_COUNT; i++)
    {
        sHfcWatchState.optionsState[i] = HfcWatch_IsOptionUnlocked((enum HfcWatchOption)i)
            ? HFC_WATCH_UNLOCKED
            : HFC_WATCH_LOCKED;
    }
}

// Verificar si una opción está desbloqueada
bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option)
{
    u16 requiredFlag = sHfcWatchRequiredFlags[option];

    if (requiredFlag == 0)
        return TRUE;

    return FlagGet(requiredFlag);
}

// Lanzar una opción seleccionada
void HfcWatch_LaunchOption(enum HfcWatchOption option)
{
    switch (option)
    {
        case HFC_WATCH_HORIZON:
            // TODO: Llamar a OpenHorizonFromStartMenu() o equivalente
            break;
        case HFC_WATCH_POKENAV:
            // TODO: Abrir PokéNav
            break;
        case HFC_WATCH_POKELOCATOR:
            // TODO: Abrir Poké Locator
            break;
        case HFC_WATCH_CRAFTING:
            // TODO: Abrir menú de crafteo
            break;
        case HFC_WATCH_PC:
            // TODO: Abrir PC
            break;
        case HFC_WATCH_ANOMALY_ARCHIVE:
            // TODO: Abrir Archivo de anomalías
            break;
        case HFC_WATCH_MUTANT_SIGNAL:
            // TODO: Abrir Señal mutante
            break;
        case HFC_WATCH_POKE_SIMULATOR:
            // TODO: Abrir Poké Simulator
            break;
        case HFC_WATCH_RAID_BATTLE:
            // TODO: Abrir Incursiones
            break;
        default:
            break;
    }
}