#include "global.h"
#include "unholy_ui.h"
#include "hfc_mutator.h"
#include "hfc_simulator.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "battle.h"
#include "unholy_qol.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "horizon_market.h"
#include "item.h"
#include "item_menu.h"
#include "menu.h"
#include "mutation_tiers.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "pokemon.h"
#include "pokelocator.h"
#include "pokenav.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "start_menu.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/item.h"
#include "constants/event_objects.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "new_game.h"

#define HUB_TASK_PRIORITY 8
#define HUB_COLUMNS 2
#define HUB_ROWS 3
#define HUB_PAGE_SIZE (HUB_COLUMNS * HUB_ROWS)
#define TOOL_PAGE_SIZE 5

static const u8 sFeatureIcons[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = UNHOLY_UI_ICON_SHOP,
    [HFC_WATCH_FEATURE_POKENAV] = UNHOLY_UI_ICON_NAV,
    [HFC_WATCH_FEATURE_CRAFTING] = UNHOLY_UI_ICON_CRAFT,
    [HFC_WATCH_FEATURE_POKELOCATOR] = UNHOLY_UI_ICON_LOCATOR,
    [HFC_WATCH_FEATURE_TOOLS] = UNHOLY_UI_ICON_TOOLS,
    [HFC_WATCH_FEATURE_STORAGE] = UNHOLY_UI_ICON_BOXES,
    [HFC_WATCH_FEATURE_FREQUENCY] = UNHOLY_UI_ICON_SIGNAL,
    [HFC_WATCH_FEATURE_RAID] = UNHOLY_UI_ICON_RAID,
    [HFC_WATCH_FEATURE_ARCHIVE] = UNHOLY_UI_ICON_ARCHIVE,
    [HFC_WATCH_FEATURE_LAB] = UNHOLY_UI_ICON_MUTATOR,
};

static const u8 *const sFeatureNames[HFC_WATCH_FEATURE_COUNT] =
{
    [HFC_WATCH_FEATURE_HORIZON_SHOP] = COMPOUND_STRING("HORIZON"),
    [HFC_WATCH_FEATURE_POKENAV] = COMPOUND_STRING("POKéNAV"),
    [HFC_WATCH_FEATURE_CRAFTING] = COMPOUND_STRING("CRAFTING"),
    [HFC_WATCH_FEATURE_POKELOCATOR] = COMPOUND_STRING("LOCATOR"),
    [HFC_WATCH_FEATURE_TOOLS] = COMPOUND_STRING("TOOLS"),
    [HFC_WATCH_FEATURE_STORAGE] = COMPOUND_STRING("POKéMON BOXES"),
    [HFC_WATCH_FEATURE_FREQUENCY] = COMPOUND_STRING("MUTANT SIGNAL"),
    [HFC_WATCH_FEATURE_RAID] = COMPOUND_STRING("DAILY RAID"),
    [HFC_WATCH_FEATURE_ARCHIVE] = COMPOUND_STRING("ANOMALY ARCHIVE"),
    [HFC_WATCH_FEATURE_LAB] = COMPOUND_STRING("LABORATORIO"),
};

static EWRAM_DATA u8 sWatchWindowId;
static EWRAM_DATA u8 sHubCursor;
static EWRAM_DATA struct UnholyUiPalette sWatchPalette;
static EWRAM_DATA u8 sLastFeature;
static EWRAM_DATA u8 sToolCursor;
static EWRAM_DATA bool8 sBagSortCompleted;
static EWRAM_DATA u8 sLabCursor;
static EWRAM_DATA bool8 sReturnToStart;
static EWRAM_DATA enum HfcRaidDifficulty sRaidDifficulty;
static EWRAM_DATA bool8 sRaidCaptureSelected;
static EWRAM_DATA enum HfcRaidRewardError sRaidRewardError;

static void OpenHub(void);

u8 HfcWatch_GetTier(void)
{
    return HfcWatch_TierFromItems(CheckBagHasItem(ITEM_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_SUPER_POKE_WATCH, 1),
                                  CheckBagHasItem(ITEM_HF_POKE_WATCH, 1));
}

u8 HfcWatch_CountBadges(void)
{
    u8 badges = 0;

    for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        if (FlagGet(flag))
            badges++;
    return badges;
}

void ScriptUpgradeRegisteredHfcWatch(struct ScriptContext *ctx)
{
    enum Item registered = gSaveBlock1Ptr->registeredItem;

    Script_RequestEffects(SCREFF_V1 | SCREFF_SAVE);
    if (registered == ITEM_POKE_WATCH || registered == ITEM_SUPER_POKE_WATCH)
        gSaveBlock1Ptr->registeredItem = gSpecialVar_0x8004;
}

static bool8 OpenWatchWindow(u8 width, u8 height)
{
    struct WindowTemplate template =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = width,
        .height = height,
        .paletteNum = 14,
        .baseBlock = 0x20,
    };

    sWatchWindowId = AddWindow(&template);
    if (sWatchWindowId == WINDOW_NONE)
        return FALSE;

    LoadMessageBoxAndBorderGfx();
    UnholyUi_AcquirePalette(&sWatchPalette, 14);
    PutWindowTilemap(sWatchWindowId);
    return TRUE;
}

static void CloseWatchWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sWatchWindowId, TRUE);
    RemoveWindow(sWatchWindowId);
    UnholyUi_ReleasePalette(&sWatchPalette);
    sWatchWindowId = WINDOW_NONE;
}

static void ReturnFromWatch(void)
{
    if (sReturnToStart)
        ShowStartMenu();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

static void FieldCB_ReturnToWatch(void)
{
    FadeInFromBlack();
    OpenHub();
}

static void PrintWatchText(const u8 *text, u8 x, u8 y)
{
    UnholyUi_Print(sWatchWindowId, FONT_SMALL, x, y, 184 - min(x, 184), text);
}

static void PrintRaidCaptureChance(u16 basisPoints, u8 y)
{
    u8 text[48];
    u8 *cursor = StringCopy(text, COMPOUND_STRING("Capture chance  "));
    cursor = ConvertUIntToDecimalStringN(cursor, basisPoints / 100, STR_CONV_MODE_LEFT_ALIGN, 3);
    cursor = StringCopy(cursor, COMPOUND_STRING("."));
    cursor = ConvertUIntToDecimalStringN(cursor, basisPoints % 100, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringCopy(cursor, COMPOUND_STRING("%"));
    PrintWatchText(text, 8, y);
}

static void RenderRaidSelection(void)
{
    static const u8 *const sMessages[] =
    {
        [HFC_RAID_NOT_ON_ROUTE] = COMPOUND_STRING("Travel to an outdoor route."),
        [HFC_RAID_USED_TODAY] = COMPOUND_STRING("Come back tomorrow for another raid."),
        [HFC_RAID_NEEDS_PARTY] = COMPOUND_STRING("You need two Pokémon ready to fight."),
        [HFC_RAID_UNAVAILABLE] = COMPOUND_STRING("The raid is unavailable here."),
    };
    enum HfcWatchRaidStatus status = HfcWatch_GetRaidStatus();
    const struct HfcRaidDifficultyInfo *info = HfcWatch_GetRaidDifficultyInfo(sRaidDifficulty);
    u8 text[48];
    u8 *cursor;

    if (status != HFC_RAID_READY)
    {
        PrintWatchText(sMessages[status], 8, 23);
        UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("B:BACK"));
        return;
    }

    for (u8 tier = 0; tier < HFC_RAID_DIFFICULTY_COUNT; tier++)
    {
        u8 x = 5 + 46 * tier;
        const u8 *name = HfcWatch_GetRaidDifficultyInfo(tier)->name;
        bool8 selected = tier == sRaidDifficulty;
        u8 font = GetFontIdToFit(name, FONT_SMALL_NARROW, 0, 40);
        UnholyUi_Card(sWatchWindowId, x, 24, 44, 18, selected);
        UnholyUi_PrintOnFill(sWatchWindowId, font, x + 3, 27, 40, UnholyUi_CardFill(selected), name);
    }
    if (!HfcWatch_IsRaidDifficultyUnlocked(sRaidDifficulty, HfcWatch_CountBadges()))
    {
        cursor = StringCopy(text, COMPOUND_STRING("Unlocks at "));
        cursor = ConvertUIntToDecimalStringN(cursor, info->minBadges, STR_CONV_MODE_LEFT_ALIGN, 1);
        StringCopy(cursor, COMPOUND_STRING(" badges."));
        PrintWatchText(text, 8, 52);
        UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("L/R:TIER  B:BACK"));
        return;
    }

    u16 day = VarGet(VAR_DAYS);
    u32 trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    struct HfcRaidOpponent boss = HfcWatch_GetRaidOpponent(day, trainerId, sRaidDifficulty, HFC_RAID_BOSS, 0);
    if (boss.species == SPECIES_NONE)
    {
        PrintWatchText(sMessages[HFC_RAID_UNAVAILABLE], 8, 52);
        UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("L/R:TIER  B:BACK"));
        return;
    }
    UnholyUi_Card(sWatchWindowId, 5, 51, 182, 38, FALSE);
    PrintWatchText(COMPOUND_STRING("BOSS"), 10, 53);
    cursor = StringCopy(text, GetSpeciesName(boss.species));
    cursor = StringCopy(cursor, COMPOUND_STRING("  Lv. "));
    ConvertUIntToDecimalStringN(cursor, boss.level, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintWatchText(text, 10, 65);
    const u8 *equipment = NULL;
    if (boss.gear != GEAR_NONE)
        equipment = boss.mega ? COMPOUND_STRING("Megapiedra + Artefacto") : COMPOUND_STRING("Objeto + Artefacto");
    else if (boss.heldItem != ITEM_NONE)
        equipment = boss.mega ? COMPOUND_STRING("Mega Stone") : COMPOUND_STRING("Held item");
    if (equipment != NULL)
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 10, 77, 168, equipment);

    PrintRaidCaptureChance(HfcWatch_GetRaidCaptureChance(day, trainerId, sRaidDifficulty), 91);
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 104, 175, COMPOUND_STRING("Two double battles, then the boss."));
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("L/R:TIER  A:START RAID  B:BACK"));
}

static void RenderRaidReward(void)
{
    for (u8 i = 0; i < 2; i++)
    {
        u8 x = 5 + 92 * i;
        bool8 selected = i == sRaidCaptureSelected;
        UnholyUi_Card(sWatchWindowId, x, 25, 90, 22, selected);
        UnholyUi_PrintOnFill(sWatchWindowId, FONT_SMALL, x + 6, 29, 75, UnholyUi_CardFill(selected),
            i == 0 ? COMPOUND_STRING("LOOT") : COMPOUND_STRING("CAPTURE"));
    }
    if (sRaidCaptureSelected)
    {
        PrintRaidCaptureChance(HfcWatch_GetActiveRaidCaptureChance(), 55);
        PrintWatchText(COMPOUND_STRING("If it escapes, you get no loot."), 8, 70);
    }
    else
    {
        for (u8 i = 0; i < HFC_RAID_LOOT_COUNT; i++)
        {
            struct HfcRaidLoot loot = HfcWatch_GetActiveRaidLoot(i);
            u8 text[40];
            u8 *cursor = CopyItemName(loot.item, text);
            cursor = StringCopy(cursor, COMPOUND_STRING("  x"));
            ConvertUIntToDecimalStringN(cursor, loot.quantity, STR_CONV_MODE_LEFT_ALIGN, 2);
            PrintWatchText(text, 8, 54 + 13 * i);
        }
    }

    if (sRaidRewardError == HFC_RAID_REWARD_BAG_FULL)
        PrintWatchText(COMPOUND_STRING("Make room in the Bag."), 8, 104);
    else if (sRaidRewardError == HFC_RAID_REWARD_STORAGE_FULL)
        PrintWatchText(COMPOUND_STRING("Make room for a Pokémon."), 8, 104);
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("L/R:CHOOSE  A:CONFIRM  B:BACK"));
}

static void RenderRaid(void)
{
    const u8 *message;

    switch (HfcWatch_GetRaidState())
    {
    case HFC_RAID_IDLE:
        RenderRaidSelection();
        return;
    case HFC_RAID_NEXT_BATTLE:
        message = HfcWatch_GetRaidStage() == HFC_RAID_BOSS
            ? COMPOUND_STRING("Round 3/3   Mutated boss")
            : COMPOUND_STRING("Round 2/3   Double battle");
        PrintWatchText(message, 8, 22);
        PrintWatchText(COMPOUND_STRING("Your party keeps its HP and PP."), 8, 42);
        UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("A:CONTINUE  B:ABANDON"));
        return;
    case HFC_RAID_CHOOSE_REWARD:
        RenderRaidReward();
        return;
    case HFC_RAID_LOOT_RECEIVED:
        message = COMPOUND_STRING("Loot added to your Bag.");
        break;
    case HFC_RAID_CAPTURED:
        message = COMPOUND_STRING("Pokémon caught!");
        break;
    case HFC_RAID_CAPTURE_FAILED:
        message = COMPOUND_STRING("The mutant escaped.");
        break;
    default:
        message = COMPOUND_STRING("The raid ended.");
        break;
    }

    PrintWatchText(message, 8, 23);
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("B:BACK"));
}

static bool8 IsFieldAssistEnabled(void)
{
    return UnholyQolIsEnabled(UQOL_AUTO_CUT)
        && UnholyQolIsEnabled(UQOL_AUTO_STRENGTH)
        && UnholyQolIsEnabled(UQOL_AUTO_ROCK_SMASH)
        && UnholyQolIsEnabled(UQOL_AUTO_SURF);
}

static bool8 IsToolAvailable(u8 tool)
{
    if (HfcWatch_GetTier() < HfcWatch_ToolMinTier(tool))
        return FALSE;
    if (tool == HFC_WATCH_TOOL_FLASHER)
        return FlagGet(IS_FRLG ? FLAG_BADGE01_GET : FLAG_BADGE02_GET)
            && CheckBagHasItem(ITEM_HM_FLASH, 1);
    return TRUE;
}

static void GetMapPickupProgress(u8 *collected, u8 *total)
{
    *collected = 0;
    *total = 0;
    if (gMapHeader.events == NULL)
        return;

    // Counts only visible item-ball objects on the current map. Hidden items,
    // warps and unvisited maps remain undisclosed; no pickup is performed.
    for (u8 i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *object = &gSaveBlock1Ptr->objectEventTemplates[i];
        if (object->graphicsId != OBJ_EVENT_GFX_ITEM_BALL || object->flagId == 0)
            continue;
        (*total)++;
        if (FlagGet(object->flagId))
            (*collected)++;
    }
}

void HfcWatch_SortAllBagPockets(void)
{
    // The virtual Gear/filter pockets are never persistent bag storage.
    for (enum Pocket pocket = POCKET_ITEMS; pocket < POCKET_GEARS; pocket++)
        SortItemsInBag(&gBagPockets[pocket], SORT_BY_TYPE);
}

static void RenderTools(void)
{
    static const u8 *const names[HFC_WATCH_TOOL_COUNT] =
    {
        COMPOUND_STRING("REPEL AUTO"),
        COMPOUND_STRING("CAMPO AUTO"),
        COMPOUND_STRING("HUEVOS"),
        COMPOUND_STRING("AUTO HEAL"),
        COMPOUND_STRING("ORDENAR BOLSA"),
        COMPOUND_STRING("BOTINES MAPA"),
        COMPOUND_STRING("FLASHER"),
        COMPOUND_STRING("INFINITY REPEL"),
    };
    u8 text[32];
    u8 pageStart = (sToolCursor / TOOL_PAGE_SIZE) * TOOL_PAGE_SIZE;
    for (u8 tool = pageStart; tool < min(pageStart + TOOL_PAGE_SIZE, HFC_WATCH_TOOL_COUNT); tool++)
    {
        const u8 *detail = NULL;
        s8 toggle = -1;
        bool8 available = IsToolAvailable(tool);
        if (!available)
            detail = HfcWatch_GetTier() < HfcWatch_ToolMinTier(tool)
                ? COMPOUND_STRING("OTRO RELOJ") : COMPOUND_STRING("HM + MEDALLA");
        else if (tool == HFC_WATCH_TOOL_AUTO_REPEL)
            toggle = (UnholyQolIsEnabled(UQOL_AUTO_REPEL)) ? 1 : 0;
        else if (tool == HFC_WATCH_TOOL_FIELD_ASSIST)
            toggle = (IsFieldAssistEnabled()) ? 1 : 0;
        else if (tool == HFC_WATCH_TOOL_FLASHER)
            toggle = (UnholyQolIsEnabled(UQOL_AUTO_FLASH)) ? 1 : 0;
        else if (tool == HFC_WATCH_TOOL_INFINITY_REPEL)
            toggle = (UnholyQolIsEnabled(UQOL_INFINITE_REPEL)) ? 1 : 0;
        else if (tool == HFC_WATCH_TOOL_AUTO_HEAL)
            toggle = (UnholyQolIsEnabled(UQOL_AUTO_HEAL)) ? 1 : 0;
        else if (tool == HFC_WATCH_TOOL_SORT_BAG)
            detail = sBagSortCompleted ? COMPOUND_STRING("ORDENADA") : COMPOUND_STRING("A:USAR");
        else if (tool == HFC_WATCH_TOOL_MAP_PICKUPS)
        {
            u8 collected, total;
            u8 *end;
            GetMapPickupProgress(&collected, &total);
            end = ConvertUIntToDecimalStringN(text, collected, STR_CONV_MODE_LEFT_ALIGN, 2);
            end = StringCopy(end, COMPOUND_STRING("/"));
            ConvertUIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 2);
            detail = text;
        }
        else if (tool == HFC_WATCH_TOOL_EGG_TRACKER)
        {
            u8 eggs = 0, cycles = 255;
            for (u8 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
                if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_IS_EGG))
                {
                    eggs++;
                    cycles = min(cycles, GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_FRIENDSHIP));
                }
            u8 *end = ConvertUIntToDecimalStringN(text, eggs, STR_CONV_MODE_LEFT_ALIGN, 1);
            end = StringCopy(end, COMPOUND_STRING(" H / "));
            ConvertUIntToDecimalStringN(end, eggs ? cycles : 0, STR_CONV_MODE_LEFT_ALIGN, 3);
            detail = text;
        }
        UnholyUi_ChoiceRow(sWatchWindowId, 22 + 17 * (tool - pageStart), names[tool], detail, tool == sToolCursor);
        if (toggle >= 0)
            UnholyUi_Switch(sWatchWindowId, 182, 25 + 17 * (tool - pageStart), toggle);
    }
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 145, 5, 37,
        pageStart == 0 ? COMPOUND_STRING("1/2") : COMPOUND_STRING("2/2"));
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 108, 195,
        sToolCursor == HFC_WATCH_TOOL_EGG_TRACKER ? COMPOUND_STRING("C = ciclos para eclosion")
      : sToolCursor == HFC_WATCH_TOOL_AUTO_REPEL ? COMPOUND_STRING("Reutiliza repelentes existentes")
      : sToolCursor == HFC_WATCH_TOOL_INFINITY_REPEL ? COMPOUND_STRING("Sin gasto; respeta niveles")
      : sToolCursor == HFC_WATCH_TOOL_FIELD_ASSIST ? COMPOUND_STRING("Requiere MO y medalla")
      : sToolCursor == HFC_WATCH_TOOL_FLASHER ? COMPOUND_STRING("Alumbra cuevas con MO y medalla")
      : sToolCursor == HFC_WATCH_TOOL_SORT_BAG ? COMPOUND_STRING("Ordena todos los bolsillos")
      : sToolCursor == HFC_WATCH_TOOL_MAP_PICKUPS ? COMPOUND_STRING("Bolas del mapa recogidas/total")
      : COMPOUND_STRING("Pociones tras cada combate"));
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("UP/DN:SEL  A:USAR  B:ATRAS"));
}

static void RenderLaboratory(void)
{
    UnholyUi_ChoiceRow(sWatchWindowId, 31, COMPOUND_STRING("SIMULADOR"),
                       COMPOUND_STRING("SUPER / 3 MED."), sLabCursor == 0);
    UnholyUi_ChoiceRow(sWatchWindowId, 55, COMPOUND_STRING("MUTADOR"),
                       HfcWatch_GetTier() >= HFC_WATCH_TIER_HF
                           ? COMPOUND_STRING("HF DISPONIBLE") : COMPOUND_STRING("REQUIERE HF"), sLabCursor == 1);
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 83, 192,
        sLabCursor == 0 ? COMPOUND_STRING("Entrena EV e IV pagando el coste")
                        : COMPOUND_STRING("Modifica mutaciones pagando el coste"));
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("UP/DN:SEL  A:ABRIR  B:ATRAS"));
}

static void RenderFrequency(void)
{
    u8 remaining = HfcWatch_GetFrequencyRemaining();
    bool8 canActivate = remaining == 0 && HfcWatch_CanActivateFrequency();
    u8 text[40];
    u8 *cursor;

    UnholyUi_Card(sWatchWindowId, 5, 23, 182, 23, remaining != 0);
    if (remaining != 0)
    {
        cursor = ConvertUIntToDecimalStringN(text, remaining, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(cursor, COMPOUND_STRING(" encounters left"));
    }
    else if (canActivate)
    {
        cursor = StringCopy(text, COMPOUND_STRING("Tier C/B mutation odds +"));
        cursor = ConvertUIntToDecimalStringN(cursor, HfcWatch_GetFrequencyBoost(HfcWatch_CountBadges()), STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(cursor, COMPOUND_STRING("%"));
    }
    else
    {
        StringCopy(text, FlagGet(FLAG_DAILY_HFC_FREQUENCY)
                         ? COMPOUND_STRING("Used today") : COMPOUND_STRING("No signal here"));
    }
    UnholyUi_PrintOnFill(sWatchWindowId, FONT_SMALL, 11, 28, 170, UnholyUi_CardFill(remaining != 0), text);
    if (remaining != 0)
        UnholyUi_Bar(sWatchWindowId, 11, 41, 170, remaining, 10, UnholyUi_AccentColor());
    if (remaining != 0)
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 49, 175, COMPOUND_STRING("Active for wild encounters."));
    else if (canActivate)
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 49, 175, COMPOUND_STRING("10 wild encounters, once per day."));
    else
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 8, 49, 175, FlagGet(FLAG_DAILY_HFC_FREQUENCY)
            ? COMPOUND_STRING("Come back tomorrow.") : COMPOUND_STRING("Find an area with wild encounters."));
    UnholyUi_Footer(sWatchWindowId, canActivate ? COMPOUND_STRING("A:ACTIVATE  B:BACK") : COMPOUND_STRING("B:BACK"));
}

static void RenderArchive(void)
{
    static const enum PokemonBadge order[] =
    {
        POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER,
        POKEMON_BADGE_OVERLOADED, POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
        POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
    };
    u16 registered = HfcWatch_ReadArchive();
    for (u8 i = 0; i < ARRAY_COUNT(order); i++)
    {
        u8 x = 5 + (i % 3) * 61;
        u8 y = 24 + (i / 3) * 31;
        bool8 found = (registered & order[i]) != 0;
        u8 fill = UnholyUi_CardFill(found);
        const u8 *name = HorizonMarketMutationName(order[i]);
        u8 font = GetFontIdToFit(name, FONT_SMALL_NARROW, 0, 52);
        UnholyUi_Card(sWatchWindowId, x, y, 59, 29, found);
        if (found)
            UnholyUi_Halo(sWatchWindowId, x, y, 59, 29, UnholyUi_AccentColor());
        UnholyUi_PrintOnFill(sWatchWindowId, font, x + 4, y + 1, 52, fill, name);

        u8 status[18];
        u8 *end = StringCopy(status, MutationTierGetName(MutationTierFromBadge(order[i])));
        StringCopy(end, found ? COMPOUND_STRING("  SEEN") : COMPOUND_STRING("  UNSEEN"));
        font = GetFontIdToFit(status, FONT_SMALL_NARROW, 0, 52);
        UnholyUi_PrintOnFill(sWatchWindowId, font, x + 4, y + 13, 52, fill, status);
    }
    UnholyUi_Footer(sWatchWindowId, COMPOUND_STRING("B:BACK"));
}

static void RenderPanel(u8 feature)
{
    const u8 *title = sFeatureNames[feature];
    if (feature == HFC_WATCH_FEATURE_RAID && HfcWatch_GetRaidState() == HFC_RAID_CHOOSE_REWARD)
        title = COMPOUND_STRING("RAID REWARD");
    UnholyUi_Frame(sWatchWindowId, title);
    UnholyUi_Icon(sWatchWindowId, GetWindowAttribute(sWatchWindowId, WINDOW_WIDTH) * TILE_WIDTH - 18, 6, sFeatureIcons[feature], UnholyUi_AccentColor());

    switch (feature)
    {
    case HFC_WATCH_FEATURE_RAID:
        RenderRaid();
        break;
    case HFC_WATCH_FEATURE_FREQUENCY:
        RenderFrequency();
        break;
    case HFC_WATCH_FEATURE_ARCHIVE:
        RenderArchive();
        break;
    case HFC_WATCH_FEATURE_TOOLS:
        RenderTools();
        break;
    case HFC_WATCH_FEATURE_LAB:
        RenderLaboratory();
        break;
    }
    CopyWindowToVram(sWatchWindowId, COPYWIN_FULL);
}

static void CloseWatchPanel(u8 taskId)
{
    CloseWatchWindow();
    DestroyTask(taskId);
}

static void HandleRaidPanelInput(u8 taskId)
{
    enum HfcRaidState state = HfcWatch_GetRaidState();

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (state != HFC_RAID_CHOOSE_REWARD)
            HfcWatch_ResetRaid();
        CloseWatchPanel(taskId);
        OpenHub();
        return;
    }

    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        if (state == HFC_RAID_IDLE && HfcWatch_GetRaidStatus() == HFC_RAID_READY)
        {
            u8 step = JOY_NEW(DPAD_LEFT | L_BUTTON) ? HFC_RAID_DIFFICULTY_COUNT - 1 : 1;
            sRaidDifficulty = (sRaidDifficulty + step) % HFC_RAID_DIFFICULTY_COUNT;
        }
        else if (state == HFC_RAID_CHOOSE_REWARD)
        {
            sRaidCaptureSelected = !sRaidCaptureSelected;
            sRaidRewardError = HFC_RAID_REWARD_OK;
        }
        else
            return;

        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
        return;
    }

    if (!JOY_NEW(A_BUTTON))
        return;

    if ((state == HFC_RAID_IDLE && HfcWatch_StartDailyRaid(sRaidDifficulty))
     || (state == HFC_RAID_NEXT_BATTLE && HfcWatch_ContinueRaid()))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
    }
    else if (state == HFC_RAID_CHOOSE_REWARD)
    {
        sRaidRewardError = HfcWatch_ClaimRaidReward(sRaidCaptureSelected);
        PlaySE(sRaidRewardError == HFC_RAID_REWARD_OK ? SE_SELECT : SE_FAILURE);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
    }
    else if (state == HFC_RAID_NEXT_BATTLE)
    {
        PlaySE(SE_FAILURE);
        RenderPanel(HFC_WATCH_FEATURE_RAID);
    }
}

static void HandleToolsInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
        OpenHub();
    }
    else if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        sToolCursor = (sToolCursor + (JOY_REPEAT(DPAD_UP) ? HFC_WATCH_TOOL_COUNT - 1 : 1)) % HFC_WATCH_TOOL_COUNT;
        sBagSortCompleted = FALSE;
        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_TOOLS);
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        u8 nextPage = sToolCursor / TOOL_PAGE_SIZE == 0 ? TOOL_PAGE_SIZE : 0;
        sToolCursor = min(nextPage + sToolCursor % TOOL_PAGE_SIZE, HFC_WATCH_TOOL_COUNT - 1);
        sBagSortCompleted = FALSE;
        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_TOOLS);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (!IsToolAvailable(sToolCursor))
        {
            PlaySE(SE_FAILURE);
            return;
        }
        switch (sToolCursor)
        {
        case HFC_WATCH_TOOL_AUTO_REPEL:
            UnholyQolToggleOption(UQOL_AUTO_REPEL);
            break;
        case HFC_WATCH_TOOL_FIELD_ASSIST:
        {
            bool8 turnOn = !IsFieldAssistEnabled();
            static const enum UnholyQolOption options[] =
            {
                UQOL_AUTO_CUT, UQOL_AUTO_STRENGTH, UQOL_AUTO_ROCK_SMASH, UQOL_AUTO_SURF,
            };
            for (u8 i = 0; i < ARRAY_COUNT(options); i++)
                if (UnholyQolIsEnabled(options[i]) != turnOn)
                    UnholyQolToggleOption(options[i]);
            break;
        }
        case HFC_WATCH_TOOL_FLASHER:
            UnholyQolToggleOption(UQOL_AUTO_FLASH);
            if (UnholyQolIsEnabled(UQOL_AUTO_FLASH) && gMapHeader.cave && !FlagGet(FLAG_SYS_USE_FLASH))
            {
                FlagSet(FLAG_SYS_USE_FLASH);
                SetFlashLevel(1);
                WriteFlashScanlineEffectBuffer(1);
            }
            break;
        case HFC_WATCH_TOOL_INFINITY_REPEL:
            UnholyQolToggleOption(UQOL_INFINITE_REPEL);
            break;
        case HFC_WATCH_TOOL_AUTO_HEAL:
            UnholyQolToggleOption(UQOL_AUTO_HEAL);
            break;
        case HFC_WATCH_TOOL_SORT_BAG:
            HfcWatch_SortAllBagPockets();
            sBagSortCompleted = TRUE;
            break;
        default:
            break; // Read-only trackers never change the save.
        }
        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_TOOLS);
    }
}

static void HandleLaboratoryInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
        OpenHub();
    }
    else if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        sLabCursor = !sLabCursor;
        PlaySE(SE_SELECT);
        RenderPanel(HFC_WATCH_FEATURE_LAB);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sLabCursor == 1 && HfcWatch_GetTier() < HFC_WATCH_TIER_HF)
        {
            PlaySE(SE_FAILURE);
            return;
        }
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
        if (sLabCursor == 0)
            HfcSimulator_Open(OpenHub);
        else
            HfcMutator_Open(OpenHub);
    }
}

static void Task_HfcWatchPanel(u8 taskId)
{
    u8 feature = gTasks[taskId].data[0];
    if (gPaletteFade.active)
        return;
    UnholyUi_Tick(14);

    if (feature == HFC_WATCH_FEATURE_RAID)
        HandleRaidPanelInput(taskId);
    else if (feature == HFC_WATCH_FEATURE_TOOLS)
        HandleToolsInput(taskId);
    else if (feature == HFC_WATCH_FEATURE_LAB)
        HandleLaboratoryInput(taskId);
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWatchPanel(taskId);
        OpenHub();
    }
    else if (JOY_NEW(A_BUTTON) && feature == HFC_WATCH_FEATURE_FREQUENCY && HfcWatch_ActivateFrequency())
    {
        PlaySE(SE_SELECT);
        RenderPanel(feature);
    }
}

static void OpenPanel(u8 feature)
{
    u8 height = 10;
    if (feature == HFC_WATCH_FEATURE_ARCHIVE)
        height = 18;
    else if (feature == HFC_WATCH_FEATURE_RAID)
        height = 17;
    else if (feature == HFC_WATCH_FEATURE_TOOLS || feature == HFC_WATCH_FEATURE_LAB)
        height = 18;

    u8 taskId = CreateTask(Task_HfcWatchPanel, HUB_TASK_PRIORITY);
    if (gTasks[taskId].func != Task_HfcWatchPanel)
    {
        ReturnFromWatch();
        return;
    }
    if (!OpenWatchWindow(24, height))
    {
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }

    gTasks[taskId].data[0] = feature;
    RenderPanel(feature);
}

void HfcWatch_OpenRaidAfterBattle(void)
{
    sRaidCaptureSelected = FALSE;
    sRaidRewardError = HFC_RAID_REWARD_OK;
    FreezeObjectEvents();
    LockPlayerFieldControls();
    Overworld_PlaySpecialMapMusic();
    FadeInFromBlack();
    OpenPanel(HFC_WATCH_FEATURE_RAID);
}

static void Task_LaunchHfcWatchFeature(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    CloseWatchWindow();
    DestroyTask(taskId);

    if (sLastFeature == HFC_WATCH_FEATURE_CRAFTING)
    {
        OpenUnholyCraftingMenu(OpenHub);
        return;
    }

    PlayRainStoppingSoundEffect();
    CleanupOverworldWindowsAndTilemaps();

    switch (sLastFeature)
    {
    case HFC_WATCH_FEATURE_HORIZON_SHOP:
        CreateHorizonShopMenu(FieldCB_ReturnToWatch);
        FadeScreen(FADE_FROM_BLACK, 0);
        break;
    case HFC_WATCH_FEATURE_POKENAV:
        OpenPokeNavWithFieldReturn(FieldCB_ReturnToWatch);
        break;
    case HFC_WATCH_FEATURE_STORAGE:
        OpenPokemonStorageFromField(FieldCB_ReturnToWatch);
        break;
    }
}

static struct HfcWatchProgress HubProgress(void)
{
    return (struct HfcWatchProgress){HfcWatch_CountBadges(), FlagGet(FLAG_CRAFTING_UNLOCKED)};
}

static void RenderHub(void)
{
    u8 tier = HfcWatch_GetTier();
    u8 pageStart = (sHubCursor / HUB_PAGE_SIZE) * HUB_PAGE_SIZE;
    struct HfcWatchProgress progress = HubProgress();
    UnholyUi_Frame(sWatchWindowId, COMPOUND_STRING("POKé WATCH"));
    {
        u8 clock[8];
        u8 *end;

        RtcCalcLocalTime();
        end = ConvertUIntToDecimalStringN(clock, gLocalTime.hours, STR_CONV_MODE_LEADING_ZEROS, 2);
        *end++ = CHAR_COLON;
        ConvertUIntToDecimalStringN(end, gLocalTime.minutes, STR_CONV_MODE_LEADING_ZEROS, 2);
        UnholyUi_Icon(sWatchWindowId, 84, 6, UNHOLY_UI_ICON_DOT, UnholyUi_AccentColor());
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 95, 6, 32, clock);
    }
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 130, 6, 48,
        tier == HFC_WATCH_TIER_HF ? COMPOUND_STRING("HF")
      : tier == HFC_WATCH_TIER_SUPER ? COMPOUND_STRING("SUPER") : COMPOUND_STRING("BASIC"));
    UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 178, 6, 28,
        pageStart == 0 ? COMPOUND_STRING("1/2") : COMPOUND_STRING("2/2"));
    bool32 hasRareStock = HorizonMarketHasUnseenMutation();
    for (u8 card = 0; card < HUB_PAGE_SIZE && pageStart + card < HFC_WATCH_FEATURE_COUNT; card++)
    {
        u8 feature = pageStart + card;
        u8 x = 5 + (card % HUB_COLUMNS) * 101;
        u8 y = 23 + (card / HUB_COLUMNS) * 26;
        bool8 selected = feature == sHubCursor;
        bool8 ready = HfcWatch_GetAccess(tier, feature, progress) == HFC_WATCH_READY;
        u8 fill = UnholyUi_CardFill(selected);
        UnholyUi_Card(sWatchWindowId, x, y, 97, 23, selected);
        if (selected)
            UnholyUi_Halo(sWatchWindowId, x, y, 97, 23, UnholyUi_AccentColor());
        // Eight-pixel service icon; dimmed while the service is locked.
        UnholyUi_Icon(sWatchWindowId, x + 5, y + 7, sFeatureIcons[feature],
                      ready ? UnholyUi_AccentColor() : (FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? 3 : 11));
        UnholyUi_PrintOnFill(sWatchWindowId, FONT_SMALL_NARROW, x + 17, y + (ready ? 5 : 1), 70, fill, sFeatureNames[feature]);
        if (!ready)
        {
            UnholyUi_Icon(sWatchWindowId, x + 85, y + 13, UNHOLY_UI_ICON_LOCK, FlagGet(FLAG_UNHOLY_UI_DAYLIGHT) ? 3 : 11);
            UnholyUi_PrintOnFill(sWatchWindowId, FONT_SMALL_NARROW, x + 17, y + 11, 66, fill, COMPOUND_STRING("LOCKED"));
        }
        if (feature == HFC_WATCH_FEATURE_HORIZON_SHOP && ready && hasRareStock)
            FillWindowPixelRect(sWatchWindowId, PIXEL_FILL(13), x + 87, y + 9, 3, 3);
    }
    bool8 ready = HfcWatch_GetAccess(tier, sHubCursor, progress) == HFC_WATCH_READY;
    if (!ready)
        UnholyUi_Print(sWatchWindowId, FONT_SMALL_NARROW, 7, 104, 194,
            HfcWatch_GetRequirement(tier, sHubCursor, progress));
    UnholyUi_Footer(sWatchWindowId, ready ? COMPOUND_STRING("L/R:PAGE  A:OPEN  B:BACK") : COMPOUND_STRING("L/R:PAGE  B:BACK"));
    CopyWindowToVram(sWatchWindowId, COPYWIN_FULL);
}

static void MoveHubCursor(u16 keys)
{
    u8 first = sHubCursor / HUB_PAGE_SIZE * HUB_PAGE_SIZE;
    u8 index = sHubCursor - first;
    u8 count = min(HUB_PAGE_SIZE, HFC_WATCH_FEATURE_COUNT - first);

    if (keys & (L_BUTTON | R_BUTTON))
    {
        first = first == 0 ? HUB_PAGE_SIZE : 0;
        count = min(HUB_PAGE_SIZE, HFC_WATCH_FEATURE_COUNT - first);
        index = min(index, count - 1);
    }
    else if (keys & DPAD_LEFT)
        index = index % HUB_COLUMNS == 0 ? index : index - 1;
    else if (keys & DPAD_RIGHT)
        index = index % HUB_COLUMNS == 0 && index + 1 < count ? index + 1 : index;
    else if (keys & DPAD_UP)
        index = index >= HUB_COLUMNS ? index - HUB_COLUMNS : index;
    else if (keys & DPAD_DOWN)
        index = index + HUB_COLUMNS < count ? index + HUB_COLUMNS : index;
    sHubCursor = first + index;
}

static void Task_HfcWatchHub(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    UnholyUi_Tick(14);
    // Keep the header clock current without waiting for input.
    if (++gTasks[taskId].data[1] >= 600)
    {
        gTasks[taskId].data[1] = 0;
        RenderHub();
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseWatchWindow();
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }
    u16 navigation = JOY_NEW(L_BUTTON | R_BUTTON) | JOY_REPEAT(DPAD_ANY);
    if (navigation)
    {
        u8 previous = sHubCursor;
        MoveHubCursor(navigation);
        if (previous != sHubCursor)
        {
            PlaySE(SE_SELECT);
            RenderHub();
        }
        return;
    }
    if (!JOY_NEW(A_BUTTON))
        return;
    if (HfcWatch_GetAccess(HfcWatch_GetTier(), sHubCursor, HubProgress()) != HFC_WATCH_READY)
    {
        PlaySE(SE_FAILURE);
        RenderHub();
        return;
    }
    PlaySE(SE_SELECT);
    sLastFeature = sHubCursor;
    switch (sLastFeature)
    {
    case HFC_WATCH_FEATURE_HORIZON_SHOP:
    case HFC_WATCH_FEATURE_POKENAV:
    case HFC_WATCH_FEATURE_CRAFTING:
    case HFC_WATCH_FEATURE_STORAGE:
        gTasks[taskId].func = Task_LaunchHfcWatchFeature;
        FadeScreen(FADE_TO_BLACK, 0);
        break;
    case HFC_WATCH_FEATURE_POKELOCATOR:
        CloseWatchWindow();
        DestroyTask(taskId);
        Pokelocator_OpenFromField(OpenHub);
        break;
    default:
        CloseWatchWindow();
        DestroyTask(taskId);
        OpenPanel(sLastFeature);
        break;
    }
}

static void OpenHub(void)
{
    if (HfcWatch_GetTier() == HFC_WATCH_TIER_NONE)
    {
        ReturnFromWatch();
        return;
    }
    u8 taskId = CreateTask(Task_HfcWatchHub, HUB_TASK_PRIORITY);
    if (gTasks[taskId].func != Task_HfcWatchHub)
    {
        ReturnFromWatch();
        return;
    }
    if (!OpenWatchWindow(26, 17))
    {
        DestroyTask(taskId);
        ReturnFromWatch();
        return;
    }
    FreezeObjectEvents();
    LockPlayerFieldControls();
    sHubCursor = sLastFeature;
    RenderHub();
}

void HfcWatch_OpenHub(bool8 fromStartMenu)
{
    if (FuncIsActiveTask(Task_HfcWatchHub) || FuncIsActiveTask(Task_LaunchHfcWatchFeature)
     || FuncIsActiveTask(Task_HfcWatchPanel))
        return;

    sReturnToStart = fromStartMenu;
    sLastFeature = sHubCursor;
    sRaidDifficulty = HFC_RAID_STANDARD;
    OpenHub();
}
