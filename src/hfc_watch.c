#include "global.h"
#include "hfc_watch.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include "text.h"
#include "window.h"
#include "script.h"

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
    FLAG_SYS_POKENAV_GET,   // POKENAV
    FLAG_POKELOCATOR_UNLOCKED, // POKELOCATOR
    FLAG_CRAFTING_UNLOCKED,  // CRAFTING
    FLAG_PC_UNLOCKED,        // PC
    FLAG_ANOMALY_ARCHIVE_UNLOCKED, // ANOMALY_ARCHIVE
    FLAG_MUTANT_SIGNAL_UNLOCKED, // MUTANT_SIGNAL
    FLAG_POKE_SIMULATOR_UNLOCKED, // POKE_SIMULATOR
    FLAG_RAIDS_UNLOCKED, // RAID_BATTLE
};

// Inicializar la pantalla de HFC Watch
static void CB2_InitHfcWatch(void)
{
    sHfcWatchState.currentSelection = 0;
    sHfcWatchState.isOpen = TRUE;
    sHfcWatchState.scrollOffset = 0;
    sHfcWatchState.visibleCards = 3;
    HfcWatch_UpdateOptionStates();
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
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}

// Manejo de entrada de usuario en HFC Watch
void HfcWatch_HandleInput(void)
{
    if (JOY_NEW(DPAD_UP))
    {
        if (sHfcWatchState.currentSelection > 0)
            sHfcWatchState.currentSelection--;
        else
            sHfcWatchState.currentSelection = HFC_WATCH_COUNT - 1;
        if (sHfcWatchState.currentSelection < sHfcWatchState.scrollOffset)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection;
        else if (sHfcWatchState.currentSelection >= sHfcWatchState.scrollOffset + sHfcWatchState.visibleCards)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection - sHfcWatchState.visibleCards + 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sHfcWatchState.currentSelection < HFC_WATCH_COUNT - 1)
            sHfcWatchState.currentSelection++;
        else
            sHfcWatchState.currentSelection = 0;
        if (sHfcWatchState.currentSelection < sHfcWatchState.scrollOffset)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection;
        else if (sHfcWatchState.currentSelection >= sHfcWatchState.scrollOffset + sHfcWatchState.visibleCards)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection - sHfcWatchState.visibleCards + 1;
        PlaySE(SE_SELECT);
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (HfcWatch_IsOptionUnlocked(sHfcWatchState.currentSelection))
        {
            HfcWatch_LaunchOption(sHfcWatchState.currentSelection);
        }
        else
        {
            PlaySE(SE_FAILURE);
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        CloseHfcWatch();
        PlaySE(SE_SELECT);
    }
}

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

bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option)
{
    u16 requiredFlag = sHfcWatchRequiredFlags[option];
    if (requiredFlag == 0)
        return TRUE;
    return FlagGet(requiredFlag);
}

void HfcWatch_LaunchOption(enum HfcWatchOption option)
{
    switch (option)
    {
        case HFC_WATCH_HORIZON:
            break;
        case HFC_WATCH_POKENAV:
            break;
        case HFC_WATCH_POKELOCATOR:
            OpenPokeLocatorFromHfcWatch();
            break;
        case HFC_WATCH_CRAFTING:
            break;
        case HFC_WATCH_PC:
            break;
        case HFC_WATCH_ANOMALY_ARCHIVE:
            break;
        case HFC_WATCH_MUTANT_SIGNAL:
            break;
        case HFC_WATCH_POKE_SIMULATOR:
            break;
        case HFC_WATCH_RAID_BATTLE:
            break;
        default: break;
    }
}

void HfcWatch_DrawCards(void)
{
    u8 i;
    u8 yPos = 40;
    for (i = sHfcWatchState.scrollOffset; i < HFC_WATCH_COUNT && i < sHfcWatchState.scrollOffset + sHfcWatchState.visibleCards; i++)
    {
        u8 cardY = yPos + (i - sHfcWatchState.scrollOffset) * 40;
        HfcWatch_DrawCard(i, 20, cardY);
    }
}

void HfcWatch_DrawCard(u8 index, u8 x, u8 y)
{
    u8 optionIndex = sHfcWatchState.scrollOffset + index;
    if (optionIndex >= HFC_WATCH_COUNT)
        return;

    enum HfcWatchOption option = (enum HfcWatchOption)optionIndex;
    enum HfcWatchOptionState state = sHfcWatchState.optionsState[optionIndex];

    if (sHfcWatchState.currentSelection == option)
    {
    }
}