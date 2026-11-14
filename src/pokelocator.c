#include "global.h"
#include "pokelocator.h"
#include "battle_main.h"
#include "caps.h"
#include "pokedex.h"
#include "dma3.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "hfc_watch.h"
#include "hfc_watch_services.h"
#include "horizon_market.h"
#include "item.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "mutation_icons.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "region_map.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "unholy_ui.h"
#include "window.h"
#include "config/special_badges.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/songs.h"

#define VISIBLE_CARDS 6
#define WIN_LEFT 8
#define WIN_TOP 8
#define WIN_WIDTH 224
#define WIN_HEIGHT 144
#define ICON_PALETTES 6
#define MUTATION_COUNT 9

enum LocatorMode { MODE_LIST, MODE_DETAIL, MODE_SEARCH, MODE_TOOLS, MODE_SIGNAL, MODE_SIGNAL_CONFIRM, MODE_ACTIONS, MODE_BATCH, MODE_OBTAIN, MODE_RESULT };
enum DetailPage { PAGE_HABITAT, PAGE_STATS, PAGE_TRAITS, PAGE_COUNT };

struct PokelocatorUi
{
    struct PokelocatorList list;
    struct PokelocatorZone zone;
    struct UnholyUiPalette palette;
    struct PokelocatorObtainPlan plan;
    struct PokelocatorObtainResult result;
    u8 obtainStatus, action;
    u8 catalog[POKELOCATOR_MAX_ROWS];
    u8 methods[POKELOCATOR_METHOD_COUNT];
    u8 count, cursor, scroll, methodCount, methodCursor;
    u8 group, collection, mode, page, toolCursor, windowId;
    u8 iconIds[VISIBLE_CARDS], mutationIds[MUTATION_COUNT], ownedPalettes;
    void (*onClose)(void);
};

// Resume only within this field session. No per-species save allocation or invented
// search-level bonuses: existing saves and native encounter generation stay intact.
struct LocatorSelection
{
    u16 originHeader, viewHeader, species;
    u8 method, group, collection;
    bool8 valid;
};
static EWRAM_DATA struct PokelocatorUi sUi = {0};
static EWRAM_DATA struct LocatorSelection sRemember = {0};
static const struct WindowTemplate sWindow =
{
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 18,
    .paletteNum = 14, .baseBlock = 1,
};
STATIC_ASSERT(1 + 28 * 18 <= 512, LocatorTilesBeforeFrame);

static const u8 *const sGroups[] =
{
    COMPOUND_STRING("ALL"), COMPOUND_STRING("LAND"), COMPOUND_STRING("WATER"),
    COMPOUND_STRING("FISH"), COMPOUND_STRING("ROCK"), COMPOUND_STRING("HONEY"), COMPOUND_STRING("HIDDEN"),
};
static const u8 *const sCollections[] =
{
    COMPOUND_STRING("ALL"), COMPOUND_STRING("MISSING"), COMPOUND_STRING("CAUGHT"),
};
static const u8 *const sMethods[] =
{
    COMPOUND_STRING("GRASS"), COMPOUND_STRING("SURF"), COMPOUND_STRING("ROCK SMASH"),
    COMPOUND_STRING("OLD ROD"), COMPOUND_STRING("GOOD ROD"), COMPOUND_STRING("SUPER ROD"),
    COMPOUND_STRING("HIDDEN"),
};
static const u16 sMutationBadges[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER,
    POKEMON_BADGE_OVERLOADED, POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
    POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
};
static const u16 sMutationRates[] =
{
    SPECIAL_BADGE_ALPHA_DENOMINATOR, SPECIAL_BADGE_HYPER_DENOMINATOR, SPECIAL_BADGE_SUPER_DENOMINATOR,
    SPECIAL_BADGE_OVERLOADED_DENOMINATOR, SPECIAL_BADGE_ENRAGED_DENOMINATOR, SPECIAL_BADGE_ANCESTRAL_DENOMINATOR,
    SPECIAL_BADGE_TITAN_DENOMINATOR, SPECIAL_BADGE_ASCENDED_DENOMINATOR, SPECIAL_BADGE_OMEGA_DENOMINATOR,
};

extern const u8 EventScript_PokelocatorSearchBattle[];
extern const u8 EventScript_PokelocatorSearchUnavailable[];
static void Task_Locator(u8 taskId);


static void Print(u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_Print(sUi.windowId, FONT_SMALL_NARROW, x, y, width, text);
}

static void Number(u8 *text, u32 value)
{
    ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static u16 Header(void)
{
    return sUi.zone.count ? sUi.zone.headerIds[sUi.zone.current] : HEADER_NONE;
}

static const struct PokelocatorRow *CatalogRow(void)
{
    return sUi.count ? &sUi.list.rows[sUi.catalog[sUi.cursor]] : NULL;
}

static const struct PokelocatorRow *SearchRow(void)
{
    return sUi.methodCount ? &sUi.list.rows[sUi.methods[sUi.methodCursor]] : NULL;
}

static const u8 *MethodName(u8 method)
{
    if (method == POKELOCATOR_METHOD_LAND && Header() != HEADER_NONE)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[Header()];
        if (Overworld_GetMapHeaderByGroupAndId(h->mapGroup, h->mapNum)->mapType == MAP_TYPE_UNDERGROUND)
            return COMPOUND_STRING("CAVE");
    }
    return sMethods[method];
}

static const u8 *DexLabel(u16 species)
{
    switch (Pokelocator_GetDexState(species))
    {
    case POKELOCATOR_DEX_CAUGHT: return COMPOUND_STRING("CAUGHT");
    case POKELOCATOR_DEX_SEEN: return COMPOUND_STRING("SEEN");
    default: return COMPOUND_STRING("NEW");
    }
}

static u8 *Levels(u8 *dst, const struct PokelocatorRow *row)
{
    dst = StringCopy(dst, COMPOUND_STRING("Lv "));
    dst = ConvertIntToDecimalStringN(dst, row->minLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (row->minLevel != row->maxLevel)
    {
        dst = StringAppend(dst, COMPOUND_STRING("-"));
        dst = ConvertIntToDecimalStringN(dst, row->maxLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    return dst;
}

static void SetMethods(u8 preferred)
{
    const struct PokelocatorRow *row = CatalogRow();
    sUi.methodCount = 0;
    if (row)
        for (u8 i = 0; i < sUi.list.count; i++)
            if (sUi.list.rows[i].species == row->species && Pokelocator_MethodInGroup(sUi.list.rows[i].method, sUi.group))
                sUi.methods[sUi.methodCount++] = i;
    sUi.methodCursor = 0;
    for (u8 i = 0; i < sUi.methodCount; i++)
        if (sUi.list.rows[sUi.methods[i]].method == preferred)
            sUi.methodCursor = i;
}

static void Refilter(u16 preferredSpecies, u8 preferredMethod)
{
    sUi.count = Pokelocator_BuildCatalogIndex(&sUi.list, sUi.group, sUi.collection,
                                            Pokelocator_GetDexState, sUi.catalog);
    sUi.cursor = sUi.scroll = 0;
    for (u8 i = 0; i < sUi.count; i++)
        if (sUi.list.rows[sUi.catalog[i]].species == preferredSpecies)
            sUi.cursor = i;
    sUi.scroll = sUi.cursor / VISIBLE_CARDS * VISIBLE_CARDS;
    SetMethods(preferredMethod);
}

static bool8 GroupAvailable(u8 group)
{
    for (u8 i = 0; i < sUi.list.count; i++)
        if (Pokelocator_MethodInGroup(sUi.list.rows[i].method, group)) return TRUE;
    return FALSE;
}

static void LoadArea(u8 area)
{
    sUi.zone.current = area;
    if (sUi.zone.count)
        Pokelocator_BuildAvailableList(&sUi.list, Header());
    else
        Pokelocator_ClearList(&sUi.list);
    if (sUi.group == POKELOCATOR_GROUP_ALL || !GroupAvailable(sUi.group))
    {
        sUi.group = POKELOCATOR_GROUP_LAND;
        for (u8 group = 1; group < POKELOCATOR_GROUP_COUNT; group++)
            if (GroupAvailable(group)) { sUi.group = group; break; }
    }
    Refilter(SPECIES_NONE, POKELOCATOR_METHOD_COUNT);
}

static void DestroyIcons(void)
{
    for (u8 i = 0; i < MUTATION_COUNT; i++)
    {
        MutationIcon_Destroy(sUi.mutationIds[i]);
        sUi.mutationIds[i] = MAX_SPRITES;
    }
    for (u8 i = 0; i < VISIBLE_CARDS; i++)
    {
        if (sUi.iconIds[i] != MAX_SPRITES)
            FreeAndDestroyMonIconSprite(&gSprites[sUi.iconIds[i]]);
        sUi.iconIds[i] = MAX_SPRITES;
    }
    for (u8 i = 0; i < ICON_PALETTES; i++)
        if (sUi.ownedPalettes & (1 << i))
            FreeSpritePaletteByTag(gMonIconPaletteTable[i].tag);
    sUi.ownedPalettes = 0;
}

static void MonIcon(u8 slot, u16 species, s16 x, s16 y)
{
    u8 palette = GetMonIconPaletteIndexFromSpecies(species);
    u16 tag = gMonIconPaletteTable[palette].tag;
    if (IndexOfSpritePaletteTag(tag) == 0xFF)
    {
        LoadMonIconPalette(species);
        if (IndexOfSpritePaletteTag(tag) == 0xFF)
            return;
        sUi.ownedPalettes |= 1 << palette;
    }
    u8 sprite = CreateMonIconNoPersonality(species, SpriteCB_MonIcon, WIN_LEFT + x, WIN_TOP + y, 0);
    if (sprite != MAX_SPRITES)
    {
        gSprites[sprite].oam.priority = 0;
        sUi.iconIds[slot] = sprite;
    }
}

static void DrawHeader(void)
{
    u8 text[48];
    const u8 *title = COMPOUND_STRING("POKéLOCATOR");
    if (sUi.mode == MODE_TOOLS) title = COMPOUND_STRING("SCAN TOOLS");
    if (sUi.mode == MODE_SIGNAL || sUi.mode == MODE_SIGNAL_CONFIRM) title = COMPOUND_STRING("MUTANT SIGNAL");
    if (sUi.mode == MODE_SEARCH) title = COMPOUND_STRING("PREPARE SEARCH");
    UnholyUi_Frame(sUi.windowId, title);
    // The native narrow font draws 12 pixels despite its 8-pixel metric.
    // Reserve two full lines and move the shared footer separator above them.
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(1), 5, WIN_HEIGHT - 19, WIN_WIDTH - 10, 1);
    FillWindowPixelRect(sUi.windowId, PIXEL_FILL(3), 5, 117, WIN_WIDTH - 10, 1);
    GetMapName(text, sUi.zone.mapSec, 0);
    UnholyUi_Print(sUi.windowId, FONT_SMALL_NARROW, 112, 6, 106, text);
}

static void Footer(const u8 *first, const u8 *second)
{
    Print(5, 118, 214, first);
    Print(5, 130, 214, second);
}

static void SpeciesTypes(u8 *text, u16 species)
{
    StringCopy(text, gTypesInfo[GetSpeciesType(species, 0)].name);
    if (GetSpeciesType(species, 0) != GetSpeciesType(species, 1))
    {
        StringAppend(text, COMPOUND_STRING("/"));
        StringAppend(text, gTypesInfo[GetSpeciesType(species, 1)].name);
    }
}

static void DrawCatalog(void)
{
    u8 text[80], indexes[POKELOCATOR_MAX_ROWS], groups = 0;
    for (u8 group = 1; group < POKELOCATOR_GROUP_COUNT; group++)
        groups += GroupAvailable(group);
    u8 tab = 0, width = groups ? 216 / groups : 216;
    for (u8 group = 1; group < POKELOCATOR_GROUP_COUNT; group++)
        if (GroupAvailable(group))
        {
            u8 x = 4 + tab++ * width;
            UnholyUi_Card(sUi.windowId, x, 21, width - 2, 14, group == sUi.group);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 3, 22, width - 6,
                UnholyUi_CardFill(group == sUi.group), group == POKELOCATOR_GROUP_LAND ? MethodName(POKELOCATOR_METHOD_LAND) : sGroups[group]);
        }
    u8 total = Pokelocator_BuildCatalogIndex(&sUi.list, sUi.group, POKELOCATOR_COLLECTION_ALL, Pokelocator_GetDexState, indexes);
    u8 caught = Pokelocator_BuildCatalogIndex(&sUi.list, sUi.group, POKELOCATOR_COLLECTION_CAUGHT, Pokelocator_GetDexState, indexes);
    u8 *end = StringCopy(text, caught == total && total ? COMPOUND_STRING("COMPLETE ") : COMPOUND_STRING("CAUGHT "));
    end = ConvertIntToDecimalStringN(end, caught, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 2);
    Print(5, 36, 110, text);
    if (!sUi.count)
    {
        Print(9, 57, 206, !sUi.list.count ? COMPOUND_STRING("NO UNLOCKED ENCOUNTERS HERE") : COMPOUND_STRING("NO SPECIES MATCH THIS FILTER"));
        Print(9, 75, 206, COMPOUND_STRING("START: TOOLS AND COLLECTION FILTER"));
    }
    for (u8 i = 0; i < VISIBLE_CARDS && sUi.scroll + i < sUi.count; i++)
    {
        u8 position = sUi.scroll + i;
        const struct PokelocatorRow *row = &sUi.list.rows[sUi.catalog[position]];
        u8 x = 4 + i % 3 * 36, y = 49 + i / 3 * 32;
        bool8 selected = position == sUi.cursor;
        UnholyUi_Card(sUi.windowId, x, y, 35, 31, selected);
        MonIcon(i, row->species, x + 17, y + 14);
        if (Pokelocator_GetDexState(row->species) == POKELOCATOR_DEX_CAUGHT)
            FillWindowPixelRect(sUi.windowId, PIXEL_FILL(5), x + 28, y + 25, 4, 3);
    }
    const struct PokelocatorRow *row = CatalogRow();
    if (row)
    {
        Print(117, 39, 102, GetSpeciesName(row->species));
        end = StringCopy(text, COMPOUND_STRING("No."));
        end = ConvertIntToDecimalStringN(end, SpeciesToNationalPokedexNum(row->species), STR_CONV_MODE_LEFT_ALIGN, 4);
        end = StringAppend(end, COMPOUND_STRING(" "));
        StringAppend(end, DexLabel(row->species));
        Print(117, 52, 102, text);
        SpeciesTypes(text, row->species);
        Print(117, 65, 102, text);
        for (u8 i = 0; i < NUM_ABILITY_SLOTS; i++)
        {
            u16 ability = GetSpeciesAbility(row->species, i);
            end = StringCopy(text, i == 0 ? COMPOUND_STRING("1 ") : i == 1 ? COMPOUND_STRING("2 ") : COMPOUND_STRING("H "));
            StringAppend(end, ability == ABILITY_NONE ? COMPOUND_STRING("--") : gAbilitiesInfo[ability].name);
            Print(117, 78 + i * 12, 102, text);
        }
        end = ConvertIntToDecimalStringN(text, sUi.cursor + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, COMPOUND_STRING("/"));
        end = ConvertIntToDecimalStringN(end, sUi.count, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, COMPOUND_STRING(" "));
        StringAppend(end, sCollections[sUi.collection]);
        Print(5, 104, 108, text);
    }
    Footer(COMPOUND_STRING("A:MENU  L/R:GROUP  SELECT:BATCH"), COMPOUND_STRING("+:SPECIES  START:TOOLS  B:WATCH"));
}

static void DrawDetail(void)
{
    const struct PokelocatorRow *row = SearchRow();
    u8 text[80];
    MonIcon(0, row->species, 22, 42);
    Print(43, 25, 174, GetSpeciesName(row->species));
    u8 *end = StringCopy(text, COMPOUND_STRING("NATIONAL No."));
    ConvertIntToDecimalStringN(end, SpeciesToNationalPokedexNum(row->species), STR_CONV_MODE_LEFT_ALIGN, 4);
    Print(43, 38, 174, text);
    SpeciesTypes(text, row->species);
    Print(43, 51, 174, text);
    for (u8 i = 0; i < NUM_ABILITY_SLOTS; i++)
    {
        Print(7, 66 + i * 13, 25, i == 2 ? COMPOUND_STRING("HA") : i == 0 ? COMPOUND_STRING("A1") : COMPOUND_STRING("A2"));
        u16 ability = GetSpeciesAbility(row->species, i);
        Print(35, 66 + i * 13, 182, ability == ABILITY_NONE ? COMPOUND_STRING("--") : gAbilitiesInfo[ability].name);
    }
    end = StringCopy(text, MethodName(row->method));
    end = StringAppend(end, COMPOUND_STRING(" "));
    Levels(end, row);
    Print(7, 104, 210, text);
    Footer(COMPOUND_STRING("UP/DN:METHOD  A:ACTION MENU"), COMPOUND_STRING("FULL STATS: POKéDEX / SUMMARY  B:BACK"));
}

static void DrawActions(void)
{
    static const u8 *const actions[] = {
        COMPOUND_STRING("SEARCH - WILD BATTLE"), COMPOUND_STRING("OBTAIN ONE - FREE"),
        COMPOUND_STRING("REGISTER TARGET FOR R"), COMPOUND_STRING("SPECIES PROFILE"),
    };
    Print(7, 22, 210, GetSpeciesName(CatalogRow()->species));
    for (u8 i = 0; i < ARRAY_COUNT(actions); i++)
    {
        UnholyUi_Card(sUi.windowId, 5, 38 + i * 18, 214, 17, sUi.action == i);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 10, 40 + i * 18, 204, UnholyUi_CardFill(sUi.action == i), actions[i]);
    }
    Footer(COMPOUND_STRING("UP/DN:CHOOSE  A:OPEN"), COMPOUND_STRING("B:CATALOG"));
}

static void DrawBatch(void)
{
    Print(7, 25, 210, COMPOUND_STRING("ONLY THE SELECTED CATEGORY:"));
    Print(7, 39, 210, sGroups[sUi.group]);
    const u8 *labels[] = {COMPOUND_STRING("OBTAIN ALL (INCLUDING CAUGHT)"), COMPOUND_STRING("OBTAIN ONLY NEW SPECIES")};
    for (u8 i = 0; i < 2; i++)
    {
        UnholyUi_Card(sUi.windowId, 5, 58 + i * 22, 214, 20, sUi.action == i);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 10, 62 + i * 22, 204, UnholyUi_CardFill(sUi.action == i), labels[i]);
    }
    Footer(COMPOUND_STRING("UP/DN:CHOOSE  A:REVIEW"), COMPOUND_STRING("B:CANCEL - NO POKéMON OBTAINED"));
}

static void DrawObtain(void)
{
    u8 text[80];
    Print(7, 25, 210, sUi.plan.kind == POKELOCATOR_OBTAIN_ONE ? GetSpeciesName(sUi.plan.selectedSpecies)
        : sUi.plan.kind == POKELOCATOR_OBTAIN_ALL ? COMPOUND_STRING("OBTAIN ALL IN THIS CATEGORY?") : COMPOUND_STRING("OBTAIN NEW IN THIS CATEGORY?"));
    u8 *end = ConvertIntToDecimalStringN(text, sUi.plan.count, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING(" POKéMON  AT Lv "));
    ConvertIntToDecimalStringN(end, sUi.plan.level, STR_CONV_MODE_LEFT_ALIGN, 3);
    Print(7, 44, 210, text);
    Print(7, 62, 210, COMPOUND_STRING("DELIVERY: PARTY, THEN PC"));
    end = StringCopy(text, COMPOUND_STRING("FREE SLOTS: "));
    Number(end, Pokelocator_CountFreeSlots());
    Print(7, 77, 210, text);
    Print(7, 97, 210, Pokelocator_GetObtainMessage(sUi.obtainStatus));
    Footer(sUi.obtainStatus == POKELOCATOR_OBTAIN_READY ? COMPOUND_STRING("A:CONFIRM DELIVERY  B:CANCEL") : COMPOUND_STRING("B:BACK - NOTHING HAS BEEN GIVEN"), COMPOUND_STRING("NO BATTLE; USES NATIVE GIFT RULES"));
}

static void DrawResult(void)
{
    u8 text[80];
    Print(7, 31, 210, sUi.result.status == POKELOCATOR_OBTAIN_READY ? COMPOUND_STRING("DELIVERY COMPLETE") : Pokelocator_GetObtainMessage(sUi.result.status));
    u8 *end = StringCopy(text, COMPOUND_STRING("PARTY: "));
    end = ConvertIntToDecimalStringN(end, sUi.result.party, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING("   PC: "));
    ConvertIntToDecimalStringN(end, sUi.result.pc, STR_CONV_MODE_LEFT_ALIGN, 2);
    Print(7, 55, 210, text);
    Print(7, 78, 210, sUi.result.party + sUi.result.pc ? COMPOUND_STRING("CAUGHT REGISTRY UPDATED") : COMPOUND_STRING("NO POKéMON DELIVERED"));
    Footer(COMPOUND_STRING("A / B:RETURN TO CATALOG"), COMPOUND_STRING("ALL / NEW APPLY TO ONE GROUP ONLY"));
}

static u32 CountBalls(void)
{
    u32 count = 0;
    for (u32 i = 0; i < gBagPockets[POCKET_POKE_BALLS].capacity; i++)
    {
        enum Item item = GetBagItemId(POCKET_POKE_BALLS, i);
        if (item != ITEM_NONE && item != ITEM_SAFARI_BALL && item != ITEM_PARK_BALL)
            count += GetBagItemQuantity(POCKET_POKE_BALLS, i);
    }
    return count;
}

static void DrawSearch(void)
{
    const struct PokelocatorRow *row = SearchRow();
    u8 text[80];
    UnholyUi_Print(sUi.windowId, FONT_NARROW, 40, 26, 162, GetSpeciesName(row->species));
    Print(40, 43, 110, MethodName(row->method));
    Levels(text, row);
    Print(151, 43, 52, text);
    MonIcon(0, row->species, 20, 39);
    u8 *end = StringCopy(text, COMPOUND_STRING("POKé BALLS: "));
    Number(end, CountBalls());
    Print(7, 64, 194, text);
    Print(7, 78, 194, IsPlayerPartyAndPokemonStorageFull() ? COMPOUND_STRING("STORAGE FULL - CAPTURE UNAVAILABLE")
        : gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE ? COMPOUND_STRING("CAPTURE GOES TO YOUR PARTY") : COMPOUND_STRING("CAPTURE GOES TO YOUR PC"));
    end = StringCopy(text, COMPOUND_STRING("SIGNAL: "));
    u8 remaining = HfcWatch_GetFrequencyRemaining();
    if (remaining)
    {
        end = ConvertIntToDecimalStringN(end, remaining, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(end, COMPOUND_STRING(" ENCOUNTERS LEFT"));
    }
    else
        StringAppend(end, COMPOUND_STRING("OFF"));
    Print(7, 92, 194, text);
    u8 status = Pokelocator_GetSearchStatus(Header(), row->species, row->method);
    Print(7, 106, 194, status != POKELOCATOR_SEARCH_READY ? Pokelocator_GetSearchMessage(status)
        : CountBalls() == 0 ? COMPOUND_STRING("NO BALLS - YOU CAN ONLY BATTLE") : COMPOUND_STRING("READY FOR A WILD ENCOUNTER"));
    Footer(COMPOUND_STRING("A:BEGIN SEARCH  B:CANCEL"), COMPOUND_STRING("USE A BALL IN BATTLE TO CAPTURE"));
}

static const u8 *SignalRequirement(void)
{
    struct HfcWatchProgress progress = {.badges = HfcWatch_CountBadges()};
    if (Header() != GetCurrentMapWildMonHeaderId())
        return COMPOUND_STRING("GO TO THIS AREA FIRST");
    if (HfcWatch_GetAccess(HfcWatch_GetTier(), HFC_WATCH_FEATURE_FREQUENCY, progress) != HFC_WATCH_READY)
        return HfcWatch_GetRequirement(HfcWatch_GetTier(), HFC_WATCH_FEATURE_FREQUENCY, progress);
    if (!FlagGet(FLAG_SYS_CLOCK_SET))
        return COMPOUND_STRING("SET YOUR CLOCK FIRST");
    if (HfcWatch_GetFrequencyRemaining())
        return COMPOUND_STRING("SIGNAL IS ACTIVE IN THIS AREA");
    if (FlagGet(FLAG_DAILY_HFC_FREQUENCY))
        return COMPOUND_STRING("DAILY SIGNAL ALREADY USED");
    if (!HfcWatch_CanActivateFrequency())
        return COMPOUND_STRING("NO SIGNAL AVAILABLE HERE");
    return COMPOUND_STRING("A:ACTIVATE SIGNAL");
}

static void DrawTools(void)
{
    u8 text[64];
    static const u8 *const titles[] =
    {
        COMPOUND_STRING("BROWSE AREA"), COMPOUND_STRING("MUTANT SIGNAL"), COMPOUND_STRING("COLLECTION FILTER"), COMPOUND_STRING("RETURN TO CATALOG"),
    };
    for (u8 i = 0; i < ARRAY_COUNT(titles); i++)
    {
        u8 y = 24 + i * 18;
        UnholyUi_Card(sUi.windowId, 5, y, 198, 17, i == sUi.toolCursor);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 11, y + 2, 151,
                            UnholyUi_CardFill(i == sUi.toolCursor), titles[i]);
        if (i == 0)
        {
            u8 *end = ConvertIntToDecimalStringN(text, sUi.zone.count ? sUi.zone.current + 1 : 0, STR_CONV_MODE_LEFT_ALIGN, 2);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, sUi.zone.count, STR_CONV_MODE_LEFT_ALIGN, 2);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 166, y + 2, 32,
                                UnholyUi_CardFill(i == sUi.toolCursor), text);
        }
    }
    if (sUi.toolCursor == 0)
    {
        Print(7, 103, 210, COMPOUND_STRING("L/R: FLOORS; SEARCH IN CURRENT AREA"));
    }
    else if (sUi.toolCursor == 1)
    {
        Print(7, 103, 210, SignalRequirement());
    }
    if (sUi.toolCursor == 2) Print(7, 103, 210, sCollections[sUi.collection]);
    Footer(COMPOUND_STRING("UP/DN:CHOOSE  A:OPEN / NEXT AREA"), COMPOUND_STRING("B:CATALOG"));
}

static void DrawSignal(void)
{
    u8 text[80];
    u8 remaining = Header() == GetCurrentMapWildMonHeaderId() ? HfcWatch_GetFrequencyRemaining() : 0;
    u8 boost = remaining ? HfcWatch_GetActiveFrequencyBoost() : 0;
    u8 *end = StringCopy(text, remaining ? COMPOUND_STRING("ACTIVE ") : COMPOUND_STRING("OFF "));
    end = ConvertIntToDecimalStringN(end, remaining, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING(" LEFT; C/B ROLLS +"));
    end = ConvertIntToDecimalStringN(end, boost, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringAppend(end, COMPOUND_STRING("% REL"));
    Print(5, 20, 198, text);
    for (u8 i = 0; i < MUTATION_COUNT; i++)
    {
        u8 x = 4 + i % 3 * 68, y = 33 + i / 3 * 27;
        UnholyUi_Card(sUi.windowId, x, y, 65, 26, FALSE);
        u8 fill = UnholyUi_CardFill(FALSE);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 20, y + 1, 44, fill,
                            HorizonMarketMutationName(sMutationBadges[i]));
        end = StringCopy(text, COMPOUND_STRING("1/"));
        ConvertIntToDecimalStringN(end, sMutationRates[i], STR_CONV_MODE_LEFT_ALIGN, 4);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 20, y + 13, 44, fill, text);
        u8 sprite = MutationIcon_Create(sMutationBadges[i], WIN_LEFT + x + 10, WIN_TOP + y + 12, 0);
        sUi.mutationIds[i] = sprite;
    }
    Footer(SignalRequirement(), COMPOUND_STRING("BASE ROLLS; COMBOS GATED  B:TOOLS"));
}

static void DrawSignalConfirm(void)
{
    u8 text[80];
    Print(7, 31, 194, COMPOUND_STRING("ACTIVATE SIGNAL IN THIS AREA?"));
    Print(7, 49, 194, COMPOUND_STRING("10 WILD ENCOUNTERS, ONCE PER DAY"));
    u8 *end = StringCopy(text, COMPOUND_STRING("C/B MUTATION ROLLS +"));
    end = ConvertIntToDecimalStringN(end, HfcWatch_GetFrequencyBoost(HfcWatch_CountBadges()), STR_CONV_MODE_LEFT_ALIGN, 2);
    StringAppend(end, COMPOUND_STRING("% RELATIVE"));
    Print(7, 64, 194, text);
    Print(7, 81, 194, COMPOUND_STRING("A, S AND OMEGA KEEP THEIR BASE ODDS"));
    Print(7, 97, 194, COMPOUND_STRING("CANCEL KEEPS YOUR DAILY USE"));
    Footer(COMPOUND_STRING("A:CONFIRM ACTIVATION  B:CANCEL"), SignalRequirement());
}

static void Render(void)
{
    DestroyIcons();
    DrawHeader();
    switch (sUi.mode)
    {
    case MODE_LIST: DrawCatalog(); break;
    case MODE_DETAIL: DrawDetail(); break;
    case MODE_SEARCH: DrawSearch(); break;
    case MODE_TOOLS: DrawTools(); break;
    case MODE_SIGNAL: DrawSignal(); break;
    case MODE_SIGNAL_CONFIRM: DrawSignalConfirm(); break;
    case MODE_ACTIONS: DrawActions(); break;
    case MODE_BATCH: DrawBatch(); break;
    case MODE_OBTAIN: DrawObtain(); break;
    case MODE_RESULT: DrawResult(); break;
    }
    CopyWindowToVram(sUi.windowId, COPYWIN_FULL);
}

static void RememberSelection(void)
{
    const struct PokelocatorRow *row = SearchRow();
    sRemember = (struct LocatorSelection){
        .valid = TRUE, .originHeader = GetCurrentMapWildMonHeaderId(), .viewHeader = Header(),
        .species = row ? row->species : SPECIES_NONE, .method = row ? row->method : POKELOCATOR_METHOD_COUNT,
        .group = sUi.group, .collection = sUi.collection,
    };
}

static void ReturnToCaller(void)
{
    void (*onClose)(void) = sUi.onClose;
    sUi.onClose = NULL;
    if (onClose)
        onClose();
    else
    {
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
    }
}

static void Close(u8 taskId, bool8 returnToCaller)
{
    RememberSelection();
    DestroyIcons();
    ClearStdWindowAndFrameToTransparent(sUi.windowId, TRUE);
    RemoveWindow(sUi.windowId);
    sUi.windowId = WINDOW_NONE;
    UnholyUi_ReleasePalette(&sUi.palette);
    DestroyTask(taskId);
    if (returnToCaller)
        ReturnToCaller();
    else
        sUi.onClose = NULL;
}

static void BeginSearch(u8 taskId)
{
    const struct PokelocatorRow *row = SearchRow();
    if (!row || Pokelocator_GetSearchStatus(Header(), row->species, row->method) != POKELOCATOR_SEARCH_READY
     || !Pokelocator_QueueSearch(Header(), row->species, row->method))
    { PlaySE(SE_FAILURE); Render(); return; }
    Pokelocator_RegisterTarget(row->species, row->method);
    PlaySE(SE_SELECT);
    Close(taskId, FALSE);
}

static void PrepareObtain(u8 kind)
{
    const struct PokelocatorRow *row = CatalogRow();
    sUi.obtainStatus = Pokelocator_PrepareObtain(&sUi.plan, Header(), sUi.group, kind, row ? row->species : SPECIES_NONE);
    sUi.mode = MODE_OBTAIN;
}

static void ChangeArea(s8 delta)
{
    if (sUi.zone.count > 1)
        LoadArea((sUi.zone.current + sUi.zone.count + delta) % sUi.zone.count);
}

static void Task_Locator(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        switch (sUi.mode)
        {
        case MODE_LIST: Close(taskId, TRUE); return;
        case MODE_SEARCH: sUi.mode = MODE_ACTIONS; break;
        case MODE_OBTAIN: sUi.mode = sUi.plan.kind == POKELOCATOR_OBTAIN_ONE ? MODE_ACTIONS : MODE_BATCH; break;
        case MODE_SIGNAL_CONFIRM: sUi.mode = MODE_SIGNAL; break;
        case MODE_SIGNAL: sUi.mode = MODE_TOOLS; break;
        case MODE_DETAIL: sUi.mode = MODE_LIST; break;
        default: sUi.mode = MODE_LIST; break;
        }
        Render();
        return;
    }
    if (sUi.mode == MODE_ACTIONS || sUi.mode == MODE_BATCH)
    {
        u8 count = sUi.mode == MODE_ACTIONS ? 4 : 2;
        if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
            sUi.action = (sUi.action + count + (JOY_REPEAT(DPAD_UP) ? -1 : 1)) % count;
        else if (JOY_NEW(A_BUTTON))
        {
            if (sUi.mode == MODE_BATCH) PrepareObtain(sUi.action ? POKELOCATOR_OBTAIN_NEW : POKELOCATOR_OBTAIN_ALL);
            else if (sUi.action == 0) sUi.mode = MODE_SEARCH;
            else if (sUi.action == 1) PrepareObtain(POKELOCATOR_OBTAIN_ONE);
            else if (sUi.action == 2)
            {
                const struct PokelocatorRow *row = SearchRow();
                bool32 ready = Header() == GetCurrentMapWildMonHeaderId() && row && Pokelocator_RegisterTarget(row->species, row->method);
                PlaySE(ready ? SE_SELECT : SE_FAILURE);
                if (ready) sUi.mode = MODE_LIST;
            }
            else sUi.mode = MODE_DETAIL;
        }
        else return;
        PlaySE(SE_SELECT); Render(); return;
    }
    if (sUi.mode == MODE_OBTAIN)
    {
        if (JOY_NEW(A_BUTTON) && sUi.obtainStatus == POKELOCATOR_OBTAIN_READY)
        {
            sUi.result = Pokelocator_ExecuteObtain(&sUi.plan);
            sUi.mode = MODE_RESULT;
            PlaySE(sUi.result.status == POKELOCATOR_OBTAIN_READY ? SE_SELECT : SE_FAILURE);
            LoadArea(sUi.zone.current);
            Render();
        }
        return;
    }
    if (sUi.mode == MODE_RESULT)
    {
        if (JOY_NEW(A_BUTTON)) { sUi.mode = MODE_LIST; Render(); }
        return;
    }
    if (sUi.mode == MODE_SEARCH)
    {
        if (JOY_NEW(A_BUTTON)) BeginSearch(taskId);
        return;
    }
    if (sUi.mode == MODE_SIGNAL_CONFIRM)
    {
        if (JOY_NEW(A_BUTTON))
        {
            bool32 activated = Header() == GetCurrentMapWildMonHeaderId() && HfcWatch_ActivateFrequency();
            PlaySE(activated ? SE_SELECT : SE_FAILURE);
            sUi.mode = MODE_SIGNAL;
            Render();
        }
        return;
    }
    if (sUi.mode == MODE_SIGNAL)
    {
        if (JOY_NEW(A_BUTTON))
        {
            bool32 ready = Header() == GetCurrentMapWildMonHeaderId() && HfcWatch_CanActivateFrequency();
            PlaySE(ready ? SE_SELECT : SE_FAILURE);
            if (ready) sUi.mode = MODE_SIGNAL_CONFIRM;
            Render();
        }
        return;
    }
    if (sUi.mode == MODE_TOOLS)
    {
        if (JOY_REPEAT(DPAD_UP) || JOY_REPEAT(DPAD_DOWN))
            sUi.toolCursor = (sUi.toolCursor + (JOY_REPEAT(DPAD_UP) ? 3 : 1)) % 4;
        else if (sUi.toolCursor == 0 && (JOY_NEW(L_BUTTON | DPAD_LEFT) || JOY_NEW(R_BUTTON | DPAD_RIGHT)))
            ChangeArea(JOY_NEW(L_BUTTON | DPAD_LEFT) ? -1 : 1);
        else if (JOY_NEW(A_BUTTON))
        {
            if (sUi.toolCursor == 0) ChangeArea(1);
            else if (sUi.toolCursor == 1) sUi.mode = MODE_SIGNAL;
            else if (sUi.toolCursor == 2)
            {
                sUi.collection = (sUi.collection + 1) % POKELOCATOR_COLLECTION_COUNT;
                Refilter(SPECIES_NONE, POKELOCATOR_METHOD_COUNT);
            }
            else sUi.mode = MODE_LIST;
        }
        else return;
        PlaySE(SE_SELECT);
        Render();
        return;
    }
    if (sUi.mode == MODE_LIST)
    {
        if (JOY_NEW(START_BUTTON)) sUi.mode = MODE_TOOLS;
        else if (JOY_NEW(SELECT_BUTTON)) { sUi.mode = MODE_BATCH; sUi.action = 0; }
        else if (JOY_NEW(L_BUTTON | R_BUTTON))
        {
            u16 species = CatalogRow() ? CatalogRow()->species : SPECIES_NONE;
            for (u8 i = 0; i < POKELOCATOR_GROUP_COUNT; i++)
            {
                sUi.group = (sUi.group + (JOY_NEW(L_BUTTON) ? POKELOCATOR_GROUP_COUNT - 1 : 1)) % POKELOCATOR_GROUP_COUNT;
                if (sUi.group && GroupAvailable(sUi.group)) break;
            }
            Refilter(species, POKELOCATOR_METHOD_COUNT);
        }
        else if (JOY_NEW(A_BUTTON) && sUi.count)
        {
            sUi.mode = MODE_ACTIONS;
            sUi.action = 0;
        }
        else if (sUi.count && JOY_REPEAT(DPAD_ANY))
        {
            u8 direction = JOY_REPEAT(DPAD_LEFT) ? POKELOCATOR_GRID_LEFT : JOY_REPEAT(DPAD_RIGHT) ? POKELOCATOR_GRID_RIGHT
                         : JOY_REPEAT(DPAD_UP) ? POKELOCATOR_GRID_UP : POKELOCATOR_GRID_DOWN;
            s8 delta = direction == POKELOCATOR_GRID_LEFT ? -1 : direction == POKELOCATOR_GRID_RIGHT ? 1 : direction == POKELOCATOR_GRID_UP ? -3 : 3;
            s16 next = sUi.cursor + delta;
            sUi.cursor = next < 0 ? sUi.count - 1 : next >= sUi.count ? 0 : next;
            sUi.scroll = sUi.cursor / VISIBLE_CARDS * VISIBLE_CARDS;
            SetMethods(CatalogRow()->method);
        }
        else return;
        PlaySE(SE_SELECT);
        Render();
        return;
    }
    if (sUi.mode == MODE_DETAIL)
    {
        if (JOY_NEW(A_BUTTON)) { sUi.mode = MODE_ACTIONS; sUi.action = 0; }
        else if (sUi.methodCount && JOY_REPEAT(DPAD_UP | DPAD_DOWN))
            sUi.methodCursor = (sUi.methodCursor + sUi.methodCount + (JOY_REPEAT(DPAD_UP) ? -1 : 1)) % sUi.methodCount;
        else return;
        PlaySE(SE_SELECT); Render();
    }
}

void Pokelocator_OpenFromField(void (*onClose)(void))
{
    if (FuncIsActiveTask(Task_Locator))
        return;
    HideMapNamePopUpWindow();
    LockPlayerFieldControls();
    memset(&sUi, 0, sizeof(sUi));
    sUi.onClose = onClose;
    sUi.windowId = WINDOW_NONE;
    for (u8 i = 0; i < VISIBLE_CARDS; i++) sUi.iconIds[i] = MAX_SPRITES;
    for (u8 i = 0; i < MUTATION_COUNT; i++) sUi.mutationIds[i] = MAX_SPRITES;
    Pokelocator_BuildZone(&sUi.zone, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    bool8 resume = sRemember.valid && sRemember.originHeader == GetCurrentMapWildMonHeaderId();
    if (resume)
    {
        sUi.group = sRemember.group;
        sUi.collection = sRemember.collection;
        for (u8 i = 0; i < sUi.zone.count; i++)
            if (sUi.zone.headerIds[i] == sRemember.viewHeader) sUi.zone.current = i;
    }
    LoadArea(sUi.zone.current);
    if (resume) Refilter(sRemember.species, sRemember.method);
    UnholyUi_AcquirePalette(&sUi.palette, 14);
    sUi.windowId = AddWindow(&sWindow);
    if (sUi.windowId == WINDOW_NONE)
    {
        UnholyUi_ReleasePalette(&sUi.palette);
        ReturnToCaller();
        return;
    }
    u8 task = CreateTask(Task_Locator, 8);
    if (task == TASK_NONE)
    {
        RemoveWindow(sUi.windowId);
        sUi.windowId = WINDOW_NONE;
        UnholyUi_ReleasePalette(&sUi.palette);
        ReturnToCaller();
        return;
    }
    SetStandardWindowBorderStyle(sUi.windowId, FALSE);
    PutWindowTilemap(sUi.windowId);
    Render();
}
