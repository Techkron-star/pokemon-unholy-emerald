#include "global.h"
#include "unholy_ui.h"
#include "item.h"
#include "item_icon.h"
#include "config/save.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "bg.h"
#include "debug.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_object_lock.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_weather.h"
#include "gear.h"
#include "field_screen_effect.h"
#include "frontier_pass.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item_menu.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "new_game.h"
#include "option_menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokenav.h"
#include "safari_zone.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "shop.h"
#include "horizon_market.h"
#include "sound.h"
#include "sprite.h"
#include "start_menu.h"
#include "hfc_watch.h"
#include "unholy_localization.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_card.h"
#include "window.h"
#include "union_room.h"
#include "dexnav.h"
#include "wild_encounter.h"
#include "constants/battle_frontier.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// Menu actions
enum
{
    MENU_ACTION_POKEDEX,
    MENU_ACTION_POKEMON,
    MENU_ACTION_BAG,
    MENU_ACTION_POKENAV,
    MENU_ACTION_PLAYER,
    MENU_ACTION_SAVE,
    MENU_ACTION_OPTION,
    MENU_ACTION_EXIT,
    MENU_ACTION_RETIRE_SAFARI,
    MENU_ACTION_PLAYER_LINK,
    MENU_ACTION_REST_FRONTIER,
    MENU_ACTION_RETIRE_FRONTIER,
    MENU_ACTION_PYRAMID_BAG,
    MENU_ACTION_DEBUG,
    MENU_ACTION_DEXNAV,
    MENU_ACTION_CRAFTING,
    MENU_ACTION_HFC_WATCH,
};

// Save status
enum
{
    SAVE_IN_PROGRESS,
    SAVE_SUCCESS,
    SAVE_CANCELED,
    SAVE_ERROR
};

// IWRAM common
COMMON_DATA bool8 (*gMenuCallback)(void) = NULL;

// EWRAM
EWRAM_DATA static u8 sSafariBallsWindowId = 0;
EWRAM_DATA static u8 sBattlePyramidFloorWindowId = 0;
EWRAM_DATA static u8 sUnholyCraftWindowId = 0;
EWRAM_DATA static u8 sUnholyCraftIconSpriteId = 0;
EWRAM_DATA static u16 sUnholyCraftSavedPalette[16] = {0};
EWRAM_DATA static u8 sUnholyCraftRecipeCursor = 0;
EWRAM_DATA static u8 sUnholyCraftGearCursor = 0;
EWRAM_DATA static u8 sUnholyCraftPartyCursor = 0;
EWRAM_DATA static u8 sUnholyCraftMode = 0; // 0 = fabricate, 1 = equip
EWRAM_DATA static bool8 sUnholyInjectorShop = FALSE;
EWRAM_DATA static u8 sUnholyCraftStatus = 0;
EWRAM_DATA static bool8 sUnholyCraftConfirm = FALSE;
EWRAM_DATA static bool8 sUnholyCraftDetailView = FALSE;
EWRAM_DATA static bool8 sUnholyCraftMoreDetail = FALSE;
EWRAM_DATA static u8 sUnholyCraftDetailOffset = 0;
EWRAM_DATA static u8 sStartMenuCursorPos = 0;
EWRAM_DATA static u8 sNumStartMenuActions = 0;
EWRAM_DATA static u8 sCurrentStartMenuActions[11] = {0};
EWRAM_DATA static s8 sInitStartMenuData[2] = {0};

EWRAM_DATA static u8 (*sSaveDialogCallback)(void) = NULL;
EWRAM_DATA static u8 sSaveDialogTimer = 0;
EWRAM_DATA static bool8 sSavingComplete = FALSE;
EWRAM_DATA static u8 sSaveInfoWindowId = 0;

// Menu action callbacks
static bool8 StartMenuPokedexCallback(void);
static bool8 StartMenuPokemonCallback(void);
static bool8 StartMenuBagCallback(void);
static bool8 StartMenuPokeNavCallback(void);
static bool8 StartMenuPlayerNameCallback(void);
static bool8 StartMenuSaveCallback(void);
static bool8 StartMenuOptionCallback(void);
static bool8 StartMenuExitCallback(void);
static bool8 StartMenuSafariZoneRetireCallback(void);
static bool8 StartMenuLinkModePlayerNameCallback(void);
static bool8 StartMenuBattlePyramidRetireCallback(void);
static bool8 StartMenuBattlePyramidBagCallback(void);
static bool8 StartMenuDebugCallback(void);
static bool8 StartMenuDexNavCallback(void);
static bool8 StartMenuCraftingCallback(void);
static bool8 StartMenuHfcWatchCallback(void);
static void Task_UnholyCraftingMenu(u8 taskId);
static EWRAM_DATA MainCallback sUnholyCraftOnClose = NULL;
static void CloseUnholyCraftingMenu(void);

// Menu callbacks
static bool8 SaveStartCallback(void);
static bool8 SaveCallback(void);
static bool8 BattlePyramidRetireStartCallback(void);
static bool8 BattlePyramidRetireReturnCallback(void);
static bool8 BattlePyramidRetireCallback(void);
static bool8 HandleStartMenuInput(void);

// Save dialog callbacks
static u8 SaveConfirmSaveCallback(void);
static u8 SaveYesNoCallback(void);
static u8 SaveConfirmInputCallback(void);
static u8 SaveFileExistsCallback(void);
static u8 SaveConfirmOverwriteDefaultNoCallback(void);
static u8 SaveConfirmOverwriteCallback(void);
static u8 SaveOverwriteInputCallback(void);
static u8 SaveSavingMessageCallback(void);
static u8 SaveDoSaveCallback(void);
static u8 SaveSuccessCallback(void);
static u8 SaveReturnSuccessCallback(void);
static u8 SaveErrorCallback(void);
static u8 SaveReturnErrorCallback(void);
static u8 BattlePyramidConfirmRetireCallback(void);
static u8 BattlePyramidRetireYesNoCallback(void);
static u8 BattlePyramidRetireInputCallback(void);

// Task callbacks
static void StartMenuTask(u8 taskId);
static void SaveGameTask(u8 taskId);
static void Task_SaveAfterLinkBattle(u8 taskId);
static void Task_WaitForBattleTowerLinkSave(u8 taskId);
static bool8 FieldCB_ReturnToFieldStartMenu(void);

static const struct WindowTemplate sWindowTemplate_SafariBalls = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 9,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const u8 *const sPyramidFloorNames[FRONTIER_STAGES_PER_CHALLENGE + 1] =
{
    gText_Floor1,
    gText_Floor2,
    gText_Floor3,
    gText_Floor4,
    gText_Floor5,
    gText_Floor6,
    gText_Floor7,
    gText_Peak
};

static const struct WindowTemplate sWindowTemplate_PyramidFloor = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 10,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const struct WindowTemplate sWindowTemplate_PyramidPeak = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 12,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x8
};

static const u8 sText_MenuDebug[] = _("DEBUG");
static const u8 *const sStartMenuSpanishNames[] =
{
    [MENU_ACTION_POKEDEX]         = COMPOUND_STRING("POKÉDEX"),
    [MENU_ACTION_POKEMON]         = COMPOUND_STRING("POKÉMON"),
    [MENU_ACTION_BAG]             = COMPOUND_STRING("MOCHILA"),
    [MENU_ACTION_POKENAV]         = COMPOUND_STRING("POKÉNAV"),
    [MENU_ACTION_SAVE]            = COMPOUND_STRING("GUARDAR"),
    [MENU_ACTION_OPTION]          = COMPOUND_STRING("AJUSTES"),
    [MENU_ACTION_EXIT]            = COMPOUND_STRING("SALIR"),
    [MENU_ACTION_RETIRE_SAFARI]   = COMPOUND_STRING("RETIRAR"),
    [MENU_ACTION_REST_FRONTIER]   = COMPOUND_STRING("REPOSO"),
    [MENU_ACTION_RETIRE_FRONTIER] = COMPOUND_STRING("RETIRAR"),
    [MENU_ACTION_PYRAMID_BAG]     = COMPOUND_STRING("MOCHILA"),
    [MENU_ACTION_DEBUG]           = COMPOUND_STRING("DEBUG"),
    [MENU_ACTION_DEXNAV]          = COMPOUND_STRING("DEXNAV"),
    [MENU_ACTION_CRAFTING]        = COMPOUND_STRING("FABRICAR"),
    [MENU_ACTION_HFC_WATCH]       = COMPOUND_STRING("RELOJ HFC"),
};
#define TAG_UNHOLY_CRAFT_ICON 9190
#define UNHOLY_CRAFT_MATERIAL_LIMIT 3
#define UNHOLY_CRAFT_RECIPES_PER_PAGE 4
#define UNHOLY_CRAFT_DETAIL_X 108
#define UNHOLY_CRAFT_DETAIL_WIDTH 108
#define UNHOLY_CRAFT_MATERIAL_Y 94
#define UNHOLY_CRAFT_MATERIAL_STEP 11
#define UNHOLY_CRAFT_DESC_LINES 2
#define UNHOLY_CRAFT_FULL_DESC_LINES 7
#define UNHOLY_INJECTOR_PRICE 3000

enum UnholyCraftStatus
{
    CRAFT_STATUS_READY,
    CRAFT_STATUS_CONFIRM,
    CRAFT_STATUS_MATERIALS,
    CRAFT_STATUS_MONEY,
    CRAFT_STATUS_BADGE,
    CRAFT_STATUS_SPACE,
    CRAFT_STATUS_COMPLETED,
    CRAFT_STATUS_SHARE_UPGRADED,
    CRAFT_STATUS_SHARE_OWNED,
    CRAFT_STATUS_EMPTY_SLOT,
    CRAFT_STATUS_NO_GEAR,
    CRAFT_STATUS_EQUIPPED,
    CRAFT_STATUS_ALREADY,
    CRAFT_STATUS_ERROR,
};

struct UnholyCraftMaterial
{
    enum Item itemId;
    u8 quantity;
};

struct UnholyCraftRecipe
{
    enum Item result;
    const u8 *description;
    u16 badgeFlag; // Zero = no badge requirement.
    u8 materialCount;
    struct UnholyCraftMaterial materials[UNHOLY_CRAFT_MATERIAL_LIMIT];
};

static const u8 sUnholyCraftTitle[] = _("FABRICACIÓN");
static const u8 sUnholyInjectorTitle[] = _("INYECTORES");
static const u8 sUnholyCraftSkinEclipse[] = _("ECLIPSE");
static const u8 sUnholyCraftSkinDaylight[] = _("DAYLIGHT");
static const u8 sUnholyCraftControls[] = _("L/R PAG  A CREAR  START INFO  SEL EQUIP  B SALIR");
static const u8 sUnholyInjectorControls[] = _("L/R PAG  A COMPRAR  START INFO  B SALIR");
static const u8 sUnholyCraftDetailControls[] = _("A OTRA PAG.  START/B VOLVER");
static const u8 sUnholyEquipControls[] = _("A CAMBIAR  SEL CREAR  B SALIR");

static const u8 sUnholyRecipeExp[] = _("Reparte el 100% de\nEXP al equipo apto.");
static const u8 sUnholyRecipeOverloadedExp[] = _("Más EXP y crianza.\nAcelera los huevos.");
static const u8 sUnholyRecipeVisor[] = _("Fantasma x2 contra\nNormal, Psi. y Hada.");
static const u8 sUnholyRecipeHeart[] = _("Físicos: +20% al\nactivar; +10% luego.");
static const u8 sUnholyRecipeMask[] = _("Siniestro: +30% al\nactivar; +21% luego.");
static const u8 sUnholyRecipeScepter[] = _("Siniestro x2 contra\nNorm., Lucha, Psi.\ny Hada.");
static const u8 sUnholyRecipeImpulse[] = _("Especiales: +20% al\nactivar; +10% luego.");
static const u8 sUnholyRecipeIce[] = _("Hielo: x2 prob.\nde congelación.\nHielo cura 1/16 PS.");
static const u8 sUnholyCraftReady[] = _("Elige una receta.");
static const u8 sUnholyCraftConfirmText[] = _("¿Crear este objeto?");
static const u8 sUnholyInjectorConfirmText[] = _("¿Comprar este inyector?");
static const u8 sUnholyCraftNeedMats[] = _("Faltan materiales.");
static const u8 sUnholyCraftNeedMoney[] = _("No tienes suficiente dinero.");
static const u8 sUnholyCraftNeedBadge[] = _("Falta una medalla.");
static const u8 sUnholyCraftNoSpace[] = _("Mochila llena.");
static const u8 sUnholyCraftDone[] = _("Guardado en mochila.");
static const u8 sUnholyCraftNoMon[] = _("Espacio de equipo vacío.");
static const u8 sUnholyCraftNoGear[] = _("Artefacto fuera de mochila.");
static const u8 sUnholyCraftEquipped[] = _("Artefacto equipado.");
static const u8 sUnholyCraftAlready[] = _("Ya está equipado.");
static const u8 sUnholyCraftError[] = _("No se pudo crear.");

static const u8 *const sUnholyCraftStatuses[] =
{
    [CRAFT_STATUS_READY] = sUnholyCraftReady,
    [CRAFT_STATUS_CONFIRM] = sUnholyCraftConfirmText,
    [CRAFT_STATUS_MATERIALS] = sUnholyCraftNeedMats,
    [CRAFT_STATUS_MONEY] = sUnholyCraftNeedMoney,
    [CRAFT_STATUS_BADGE] = sUnholyCraftNeedBadge,
    [CRAFT_STATUS_SPACE] = sUnholyCraftNoSpace,
    [CRAFT_STATUS_COMPLETED] = sUnholyCraftDone,
    [CRAFT_STATUS_SHARE_UPGRADED] = COMPOUND_STRING("Repartir EXP mejorado."),
    [CRAFT_STATUS_SHARE_OWNED] = COMPOUND_STRING("Ya tienes esta mejora."),
    [CRAFT_STATUS_EMPTY_SLOT] = sUnholyCraftNoMon,
    [CRAFT_STATUS_NO_GEAR] = sUnholyCraftNoGear,
    [CRAFT_STATUS_EQUIPPED] = sUnholyCraftEquipped,
    [CRAFT_STATUS_ALREADY] = sUnholyCraftAlready,
    [CRAFT_STATUS_ERROR] = sUnholyCraftError,
};

// Every ingredient is consumed; badges only unlock specific recipes.
// The resulting Gear item can be installed via the EQUIP tab without
// replacing a held item. Repeating a recipe creates another physical copy.
static const struct UnholyCraftRecipe sUnholyCraftRecipes[] =
{
    { ITEM_SUPER_EXP_SHARE, sUnholyRecipeExp, FLAG_BADGE03_GET, 1,
      {{ ITEM_EXP_SHARE, 1 }} },
    { ITEM_OVERLOADED_EXP_SHARE, sUnholyRecipeOverloadedExp, FLAG_BADGE06_GET, 2,
      {{ ITEM_SUPER_EXP_SHARE, 1 }, { ITEM_NUGGET, 1 }} },
    { ITEM_GEAR_VISOR_COSMICO, sUnholyRecipeVisor, 0, 3,
      {{ ITEM_SPELL_TAG, 2 }, { ITEM_MOON_STONE, 2 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_CORAZON_TITAN, sUnholyRecipeHeart, 0, 3,
      {{ ITEM_MUSCLE_BAND, 1 }, { ITEM_METAL_COAT, 2 }, { ITEM_HARD_STONE, 2 }} },
    { ITEM_GEAR_SKI_MASK, sUnholyRecipeMask, 0, 3,
      {{ ITEM_BLACK_GLASSES, 1 }, { ITEM_DARK_GEM, 3 }, { ITEM_DUSK_STONE, 1 }} },
    { ITEM_GEAR_CETRO_SINIESTRO, sUnholyRecipeScepter, FLAG_BADGE07_GET, 2,
      {{ ITEM_BLACK_GLASSES, 1 }, { ITEM_SPELL_TAG, 2 }} },
    { ITEM_GEAR_IMPULSO_COSMICO, sUnholyRecipeImpulse, 0, 3,
      {{ ITEM_WISE_GLASSES, 1 }, { ITEM_STAR_PIECE, 2 }, { ITEM_MOON_STONE, 1 }} },
    { ITEM_GEAR_PERMAFROST_FRAGMENT, sUnholyRecipeIce, 0, 3,
      {{ ITEM_ICE_STONE, 2 }, { ITEM_NEVER_MELT_ICE, 1 }, { ITEM_MYSTIC_WATER, 1 }} },
    { ITEM_GEAR_STORM_STAFF, COMPOUND_STRING("Repite el especial\nal 25% de potencia;\nsin efecto extra."), 0, 3,
      {{ ITEM_THUNDER_STONE, 1 }, { ITEM_MAGNET, 1 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_SHACKLE, COMPOUND_STRING("Golpe dañino: 20%\nde cortar la cadena\ndel rival."), 0, 2,
      {{ ITEM_METAL_COAT, 1 }, { ITEM_HARD_STONE, 1 }} },
    // Initial progression-aware recipes. Costs are provisional until economy QA.
    { ITEM_GEAR_ARIETE_NAUTILO, COMPOUND_STRING("Contacto con daño\nbaja Defensa -1."), FLAG_BADGE03_GET, 3,
      {{ ITEM_SHELL_BELL, 1 }, { ITEM_HARD_STONE, 2 }, { ITEM_MYSTIC_WATER, 1 }} },
    { ITEM_GEAR_CARTEL_BUSQUEDA, COMPOUND_STRING("+5% daño por nivel\npositivo del rival;\nmáximo +60%."), FLAG_BADGE05_GET, 3,
      {{ ITEM_BIG_PEARL, 2 }, { ITEM_IRON_BALL, 1 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_ANILLO_FEERICO, COMPOUND_STRING("Usuario Hada recibe\n25% menos daño\nde tipo Veneno."), FLAG_BADGE03_GET, 2,
      {{ ITEM_FAIRY_GEM, 2 }, { ITEM_MOON_STONE, 1 }} },
    { ITEM_GEAR_BATERIA_RESERVA, COMPOUND_STRING("Bajo 50% Stamina:\nrecupera 20%; luego\n+4% en recarga."), FLAG_BADGE02_GET, 2,
      {{ ITEM_MAGNET, 2 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_BOTAS_UMBRAL, COMPOUND_STRING("Anula las trampas\nal entrar una vez\npor recarga."), FLAG_BADGE03_GET, 2,
      {{ ITEM_IRON, 2 }, { ITEM_CLEANSE_TAG, 1 }} },
    { ITEM_GEAR_VELO_CRIPTICO, COMPOUND_STRING("Anula un efecto\nsecundario ajeno;\nrecarga: 3 turnos."), FLAG_BADGE04_GET, 2,
      {{ ITEM_CLEANSE_TAG, 2 }, { ITEM_BRIGHT_POWDER, 1 }} },
    { ITEM_GEAR_AMULETO_INQUEBRANTABLE, COMPOUND_STRING("Impide una bajada\nde estadísticas del\nrival por recarga."), FLAG_BADGE04_GET, 2,
      {{ ITEM_EVIOLITE, 1 }, { ITEM_LIGHT_CLAY, 1 }} },
    { ITEM_GEAR_CRISOL_ESPIRITUAL, COMPOUND_STRING("Cura un estado\ngrave cada 3\nturnos."), FLAG_BADGE03_GET, 2,
      {{ ITEM_LUM_BERRY, 3 }, { ITEM_SHELL_BELL, 1 }} },
    { ITEM_GEAR_LENTE_CERTEZA, COMPOUND_STRING("El siguiente ataque\nignora la evasión.\nRecarga: 3 turnos."), FLAG_BADGE04_GET, 2,
      {{ ITEM_WIDE_LENS, 1 }, { ITEM_BRIGHT_POWDER, 1 }} },
    { ITEM_GEAR_MODULO_VECTORIAL, COMPOUND_STRING("Pivotes gastan 25%\nmenos Stamina; 8%\nmenos en recarga."), FLAG_BADGE03_GET, 3,
      {{ ITEM_MAGNET, 1 }, { ITEM_MYSTIC_WATER, 1 }, { ITEM_STAR_PIECE, 1 }} },
    // Everyday audio utilities can be crafted; unique emergency/raid gear cannot.
    { ITEM_GEAR_AMPLIFICADOR, COMPOUND_STRING("Sonido: +25% daño\nal activar; gasta\n10% más Stamina."), FLAG_BADGE03_GET, 2,
      {{ ITEM_MAGNET, 1 }, { ITEM_STAR_PIECE, 1 }} },
    { ITEM_GEAR_HEADPHONES, COMPOUND_STRING("Sonido: -40% daño;\nsin efectos\nsecundarios."), FLAG_BADGE03_GET, 2,
      {{ ITEM_SILK_SCARF, 1 }, { ITEM_MAGNET, 1 }} },
    { ITEM_GEAR_ARMADURA_METEORICA, COMPOUND_STRING("Letal: sobrevive\ncon 30% PS; baja\nDef. y quema rival."), 0, 2,
      {{ ITEM_METAL_COAT, 1 }, { ITEM_METEORITE, 1 }} },
};

// La investigadora de Ruta 102 ofrece los 18 inyectores a cambio de
// Polvoestelar, una gema del tipo correspondiente y 3.000 pokédólares.
// Se mantienen los mismos ingredientes que las recetas anteriores.
static const struct UnholyCraftRecipe sUnholyInjectorRecipes[] =
{
#define GEAR_BLOOD_VARIANT(type, name, moveTypeName, bagIcon) \
    { ITEM_GEAR_BLOOD_##type, COMPOUND_STRING("Mov. " moveTypeName ":\n+40% al activar;\n+12% en recarga."), \
      0, 2, {{ ITEM_STARDUST, 1 }, { ITEM_##type##_GEM, 1 }} },
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
};

static const struct UnholyCraftRecipe *UnholyCraftActiveRecipes(void)
{
    return sUnholyInjectorShop ? sUnholyInjectorRecipes : sUnholyCraftRecipes;
}

static u8 UnholyCraftActiveRecipeCount(void)
{
    return sUnholyInjectorShop ? ARRAY_COUNT(sUnholyInjectorRecipes) : ARRAY_COUNT(sUnholyCraftRecipes);
}

static const struct WindowTemplate sUnholyCraftWindow =
{
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1,
    .width = 28, .height = 18, .paletteNum = 14, .baseBlock = 0x20,
};

static const struct MenuAction sStartMenuItems[] =
{
    [MENU_ACTION_POKEDEX]         = {gText_MenuPokedex, {.u8_void = StartMenuPokedexCallback}},
    [MENU_ACTION_POKEMON]         = {gText_MenuPokemon, {.u8_void = StartMenuPokemonCallback}},
    [MENU_ACTION_BAG]             = {gText_MenuBag,     {.u8_void = StartMenuBagCallback}},
    [MENU_ACTION_POKENAV]         = {gText_MenuPokenav, {.u8_void = StartMenuPokeNavCallback}},
    [MENU_ACTION_PLAYER]          = {gText_MenuPlayer,  {.u8_void = StartMenuPlayerNameCallback}},
    [MENU_ACTION_SAVE]            = {gText_MenuSave,    {.u8_void = StartMenuSaveCallback}},
    [MENU_ACTION_OPTION]          = {gText_MenuOption,  {.u8_void = StartMenuOptionCallback}},
    [MENU_ACTION_EXIT]            = {gText_MenuExit,    {.u8_void = StartMenuExitCallback}},
    [MENU_ACTION_RETIRE_SAFARI]   = {gText_MenuRetire,  {.u8_void = StartMenuSafariZoneRetireCallback}},
    [MENU_ACTION_PLAYER_LINK]     = {gText_MenuPlayer,  {.u8_void = StartMenuLinkModePlayerNameCallback}},
    [MENU_ACTION_REST_FRONTIER]   = {gText_MenuRest,    {.u8_void = StartMenuSaveCallback}},
    [MENU_ACTION_RETIRE_FRONTIER] = {gText_MenuRetire,  {.u8_void = StartMenuBattlePyramidRetireCallback}},
    [MENU_ACTION_PYRAMID_BAG]     = {gText_MenuBag,     {.u8_void = StartMenuBattlePyramidBagCallback}},
    [MENU_ACTION_DEBUG]           = {sText_MenuDebug,   {.u8_void = StartMenuDebugCallback}},
    [MENU_ACTION_DEXNAV]          = {gText_MenuDexNav,  {.u8_void = StartMenuDexNavCallback}},
    [MENU_ACTION_CRAFTING]       = {gText_MenuCrafting, {.u8_void = StartMenuCraftingCallback}},
    [MENU_ACTION_HFC_WATCH]      = {gText_MenuHfcWatch, {.u8_void = StartMenuHfcWatchCallback}},
};

static const struct BgTemplate sBgTemplates_LinkBattleSave[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sWindowTemplates_LinkBattleSave[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x194
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sSaveInfoWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 14,
    .height = 10,
    .paletteNum = 15,
    .baseBlock = 8
};

// Local functions
static void BuildStartMenuActions(void);
static void AddStartMenuAction(u8 action);
static void BuildNormalStartMenu(void);
static void BuildDebugStartMenu(void);
static void BuildSafariZoneStartMenu(void);
static void BuildLinkModeStartMenu(void);
static void BuildUnionRoomStartMenu(void);
static void BuildBattlePikeStartMenu(void);
static void BuildBattlePyramidStartMenu(void);
static void BuildMultiPartnerRoomStartMenu(void);
static void ShowSafariBallsWindow(void);
static void ShowPyramidFloorWindow(void);
static void RemoveExtraStartMenuWindows(void);
static bool32 PrintStartMenuActions(s8 *pIndex, u32 count);
static bool32 InitStartMenuStep(void);
static void InitStartMenu(void);
static void CreateStartMenuTask(TaskFunc followupFunc);
static void InitSave(void);
static u8 RunSaveCallback(void);
static void ShowSaveMessage(const u8 *message, u8 (*saveCallback)(void));
static void HideSaveMessageWindow(void);
static void HideSaveInfoWindow(void);
static void SaveStartTimer(void);
static bool8 SaveSuccesTimer(void);
static bool8 SaveErrorTimer(void);
static void InitBattlePyramidRetire(void);
static void VBlankCB_LinkBattleSave(void);
static bool32 InitSaveWindowAfterLinkBattle(u8 *par1);
static void CB2_SaveAfterLinkBattle(void);
static void ShowSaveInfoWindow(void);
static void RemoveSaveInfoWindow(void);
static void HideStartMenuWindow(void);
static void HideStartMenuDebug(void);

static void BuildStartMenuActions(void)
{
    sNumStartMenuActions = 0;

    if (IsOverworldLinkActive() == TRUE)
    {
        BuildLinkModeStartMenu();
    }
    else if (InUnionRoom() == TRUE)
    {
        BuildUnionRoomStartMenu();
    }
    else if (GetSafariZoneFlag() == TRUE)
    {
        BuildSafariZoneStartMenu();
    }
    else if (InBattlePike())
    {
        BuildBattlePikeStartMenu();
    }
    else if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        BuildBattlePyramidStartMenu();
    }
    else if (InMultiPartnerRoom())
    {
        BuildMultiPartnerRoomStartMenu();
    }
    else
    {
        if (DEBUG_OVERWORLD_MENU == TRUE && DEBUG_OVERWORLD_IN_MENU == TRUE)
            BuildDebugStartMenu();
        else
            BuildNormalStartMenu();
    }
}

static void UnholyCraftRefreshIcon(enum Item itemId)
{
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_UNHOLY_CRAFT_ICON);
        FreeSpritePaletteByTag(TAG_UNHOLY_CRAFT_ICON);
        DestroySprite(&gSprites[sUnholyCraftIconSpriteId]);
        sUnholyCraftIconSpriteId = MAX_SPRITES;
    }

    if (itemId == ITEM_NONE)
        return;

    sUnholyCraftIconSpriteId = AddItemIconSprite(TAG_UNHOLY_CRAFT_ICON, TAG_UNHOLY_CRAFT_ICON, itemId);
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        struct Sprite *icon = &gSprites[sUnholyCraftIconSpriteId];
        // Item icons contain 24x24 artwork in a 32x32 OAM slot. Keep the
        // artwork at native resolution inside the dedicated preview area.
        icon->x = sUnholyCraftMode != 0 ? 202 : sUnholyCraftDetailView ? 35 : 133;
        icon->y = 48;
        icon->oam.priority = 0;
    }
}

static void UnholyCraftPrint(u8 x, u8 y, const u8 *text)
{
    UnholyUi_Print(sUnholyCraftWindowId, y >= 120 ? FONT_SMALL_NARROW : FONT_SMALL,
        x, y, 200 - min(x, 200), text);
}

// Two clear lines are preferable to squeezing long names into an unreadable font.
static void UnholyCraftPrintName(u8 x, u8 y, u8 width, u8 fill, const u8 *name)
{
    const u8 *split = NULL;
    u8 firstLine[ITEM_NAME_LENGTH + 1];

    if (GetStringWidth(FONT_SMALL_NARROW, name, 0) > width)
    {
        for (const u8 *p = name; *p != EOS; p++)
        {
            if (*p != CHAR_SPACE)
                continue;
            *StringCopyN(firstLine, name, p - name) = EOS;
            if (GetStringWidth(FONT_SMALL_NARROW, firstLine, 0) > width)
                break;
            split = p;
        }
    }

    if (split != NULL)
    {
        *StringCopyN(firstLine, name, split - name) = EOS;
        UnholyUi_PrintOnFill(sUnholyCraftWindowId, FONT_SMALL_NARROW,
            x, y + 1, width, fill, firstLine);
        name = split + 1;
    }
    UnholyUi_PrintOnFill(sUnholyCraftWindowId,
        GetFontIdToFit(name, FONT_SMALL_NARROW, 0, width),
        x, y + (split == NULL ? 6 : 11), width, fill, name);
}

// Return the next complete line; never print across the description/material
// boundary. The same wrapper powers the short preview and full-page reader.
static const u8 *UnholyCraftNextDescriptionLine(const u8 *source, u8 *text, u8 width)
{
    u8 length = 0;
    u8 lastSpace = 0xFF;
    const u8 *end = source;

    while (*end != EOS && *end != CHAR_NEWLINE && length < 63)
    {
        text[length] = *end;
        text[length + 1] = EOS;
        if (GetStringWidth(FONT_SMALL_NARROW, text, 0) > width)
            break;
        if (*end == CHAR_SPACE)
            lastSpace = length;
        end++;
        length++;
    }
    if (*end != EOS && *end != CHAR_NEWLINE && lastSpace != 0xFF)
    {
        length = lastSpace;
        end = source + lastSpace + 1;
    }
    if (length == 0 && *end != EOS && *end != CHAR_NEWLINE)
        text[length++] = *end++;
    text[length] = EOS;
    source = end;
    if (*source == CHAR_NEWLINE)
        source++;
    while (*source == CHAR_SPACE)
        source++;
    return source;
}

static bool8 UnholyCraftPrintDescription(u8 fill, const u8 *source, u8 x, u8 y,
                                         u8 width, u8 step, u8 skip, u8 lines)
{
    for (u8 line = 0; line < skip + lines && *source != EOS; line++)
    {
        u8 text[64];
        source = UnholyCraftNextDescriptionLine(source, text, width);
        if (line < skip)
            continue;
        UnholyUi_PrintOnFill(sUnholyCraftWindowId, FONT_SMALL_NARROW,
            x, y + (line - skip) * step, width, fill, text);
    }
    return *source != EOS;
}

static void UnholyCraftPrintMaterial(u8 row, const struct UnholyCraftMaterial *ingredient)
{
    u8 window = sUnholyCraftWindowId;
    u8 y = UNHOLY_CRAFT_MATERIAL_Y + row * UNHOLY_CRAFT_MATERIAL_STEP;
    u16 owned = CountTotalItemQuantityInBag(ingredient->itemId);
    u8 amounts[10];
    u8 *end = ConvertUIntToDecimalStringN(amounts, owned, STR_CONV_MODE_LEFT_ALIGN, 5);
    end = StringCopy(end, COMPOUND_STRING("/"));
    ConvertUIntToDecimalStringN(end, ingredient->quantity, STR_CONV_MODE_LEFT_ALIGN, 3);

    const u8 *name = GetItemName(ingredient->itemId);
    u8 amountFont = GetFontIdToFit(amounts, FONT_SMALL_NARROW, 0, 37);
    u8 amountX = 213 - GetStringWidth(amountFont, amounts, 0);
    u8 nameWidth = amountX - 116;
    u8 nameFont = GetFontIdToFit(name, FONT_SMALL_NARROW, 0, nameWidth);
    UnholyUi_Print(window, nameFont, 113, y, nameWidth, name);
    UnholyUi_Print(window, amountFont, amountX, y, 37, amounts);
    if (owned < ingredient->quantity)
        FillWindowPixelRect(window, PIXEL_FILL(13), 109, y + 3, 3, 3);
}

static void UnholyCraftDrawRecipeList(void)
{
    const struct UnholyCraftRecipe *recipes = UnholyCraftActiveRecipes();
    const u8 count = UnholyCraftActiveRecipeCount();
    const u8 visible = min(count, UNHOLY_CRAFT_RECIPES_PER_PAGE);
    const u8 firstOnPage = sUnholyCraftRecipeCursor / UNHOLY_CRAFT_RECIPES_PER_PAGE * UNHOLY_CRAFT_RECIPES_PER_PAGE;
    const u8 first = min(firstOnPage, count - visible);
    u8 position[12];
    u8 *end = ConvertUIntToDecimalStringN(position, sUnholyCraftRecipeCursor + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringCopy(end, COMPOUND_STRING("/"));
    ConvertUIntToDecimalStringN(end, count, STR_CONV_MODE_LEFT_ALIGN, 2);
    UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 190, 5, 27, position);

    for (u8 row = 0; row < visible; row++)
    {
        u8 index = first + row;
        u8 y = 23 + row * 23;
        bool8 selected = index == sUnholyCraftRecipeCursor;
        const u8 *name = GetItemName(recipes[index].result);
        UnholyUi_Card(sUnholyCraftWindowId, 5, y, 99, 22, selected);
        UnholyCraftPrintName(10, y + 1, 90, UnholyUi_CardFill(selected), name);
    }
}

static void UnholyCraftDrawRecipeDetail(const struct UnholyCraftRecipe *recipe)
{
    u8 window = sUnholyCraftWindowId;
    u8 fill = UnholyUi_CardFill(FALSE);
    const u8 *name = GetItemName(recipe->result);
    UnholyUi_Card(window, UNHOLY_CRAFT_DETAIL_X, 23, UNHOLY_CRAFT_DETAIL_WIDTH, 56, FALSE);
    // Leave the name and art their own rows; full descriptions have a page.
    // Title starts after the art; description starts beneath both.
    UnholyUi_Card(window, 111, 23, 29, 26, FALSE);
    UnholyCraftPrintName(142, 24, 70, fill, name);

    // Los artefactos comparten descripción con la mochila y los resúmenes,
    // evitando copias antiguas en inglés o con valores desactualizados.
    const u8 *description = sUnholyCraftStatus == CRAFT_STATUS_CONFIRM && sUnholyInjectorShop ? sUnholyInjectorConfirmText
                          : sUnholyCraftStatus != CRAFT_STATUS_READY ? sUnholyCraftStatuses[sUnholyCraftStatus]
                          : Gear_FromItem(recipe->result) != GEAR_NONE ? GetItemDescription(recipe->result)
                          : recipe->description;
    UnholyCraftPrintDescription(fill, description, 113, 51, 98, 11, 0, UNHOLY_CRAFT_DESC_LINES);

    if (sUnholyInjectorShop)
    {
        u8 price[24];
        u8 *end = StringCopy(price, COMPOUND_STRING("PRECIO $"));
        ConvertUIntToDecimalStringN(end, UNHOLY_INJECTOR_PRICE, STR_CONV_MODE_LEFT_ALIGN, 4);
        UnholyUi_Print(window, FONT_SMALL_NARROW, 109, 83, 94, price);
    }
    else
        UnholyUi_Print(window, FONT_SMALL_NARROW, 109, 83, 77, COMPOUND_STRING("MATERIALES"));
    UnholyUi_Print(window, FONT_SMALL_NARROW, 193, 83, 24, COMPOUND_STRING("T/N"));
    for (u8 i = 0; i < recipe->materialCount; i++)
        UnholyCraftPrintMaterial(i, &recipe->materials[i]);
    if (recipe->badgeFlag && recipe->materialCount < UNHOLY_CRAFT_MATERIAL_LIMIT)
        UnholyUi_Print(window, FONT_SMALL_NARROW, 113, UNHOLY_CRAFT_MATERIAL_Y + recipe->materialCount * UNHOLY_CRAFT_MATERIAL_STEP, 99,
            FlagGet(recipe->badgeFlag) ? COMPOUND_STRING("Medalla obtenida") : COMPOUND_STRING("Falta medalla"));
}

static void UnholyCraftDrawFullDescription(const struct UnholyCraftRecipe *recipe)
{
    u8 fill = UnholyUi_CardFill(FALSE);
    const u8 *name = GetItemName(recipe->result);
    const u8 *description = Gear_FromItem(recipe->result) != GEAR_NONE
        ? GetItemDescription(recipe->result) : recipe->description;

    UnholyUi_Card(sUnholyCraftWindowId, 5, 23, 212, 105, FALSE);
    UnholyUi_Card(sUnholyCraftWindowId, 8, 24, 30, 27, FALSE);
    UnholyCraftPrintName(45, 26, 162, fill, name);
    sUnholyCraftMoreDetail = UnholyCraftPrintDescription(fill, description,
        12, 49, 196, 11, sUnholyCraftDetailOffset, UNHOLY_CRAFT_FULL_DESC_LINES);
    if (sUnholyCraftMoreDetail)
        UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 192, 117, 16, COMPOUND_STRING("+"));
}

static void UnholyCraftRenderMenu(void)
{
    const u8 *skin = FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? sUnholyCraftSkinDaylight : sUnholyCraftSkinEclipse;
    enum Item iconItem;

    UnholyUi_Frame(sUnholyCraftWindowId, sUnholyInjectorShop ? sUnholyInjectorTitle : sUnholyCraftTitle);
    if (sUnholyInjectorShop)
    {
        u8 money[24];
        u8 *end = StringCopy(money, COMPOUND_STRING("$"));
        ConvertUIntToDecimalStringN(end, GetMoney(&gSaveBlock1Ptr->money), STR_CONV_MODE_LEFT_ALIGN, 6);
        UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 145, 3, 56, money);
    }
    else
        UnholyCraftPrint(137, 3, skin);

    if (sUnholyCraftMode == 0)
    {
        const struct UnholyCraftRecipe *recipe = &UnholyCraftActiveRecipes()[sUnholyCraftRecipeCursor];

        if (sUnholyCraftDetailView)
        {
            UnholyCraftDrawFullDescription(recipe);
            UnholyUi_Footer(sUnholyCraftWindowId, sUnholyCraftDetailControls);
        }
        else
        {
            UnholyCraftDrawRecipeList();
            UnholyCraftDrawRecipeDetail(recipe);
            UnholyUi_Footer(sUnholyCraftWindowId, sUnholyCraftConfirm ? (sUnholyInjectorShop
                            ? COMPOUND_STRING("A:CONFIRMAR  B:CANCELAR") : COMPOUND_STRING("A CONFIRMAR  B CANCELAR"))
                            : sUnholyInjectorShop ? sUnholyInjectorControls : sUnholyCraftControls);
        }
        iconItem = recipe->result;
    }
    else
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sUnholyCraftPartyCursor];
        enum Gear existing = Gear_GetEquipped(mon);
        enum Gear chosen = sUnholyCraftGearCursor;

        UnholyUi_Card(sUnholyCraftWindowId, 5, 23, 198, 37, FALSE);
        UnholyUi_Card(sUnholyCraftWindowId, 167, 23, 29, 26, FALSE);
        UnholyUi_Card(sUnholyCraftWindowId, 5, 63, 198, 53, FALSE);
        UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 10, 25, 151,
            COMPOUND_STRING("EQUIPAR ARTEFACTO"));
        StringCopy(gStringVar4, chosen == GEAR_NONE ? COMPOUND_STRING("> QUITAR ARTEFACTO") : GetItemName(Gear_ToItem(chosen)));
        UnholyUi_Print(sUnholyCraftWindowId, GetFontIdToFit(gStringVar4, FONT_SMALL_NARROW, 0, 151),
            10, 40, 151, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("EQUIPO "));
        ConvertIntToDecimalStringN(gStringVar3, sUnholyCraftPartyCursor + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(gStringVar4, gStringVar3);
        StringAppend(gStringVar4, COMPOUND_STRING(": "));
        if (GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
        {
            GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
            StringAppend(gStringVar4, gStringVar1);
        }
        else
            StringAppend(gStringVar4, COMPOUND_STRING("VACÍO"));
        UnholyCraftPrint(10, 65, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("EQUIPADO: "));
        StringAppend(gStringVar4, Gear_GetLocalizedName(existing));
        UnholyCraftPrint(10, 78, gStringVar4);

        StringCopy(gStringVar4, COMPOUND_STRING("EN MOCHILA: "));
        ConvertIntToDecimalStringN(gStringVar3, chosen == GEAR_NONE ? 0 : CountTotalItemQuantityInBag(Gear_ToItem(chosen)), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, chosen == GEAR_NONE ? COMPOUND_STRING("N/A") : gStringVar3);
        UnholyCraftPrint(10, 91, gStringVar4);

        if (sUnholyCraftStatus != CRAFT_STATUS_READY)
            UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 10, 104, 187,
                sUnholyCraftStatuses[sUnholyCraftStatus]);
        else
            UnholyUi_Print(sUnholyCraftWindowId, FONT_SMALL_NARROW, 10, 104, 187,
                COMPOUND_STRING("L/R ARTEF.  ARR/ABA EQUIPO"));
        UnholyUi_Footer(sUnholyCraftWindowId, sUnholyEquipControls);
        iconItem = chosen == GEAR_NONE ? ITEM_NONE : Gear_ToItem(chosen);
    }

    CopyWindowToVram(sUnholyCraftWindowId, COPYWIN_FULL);
    UnholyCraftRefreshIcon(iconItem);
}

static u8 UnholyCraftCheckRecipe(const struct UnholyCraftRecipe *recipe)
{
    if (recipe->badgeFlag != 0 && !FlagGet(recipe->badgeFlag))
        return CRAFT_STATUS_BADGE;
    if (recipe->result == ITEM_OVERLOADED_EXP_SHARE && CheckBagHasItem(recipe->result, 1))
        return CRAFT_STATUS_SHARE_OWNED;

    for (u8 i = 0; i < recipe->materialCount; i++)
    {
        if (!CheckBagHasItem(recipe->materials[i].itemId, recipe->materials[i].quantity))
            return CRAFT_STATUS_MATERIALS;
    }

    if (sUnholyInjectorShop && !IsEnoughMoney(&gSaveBlock1Ptr->money, UNHOLY_INJECTOR_PRICE))
        return CRAFT_STATUS_MONEY;

    if (!CheckBagHasSpace(recipe->result, 1)
     && !(recipe->result == ITEM_OVERLOADED_EXP_SHARE && CountTotalItemQuantityInBag(ITEM_SUPER_EXP_SHARE) == 1))
        return CRAFT_STATUS_SPACE;
    return CRAFT_STATUS_READY;
}

static u8 UnholyCraftFabricate(const struct UnholyCraftRecipe *recipe)
{
    u8 status = UnholyCraftCheckRecipe(recipe);

    if (status != CRAFT_STATUS_READY)
        return status;

    for (u8 i = 0; i < recipe->materialCount; i++)
    {
        if (!RemoveBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity))
        {
            // Guard even against unexpected inventory failure: all-or-nothing.
            while (i > 0)
            {
                i--;
                AddBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity);
            }
            return CRAFT_STATUS_ERROR;
        }
    }

    if (!AddBagItem(recipe->result, 1))
    {
        for (u8 i = 0; i < recipe->materialCount; i++)
            AddBagItem(recipe->materials[i].itemId, recipe->materials[i].quantity);
        return CRAFT_STATUS_SPACE;
    }

    // Money is charged only once all items have been exchanged successfully.
    if (sUnholyInjectorShop)
        RemoveMoney(&gSaveBlock1Ptr->money, UNHOLY_INJECTOR_PRICE);

    if (recipe->result == ITEM_OVERLOADED_EXP_SHARE)
    {
        if (gSaveBlock1Ptr->registeredItem == ITEM_SUPER_EXP_SHARE)
            gSaveBlock1Ptr->registeredItem = ITEM_OVERLOADED_EXP_SHARE;
        FlagClear(FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED);
        FlagClear(I_EXP_SHARE_FLAG);
        FlagSet(FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED);
        return CRAFT_STATUS_SHARE_UPGRADED;
    }
    return CRAFT_STATUS_COMPLETED;
}

static u8 UnholyCraftSwapGear(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sUnholyCraftPartyCursor];
    enum Gear selected = sUnholyCraftGearCursor;
    enum Gear previous;
    enum Item incoming, outgoing;

    if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
        return CRAFT_STATUS_EMPTY_SLOT;

    previous = Gear_GetEquipped(mon);
    if (previous == selected)
        return CRAFT_STATUS_ALREADY;

    incoming = Gear_ToItem(selected);
    outgoing = Gear_ToItem(previous);

    // Do not duplicate a Gear: remove its tangible inventory copy before equipping.
    if (incoming != ITEM_NONE && !RemoveBagItem(incoming, 1))
        return CRAFT_STATUS_NO_GEAR;

    // On replacement or removal the old Gear returns to the Bag.
    // If this fails, put the new one back and leave Pokemon data unchanged.
    if (outgoing != ITEM_NONE && !AddBagItem(outgoing, 1))
    {
        if (incoming != ITEM_NONE)
            AddBagItem(incoming, 1);
        return CRAFT_STATUS_SPACE;
    }

    Gear_SetEquipped(mon, selected);
    return CRAFT_STATUS_EQUIPPED;
}

static void ReturnFromUnholyCrafting(void)
{
    MainCallback onClose = sUnholyCraftOnClose;

    sUnholyCraftOnClose = NULL;
    if (onClose != NULL)
        onClose();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

static void OpenUnholyCraftingMenuInternal(MainCallback onClose, bool8 injectorShop)
{
    sUnholyCraftOnClose = onClose;
    sUnholyInjectorShop = injectorShop;
    sUnholyCraftRecipeCursor = 0;
    sUnholyCraftMode = 0;
    sUnholyCraftGearCursor = GEAR_VISOR_COSMICO;
    sUnholyCraftStatus = CRAFT_STATUS_READY;
    sUnholyCraftConfirm = FALSE;
    sUnholyCraftDetailView = FALSE;
    sUnholyCraftMoreDetail = FALSE;
    sUnholyCraftDetailOffset = 0;
    sUnholyCraftIconSpriteId = MAX_SPRITES;

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(14)], sUnholyCraftSavedPalette, sizeof(sUnholyCraftSavedPalette));
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    sUnholyCraftWindowId = AddWindow(&sUnholyCraftWindow);
    if (sUnholyCraftWindowId == 0xFF)
    {
        LoadPalette(sUnholyCraftSavedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
        FadeScreen(FADE_FROM_BLACK, 0);
        ReturnFromUnholyCrafting();
        return;
    }
    SetStandardWindowBorderStyle(sUnholyCraftWindowId, FALSE);
    PutWindowTilemap(sUnholyCraftWindowId);
    UnholyCraftRenderMenu();
    CreateTask(Task_UnholyCraftingMenu, 8);
    FadeScreen(FADE_FROM_BLACK, 0);
}

void OpenUnholyCraftingMenu(MainCallback onClose)
{
    OpenUnholyCraftingMenuInternal(onClose, FALSE);
}

// Called by the Route 102 researcher's field script, not by the START menu.
void OpenUnholyInjectorMenu(void)
{
    OpenUnholyCraftingMenuInternal(NULL, TRUE);
}

static void CloseUnholyCraftingMenu(void)
{
    if (sUnholyCraftIconSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_UNHOLY_CRAFT_ICON);
        FreeSpritePaletteByTag(TAG_UNHOLY_CRAFT_ICON);
        DestroySprite(&gSprites[sUnholyCraftIconSpriteId]);
        sUnholyCraftIconSpriteId = MAX_SPRITES;
    }
    ClearStdWindowAndFrameToTransparent(sUnholyCraftWindowId, TRUE);
    RemoveWindow(sUnholyCraftWindowId);
    LoadPalette(sUnholyCraftSavedPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
}

static void Task_UnholyCraftingMenu(u8 taskId)
{
    bool8 changed = FALSE;

    if (gPaletteFade.active)
        return;

    if (sUnholyCraftDetailView)
    {
        if (JOY_NEW(B_BUTTON | START_BUTTON))
        {
            sUnholyCraftDetailView = FALSE;
            sUnholyCraftDetailOffset = 0;
        }
        else if (JOY_NEW(A_BUTTON))
        {
            // Keep long gear descriptions legible, with no hidden or
            // overlapped lines. A cycles back to the first page at the end.
            sUnholyCraftDetailOffset = sUnholyCraftMoreDetail
                ? sUnholyCraftDetailOffset + UNHOLY_CRAFT_FULL_DESC_LINES : 0;
        }
        else
            return;
        PlaySE(SE_SELECT);
        UnholyCraftRenderMenu();
        return;
    }

    if (JOY_NEW(B_BUTTON))
    {
        if (sUnholyCraftConfirm)
        {
            sUnholyCraftConfirm = FALSE;
            sUnholyCraftStatus = CRAFT_STATUS_READY;
            UnholyCraftRenderMenu();
            return;
        }
        CloseUnholyCraftingMenu();
        DestroyTask(taskId);
        ReturnFromUnholyCrafting();
        return;
    }

    if (!sUnholyCraftMode && JOY_NEW(START_BUTTON))
    {
        sUnholyCraftDetailView = TRUE;
        sUnholyCraftDetailOffset = 0;
        sUnholyCraftConfirm = FALSE;
        sUnholyCraftStatus = CRAFT_STATUS_READY;
        PlaySE(SE_SELECT);
        UnholyCraftRenderMenu();
        return;
    }

    if (!sUnholyInjectorShop && JOY_NEW(SELECT_BUTTON))
    {
        sUnholyCraftMode ^= 1;
        changed = TRUE;
    }
    else if (!sUnholyCraftMode && (JOY_REPEAT(DPAD_ANY) || JOY_NEW(L_BUTTON | R_BUTTON)))
    {
        const u8 last = UnholyCraftActiveRecipeCount() - 1;
        u8 step = JOY_NEW(L_BUTTON | R_BUTTON) ? UNHOLY_CRAFT_RECIPES_PER_PAGE : 1;
        bool32 previous = JOY_NEW(L_BUTTON) || (!JOY_NEW(R_BUTTON) && JOY_REPEAT(DPAD_UP | DPAD_LEFT));
        u8 next = previous ? max(0, (s16)sUnholyCraftRecipeCursor - step)
                           : min(sUnholyCraftRecipeCursor + step, last);
        if (next == sUnholyCraftRecipeCursor)
            return;
        sUnholyCraftRecipeCursor = next;
        changed = TRUE;
    }
    else if (sUnholyCraftMode && JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        bool32 previous = JOY_NEW(DPAD_LEFT | L_BUTTON);
        sUnholyCraftGearCursor = (sUnholyCraftGearCursor + (previous ? GEAR_COUNT - 1 : 1)) % GEAR_COUNT;
        changed = TRUE;
    }
    else if (sUnholyCraftMode && JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        sUnholyCraftPartyCursor = (sUnholyCraftPartyCursor + (JOY_NEW(DPAD_UP) ? PARTY_SIZE - 1 : 1)) % PARTY_SIZE;
        changed = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sUnholyCraftMode)
        {
            sUnholyCraftStatus = UnholyCraftSwapGear();
            PlaySE(sUnholyCraftStatus == CRAFT_STATUS_EQUIPPED ? SE_SUCCESS : SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
        else if (!sUnholyInjectorShop && sUnholyCraftRecipeCursor == 0)
        {
            CloseUnholyCraftingMenu();
            DestroyTask(taskId);
        sUnholyCraftOnClose = NULL;
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            ScriptContext_SetupScript(EventScript_CraftSuperExpShare_Recipe);
            return;
        }
        else if (!sUnholyCraftConfirm)
        {
            sUnholyCraftStatus = UnholyCraftCheckRecipe(&UnholyCraftActiveRecipes()[sUnholyCraftRecipeCursor]);
            if (sUnholyCraftStatus == CRAFT_STATUS_READY)
            {
                sUnholyCraftConfirm = TRUE;
                sUnholyCraftStatus = CRAFT_STATUS_CONFIRM;
            }
            else
                PlaySE(SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
        else
        {
            sUnholyCraftStatus = UnholyCraftFabricate(&UnholyCraftActiveRecipes()[sUnholyCraftRecipeCursor]);
            sUnholyCraftConfirm = FALSE;
            PlaySE(sUnholyCraftStatus == CRAFT_STATUS_COMPLETED || sUnholyCraftStatus == CRAFT_STATUS_SHARE_UPGRADED
                ? SE_SUCCESS : SE_FAILURE);
            UnholyCraftRenderMenu();
            return;
        }
    }

    if (changed)
    {
        sUnholyCraftConfirm = FALSE;
        sUnholyCraftStatus = CRAFT_STATUS_READY;
        PlaySE(SE_SELECT);
        UnholyCraftRenderMenu();
    }
}

static bool8 StartMenuCraftingCallback(void)
{
    if (!gPaletteFade.active)
    {
        RemoveExtraStartMenuWindows();
        ClearStdWindowAndFrameToTransparent(GetStartMenuWindowId(), FALSE);
        RemoveStartMenuWindow();
        OpenUnholyCraftingMenu(NULL);
        return TRUE;
    }
    return FALSE;
}

// Same hand-off as the crafting menu: drop the START window, the hub opens over the field.
static bool8 StartMenuHfcWatchCallback(void)
{
    if (!gPaletteFade.active)
    {
        RemoveExtraStartMenuWindows();
        ClearStdWindowAndFrameToTransparent(GetStartMenuWindowId(), FALSE);
        RemoveStartMenuWindow();
        HfcWatch_OpenHub(TRUE);
        return TRUE;
    }
    return FALSE;
}

static void AddStartMenuAction(u8 action)
{
    AppendToList(sCurrentStartMenuActions, &sNumStartMenuActions, action);
}

static void BuildNormalStartMenu(void)
{
    bool8 hasWatch = HfcWatch_GetTier() != HFC_WATCH_TIER_NONE;

    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEDEX);

    if (DN_FLAG_DEXNAV_GET != 0 && FlagGet(DN_FLAG_DEXNAV_GET))
        AddStartMenuAction(MENU_ACTION_DEXNAV);

    if (FlagGet(FLAG_SYS_POKEMON_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEMON);

    AddStartMenuAction(MENU_ACTION_BAG);
    // With a watch, Horizon and PokeNav live inside its hub (one entry instead of two).
    if (hasWatch)
        AddStartMenuAction(MENU_ACTION_HFC_WATCH);
    if (!hasWatch && FlagGet(FLAG_CRAFTING_UNLOCKED) == TRUE)
        AddStartMenuAction(MENU_ACTION_CRAFTING);

    if (!hasWatch && FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKENAV);

    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_SAVE);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildDebugStartMenu(void)
{
    bool8 hasWatch = HfcWatch_GetTier() != HFC_WATCH_TIER_NONE;

    AddStartMenuAction(MENU_ACTION_DEBUG);
    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEDEX);
    if (FlagGet(FLAG_SYS_POKEMON_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);
    if (hasWatch)
        AddStartMenuAction(MENU_ACTION_HFC_WATCH);
    if (!hasWatch && FlagGet(FLAG_CRAFTING_UNLOCKED) == TRUE)
        AddStartMenuAction(MENU_ACTION_CRAFTING);
    if (!hasWatch && FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
        AddStartMenuAction(MENU_ACTION_POKENAV);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_SAVE);
    AddStartMenuAction(MENU_ACTION_OPTION);
}

static void BuildSafariZoneStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_RETIRE_SAFARI);
    AddStartMenuAction(MENU_ACTION_POKEDEX);
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildLinkModeStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);

    if (FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
    {
        AddStartMenuAction(MENU_ACTION_POKENAV);
    }

    AddStartMenuAction(MENU_ACTION_PLAYER_LINK);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildUnionRoomStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_BAG);

    if (FlagGet(FLAG_SYS_POKENAV_GET) == TRUE)
    {
        AddStartMenuAction(MENU_ACTION_POKENAV);
    }

    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildBattlePikeStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEDEX);
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildBattlePyramidStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PYRAMID_BAG);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_REST_FRONTIER);
    AddStartMenuAction(MENU_ACTION_RETIRE_FRONTIER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void BuildMultiPartnerRoomStartMenu(void)
{
    AddStartMenuAction(MENU_ACTION_POKEMON);
    AddStartMenuAction(MENU_ACTION_PLAYER);
    AddStartMenuAction(MENU_ACTION_OPTION);
    AddStartMenuAction(MENU_ACTION_EXIT);
}

static void ShowSafariBallsWindow(void)
{
    sSafariBallsWindowId = AddWindow(&sWindowTemplate_SafariBalls);
    PutWindowTilemap(sSafariBallsWindowId);
    DrawStdWindowFrame(sSafariBallsWindowId, FALSE);
    if (IS_FRLG)
    {
        ConvertIntToDecimalStringN(gStringVar1, gSafariZoneStepCounter, STR_CONV_MODE_RIGHT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, 600, STR_CONV_MODE_RIGHT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar3, gNumSafariBalls, STR_CONV_MODE_RIGHT_ALIGN, 2);
        StringExpandPlaceholders(gStringVar4, gText_MenuSafariStats);
        AddTextPrinterParameterized(sSafariBallsWindowId, FONT_NORMAL, gStringVar4, 4, 3, 0xFF, NULL);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, gNumSafariBalls, STR_CONV_MODE_RIGHT_ALIGN, 2);
        StringExpandPlaceholders(gStringVar4, gText_SafariBallStock);
        AddTextPrinterParameterized(sSafariBallsWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sSafariBallsWindowId, COPYWIN_GFX);
}

static void ShowPyramidFloorWindow(void)
{
    if (gSaveBlock2Ptr->frontier.curChallengeBattleNum == FRONTIER_STAGES_PER_CHALLENGE)
        sBattlePyramidFloorWindowId = AddWindow(&sWindowTemplate_PyramidPeak);
    else
        sBattlePyramidFloorWindowId = AddWindow(&sWindowTemplate_PyramidFloor);

    PutWindowTilemap(sBattlePyramidFloorWindowId);
    DrawStdWindowFrame(sBattlePyramidFloorWindowId, FALSE);
    StringCopy(gStringVar1, sPyramidFloorNames[gSaveBlock2Ptr->frontier.curChallengeBattleNum]);
    StringExpandPlaceholders(gStringVar4, gText_BattlePyramidFloor);
    AddTextPrinterParameterized(sBattlePyramidFloorWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sBattlePyramidFloorWindowId, COPYWIN_GFX);
}

static void RemoveExtraStartMenuWindows(void)
{
    if (GetSafariZoneFlag())
    {
        ClearStdWindowAndFrameToTransparent(sSafariBallsWindowId, FALSE);
        CopyWindowToVram(sSafariBallsWindowId, COPYWIN_GFX);
        RemoveWindow(sSafariBallsWindowId);
    }
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        ClearStdWindowAndFrameToTransparent(sBattlePyramidFloorWindowId, FALSE);
        RemoveWindow(sBattlePyramidFloorWindowId);
    }
}

static bool32 PrintStartMenuActions(s8 *pIndex, u32 count)
{
    s8 index = *pIndex;

    do
    {
        if (sStartMenuItems[sCurrentStartMenuActions[index]].func.u8_void == StartMenuPlayerNameCallback)
        {
            PrintPlayerNameOnWindow(GetStartMenuWindowId(), sStartMenuItems[sCurrentStartMenuActions[index]].text, 8, (index << 4) + 9);
        }
        else
        {
            u8 action = sCurrentStartMenuActions[index];
            const u8 *text = sStartMenuItems[action].text;

            if (UnholyIsSpanish() && sStartMenuSpanishNames[action] != NULL)
                text = sStartMenuSpanishNames[action];
            StringExpandPlaceholders(gStringVar4, text);
            AddTextPrinterParameterized(GetStartMenuWindowId(), FONT_NORMAL, gStringVar4, 8, (index << 4) + 9, TEXT_SKIP_DRAW, NULL);
        }

        if (sCurrentStartMenuActions[index] == MENU_ACTION_HFC_WATCH
         && HorizonMarketHasUnseenMutation())
        {
            // The standard menu palette already includes saturated red in color 4.
            // A three-pixel notification dot fits in the seven-tile START menu.
            FillWindowPixelRect(GetStartMenuWindowId(), PIXEL_FILL(4), 51, (index << 4) + 13, 3, 3);
            CopyWindowToVram(GetStartMenuWindowId(), COPYWIN_GFX);
        }

        index++;
        if (index >= sNumStartMenuActions)
        {
            *pIndex = index;
            return TRUE;
        }

        count--;
    }
    while (count != 0);

    *pIndex = index;
    return FALSE;
}

static bool32 InitStartMenuStep(void)
{
    s8 state = sInitStartMenuData[0];

    switch (state)
    {
    case 0:
        sInitStartMenuData[0]++;
        break;
    case 1:
        BuildStartMenuActions();
        sInitStartMenuData[0]++;
        break;
    case 2:
        LoadMessageBoxAndBorderGfx();
        DrawStdWindowFrame(AddStartMenuWindow(sNumStartMenuActions), FALSE);
        sInitStartMenuData[1] = 0;
        sInitStartMenuData[0]++;
        break;
    case 3:
        if (GetSafariZoneFlag())
            ShowSafariBallsWindow();
        if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
            ShowPyramidFloorWindow();
        sInitStartMenuData[0]++;
        break;
    case 4:
        if (PrintStartMenuActions(&sInitStartMenuData[1], 2))
            sInitStartMenuData[0]++;
        break;
    case 5:
        sStartMenuCursorPos = InitMenuNormal(GetStartMenuWindowId(), FONT_NORMAL, 0, 9, 16, sNumStartMenuActions, sStartMenuCursorPos);
        CopyWindowToVram(GetStartMenuWindowId(), COPYWIN_MAP);
        return TRUE;
    }

    return FALSE;
}

static void InitStartMenu(void)
{
    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    while (!InitStartMenuStep())
        ;
}

static void StartMenuTask(u8 taskId)
{
    if (InitStartMenuStep() == TRUE)
        SwitchTaskToFollowupFunc(taskId);
}

static void CreateStartMenuTask(TaskFunc followupFunc)
{
    u8 taskId;

    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    taskId = CreateTask(StartMenuTask, 0x50);
    SetTaskFuncWithFollowupFunc(taskId, StartMenuTask, followupFunc);
}

static bool8 FieldCB_ReturnToFieldStartMenu(void)
{
    if (InitStartMenuStep() == FALSE)
    {
        return FALSE;
    }

    ReturnToFieldOpenStartMenu();
    return TRUE;
}

void ShowReturnToFieldStartMenu(void)
{
    sInitStartMenuData[0] = 0;
    sInitStartMenuData[1] = 0;
    gFieldCallback2 = FieldCB_ReturnToFieldStartMenu;
}

void Task_ShowStartMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
    case 0:
        if (InUnionRoom() == TRUE)
            SetUsingUnionRoomStartMenu();

        gMenuCallback = HandleStartMenuInput;
        task->data[0]++;
        break;
    case 1:
        if (gMenuCallback() == TRUE)
            DestroyTask(taskId);
        break;
    }
}

void ShowStartMenu(void)
{
    if (!IsOverworldLinkActive())
    {
        FreezeObjectEvents();
        PlayerFreeze();
        StopPlayerAvatar();
    }
    CreateStartMenuTask(Task_ShowStartMenu);
    LockPlayerFieldControls();
}

static bool8 HandleStartMenuInput(void)
{
    if (JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        sStartMenuCursorPos = Menu_MoveCursor(-1);
    }

    if (JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        sStartMenuCursorPos = Menu_MoveCursor(1);
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sStartMenuItems[sCurrentStartMenuActions[sStartMenuCursorPos]].func.u8_void == StartMenuPokedexCallback)
        {
            if (GetNationalPokedexCount(FLAG_GET_SEEN) == 0)
                return FALSE;
        }
        if (sCurrentStartMenuActions[sStartMenuCursorPos] == MENU_ACTION_DEXNAV
          && MapHasNoEncounterData())
            return FALSE;

        gMenuCallback = sStartMenuItems[sCurrentStartMenuActions[sStartMenuCursorPos]].func.u8_void;

        if (gMenuCallback != StartMenuSaveCallback
            && gMenuCallback != StartMenuExitCallback
            && gMenuCallback != StartMenuDebugCallback
            && gMenuCallback != StartMenuHfcWatchCallback
            && gMenuCallback != StartMenuSafariZoneRetireCallback
            && gMenuCallback != StartMenuBattlePyramidRetireCallback)
        {
           FadeScreen(FADE_TO_BLACK, 0);
        }

        return FALSE;
    }

    if (JOY_NEW(START_BUTTON | B_BUTTON))
    {
        RemoveExtraStartMenuWindows();
        HideStartMenu();
        return TRUE;
    }

    return FALSE;
}

bool8 StartMenuPokedexCallback(void)
{
    if (!gPaletteFade.active)
    {
        IncrementGameStat(GAME_STAT_CHECKED_POKEDEX);
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_OpenPokedex);

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPokemonCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_PartyMenuFromStartMenu); // Display party menu

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuBagCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_BagMenuFromStartMenu); // Display bag menu

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPokeNavCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitPokeNav);  // Display PokéNav

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuPlayerNameCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();

        if (IsOverworldLinkActive() || InUnionRoom())
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card
        else if (FlagGet(FLAG_SYS_FRONTIER_PASS))
            ShowFrontierPass(CB2_ReturnToFieldWithOpenMenu); // Display frontier pass
        else
            ShowPlayerTrainerCard(CB2_ReturnToFieldWithOpenMenu); // Display trainer card

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuSaveCallback(void)
{
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        RemoveExtraStartMenuWindows();

    gMenuCallback = SaveStartCallback; // Display save menu

    return FALSE;
}

static bool8 StartMenuOptionCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitOptionMenu); // Display option menu
        gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuExitCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenu(); // Hide start menu

    return TRUE;
}

static bool8 StartMenuDebugCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenuDebug(); // Hide start menu without enabling movement

    if (DEBUG_OVERWORLD_MENU)
    {
        FreezeObjectEvents();
        Debug_ShowMainMenu();
    }

return TRUE;
}

static bool8 StartMenuSafariZoneRetireCallback(void)
{
    RemoveExtraStartMenuWindows();
    HideStartMenu();
    SafariZoneRetirePrompt();

    return TRUE;
}

static void HideStartMenuDebug(void)
{
    PlaySE(SE_SELECT);
    ClearStdWindowAndFrame(GetStartMenuWindowId(), TRUE);
    RemoveStartMenuWindow();
}

static bool8 StartMenuLinkModePlayerNameCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        CleanupOverworldWindowsAndTilemaps();
        ShowTrainerCardInLink(gLocalLinkPlayerId, CB2_ReturnToFieldWithOpenMenu);

        return TRUE;
    }

    return FALSE;
}

static bool8 StartMenuBattlePyramidRetireCallback(void)
{
    gMenuCallback = BattlePyramidRetireStartCallback; // Confirm retire

    return FALSE;
}

// Functionally unused
void ShowBattlePyramidStartMenu(void)
{
    ClearDialogWindowAndFrameToTransparent(0, FALSE);
    ScriptUnfreezeObjectEvents();
    CreateStartMenuTask(Task_ShowStartMenu);
    LockPlayerFieldControls();
}

static bool8 StartMenuBattlePyramidBagCallback(void)
{
    if (!gPaletteFade.active)
    {
        PlayRainStoppingSoundEffect();
        RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_PyramidBagMenuFromStartMenu);

        return TRUE;
    }

    return FALSE;
}

static bool8 SaveStartCallback(void)
{
    InitSave();
    gMenuCallback = SaveCallback;

    return FALSE;
}

static bool8 SaveCallback(void)
{
    switch (RunSaveCallback())
    {
    case SAVE_IN_PROGRESS:
        return FALSE;
    case SAVE_CANCELED: // Back to start menu
        ClearDialogWindowAndFrameToTransparent(0, FALSE);
        InitStartMenu();
        gMenuCallback = HandleStartMenuInput;
        return FALSE;
    case SAVE_SUCCESS:
    case SAVE_ERROR:    // Close start menu
        ClearDialogWindowAndFrameToTransparent(0, TRUE);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        SoftResetInBattlePyramid();
        return TRUE;
    }

    return FALSE;
}

static bool8 BattlePyramidRetireStartCallback(void)
{
    InitBattlePyramidRetire();
    gMenuCallback = BattlePyramidRetireCallback;

    return FALSE;
}

static bool8 BattlePyramidRetireReturnCallback(void)
{
    InitStartMenu();
    gMenuCallback = HandleStartMenuInput;

    return FALSE;
}

static bool8 BattlePyramidRetireCallback(void)
{
    switch (RunSaveCallback())
    {
    case SAVE_SUCCESS: // No (Stay in battle pyramid)
        RemoveExtraStartMenuWindows();
        gMenuCallback = BattlePyramidRetireReturnCallback;
        return FALSE;
    case SAVE_IN_PROGRESS:
        return FALSE;
    case SAVE_CANCELED: // Yes (Retire from battle pyramid)
        ClearDialogWindowAndFrameToTransparent(0, TRUE);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        ScriptContext_SetupScript(BattlePyramid_Retire);
        return TRUE;
    }

    return FALSE;
}

static void InitSave(void)
{
    SaveMapView();
    sSaveDialogCallback = SaveConfirmSaveCallback;
    sSavingComplete = FALSE;
}

static u8 RunSaveCallback(void)
{
    // True if text is still printing
    if (RunTextPrintersAndIsPrinter0Active() == TRUE)
    {
        return SAVE_IN_PROGRESS;
    }

    sSavingComplete = FALSE;
    return sSaveDialogCallback();
}

void SaveGame(void)
{
    InitSave();
    CreateTask(SaveGameTask, 0x50);
}

static void ShowSaveMessage(const u8 *message, u8 (*saveCallback)(void))
{
    StringExpandPlaceholders(gStringVar4, message);
    LoadMessageBoxAndFrameGfx(0, TRUE);
    AddTextPrinterForMessage(TRUE);
    sSavingComplete = TRUE;
    sSaveDialogCallback = saveCallback;
}

static void SaveGameTask(u8 taskId)
{
    u8 status = RunSaveCallback();

    switch (status)
    {
    case SAVE_CANCELED:
    case SAVE_ERROR:
        gSpecialVar_Result = 0;
        break;
    case SAVE_SUCCESS:
        gSpecialVar_Result = status;
        break;
    case SAVE_IN_PROGRESS:
        return;
    }

    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void HideSaveMessageWindow(void)
{
    ClearDialogWindowAndFrame(0, TRUE);
}

static void HideSaveInfoWindow(void)
{
    RemoveSaveInfoWindow();
}

static void SaveStartTimer(void)
{
    sSaveDialogTimer = 60;
}

static bool8 SaveSuccesTimer(void)
{
    sSaveDialogTimer--;

    if (JOY_HELD(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        return TRUE;
    }
    if (sSaveDialogTimer == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static bool8 SaveErrorTimer(void)
{
    if (sSaveDialogTimer != 0)
    {
        sSaveDialogTimer--;
    }
    else if (JOY_HELD(A_BUTTON))
    {
        return TRUE;
    }

    return FALSE;
}

static u8 SaveConfirmSaveCallback(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), FALSE);
    RemoveStartMenuWindow();
    ShowSaveInfoWindow();

    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
    {
        ShowSaveMessage(gText_BattlePyramidConfirmRest, SaveYesNoCallback);
    }
    else
    {
        ShowSaveMessage(gText_ConfirmSave, SaveYesNoCallback);
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveYesNoCallback(void)
{
    DisplayYesNoMenuDefaultYes(); // Show Yes/No menu
    sSaveDialogCallback = SaveConfirmInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        switch (gSaveFileStatus)
        {
        case SAVE_STATUS_EMPTY:
        case SAVE_STATUS_CORRUPT:
            if (gDifferentSaveFile == FALSE && !SKIP_SAVE_CONFIRMATION)
            {
                sSaveDialogCallback = SaveFileExistsCallback;
                return SAVE_IN_PROGRESS;
            }

            sSaveDialogCallback = SaveSavingMessageCallback;
            return SAVE_IN_PROGRESS;
        default:
            if (SKIP_SAVE_CONFIRMATION)
                sSaveDialogCallback = SaveSavingMessageCallback;
            else
                sSaveDialogCallback = SaveFileExistsCallback;
            return SAVE_IN_PROGRESS;
        }
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveInfoWindow();
        HideSaveMessageWindow();
        return SAVE_CANCELED;
    }

    return SAVE_IN_PROGRESS;
}

// A different save file exists
static u8 SaveFileExistsCallback(void)
{
    if (gDifferentSaveFile == TRUE)
    {
        ShowSaveMessage(gText_DifferentSaveFile, SaveConfirmOverwriteDefaultNoCallback);
    }
    else
    {
        ShowSaveMessage(gText_AlreadySavedFile, SaveConfirmOverwriteCallback);
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmOverwriteDefaultNoCallback(void)
{
    DisplayYesNoMenuWithDefault(1); // Show Yes/No menu (No selected as default)
    sSaveDialogCallback = SaveOverwriteInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveConfirmOverwriteCallback(void)
{
    DisplayYesNoMenuDefaultYes(); // Show Yes/No menu
    sSaveDialogCallback = SaveOverwriteInputCallback;
    return SAVE_IN_PROGRESS;
}

static u8 SaveOverwriteInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        sSaveDialogCallback = SaveSavingMessageCallback;
        return SAVE_IN_PROGRESS;
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveInfoWindow();
        HideSaveMessageWindow();
        return SAVE_CANCELED;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveSavingMessageCallback(void)
{
    ShowSaveMessage(gText_SavingDontTurnOff, SaveDoSaveCallback);
    return SAVE_IN_PROGRESS;
}

static u8 SaveDoSaveCallback(void)
{
    u8 saveStatus;

    IncrementGameStat(GAME_STAT_SAVED_GAME);
    PausePyramidChallenge();

    if (gDifferentSaveFile == TRUE)
    {
        saveStatus = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
        gDifferentSaveFile = FALSE;
    }
    else
    {
        saveStatus = TrySavingData(SAVE_NORMAL);
    }

    if (saveStatus == SAVE_STATUS_OK)
        ShowSaveMessage(gText_PlayerSavedGame, SaveSuccessCallback);
    else
        ShowSaveMessage(gText_SaveError, SaveErrorCallback);

    SaveStartTimer();
    return SAVE_IN_PROGRESS;
}

static u8 SaveSuccessCallback(void)
{
    if (!IsTextPrinterActiveOnWindow(0))
    {
        PlaySE(SE_SAVE);
        sSaveDialogCallback = SaveReturnSuccessCallback;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveReturnSuccessCallback(void)
{
    if (!IsSEPlaying() && SaveSuccesTimer())
    {
        HideSaveInfoWindow();
        return SAVE_SUCCESS;
    }
    else
    {
        return SAVE_IN_PROGRESS;
    }
}

static u8 SaveErrorCallback(void)
{
    if (!IsTextPrinterActiveOnWindow(0))
    {
        PlaySE(SE_BOO);
        sSaveDialogCallback = SaveReturnErrorCallback;
    }

    return SAVE_IN_PROGRESS;
}

static u8 SaveReturnErrorCallback(void)
{
    if (!SaveErrorTimer())
    {
        return SAVE_IN_PROGRESS;
    }
    else
    {
        HideSaveInfoWindow();
        return SAVE_ERROR;
    }
}

static void InitBattlePyramidRetire(void)
{
    sSaveDialogCallback = BattlePyramidConfirmRetireCallback;
    sSavingComplete = FALSE;
}

static u8 BattlePyramidConfirmRetireCallback(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), FALSE);
    RemoveStartMenuWindow();
    ShowSaveMessage(gText_BattlePyramidConfirmRetire, BattlePyramidRetireYesNoCallback);

    return SAVE_IN_PROGRESS;
}

static u8 BattlePyramidRetireYesNoCallback(void)
{
    DisplayYesNoMenuWithDefault(1); // Show Yes/No menu (No selected as default)
    sSaveDialogCallback = BattlePyramidRetireInputCallback;

    return SAVE_IN_PROGRESS;
}

static u8 BattlePyramidRetireInputCallback(void)
{
    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: // Yes
        return SAVE_CANCELED;
    case MENU_B_PRESSED:
    case 1: // No
        HideSaveMessageWindow();
        return SAVE_SUCCESS;
    }

    return SAVE_IN_PROGRESS;
}

static void VBlankCB_LinkBattleSave(void)
{
    TransferPlttBuffer();
}

static bool32 InitSaveWindowAfterLinkBattle(u8 *state)
{
    switch (*state)
    {
    case 0:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
        SetVBlankCallback(NULL);
        ScanlineEffect_Stop();
        DmaClear16(3, PLTT, PLTT_SIZE);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        break;
    case 1:
        ResetSpriteData();
        ResetTasks();
        ResetPaletteFade();
        ScanlineEffect_Clear();
        break;
    case 2:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates_LinkBattleSave, ARRAY_COUNT(sBgTemplates_LinkBattleSave));
        InitWindows(sWindowTemplates_LinkBattleSave);
        LoadUserWindowBorderGfx_(0, 8, BG_PLTT_ID(14));
        Menu_LoadStdPalAt(BG_PLTT_ID(15));
        break;
    case 3:
        ShowBg(0);
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        SetVBlankCallback(VBlankCB_LinkBattleSave);
        EnableInterrupts(1);
        break;
    case 4:
        return TRUE;
    }

    (*state)++;
    return FALSE;
}

void CB2_SetUpSaveAfterLinkBattle(void)
{
    if (InitSaveWindowAfterLinkBattle(&gMain.state))
    {
        CreateTask(Task_SaveAfterLinkBattle, 0x50);
        SetMainCallback2(CB2_SaveAfterLinkBattle);
    }
}

static void CB2_SaveAfterLinkBattle(void)
{
    RunTasks();
    UpdatePaletteFade();
}

static void Task_SaveAfterLinkBattle(u8 taskId)
{
    s16 *state = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        switch (*state)
        {
        case 0:
            FillWindowPixelBuffer(0, PIXEL_FILL(1));
            AddTextPrinterParameterized2(0,
                                        FONT_NORMAL,
                                        gText_SavingDontTurnOffPower,
                                        TEXT_SKIP_DRAW,
                                        NULL,
                                        TEXT_COLOR_DARK_GRAY,
                                        TEXT_COLOR_WHITE,
                                        TEXT_COLOR_LIGHT_GRAY);
            DrawTextBorderOuter(0, 8, 14);
            PutWindowTilemap(0);
            CopyWindowToVram(0, COPYWIN_FULL);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);

            if (gWirelessCommType != 0 && InUnionRoom())
            {
                if (Link_AnyPartnersPlayingFRLG_JP())
                {
                    *state = 1;
                }
                else
                {
                    *state = 5;
                }
            }
            else
            {
                gSoftResetDisabled = TRUE;
                *state = 1;
            }
            break;
        case 1:
            SetContinueGameWarpStatusToDynamicWarp();
            WriteSaveBlock2();
            *state = 2;
            break;
        case 2:
            if (WriteSaveBlock1Sector())
            {
                ClearContinueGameWarpStatus2();
                *state = 3;
                gSoftResetDisabled = FALSE;
            }
            break;
        case 3:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            *state = 4;
            break;
        case 4:
            FreeAllWindowBuffers();
            SetMainCallback2(gMain.savedCallback);
            DestroyTask(taskId);
            break;
        case 5:
            CreateTask(Task_LinkFullSave, 5);
            *state = 6;
            break;
        case 6:
            if (!FuncIsActiveTask(Task_LinkFullSave))
            {
                *state = 3;
            }
            break;
        }
    }
}

static void ShowSaveInfoWindow(void)
{
    struct WindowTemplate saveInfoWindow = sSaveInfoWindowTemplate;
    enum Gender gender;
    u8 color;
    u32 xOffset;
    u32 yOffset;

    if (!FlagGet(FLAG_SYS_POKEDEX_GET))
    {
        saveInfoWindow.height -= 2;
    }

    sSaveInfoWindowId = AddWindow(&saveInfoWindow);
    DrawStdWindowFrame(sSaveInfoWindowId, FALSE);

    gender = gSaveBlock2Ptr->playerGender;
    color = TEXT_COLOR_RED;  // Red when female, blue when male.

    if (gender == MALE)
        color = TEXT_COLOR_BLUE;

    // Print region name
    yOffset = 1;
    BufferSaveMenuText(SAVE_MENU_LOCATION, gStringVar4, TEXT_COLOR_GREEN);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, 0, yOffset, TEXT_SKIP_DRAW, NULL);

    // Print player name
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingPlayer, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_NAME, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    PrintPlayerNameOnWindow(sSaveInfoWindowId, gStringVar4, xOffset, yOffset);

    // Print badge count
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingBadges, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_BADGES, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);

    if (FlagGet(FLAG_SYS_POKEDEX_GET) == TRUE)
    {
        // Print Pokédex count
        yOffset += 16;
        AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingPokedex, 0, yOffset, TEXT_SKIP_DRAW, NULL);
        BufferSaveMenuText(SAVE_MENU_CAUGHT, gStringVar4, color);
        xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
        AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);
    }

    // Print play time
    yOffset += 16;
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gText_SavingTime, 0, yOffset, TEXT_SKIP_DRAW, NULL);
    BufferSaveMenuText(SAVE_MENU_PLAY_TIME, gStringVar4, color);
    xOffset = GetStringRightAlignXOffset(FONT_NORMAL, gStringVar4, 0x70);
    AddTextPrinterParameterized(sSaveInfoWindowId, FONT_NORMAL, gStringVar4, xOffset, yOffset, TEXT_SKIP_DRAW, NULL);

    CopyWindowToVram(sSaveInfoWindowId, COPYWIN_GFX);
}

static void RemoveSaveInfoWindow(void)
{
    ClearStdWindowAndFrame(sSaveInfoWindowId, FALSE);
    RemoveWindow(sSaveInfoWindowId);
}

static void Task_WaitForBattleTowerLinkSave(u8 taskId)
{
    if (!FuncIsActiveTask(Task_LinkFullSave))
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

#define tInBattleTower data[2]

void SaveForBattleTowerLink(void)
{
    u8 taskId = CreateTask(Task_LinkFullSave, 5);
    gTasks[taskId].tInBattleTower = TRUE;
    gTasks[CreateTask(Task_WaitForBattleTowerLinkSave, 6)].data[1] = taskId;
}

#undef tInBattleTower

static void HideStartMenuWindow(void)
{
    ClearStdWindowAndFrame(GetStartMenuWindowId(), TRUE);
    RemoveStartMenuWindow();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void HideStartMenu(void)
{
    PlaySE(SE_SELECT);
    HideStartMenuWindow();
}

void AppendToList(u8 *list, u8 *pos, u8 newEntry)
{
    list[*pos] = newEntry;
    (*pos)++;
}

static bool8 StartMenuDexNavCallback(void)
{
    CreateTask(Task_OpenDexNavFromStartMenu, 0);
    return TRUE;
}

void Script_ForceSaveGame(struct ScriptContext *ctx)
{
    SaveGame();
    ShowSaveInfoWindow();
    gMenuCallback = SaveCallback;
    sSaveDialogCallback = SaveSavingMessageCallback;
}
