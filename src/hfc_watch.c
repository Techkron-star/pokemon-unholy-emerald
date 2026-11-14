#include "global.h"
#include "hfc_watch.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "menu.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "text.h"
#include "window.h"

// Estado del menú HFC Watch
static EWRAM_DATA struct HfcWatchState sHfcWatchState = {0};

// Nombres de las opciones del menú
static const u8 *sHfcWatchOptionNames[HFC_WATCH_COUNT] =
{
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
static const u8 *sHfcWatchOptionDescriptions[HFC_WATCH_COUNT] =
{
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

// Inicializar la pantalla de HFC Watch
static void CB2_InitHfcWatch(void)
{
    sHfcWatchState.currentSelection = 0;
    sHfcWatchState.isOpen = TRUE;
    sHfcWatchState.scrollOffset = 0;
    sHfcWatchState.visibleCards = 3;
    sHfcWatchState.watchTier = HfcWatch_GetWatchTier();
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

// Determinar el nivel de reloj actual basado en los flags desbloqueados
enum HfcWatchTier HfcWatch_GetWatchTier(void)
{
    if (FlagGet(FLAG_RAIDS_UNLOCKED) || FlagGet(FLAG_POKE_SIMULATOR_UNLOCKED)
     || FlagGet(FLAG_MUTANT_SIGNAL_UNLOCKED) || FlagGet(FLAG_ANOMALY_ARCHIVE_UNLOCKED))
    {
        return HFC_WATCH_TIER_HF;
    }
    if (FlagGet(FLAG_POKELOCATOR_UNLOCKED) || FlagGet(FLAG_CRAFTING_UNLOCKED)
     || FlagGet(FLAG_PC_UNLOCKED))
    {
        return HFC_WATCH_TIER_SUPER;
    }
    if (FlagGet(FLAG_HORIZON_UNLOCKED) || FlagGet(FLAG_SYS_POKENAV_GET))
    {
        return HFC_WATCH_TIER_POKE;
    }
    return HFC_WATCH_TIER_NONE;
}

// Establecer el nivel de reloj (llamado al obtener un reloj nuevo)
// El reloj nuevo siempre sustituye al anterior (progresión lineal)
void HfcWatch_SetWatchTier(enum HfcWatchTier tier)
{
    // El tier se deriva dinámicamente de los flags, por lo que esta función
    // existe por compatibilidad pero el valor real se calcula en HfcWatch_GetWatchTier.
}

// Manejo de entrada de usuario en HFC Watch
void HfcWatch_HandleInput(void)
{
    if (sHfcWatchState.visibleCount == 0)
    {
        if (JOY_NEW(B_BUTTON))
        {
            CloseHfcWatch();
            PlaySE(SE_SELECT);
        }
        return;
    }

    if (JOY_NEW(DPAD_UP))
    {
        if (sHfcWatchState.currentSelection > 0)
            sHfcWatchState.currentSelection--;
        else
            sHfcWatchState.currentSelection = sHfcWatchState.visibleCount - 1;

        if (sHfcWatchState.currentSelection < sHfcWatchState.scrollOffset)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection;
        else if (sHfcWatchState.currentSelection >= sHfcWatchState.scrollOffset + sHfcWatchState.visibleCards)
            sHfcWatchState.scrollOffset = sHfcWatchState.currentSelection - sHfcWatchState.visibleCards + 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sHfcWatchState.currentSelection < sHfcWatchState.visibleCount - 1)
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
        u8 realIndex = sHfcWatchState.visibleIndices[sHfcWatchState.currentSelection];
        if (sHfcWatchState.optionsState[realIndex] == HFC_WATCH_UNLOCKED)
        {
            HfcWatch_LaunchOption((enum HfcWatchOption)realIndex);
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

// Actualizar estados de las opciones: solo mostrar las del tier actual o inferior
void HfcWatch_UpdateOptionStates(void)
{
    u8 i;
    u8 visibleIndex = 0;

    sHfcWatchState.watchTier = HfcWatch_GetWatchTier();

    for (i = 0; i < HFC_WATCH_COUNT; i++)
    {
        // Si la opción requiere un tier superior al actual, está oculta (no se muestra)
        if (sHfcWatchMinTier[i] > sHfcWatchState.watchTier)
        {
            sHfcWatchState.optionsState[i] = HFC_WATCH_HIDDEN;
            continue;
        }

        // Si tiene un flag adicional y no está activado, está oculta (no se muestra como locked)
        if (sHfcWatchRequiredFlags[i] != 0 && !FlagGet(sHfcWatchRequiredFlags[i]))
        {
            sHfcWatchState.optionsState[i] = HFC_WATCH_HIDDEN;
            continue;
        }

        sHfcWatchState.optionsState[i] = HFC_WATCH_UNLOCKED;
        sHfcWatchState.visibleIndices[visibleIndex] = i;
        visibleIndex++;
    }

    sHfcWatchState.visibleCount = visibleIndex;

    // Ajustar selección si se salió de rango
    if (sHfcWatchState.currentSelection >= sHfcWatchState.visibleCount)
        sHfcWatchState.currentSelection = (sHfcWatchState.visibleCount > 0) ? sHfcWatchState.visibleCount - 1 : 0;

    // Ajustar scroll
    if (sHfcWatchState.scrollOffset >= sHfcWatchState.visibleCount)
        sHfcWatchState.scrollOffset = (sHfcWatchState.visibleCount > 0) ? sHfcWatchState.visibleCount - sHfcWatchState.visibleCards : 0;
    if (sHfcWatchState.scrollOffset > sHfcWatchState.visibleCount - sHfcWatchState.visibleCards)
        sHfcWatchState.scrollOffset = sHfcWatchState.visibleCount - sHfcWatchState.visibleCards;
    if (sHfcWatchState.scrollOffset < 0)
        sHfcWatchState.scrollOffset = 0;
}

bool8 HfcWatch_IsOptionUnlocked(enum HfcWatchOption option)
{
    return sHfcWatchState.optionsState[option] == HFC_WATCH_UNLOCKED;
}

// Lanza la opción seleccionada
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

// Dibujar las cards visibles
void HfcWatch_DrawCards(void)
{
    u8 i;
    u8 yPos = 40;

    for (i = sHfcWatchState.scrollOffset; i < sHfcWatchState.scrollOffset + sHfcWatchState.visibleCards && i < sHfcWatchState.visibleCount; i++)
    {
        u8 realIndex = sHfcWatchState.visibleIndices[i];
        u8 cardY = yPos + (i - sHfcWatchState.scrollOffset) * 56;
        HfcWatch_DrawCard(realIndex, 20, cardY, i == sHfcWatchState.currentSelection);
    }
}

// Dibujar una tarjeta individual
void HfcWatch_DrawCard(u8 optionIndex, u8 x, u8 y, bool8 isSelected)
{
    u8 windowId;

    if (optionIndex >= HFC_WATCH_COUNT)
        return;

    // Crear ventana para la card
    windowId = AddWindow(&(struct WindowTemplate){
        .bg = 0,
        .tilemapLeft = x / 8,
        .tilemapTop = y / 8,
        .width = 26,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x100
    });

    DrawStdWindowFrame(windowId, FALSE);

    // Imprimir nombre
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sHfcWatchOptionNames[optionIndex], 8, 1, TEXT_SKIP_DRAW, NULL);

    // Imprimir descripción
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sHfcWatchOptionDescriptions[optionIndex], 8, 17, TEXT_SKIP_DRAW, NULL);

    // Si está seleccionada, agregar flecha
    if (isSelected)
    {
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_SelectorArrow, 0, 1, TEXT_SKIP_DRAW, NULL);
    }

    CopyWindowToVram(windowId, COPYWIN_FULL);
}