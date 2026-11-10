// Tabla de Gears. Una entrada por constante de include/constants/gears.h.
// Guía para crear Gears: docs/GEARS.md
//
// El sistema está listo pero todavía no hay Gears definidos: solo existe GEAR_NONE.
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
};
