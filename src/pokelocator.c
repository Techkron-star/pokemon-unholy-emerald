#include "global.h"
#include "pokelocator.h"
#include "battle_main.h"
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

#define VISIBLE_CARDS 4
#define WIN_LEFT 16
#define WIN_TOP 8
#define WIN_WIDTH 208
#define WIN_HEIGHT 144
#define ICON_PALETTES 6
#define MUTATION_COUNT 9

enum LocatorMode { MODE_LIST, MODE_DETAIL, MODE_SEARCH, MODE_TOOLS, MODE_SIGNAL, MODE_SIGNAL_CONFIRM };
enum DetailPage { PAGE_HABITAT, PAGE_STATS, PAGE_TRAITS, PAGE_COUNT };

struct PokelocatorUi
{
    struct PokelocatorList list;
    struct PokelocatorZone zone;
    struct UnholyUiPalette palette;
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
    .bg = 0, .tilemapLeft = 2, .tilemapTop = 1, .width = 26, .height = 18,
    .paletteNum = 14, .baseBlock = 0x40,
};
STATIC_ASSERT(0x40 + 26 * 18 <= STD_WINDOW_BASE_TILE_NUM, LocatorTilesBeforeFrame);

static const u8 *const sGroups[] =
{
    COMPOUND_STRING("ALL"), COMPOUND_STRING("GRASS"), COMPOUND_STRING("SURF"),
    COMPOUND_STRING("RODS"), COMPOUND_STRING("OTHER"),
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
static void Task_StartSearch(u8 taskId);

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

static void Chance(u8 *dst, const struct PokelocatorRow *row)
{
    if (!Pokelocator_RowHasChance(row))
    {
        StringCopy(dst, COMPOUND_STRING("--"));
        return;
    }
    u16 chance = Pokelocator_GetChanceTenths(row);
    dst = ConvertIntToDecimalStringN(dst, chance / 10, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (chance % 10)
    {
        dst = StringAppend(dst, COMPOUND_STRING("."));
        dst = ConvertIntToDecimalStringN(dst, chance % 10, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
    StringAppend(dst, COMPOUND_STRING("%"));
}

static void SetMethods(u8 preferred)
{
    const struct PokelocatorRow *row = CatalogRow();
    sUi.methodCount = row ? Pokelocator_GetSpeciesMethods(&sUi.list, row->species, sUi.methods) : 0;
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

static void LoadArea(u8 area)
{
    sUi.zone.current = area;
    if (sUi.zone.count)
        Pokelocator_BuildListForHeader(&sUi.list, Header(), Pokelocator_IsSpeciesRevealed);
    else
        Pokelocator_ClearList(&sUi.list);
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
    UnholyUi_Print(sUi.windowId, FONT_SMALL_NARROW, 112, 6, 90, text);
}

static void Footer(const u8 *first, const u8 *second)
{
    Print(5, 118, 198, first);
    Print(5, 130, 198, second);
}

static void DrawCatalog(void)
{
    u8 text[64], indexes[POKELOCATOR_MAX_ROWS];
    for (u8 i = 0; i < POKELOCATOR_GROUP_COUNT; i++)
    {
        u8 x = 4 + i * 40;
        UnholyUi_Card(sUi.windowId, x, 21, 39, 14, i == sUi.group);
        const u8 *label = i == POKELOCATOR_GROUP_LAND ? MethodName(POKELOCATOR_METHOD_LAND) : sGroups[i];
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 3, 22, 33,
                            UnholyUi_CardFill(i == sUi.group), label);
    }
    if (!sUi.count)
    {
        Print(9, 55, 190, sUi.list.count == 0 ? COMPOUND_STRING("NO WILD POKéMON IN THIS AREA")
              : sUi.collection == POKELOCATOR_COLLECTION_MISSING ? COMPOUND_STRING("NO MISSING SPECIES IN THIS FILTER")
              : COMPOUND_STRING("NO SPECIES MATCH THIS FILTER"));
        Print(9, 70, 190, COMPOUND_STRING("L/R: METHOD   SELECT: COLLECTION"));
    }
    for (u8 i = 0; i < VISIBLE_CARDS && sUi.scroll + i < sUi.count; i++)
    {
        u8 position = sUi.scroll + i;
        const struct PokelocatorRow *row = &sUi.list.rows[sUi.catalog[position]];
        u8 x = 4 + (i % 2) * 102, y = 36 + (i / 2) * 34;
        bool8 selected = position == sUi.cursor;
        u8 fill = UnholyUi_CardFill(selected);
        UnholyUi_Card(sUi.windowId, x, y, 98, 33, selected);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 34, y + 1, 61, fill, GetSpeciesName(row->species));
        Levels(text, row);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 34, y + 15, 33, fill, text);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 67, y + 15, 29, fill, DexLabel(row->species));
        MonIcon(i, row->species, x + 16, y + 16);
    }
    u8 total = Pokelocator_BuildCatalogIndex(&sUi.list, sUi.group, POKELOCATOR_COLLECTION_ALL,
                                            Pokelocator_GetDexState, indexes);
    u8 caught = Pokelocator_BuildCatalogIndex(&sUi.list, sUi.group, POKELOCATOR_COLLECTION_CAUGHT,
                                             Pokelocator_GetDexState, indexes);
    u8 *end = StringCopy(text, COMPOUND_STRING("CAUGHT "));
    end = ConvertIntToDecimalStringN(end, caught, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING("/"));
    end = ConvertIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING("  SHOW: "));
    StringAppend(end, sCollections[sUi.collection]);
    Print(5, 104, 166, text);
    end = ConvertIntToDecimalStringN(text, sUi.count ? sUi.cursor + 1 : 0, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringAppend(end, COMPOUND_STRING("/"));
    ConvertIntToDecimalStringN(end, sUi.count, STR_CONV_MODE_LEFT_ALIGN, 2);
    Print(177, 104, 28, text);
    Footer(COMPOUND_STRING("A:DETAIL  L/R:METHOD  SELECT:COLLECTION"), COMPOUND_STRING("+:CHOOSE  START:TOOLS  B:BACK"));
}

static void DrawSpeciesHeader(void)
{
    const struct PokelocatorRow *row = CatalogRow();
    u8 text[64];
    for (u8 page = 0; page < PAGE_COUNT; page++)
    {
        u8 x = 4 + page * 68;
        UnholyUi_Card(sUi.windowId, x, 21, 65, 14, sUi.page == page);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, x + 8, 22, 56,
            UnholyUi_CardFill(sUi.page == page), page == PAGE_HABITAT ? COMPOUND_STRING("HABITAT") : page == PAGE_STATS ? COMPOUND_STRING("STATS") : COMPOUND_STRING("TRAITS"));
    }
    UnholyUi_Print(sUi.windowId, FONT_NARROW, 40, 35, 162, GetSpeciesName(row->species));
    u8 *end = StringCopy(text, gTypesInfo[GetSpeciesType(row->species, 0)].name);
    if (GetSpeciesType(row->species, 0) != GetSpeciesType(row->species, 1))
    {
        end = StringAppend(end, COMPOUND_STRING(" / "));
        StringAppend(end, gTypesInfo[GetSpeciesType(row->species, 1)].name);
    }
    Print(40, 50, 112, text);
    Print(160, 50, 43, DexLabel(row->species));
    MonIcon(0, row->species, 20, 48);
}

static void DrawDetail(void)
{
    const struct PokelocatorRow *selected = SearchRow();
    u8 text[80];
    DrawSpeciesHeader();
    if (sUi.page == PAGE_HABITAT)
    {
        u8 first = sUi.methodCursor >= 3 ? sUi.methodCursor - 2 : 0;
        for (u8 i = first; i < sUi.methodCount && i < first + 3; i++)
        {
            const struct PokelocatorRow *row = &sUi.list.rows[sUi.methods[i]];
            u8 y = 64 + (i - first) * 14;
            bool8 focused = i == sUi.methodCursor;
            u8 fill = UnholyUi_CardFill(focused);
            UnholyUi_Card(sUi.windowId, 4, y, 200, 14, focused);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 8, y + 1, 80, fill, MethodName(row->method));
            Levels(text, row);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 91, y + 1, 58, fill, text);
            Chance(text, row);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 164, y + 1, 36, fill, text);
        }
        u8 status = Pokelocator_GetSearchStatus(Header(), selected->species, selected->method);
        Print(5, 106, 198, status == POKELOCATOR_SEARCH_READY ? COMPOUND_STRING("METHOD READY - A TO PREPARE SEARCH") : Pokelocator_GetSearchMessage(status));
        Footer(COMPOUND_STRING("A:SEARCH  UP/DN:METHOD  L/R:PAGE"), COMPOUND_STRING("<>:SPECIES  B:CATALOG"));
    }
    else if (sUi.page == PAGE_STATS)
    {
        static const u8 *const labels[] =
        {
            COMPOUND_STRING("HP"), COMPOUND_STRING("ATK"), COMPOUND_STRING("DEF"),
            COMPOUND_STRING("SPE"), COMPOUND_STRING("SPA"), COMPOUND_STRING("SPD"),
        };
        Print(5, 63, 198, COMPOUND_STRING("BASE STATS"));
        for (u8 i = 0; i < NUM_STATS; i++)
        {
            u8 x = 5 + i % 2 * 102, y = 76 + i / 2 * 14;
            Print(x, y, 27, labels[i]);
            Number(text, GetSpeciesBaseStat(selected->species, i));
            Print(x + 29, y, 33, text);
        }
        Footer(COMPOUND_STRING("A:SEARCH  L/R:PAGE  <>:SPECIES"), COMPOUND_STRING("B:CATALOG"));
    }
    else
    {
        for (u8 i = 0; i < NUM_ABILITY_SLOTS; i++)
        {
            enum Ability ability = GetSpeciesAbility(selected->species, i);
            u8 y = 64 + i * 17;
            Print(5, y, 24, i == 2 ? COMPOUND_STRING("HA") : i == 0 ? COMPOUND_STRING("A1") : COMPOUND_STRING("A2"));
            Print(30, y, 171, ability == ABILITY_NONE ? COMPOUND_STRING("--") : gAbilitiesInfo[ability].name);
        }
        Footer(COMPOUND_STRING("A:SEARCH  L/R:PAGE  <>:SPECIES"), COMPOUND_STRING("B:CATALOG"));
    }
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
        COMPOUND_STRING("BROWSE AREA"), COMPOUND_STRING("MUTANT SIGNAL"), COMPOUND_STRING("RETURN TO CATALOG"),
    };
    for (u8 i = 0; i < ARRAY_COUNT(titles); i++)
    {
        u8 y = 32 + i * 19;
        UnholyUi_Card(sUi.windowId, 5, y, 198, 17, i == sUi.toolCursor);
        UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 11, y + 4, 151,
                            UnholyUi_CardFill(i == sUi.toolCursor), titles[i]);
        if (i == 0)
        {
            u8 *end = ConvertIntToDecimalStringN(text, sUi.zone.count ? sUi.zone.current + 1 : 0, STR_CONV_MODE_LEFT_ALIGN, 2);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, sUi.zone.count, STR_CONV_MODE_LEFT_ALIGN, 2);
            UnholyUi_PrintOnFill(sUi.windowId, FONT_SMALL_NARROW, 166, y + 4, 32,
                                UnholyUi_CardFill(i == sUi.toolCursor), text);
        }
    }
    if (sUi.toolCursor == 0)
    {
        Print(7, 94, 194, COMPOUND_STRING("L/R: BROWSE FLOORS IN THIS ZONE"));
        Print(7, 106, 194, COMPOUND_STRING("SEARCH ONLY IN YOUR CURRENT AREA"));
    }
    else if (sUi.toolCursor == 1)
    {
        Print(7, 94, 194, COMPOUND_STRING("VIEW MUTATIONS AND SIGNAL STATUS"));
        Print(7, 106, 194, SignalRequirement());
    }
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

static void Task_StartSearch(u8 taskId)
{
    if (gPaletteFade.active || IsDma3ManagerBusyWithBgCopy())
        return;
    const s16 *data = gTasks[taskId].data;
    bool32 ready = Pokelocator_CreateSearchMon((u16)data[0], (u16)data[1], data[2]);
    ScriptContext_SetupScript(ready ? EventScript_PokelocatorSearchBattle : EventScript_PokelocatorSearchUnavailable);
    DestroyTask(taskId);
}

static void BeginSearch(u8 taskId)
{
    const struct PokelocatorRow *row = SearchRow();
    if (!row || Pokelocator_GetSearchStatus(Header(), row->species, row->method) != POKELOCATOR_SEARCH_READY)
    {
        PlaySE(SE_FAILURE);
        Render();
        return;
    }
    u8 next = CreateTask(Task_StartSearch, 8);
    if (next == TASK_NONE)
    {
        PlaySE(SE_FAILURE);
        return;
    }
    gTasks[next].data[0] = Header();
    gTasks[next].data[1] = row->species;
    gTasks[next].data[2] = row->method;
    PlaySE(SE_SELECT);
    Close(taskId, FALSE);
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
        case MODE_SEARCH: sUi.mode = MODE_DETAIL; break;
        case MODE_SIGNAL_CONFIRM: sUi.mode = MODE_SIGNAL; break;
        case MODE_SIGNAL: sUi.mode = MODE_TOOLS; break;
        case MODE_DETAIL: sUi.mode = MODE_LIST; break;
        default: sUi.mode = MODE_LIST; break;
        }
        Render();
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
            sUi.toolCursor = (sUi.toolCursor + (JOY_REPEAT(DPAD_UP) ? 2 : 1)) % 3;
        else if (sUi.toolCursor == 0 && (JOY_NEW(L_BUTTON | DPAD_LEFT) || JOY_NEW(R_BUTTON | DPAD_RIGHT)))
            ChangeArea(JOY_NEW(L_BUTTON | DPAD_LEFT) ? -1 : 1);
        else if (JOY_NEW(A_BUTTON))
        {
            if (sUi.toolCursor == 0) ChangeArea(1);
            else if (sUi.toolCursor == 1) sUi.mode = MODE_SIGNAL;
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
        else if (JOY_NEW(SELECT_BUTTON))
        {
            u16 species = CatalogRow() ? CatalogRow()->species : SPECIES_NONE;
            sUi.collection = (sUi.collection + 1) % POKELOCATOR_COLLECTION_COUNT;
            Refilter(species, POKELOCATOR_METHOD_COUNT);
        }
        else if (JOY_NEW(L_BUTTON | R_BUTTON))
        {
            u16 species = CatalogRow() ? CatalogRow()->species : SPECIES_NONE;
            sUi.group = (sUi.group + (JOY_NEW(L_BUTTON) ? POKELOCATOR_GROUP_COUNT - 1 : 1)) % POKELOCATOR_GROUP_COUNT;
            Refilter(species, POKELOCATOR_METHOD_COUNT);
        }
        else if (JOY_NEW(A_BUTTON) && sUi.count)
        {
            sUi.mode = MODE_DETAIL;
            sUi.page = PAGE_HABITAT;
        }
        else if (sUi.count && JOY_REPEAT(DPAD_ANY))
        {
            u8 direction = JOY_REPEAT(DPAD_LEFT) ? POKELOCATOR_GRID_LEFT : JOY_REPEAT(DPAD_RIGHT) ? POKELOCATOR_GRID_RIGHT
                         : JOY_REPEAT(DPAD_UP) ? POKELOCATOR_GRID_UP : POKELOCATOR_GRID_DOWN;
            Pokelocator_MoveGrid(&sUi.cursor, &sUi.scroll, sUi.count, direction);
            SetMethods(CatalogRow()->method);
        }
        else return;
        PlaySE(SE_SELECT);
        Render();
        return;
    }
    if (sUi.mode == MODE_DETAIL)
    {
        if (JOY_NEW(A_BUTTON)) sUi.mode = MODE_SEARCH;
        else if (JOY_NEW(L_BUTTON | R_BUTTON)) sUi.page = (sUi.page + (JOY_NEW(L_BUTTON) ? PAGE_COUNT - 1 : 1)) % PAGE_COUNT;
        else if (JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
        {
            sUi.cursor = (sUi.cursor + sUi.count + (JOY_REPEAT(DPAD_LEFT) ? -1 : 1)) % sUi.count;
            sUi.scroll = sUi.cursor / VISIBLE_CARDS * VISIBLE_CARDS;
            SetMethods(CatalogRow()->method);
        }
        else if (sUi.page == PAGE_HABITAT && JOY_REPEAT(DPAD_UP | DPAD_DOWN))
            sUi.methodCursor = (sUi.methodCursor + sUi.methodCount + (JOY_REPEAT(DPAD_UP) ? -1 : 1)) % sUi.methodCount;
        else return;
        PlaySE(SE_SELECT);
        Render();
    }
}

void Pokelocator_OpenFromField(void (*onClose)(void))
{
    if (FuncIsActiveTask(Task_Locator) || FuncIsActiveTask(Task_StartSearch))
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
