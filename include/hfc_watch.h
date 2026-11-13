#ifndef GUARD_HFC_WATCH_H
#define GUARD_HFC_WATCH_H

#include "global.h"

// Declaraciones de strings de texto (definidos en data/text/hfc_watch.inc)
extern const u8 gText_HfcWatch_Horizon[];
extern const u8 gText_HfcWatch_HorizonDesc[];
extern const u8 gText_HfcWatch_PokeNav[];
extern const u8 gText_HfcWatch_PokeNavDesc[];
extern const u8 gText_HfcWatch_PokeLocator[];
extern const u8 gText_HfcWatch_PokeLocatorDesc[];
extern const u8 gText_HfcWatch_Crafting[];
extern const u8 gText_HfcWatch_CraftingDesc[];
extern const u8 gText_HfcWatch_PC[];
extern const u8 gText_HfcWatch_PCDesc[];
extern const u8 gText_HfcWatch_AnomalyArchive[];
extern const u8 gText_HfcWatch_AnomalyArchiveDesc[];
extern const u8 gText_HfcWatch_MutantSignal[];
extern const u8 gText_HfcWatch_MutantSignalDesc[];
extern const u8 gText_HfcWatch_PokeSimulator[];
extern const u8 gText_HfcWatch_PokeSimulatorDesc[];
extern const u8 gText_HfcWatch_RaidBattle[];
extern const u8 gText_HfcWatch_RaidBattleDesc[];

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
    const u8 *name;          // Nombre mostrado en tarjeta
    const u8 *description;   // Descripción corta en tarjeta
    u8 requiredBadge;        // Insignia requerida (0 = sin requisito)
    u8 requiredFlag;         // Flag requerido (0 = sin requisito)
    u8 iconSpriteId;         // ID del sprite del ícono de tarjeta
};

// Estado global de HFC Watch
struct HfcWatchState
{
    u8 currentSelection;     // Opción seleccionada (0-8)
    u8 isOpen;               // Si el menú está abierto
    u8 optionsState[HFC_WATCH_COUNT]; // Estado de cada opción
    u8 scrollOffset;         // Desplazamiento de tarjetas
    u8 visibleCards;         // Cuántas tarjetas se muestran a la vez
};

// Prototipos de funciones
void CB2_InitHfcWatch(void);
void OpenHfcWatchFromStartMenu(void);
void CloseHfcWatch(void);
void HfcWatch_HandleInput(void);
void HfcWatch_UpdateOptionStates(void);
bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option);
void HfcWatch_LaunchOption(enum HfcWatchOption option);
void HfcWatch_DrawCards(void);
void HfcWatch_DrawCard(u8 index, u8 x, u8 y);

#endif // GUARD_HFC_WATCH_H