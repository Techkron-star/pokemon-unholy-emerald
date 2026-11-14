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

// Niveles del reloj (cada nivel sustituye al anterior)
enum HfcWatchTier
{
    HFC_WATCH_TIER_NONE,      // Sin reloj todavía
    HFC_WATCH_TIER_POKE,      // Poké Watch (Horizon, PokéNav)
    HFC_WATCH_TIER_SUPER,     // Super Poké Watch (+ PokéLocator, PC, Crafting)
    HFC_WATCH_TIER_HF,        // HF Poké Watch (+ AnomalyArchive, MutantSignal, PokeSimulator, RaidBattle)
    HFC_WATCH_TIER_COUNT,
};

// Estado de cada opción del menú
enum HfcWatchOptionState
{
    HFC_WATCH_HIDDEN,   // No existe para este nivel de reloj
    HFC_WATCH_LOCKED,   // Existe pero bloqueada por flag
    HFC_WATCH_UNLOCKED, // Disponible
    HFC_WATCH_ACTIVE,   // Actualmente en uso
};

// Opciones del menú HFC Watch
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

// Nivel mínimo de reloj requerido para cada opción
static const enum HfcWatchTier sHfcWatchMinTier[HFC_WATCH_COUNT] =
{
    HFC_WATCH_TIER_POKE,      // HORIZON - Poké Watch
    HFC_WATCH_TIER_POKE,      // POKENAV - Poké Watch
    HFC_WATCH_TIER_SUPER,     // POKELOCATOR - Super Poké Watch
    HFC_WATCH_TIER_SUPER,     // CRAFTING - Super Poké Watch
    HFC_WATCH_TIER_SUPER,     // PC - Super Poké Watch
    HFC_WATCH_TIER_HF,        // ANOMALY_ARCHIVE - HF Poké Watch
    HFC_WATCH_TIER_HF,        // MUTANT_SIGNAL - HF Poké Watch
    HFC_WATCH_TIER_HF,        // POKE_SIMULATOR - HF Poké Watch
    HFC_WATCH_TIER_HF,        // RAID_BATTLE - HF Poké Watch
};

// Banderas requeridas para cada opción (0 = sin requisito especial además del tier)
static const u16 sHfcWatchRequiredFlags[HFC_WATCH_COUNT] =
{
    FLAG_HORIZON_UNLOCKED,   // HORIZON
    FLAG_SYS_POKENAV_GET,   // POKENAV
    FLAG_POKELOCATOR_UNLOCKED, // POKELOCATOR
    FLAG_CRAFTING_UNLOCKED,  // CRAFTING
    FLAG_PC_UNLOCKED,        // PC
    FLAG_ANOMALY_ARCHIVE_UNLOCKED, // ANOMALY_ARCHIVE
    FLAG_MUTANT_SIGNAL_UNLOCKED, // MUTANT_SIGNAL
    FLAG_POKE_SIMULATOR_UNLOCKED, // POKE_SIMULATOR
    FLAG_RAIDS_UNLOCKED, // RAID_BATTLE
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
    u8 currentSelection;     // Opción seleccionada (índice en la lista visible)
    u8 isOpen;               // Si el menú está abierto
    enum HfcWatchTier watchTier;  // Nivel actual del reloj
    u8 optionsState[HFC_WATCH_COUNT]; // Estado de cada opción
    u8 visibleCards;         // Cuántas tarjetas se muestran a la vez
    u8 scrollOffset;         // Desplazamiento de tarjetas
    u8 visibleCount;         // Cuántas opciones están visibles (no ocultas)
    u8 visibleIndices[HFC_WATCH_COUNT]; // Mapeo: índice visible -> opción real
};

// Prototipos de funciones
void CB2_InitHfcWatch(void);
void OpenHfcWatchFromStartMenu(void);
void CloseHfcWatch(void);
void HfcWatch_HandleInput(void);
void HfcWatch_UpdateOptionStates(void);
bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option);
enum HfcWatchTier HfcWatch_GetWatchTier(void);
void HfcWatch_SetWatchTier(enum HfcWatchTier tier);
void HfcWatch_LaunchOption(enum HfcWatchOption option);
void HfcWatch_DrawCards(void);
void HfcWatch_DrawCard(u8 index, u8 x, u8 y, bool8 isSelected);

#endif // GUARD_HFC_WATCH_H