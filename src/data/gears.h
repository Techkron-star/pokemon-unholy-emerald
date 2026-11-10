// Tabla de Gears. Una entrada por constante de include/constants/gears.h.
// Guía para crear Gears: docs/GEARS.md
//
// Primer grupo confirmado (10/11/2026). El HUD usa iconos NULL hasta que
// se prepare el arte 16x32; esto no afecta a los efectos del Gear.
//
// Plantilla (copia, descomenta y rellena; los números son SIEMPRE decisión de diseño):
//
//     [GEAR_EJEMPLO] =
//     {
//         .name = COMPOUND_STRING("Nombre"),
//         .description = COMPOUND_STRING("Qué hace.\nSegunda línea."),
//         .iconGfx = INCBIN_U32("graphics/gears/ejemplo/icon.4bpp"),
//         .iconPal = INCBIN_U16("graphics/gears/ejemplo/icon.gbapal"),
//         .trigger = GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE,
//         .cooldownTurns = X,
//         .typeMask = GEAR_TYPE(TYPE_DARK),
//         .overloadPowerPercent = X,
//         .residualPowerPercent = X,
//     },

const struct GearInfo gGearsInfo[GEAR_COUNT] =
{
    [GEAR_NONE] = {0},
    [GEAR_VISOR_COSMICO] =
    {
        .name = COMPOUND_STRING("Visor Cosmico"),
        .description = COMPOUND_STRING("Potencia ataques especiales.\nSobrecarga y recarga."),
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
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_ICE),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
};
