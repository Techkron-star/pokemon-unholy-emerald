#ifndef GUARD_HFC_WATCH_H
#define GUARD_HFC_WATCH_H

#include "global.h"

// Número de opciones en el menú HFC Watch
#define HFC_WATCH_OPTION_COUNT 9

enum HfcWatchOption
{
    HFC_WATCH_HORIZON,      // Mercado Horizon
    HFC_WATCH_POKENAV,      // PokéNav
    HFC_WATCH_POKELOCATOR,  // Poké Locator
    HFC_WATCH_CRAFTING,     // Crafteo de Gears
    HFC_WATCH_PC,           // Sistema PC
    HFC_WATCH_ANOMALY_ARCHIVE, // Archivo de anomalías
    HFC_WATCH_MUTANT_SIGNAL, // Señal mutante
    HFC_WATCH_POKE_SIMULATOR, // Poké Simulator
    HFC_WATCH_RAID_BATTLE,   // Incursiones/Raids
    HFC_WATCH_COUNT,
};

// Estado de cada opción (desbloqueada o no)
enum HfcWatchOptionState
{
    HFC_WATCH_LOCKED,   // No disponible
    HFC_WATCH_UNLOCKED, // Disponible
    HFC_WATCH_ACTIVE,   // Actualmente en uso
};

// Estructura de una opción del menú HFC Watch
struct HfcWatchMenuEntry
{
    enum HfcWatchOption option;
    enum HfcWatchOptionState state;
    const u8 *name;          // Nombre mostrado
    const u8 *description;   // Descripción corta
    u8 requiredBadge;        // Insignia requerida (0 = sin requisito)
    u8 requiredFlag;         // Flag requerido (0 = sin requisito)
};

// Estado global de HFC Watch
struct HfcWatchState
{
    u8 currentSelection;     // Opción seleccionada (0-8)
    u8 isOpen;               // Si el menú está abierto
    u8 optionsState[HFC_WATCH_COUNT]; // Estado de cada opción
};

// Prototipos de funciones
void CB2_InitHfcWatch(void);
void OpenHfcWatchFromStartMenu(void);
void CloseHfcWatch(void);
void HfcWatch_HandleInput(void);
void HfcWatch_UpdateOptionStates(void);
bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option);
void HfcWatch_LaunchOption(enum HfcWatchOption option);

#endif // GUARD_HFC_WATCH_H