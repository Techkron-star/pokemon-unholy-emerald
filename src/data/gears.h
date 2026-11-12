// Tabla de Gears. Una entrada por constante de include/constants/gears.h.
// Guía para crear Gears: docs/GEARS.md
//
// Plantilla (copia, descomenta y rellena; los números son SIEMPRE decisión de diseño):
//
//     [GEAR_EJEMPLO] =
//     {
//         .name = COMPOUND_STRING("Nombre"),
//         .description = COMPOUND_STRING("Qué hace.\nSegunda línea."),
//         .iconGfx = sGearIconGfx_Ejemplo,
//         .iconPal = sGearIconPal_Ejemplo,
//         .trigger = GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE,
//         .cooldownTurns = X,
//         .typeMask = GEAR_TYPE(TYPE_DARK),
//         .overloadPowerPercent = X,
//         .residualPowerPercent = X,
//     },

static const u32 sGearIconGfx_VisorCosmico[] = INCGFX_U32("graphics/gears/visor_cosmico/icon.png", ".4bpp");
static const u16 sGearIconPal_VisorCosmico[] = INCGFX_U16("graphics/gears/visor_cosmico/icon.pal", ".gbapal");
static const u32 sGearIconGfx_CorazonTitan[] = INCGFX_U32("graphics/gears/corazon_titan/icon.png", ".4bpp");
static const u16 sGearIconPal_CorazonTitan[] = INCGFX_U16("graphics/gears/corazon_titan/icon.pal", ".gbapal");
static const u32 sGearIconGfx_SkiMask[] = INCGFX_U32("graphics/gears/ski_mask/icon.png", ".4bpp");
static const u16 sGearIconPal_SkiMask[] = INCGFX_U16("graphics/gears/ski_mask/icon.pal", ".gbapal");
static const u32 sGearIconGfx_CetroSiniestro[] = INCGFX_U32("graphics/gears/cetro_siniestro/icon.png", ".4bpp");
static const u16 sGearIconPal_CetroSiniestro[] = INCGFX_U16("graphics/gears/cetro_siniestro/icon.pal", ".gbapal");
static const u32 sGearIconGfx_ImpulsoCosmico[] = INCGFX_U32("graphics/gears/impulso_cosmico/icon.png", ".4bpp");
static const u16 sGearIconPal_ImpulsoCosmico[] = INCGFX_U16("graphics/gears/impulso_cosmico/icon.pal", ".gbapal");
static const u32 sGearIconGfx_PermafrostFragment[] = INCGFX_U32("graphics/gears/permafrost_fragment/icon.png", ".4bpp");
static const u16 sGearIconPal_PermafrostFragment[] = INCGFX_U16("graphics/gears/permafrost_fragment/icon.pal", ".gbapal");

const struct GearInfo gGearsInfo[GEAR_COUNT] =
{
    [GEAR_NONE] = {0},
    [GEAR_VISOR_COSMICO] =
    {
        .name = COMPOUND_STRING("Visor Cosmico"),
        .description = COMPOUND_STRING("Potencia ataques especiales.\nSobrecarga y recarga."),
        .iconGfx = sGearIconGfx_VisorCosmico,
        .iconPal = sGearIconPal_VisorCosmico,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .categoryMask = GEAR_CATEGORY_SPECIAL,
        .overloadPowerPercent = 120, // +20 % durante el turno de sobrecarga
        .residualPowerPercent = 110, // +10 % los tres turnos siguientes
    },
    [GEAR_CORAZON_TITAN] =
    {
        .name = COMPOUND_STRING("Corazon de Titan"),
        .description = COMPOUND_STRING("Potencia ataques fisicos.\nSobrecarga y recarga."),
        .iconGfx = sGearIconGfx_CorazonTitan,
        .iconPal = sGearIconPal_CorazonTitan,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .categoryMask = GEAR_CATEGORY_PHYSICAL,
        .overloadPowerPercent = 120,
        .residualPowerPercent = 110,
    },
    [GEAR_SKI_MASK] =
    {
        .name = COMPOUND_STRING("Ski Mask"),
        .description = COMPOUND_STRING("Siniestro: +30% cargado,\n+21% durante recarga."),
        .iconGfx = sGearIconGfx_SkiMask,
        .iconPal = sGearIconPal_SkiMask,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_DARK),
        .overloadPowerPercent = 130,
        .residualPowerPercent = 121,
    },
    [GEAR_CETRO_SINIESTRO] =
    {
        .name = COMPOUND_STRING("Cetro Siniestro"),
        .description = COMPOUND_STRING("Cambia efectividad\nde ataques Siniestros."),
        .iconGfx = sGearIconGfx_CetroSiniestro,
        .iconPal = sGearIconPal_CetroSiniestro,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_DARK),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
    [GEAR_IMPULSO_COSMICO] =
    {
        .name = COMPOUND_STRING("Impulso Cosmico"),
        .description = COMPOUND_STRING("Cambia efectividad\nde ataques Fantasma."),
        .iconGfx = sGearIconGfx_ImpulsoCosmico,
        .iconPal = sGearIconPal_ImpulsoCosmico,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_GHOST),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
    [GEAR_PERMAFROST_FRAGMENT] =
    {
        .name = COMPOUND_STRING("Permafrost Fragment"),
        .description = COMPOUND_STRING("Hielo: congela con mas\nfrecuencia; cura Hielo."),
        .iconGfx = sGearIconGfx_PermafrostFragment,
        .iconPal = sGearIconPal_PermafrostFragment,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_ICE),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
};
