#include "global.h"
#include "horizon.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "data.h"
#include "menu.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "start_menu.h"
#include "text.h"
#include "window.h"

// Estado del mercado Horizon
static EWRAM_DATA struct HorizonState sHorizonState = {0};

// Ofertas de Pokémon (máximo 2 ofertas simultáneas)
static EWRAM_DATA struct HorizonPokemonOffer sHorizonPokemonOffers[2] = {0};

// Ofertas de MT (máximo 6 MTs)
static EWRAM_DATA struct HorizonDiscOffer sHorizonDiscOffers[6] = {0};

// Ofertas de ítems (máximo 5 objetos de RNG + 3 básicos)
static EWRAM_DATA struct HorizonItemOffer sHorizonItemOffers[8] = {0};

// Ofertas de huevos (máximo 5 huevos)
static EWRAM_DATA struct HorizonEggOffer sHorizonEggOffers[5] = {0};

// Ventanas de UI
static EWRAM_DATA u8 sHorizonWindowIds[5] = {0};

// Contador de frames para animación de pestañas
static EWRAM_DATA u16 sHorizonFrameCounter = 0;

// Inicializar pantalla de Horizon
static void CB2_InitHorizon(void)
{
    // Inicializar estado
    sHorizonState.currentTab = HORIZON_TAB_PKMN;
    sHorizonState.isOpen = TRUE;
    sHorizonState.rareStockAlert = FALSE;
    sHorizonState.pokemonOfferCount = 0;
    sHorizonState.discsOfferCount = 0;
    sHorizonState.itemsOfferCount = 0;
    sHorizonState.eggsOfferCount = 0;
    sHorizonFrameCounter = 0;

    // Cargar stock persistente desde variables de SaveBlock
    Horizon_UpdateStock();

    // Crear ventanas de UI (pestañas, paneles, etc.)
    // TODO: Implementar creación de ventanas con CreateWindowFromRect
    // TODO: Mostrar pestañas horizontales (27×3 tiles root, 27×14 panel inferior)

    SetMainCallback2(CB2_InitHorizon);
}

// Abrir Horizon desde el menú START
void OpenHorizonFromStartMenu(void)
{
    // Verificar si Horizon está disponible
    // TODO: Verificar flags de desbloqueo (ej. FLAG_HORIZON_UNLOCKED)

    SetMainCallback2(CB2_InitHorizon);
}

// Cerrar Horizon de forma segura
void CloseHorizon(void)
{
    // Liberar ventanas, ícono de huevo y paleta temporal
    // TODO: Destruir ventanas de UI creadas

    // Reconstruir el campo y el menú START
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}

// Manejo de entrada de usuario en Horizon
void Horizon_HandleInput(void)
{
    // Navegación L/R entre pestañas
    if (JOY_NEW(DPAD_LEFT) || JOY_NEW(L_BUTTON))
    {
        if (sHorizonState.currentTab > 0)
            sHorizonState.currentTab--;
        else
            sHorizonState.currentTab = HORIZON_TAB_COUNT - 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_RIGHT) || JOY_NEW(R_BUTTON))
    {
        if (sHorizonState.currentTab < HORIZON_TAB_COUNT - 1)
            sHorizonState.currentTab++;
        else
            sHorizonState.currentTab = 0;
        PlaySE(SE_SELECT);
    }

    // A para abrir/confirmar, B para regresar
    if (JOY_NEW(A_BUTTON))
    {
        // TODO: Abrir pestaña seleccionada o confirmar oferta
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        CloseHorizon();
        PlaySE(SE_SELECT);
    }
}

// Actualizar stock persistente basado en semillas
void Horizon_UpdateStock(void)
{
    // TODO: Leer semillas persistentes desde variables de SaveBlock
    // sHorizonState.pokemonOfferCount se basa en 0x40F7–0x40FE
    // sHorizonState.discsOfferCount se basa en 0x40B8 y 0x40BB
    // sHorizonState.itemsOfferCount se basa en 0x40B8 y 0x40BB
    // sHorizonState.eggsOfferCount se basa en 0x40DB, 0x40DC, 0x40E5, etc.

    // Por ahora inicializar contadores en 0 hasta conectar con SaveBlock
    sHorizonState.pokemonOfferCount = 0;
    sHorizonState.discsOfferCount = 0;
    sHorizonState.itemsOfferCount = 0;
    sHorizonState.eggsOfferCount = 0;
    sHorizonState.rareStockAlert = FALSE;
}

// Verificar si hay stock raro disponible
bool8 Horizon_HasRareStock(void)
{
    return sHorizonState.rareStockAlert;
}

// Establecer alerta de stock raro
void Horizon_SetRareStockAlert(bool8 set)
{
    sHorizonState.rareStockAlert = set;
}

// Obtener pestaña actual
u8 Horizon_GetCurrentTab(void)
{
    return sHorizonState.currentTab;
}

// Establecer pestaña actual
void Horizon_SetCurrentTab(u8 tab)
{
    if (tab < HORIZON_TAB_COUNT)
        sHorizonState.currentTab = tab;
}

// Incrementar contador de frames (llamado cada frame)
void Horizon_UpdateFrameCounter(void)
{
    sHorizonFrameCounter++;
}

// Obtener valor del contador de frames
u16 Horizon_GetFrameCounter(void)
{
    return sHorizonFrameCounter;
}