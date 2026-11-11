#ifndef GUARD_HORIZON_H
#define GUARD_HORIZON_H

#include "global.h"

// Número de pestañas horizontales en Horizon
#define HORIZON_TAB_COUNT 5

enum HorizonTab
{
    HORIZON_TAB_PKMN,    // Pokémon en oferta (con mutaciones)
    HORIZON_TAB_DISCS,   // MT (Movimientos Técnicos)
    HORIZON_TAB_ITEMS,   // Ítems básicos y de RNG
    HORIZON_TAB_EGGS,    // Huevos de diferentes calidades
    HORIZON_TAB_MOVES,   // Catálogo de movimientos
};

// Oferta de Pokémon en Horizon
struct HorizonPokemonOffer
{
    u16 species;           // Especie del Pokémon
    u16 level;             // Nivel
    u32 mutationMask;      // Máscara de mutaciones activas
    u32 price;             // Precio en pokedólares
    u8 stock;              // Stock disponible (1 = una plaza)
    u8 flags;              // Banderas de oferta (raro, etc.)
};

// Oferta de MT en Horizon
struct HorizonDiscOffer
{
    u16 itemId;            // ID del MT
    u8 stock;              // Stock disponible
    u8 flags;              // Banderas de oferta
};

// Oferta de ítem en Horizon
struct HorizonItemOffer
{
    u16 itemId;            // ID del ítem
    u8 stock;              // Stock disponible
    u8 flags;              // Banderas de oferta
};

// Oferta de huevo en Horizon
struct HorizonEggOffer
{
    u8 qualityRank;        // Calidad: 0=Básico, 1=Medio, 2=Alto, 3=Mítico, 4=Legendario
    u8 eggQualityRoll;     // Roll de calidad: Estándar / Radiante / Prismática
    u16 price;             // Precio en pokedólares
    u8 stock;              // Stock disponible
};

// Estado del mercado Horizon
struct HorizonState
{
    u8 currentTab;         // Pestaña actual (0-4)
    u8 isOpen;             // Si Horizon está abierto
    u8 rareStockAlert;     // Alerta de stock raro (punto rojo)
    u16 pokemonOfferCount; // Número de ofertas de Pokémon activas
    u16 discsOfferCount;   // Número de ofertas de MT activas
    u16 itemsOfferCount;   // Número de ofertas de ítems activos
    u16 eggsOfferCount;    // Número de ofertas de huevos activos
};

// Prototipos de funciones
void CB2_InitHorizon(void);
void OpenHorizonFromStartMenu(void);
void CloseHorizon(void);
void Horizon_HandleInput(void);
void Horizon_UpdateStock(void);
bool8 Horizon_HasRareStock(void);
void Horizon_SetRareStockAlert(bool8 set);
u8 Horizon_GetCurrentTab(void);
void Horizon_SetCurrentTab(u8 tab);

#endif // GUARD_HORIZON_H