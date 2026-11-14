#include "global.h"
#include "gear_data.h"
#include "constants/items.h"
#include "constants/types.h"

// ---------------------------------------------------------------------------
// Plantillas de los 27 Gears de Unholy Emerald
// ---------------------------------------------------------------------------
// Notas de implementación:
//  - overloadPercent y residualPercent están en partes por mil (‰).
//    Ejemplo: 200 = +20 %, 100 = +10 %, 210 = +21 %.
//  - Para Gears Sangre, typeBoost indica el tipo de ataque que se beneficia.
//  - Para Cetro Siniestro / Visor Cósmico, effectivenessType* indica los
//    tipos objetivo que reciben ×2 de efectividad.
// ---------------------------------------------------------------------------

static const struct GearTemplate sGearTemplates[GEAR_TEMPLATE_COUNT] =
{
    // --- Gears únicos (nombrados) -----------------------------------------
    {
        .itemId        = ITEM_VISOR_COSMICO,
        .name          = gText_ItemVisitorCosmico,
        .effectType    = GEAR_EFFECT_EFFECTIVENESS,
        .typeBoost     = TYPE_NONE,
        .overloadPercent = 0,
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_GHOST,
        .effectivenessType2 = TYPE_NORMAL,
        .effectivenessType3 = TYPE_PSYCHIC,
        .effectivenessType4 = TYPE_FAIRY,
        .consumable   = FALSE,
        .alwaysActive = TRUE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_CORAZON_TITAN,
        .name          = gText_ItemCorazonTitan,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = GEAR_TYPE_PHYSICAL, // marca especial: ataques físicos
        .overloadPercent = 200,         // +20 % físico al sobrecargarse
        .residualPercent   = 100,       // +10 % residual
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SKI_MASK,
        .name          = gText_ItemSkiMask,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_DARK,
        .overloadPercent = 300,         // +30 % siniestro al sobrecargarse
        .residualPercent   = 210,       // +21 % constante durante recarga
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_CETRO_SINIESTRO,
        .name          = gText_ItemCetroSiniestro,
        .effectType    = GEAR_EFFECT_EFFECTIVENESS,
        .typeBoost     = TYPE_NONE,
        .overloadPercent = 0,
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_DARK,
        .effectivenessType2 = TYPE_PSYCHIC,
        .effectivenessType3 = TYPE_FAIRY,
        .effectivenessType4 = TYPE_NORMAL,
        .consumable   = FALSE,
        .alwaysActive = TRUE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_IMPULSO_COSMICO,
        .name          = gText_ItemImpulsoCosmico,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = GEAR_TYPE_SPECIAL, // marca especial: ataques especiales
        .overloadPercent = 200,        // +20 % especial al sobrecargarse
        .residualPercent   = 100,      // +10 % residual
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_PERMAFROST_FRAGMENT,
        .name          = gText_ItemPermafrostFragment,
        .effectType    = GEAR_EFFECT_STATUS_CHANCE | GEAR_EFFECT_HEAL_PER_TURN,
        .typeBoost     = TYPE_ICE,
        .overloadPercent = 0,
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 16,   // cura 1/16 de PS máx por turno si es tipo Hielo
    },
    {
        .itemId        = ITEM_TESERACTO,
        .name          = gText_ItemTesseracto,
        .effectType    = GEAR_EFFECT_CONSUMABLE,
        .typeBoost     = TYPE_NONE,
        .overloadPercent = 0,
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = TRUE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_ANCLA_REALIDAD,
        .name          = gText_ItemAnclaRealidad,
        .effectType    = GEAR_EFFECT_CONSUMABLE,
        .typeBoost     = TYPE_NONE,
        .overloadPercent = 0,
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = TRUE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_COLMILLO_RAYQUAZA,
        .name          = gText_ItemColmilloRayquaza,
        .effectType    = GEAR_EFFECT_STAT_BOOST_ON_KO,
        .typeBoost     = TYPE_FLYING | TYPE_DRAGON, // marca: Volador o Dragón
        .overloadPercent = 100,         // +10 % a estadísticas
        .residualPercent   = 0,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },

    // --- Gears Sangre (Inyecciones sanguíneas) — uno por tipo -------------
    {
        .itemId        = ITEM_SANGRE_NORMAL,
        .name          = gText_ItemSangreNormal,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_NORMAL,
        .overloadPercent = 400,         // gran bonus al primer ataque compatible
        .residualPercent   = 100,       // bonus residual menor
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_LUCHA,
        .name          = gText_ItemSangreLucha,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_FIGHTING,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_VOLADOR,
        .name          = gText_ItemSangreVolador,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_FLYING,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_VENENO,
        .name          = gText_ItemSangreVeneno,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_POISON,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_TIERRA,
        .name          = gText_ItemSangreTierra,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_GROUND,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_ROCA,
        .name          = gText_ItemSangreRoca,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_ROCK,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_BICHO,
        .name          = gText_ItemSangreBicho,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_BUG,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_FANTASMA,
        .name          = gText_ItemSangreFantasma,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_GHOST,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_ACERO,
        .name          = gText_ItemSangreAcero,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_STEEL,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_FUEGO,
        .name          = gText_ItemSangreFuego,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_FIRE,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_AGUA,
        .name          = gText_ItemSangreAgua,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_WATER,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_PLANTA,
        .name          = gText_ItemSangrePlanta,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_GRASS,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_ELECTRICO,
        .name          = gText_ItemSangreElectrico,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_ELECTRIC,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_PSIQUICO,
        .name          = gText_ItemSangrePsiquico,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_PSYCHIC,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_HIELO,
        .name          = gText_ItemSangreHielo,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_ICE,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_DRAGON,
        .name          = gText_ItemSangreDragon,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_DRAGON,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_SINIESTRO,
        .name          = gText_ItemSangreSiniestro,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_DARK,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
    {
        .itemId        = ITEM_SANGRE_HADA,
        .name          = gText_ItemSangreHada,
        .effectType    = GEAR_EFFECT_TYPE_BOOST,
        .typeBoost     = TYPE_FAIRY,
        .overloadPercent = 400,
        .residualPercent   = 100,
        .effectivenessType1 = TYPE_NONE,
        .effectivenessType2 = TYPE_NONE,
        .effectivenessType3 = TYPE_NONE,
        .effectivenessType4 = TYPE_NONE,
        .consumable   = FALSE,
        .alwaysActive = FALSE,
        .healFraction = 0,
    },
};

const struct GearTemplate *Gear_GetTemplate(u16 itemId)
{
    u8 i;
    for (i = 0; i < GEAR_TEMPLATE_COUNT; i++)
    {
        if (sGearTemplates[i].itemId == itemId)
            return &sGearTemplates[i];
    }
    return NULL;
}

bool8 Gear_IsGearItem(u16 itemId)
{
    return Gear_GetTemplate(itemId) != NULL;
}

bool8 Gear_IsBloodGear(u16 itemId)
{
    return (itemId >= ITEM_SANGRE_NORMAL && itemId <= ITEM_SANGRE_HADA);
}

u16 Gear_GetBloodGearType(u16 itemId)
{
    const struct GearTemplate *tpl = Gear_GetTemplate(itemId);
    if (tpl != NULL && Gear_IsBloodGear(itemId))
        return tpl->typeBoost;
    return TYPE_NONE;
}