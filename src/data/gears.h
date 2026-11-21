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

static const u32 sBloodGearIcon[] = INCGFX_U32("graphics/gears/blood_vial/icon.png", ".4bpp");

static const u32 sStormStaffIcon[] = INCGFX_U32("graphics/gears/storm_staff/icon.png", ".4bpp");
static const u32 sShackleIcon[] = INCGFX_U32("graphics/gears/shackle/icon.png", ".4bpp");
static const u32 sRecentGearIcon_EscudoAegis[] = INCGFX_U32("graphics/gears/escudo_aegis/icon.png", ".4bpp");
static const u16 sRecentGearPalette_EscudoAegis[] = INCGFX_U16("graphics/gears/escudo_aegis/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_ArieteNautilo[] = INCGFX_U32("graphics/gears/ariete_nautilo/icon.png", ".4bpp");
static const u16 sRecentGearPalette_ArieteNautilo[] = INCGFX_U16("graphics/gears/ariete_nautilo/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_GuantesCohete[] = INCGFX_U32("graphics/gears/guantes_cohete/icon.png", ".4bpp");
static const u16 sRecentGearPalette_GuantesCohete[] = INCGFX_U16("graphics/gears/guantes_cohete/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_TunicaRunica[] = INCGFX_U32("graphics/gears/tunica_runica/icon.png", ".4bpp");
static const u16 sRecentGearPalette_TunicaRunica[] = INCGFX_U16("graphics/gears/tunica_runica/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_CartelBusqueda[] = INCGFX_U32("graphics/gears/cartel_busqueda/icon.png", ".4bpp");
static const u16 sRecentGearPalette_CartelBusqueda[] = INCGFX_U16("graphics/gears/cartel_busqueda/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_NihilEngine[] = INCGFX_U32("graphics/gears/nihil_engine/icon.png", ".4bpp");
static const u16 sRecentGearPalette_NihilEngine[] = INCGFX_U16("graphics/gears/nihil_engine/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_AnilloFeerico[] = INCGFX_U32("graphics/gears/anillo_feerico/icon.png", ".4bpp");
static const u16 sRecentGearPalette_AnilloFeerico[] = INCGFX_U16("graphics/gears/anillo_feerico/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_PiedraAfilar[] = INCGFX_U32("graphics/gears/piedra_afilar/icon.png", ".4bpp");
static const u16 sRecentGearPalette_PiedraAfilar[] = INCGFX_U16("graphics/gears/piedra_afilar/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_BateriaReserva[] = INCGFX_U32("graphics/gears/bateria_reserva/icon.png", ".4bpp");
static const u16 sRecentGearPalette_BateriaReserva[] = INCGFX_U16("graphics/gears/bateria_reserva/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_BotasUmbral[] = INCGFX_U32("graphics/gears/botas_umbral/icon.png", ".4bpp");
static const u16 sRecentGearPalette_BotasUmbral[] = INCGFX_U16("graphics/gears/botas_umbral/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_VeloCriptico[] = INCGFX_U32("graphics/gears/velo_criptico/icon.png", ".4bpp");
static const u16 sRecentGearPalette_VeloCriptico[] = INCGFX_U16("graphics/gears/velo_criptico/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_AmuletoInquebrantable[] = INCGFX_U32("graphics/gears/amuleto_inquebrantable/icon.png", ".4bpp");
static const u16 sRecentGearPalette_AmuletoInquebrantable[] = INCGFX_U16("graphics/gears/amuleto_inquebrantable/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_CrisolEspiritual[] = INCGFX_U32("graphics/gears/crisol_espiritual/icon.png", ".4bpp");
static const u16 sRecentGearPalette_CrisolEspiritual[] = INCGFX_U16("graphics/gears/crisol_espiritual/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_LenteCerteza[] = INCGFX_U32("graphics/gears/lente_certeza/icon.png", ".4bpp");
static const u16 sRecentGearPalette_LenteCerteza[] = INCGFX_U16("graphics/gears/lente_certeza/icon.pal", ".gbapal");
static const u32 sRecentGearIcon_ModuloVectorial[] = INCGFX_U32("graphics/gears/modulo_vectorial/icon.png", ".4bpp");
static const u16 sRecentGearPalette_ModuloVectorial[] = INCGFX_U16("graphics/gears/modulo_vectorial/icon.pal", ".gbapal");
static const u32 sNewGearIcon_Amplificador[] = INCGFX_U32("graphics/gears/amplificador/icon.png", ".4bpp");
static const u16 sNewGearPalette_Amplificador[] = INCGFX_U16("graphics/gears/amplificador/icon.pal", ".gbapal");
static const u32 sNewGearIcon_Headphones[] = INCGFX_U32("graphics/gears/headphones/icon.png", ".4bpp");
static const u16 sNewGearPalette_Headphones[] = INCGFX_U16("graphics/gears/headphones/icon.pal", ".gbapal");
static const u32 sNewGearIcon_BotonPanico[] = INCGFX_U32("graphics/gears/boton_panico/icon.png", ".4bpp");
static const u16 sNewGearPalette_BotonPanico[] = INCGFX_U16("graphics/gears/boton_panico/icon.pal", ".gbapal");
static const u32 sNewGearIcon_NucleoHydra[] = INCGFX_U32("graphics/gears/nucleo_hydra/icon.png", ".4bpp");
static const u16 sNewGearPalette_NucleoHydra[] = INCGFX_U16("graphics/gears/nucleo_hydra/icon.pal", ".gbapal");
static const u32 sNewGearIcon_CascoHoplita[] = INCGFX_U32("graphics/gears/casco_hoplita/icon.png", ".4bpp");
static const u16 sNewGearPalette_CascoHoplita[] = INCGFX_U16("graphics/gears/casco_hoplita/icon.pal", ".gbapal");

static const u32 sArtifactIcon_ArmaduraMeteorica[] = INCGFX_U32("graphics/gears/armadura_meteorica/icon.png", ".4bpp");
static const u16 sArtifactPal_ArmaduraMeteorica[] = INCGFX_U16("graphics/gears/armadura_meteorica/icon.pal", ".gbapal");
static const u32 sArtifactIcon_EscudoPaladin[] = INCGFX_U32("graphics/gears/escudo_paladin/icon.png", ".4bpp");
static const u16 sArtifactPal_EscudoPaladin[] = INCGFX_U16("graphics/gears/escudo_paladin/icon.pal", ".gbapal");
static const u32 sArtifactIcon_EstandarteGuerra[] = INCGFX_U32("graphics/gears/estandarte_guerra/icon.png", ".4bpp");
static const u16 sArtifactPal_EstandarteGuerra[] = INCGFX_U16("graphics/gears/estandarte_guerra/icon.pal", ".gbapal");
static const u32 sArtifactIcon_TorreVigilancia[] = INCGFX_U32("graphics/gears/torre_vigilancia/icon.png", ".4bpp");
static const u16 sArtifactPal_TorreVigilancia[] = INCGFX_U16("graphics/gears/torre_vigilancia/icon.pal", ".gbapal");
static const u32 sArtifactIcon_Faro[] = INCGFX_U32("graphics/gears/faro/icon.png", ".4bpp");
static const u16 sArtifactPal_Faro[] = INCGFX_U16("graphics/gears/faro/icon.pal", ".gbapal");
static const u32 sArtifactIcon_Rompeolas[] = INCGFX_U32("graphics/gears/rompeolas/icon.png", ".4bpp");
static const u16 sArtifactPal_Rompeolas[] = INCGFX_U16("graphics/gears/rompeolas/icon.pal", ".gbapal");
static const u32 sArtifactIcon_RedSilos[] = INCGFX_U32("graphics/gears/red_silos/icon.png", ".4bpp");
static const u16 sArtifactPal_RedSilos[] = INCGFX_U16("graphics/gears/red_silos/icon.pal", ".gbapal");
static const u32 sArtifactIcon_TableroTrifasico[] = INCGFX_U32("graphics/gears/tablero_trifasico/icon.png", ".4bpp");
static const u16 sArtifactPal_TableroTrifasico[] = INCGFX_U16("graphics/gears/tablero_trifasico/icon.pal", ".gbapal");
static const u32 sArtifactIcon_CRT[] = INCGFX_U32("graphics/gears/crt/icon.png", ".4bpp");
static const u16 sArtifactPal_CRT[] = INCGFX_U16("graphics/gears/crt/icon.pal", ".gbapal");
static const u32 sArtifactIcon_Deshumidificador[] = INCGFX_U32("graphics/gears/deshumidificador/icon.png", ".4bpp");
static const u16 sArtifactPal_Deshumidificador[] = INCGFX_U16("graphics/gears/deshumidificador/icon.pal", ".gbapal");
static const u32 sArtifactIcon_BombaAriete[] = INCGFX_U32("graphics/gears/bomba_ariete/icon.png", ".4bpp");
static const u16 sArtifactPal_BombaAriete[] = INCGFX_U16("graphics/gears/bomba_ariete/icon.pal", ".gbapal");
static const u32 sArtifactIcon_Glicerina[] = INCGFX_U32("graphics/gears/glicerina/icon.png", ".4bpp");
static const u16 sArtifactPal_Glicerina[] = INCGFX_U16("graphics/gears/glicerina/icon.pal", ".gbapal");
static const u32 sArtifactIcon_Abono[] = INCGFX_U32("graphics/gears/abono/icon.png", ".4bpp");
static const u16 sArtifactPal_Abono[] = INCGFX_U16("graphics/gears/abono/icon.pal", ".gbapal");

const struct GearInfo gGearsInfo[GEAR_COUNT] =
{
    [GEAR_NONE] = {0},
    [GEAR_VISOR_COSMICO] =
    {
        .name = COMPOUND_STRING("Visor Cósmico"),
        .description = COMPOUND_STRING("Fantasma x2 contra Normal,\nPsíquico y Hada."),
        .iconGfx = sGearIconGfx_VisorCosmico,
        .iconPal = sGearIconPal_VisorCosmico,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_GHOST),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
    [GEAR_CORAZON_TITAN] =
    {
        .name = COMPOUND_STRING("Corazón Titán"),
        .description = COMPOUND_STRING("Físicos: +20% al cargar;\n+10% durante recarga."),
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
        .description = COMPOUND_STRING("Siniestro: +30% cargado;\n+21% durante recarga."),
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
        .description = COMPOUND_STRING("Siniestro x2 contra Normal,\nLucha, Psíquico y Hada."),
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
        .name = COMPOUND_STRING("Impulso Cósmico"),
        .description = COMPOUND_STRING("Especiales: +20% al cargar;\n+10% durante recarga."),
        .iconGfx = sGearIconGfx_ImpulsoCosmico,
        .iconPal = sGearIconPal_ImpulsoCosmico,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .categoryMask = GEAR_CATEGORY_SPECIAL,
        .overloadPowerPercent = 120, // +20 % durante el turno de sobrecarga
        .residualPowerPercent = 110, // +10 % los tres turnos siguientes
    },
    [GEAR_PERMAFROST_FRAGMENT] =
    {
        .name = COMPOUND_STRING("Permafrost Fragment"),
        .description = COMPOUND_STRING("Congelación Hielo x2;\nusuario Hielo: +1/16 PS."),
        .iconGfx = sGearIconGfx_PermafrostFragment,
        .iconPal = sGearIconPal_PermafrostFragment,
        .trigger = GEAR_TRIGGER_TIMED,
        .overloadTurns = 1,
        .cooldownTurns = 3,
        .typeMask = GEAR_TYPE(TYPE_ICE),
        .overloadPowerPercent = 100,
        .residualPowerPercent = 100,
    },
#define GEAR_BLOOD_VARIANT(type, displayName, moveTypeName, bagIcon) \
    [GEAR_BLOOD_##type] = \
    { \
        .name = COMPOUND_STRING(displayName), \
        .description = COMPOUND_STRING(moveTypeName ": primer golpe\n+40%; luego +12% x3."), \
        .iconGfx = sBloodGearIcon, \
        .iconPal = gItemIconPalette_Blood_##type, \
        .trigger = GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE, \
        .cooldownTurns = 3, \
        .typeMask = GEAR_TYPE(TYPE_##type), \
        .overloadPowerPercent = 140, \
        .residualPowerPercent = 112, \
    },
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
    [GEAR_STORM_STAFF] =
    {
        .name = COMPOUND_STRING("Báculo Tormentoso"),
        .description = COMPOUND_STRING("Repite ataque especial al\n25%; sin efectos extra."),
        .iconGfx = sStormStaffIcon,
        .iconPal = gItemIconPalette_StormStaff,
        .trigger = GEAR_TRIGGER_PASSIVE,
    },
    [GEAR_SHACKLE] =
    {
        .name = COMPOUND_STRING("Grillete"),
        .description = COMPOUND_STRING("Cada golpe: 20% de impedir\nla cadena enemiga."),
        .iconGfx = sShackleIcon,
        .iconPal = gItemIconPalette_Shackle,
        .trigger = GEAR_TRIGGER_PASSIVE,
    },
#define RECENT_GEAR(id, gfx, path, displayName, displayDescription) \
    [GEAR_##id] = \
    { \
        .name = COMPOUND_STRING(displayName), \
        .description = COMPOUND_STRING(displayDescription), \
        .iconGfx = sRecentGearIcon_##gfx, \
        .iconPal = sRecentGearPalette_##gfx, \
        .trigger = (GEAR_##id == GEAR_BATERIA_RESERVA \
                 || GEAR_##id == GEAR_BOTAS_UMBRAL \
                 || GEAR_##id == GEAR_VELO_CRIPTICO \
                 || GEAR_##id == GEAR_AMULETO_INQUEBRANTABLE \
                 || GEAR_##id == GEAR_CRISOL_ESPIRITUAL \
                 || GEAR_##id == GEAR_LENTE_CERTEZA \
                 || GEAR_##id == GEAR_MODULO_VECTORIAL \
                   ? GEAR_TRIGGER_EVENT : GEAR_TRIGGER_PASSIVE), \
        .cooldownTurns = 3, \
    },
#include "constants/recent_gears.h"
#undef RECENT_GEAR
#define NEW_GEAR(id, gfx, visibleName, shortDescription) \
    [GEAR_##id] = \
    { \
        .name = COMPOUND_STRING(visibleName), \
        .description = COMPOUND_STRING(shortDescription), \
        .iconGfx = sNewGearIcon_##gfx, \
        .iconPal = sNewGearPalette_##gfx, \
        .trigger = GEAR_TRIGGER_EVENT, \
        .cooldownTurns = GEAR_##id == GEAR_NUCLEO_HYDRA ? 0 : 3, \
    },
#include "constants/gear_batch8.h"
#undef NEW_GEAR
    [GEAR_ARMADURA_METEORICA] =
    {
        .name = COMPOUND_STRING("Armadura Meteórica"),
        .description = COMPOUND_STRING("Aguanta 1 golpe letal con\n30% PS. Baja Defensas;\nceniza quema al rival."),
        .iconGfx = sArtifactIcon_ArmaduraMeteorica,
        .iconPal = sArtifactPal_ArmaduraMeteorica,
        .trigger = GEAR_TRIGGER_EVENT,
        .cooldownTurns = 3,
    },
    [GEAR_ESCUDO_PALADIN] =
    {
        .name = COMPOUND_STRING("Escudo de paladín"),
        .description = COMPOUND_STRING("Puede anular un efecto hostil por turno.\nSu probabilidad aumenta al fallar."),
        .iconGfx = sArtifactIcon_EscudoPaladin,
        .iconPal = sArtifactPal_EscudoPaladin,
        .trigger = GEAR_TRIGGER_PASSIVE,
    },
#define EXTRA_ARTIFACT(id, title, text, gfx) \
    [GEAR_##id] = { \
        .name = COMPOUND_STRING(title), \
        .description = COMPOUND_STRING(text), \
        .iconGfx = sArtifactIcon_##gfx, \
        .iconPal = sArtifactPal_##gfx, \
        .trigger = GEAR_TRIGGER_PASSIVE, \
    },
    EXTRA_ARTIFACT(ESTANDARTE_GUERRA, "Estandarte de guerra", "Control cura un 10% al aliado.\n30% de limpiar un estado.", EstandarteGuerra)
    EXTRA_ARTIFACT(TORRE_VIGILANCIA, "Torre de vigilancia", "Cada 3 turnos sube Precisión\ny borra sus reducciones.", TorreVigilancia)
    EXTRA_ARTIFACT(FARO, "Faro", "Niega beneficios climáticos\nal rival cada 2 turnos.", Faro)
    EXTRA_ARTIFACT(ROMPEOLAS, "Rompeolas", "Reduce golpes de Agua cada\n2 turnos; más en área.", Rompeolas)
    EXTRA_ARTIFACT(RED_SILOS, "Red de silos", "Proyectiles lanzan hasta 4\nréplicas decrecientes.", RedSilos)
    EXTRA_ARTIFACT(TABLERO_TRIFASICO, "Tablero trifásico", "Electricidad rebota al otro\nrival con 25% de daño.", TableroTrifasico)
    EXTRA_ARTIFACT(CRT, "CRT", "Extiende pantallas y comparte\nProtección con el aliado.", CRT)
    EXTRA_ARTIFACT(DESHUMIDIFICADOR, "Deshumidificador", "Acorta climas y neblina\nentre 1 y 2 turnos.", Deshumidificador)
    EXTRA_ARTIFACT(BOMBA_ARIETE, "Bomba de ariete", "Cada 2 turnos repite Agua\ncon 25% de potencia.", BombaAriete)
    EXTRA_ARTIFACT(GLICERINA, "Glicerina", "Todos los ataques de Fuego\npueden causar quemaduras.", Glicerina)
    EXTRA_ARTIFACT(ABONO, "Abono", "Regenera a los tipo Planta;\ndoble con Enraizado.", Abono)
#undef EXTRA_ARTIFACT
};
