#include "global.h"
#include "pc_search.h"
#include "unholy_ui.h"
#include "battle_main.h"
#include "data.h"
#include "horizon_market.h"
#include "menu.h"
#include "malloc.h"
#include "main.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/gears.h"
#include "constants/songs.h"

#define SEARCH_SLOTS (TOTAL_BOXES_COUNT * IN_BOX_COUNT)
#define SEARCH_ROWS 7

static EWRAM_DATA struct PcSearchUi
{
    struct PcSearchQuery draft;
    struct PcSearchQuery *query;
    struct UnholyUiPalette palette;
    void (*onClose)(u16 result);
    u8 matches[PC_SEARCH_MASK_BYTES(SEARCH_SLOTS)];
    u16 count;
    u16 after;
    u16 species;
    u8 window;
    u8 row;
    u8 typeIndex;
    u8 mutationIndex;
} *sSearch;

// BG0 has one 16 KiB character block; end tile 0x1F8 stays below BG1.
static const struct WindowTemplate sSearchWindow =
{
    .bg = 0, .tilemapLeft = 3, .tilemapTop = 1, .width = 24, .height = 17,
    .paletteNum = 14, .baseBlock = 0x60,
};

static const u8 sTypes[] =
{
    TYPE_NONE, TYPE_NORMAL, TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND,
    TYPE_ROCK, TYPE_BUG, TYPE_GHOST, TYPE_STEEL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS,
    TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_DARK, TYPE_FAIRY,
};
static const u16 sMutations[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER, POKEMON_BADGE_OVERLOADED,
    POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL, POKEMON_BADGE_TITAN,
    POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
};
static const u8 *const sLabels[] =
{
    COMPOUND_STRING("SPECIES"), COMPOUND_STRING("TYPE"), COMPOUND_STRING("MUTATION"),
    COMPOUND_STRING("FAVORITE"), COMPOUND_STRING("SHINY"), COMPOUND_STRING("GEAR"),
    COMPOUND_STRING("FIND NEXT"),
};

static void BuildMatches(void)
{
    const struct PcSearchQuery *query = &sSearch->draft;
    memset(sSearch->matches, 0, sizeof(sSearch->matches));
    sSearch->count = 0;
    for (u16 pos = 0; pos < SEARCH_SLOTS; pos++)
    {
        u8 box = pos / IN_BOX_COUNT, slot = pos % IN_BOX_COUNT;
        struct PcSearchMon mon = {0};
        if (!GetBoxMonDataAt(box, slot, MON_DATA_SANITY_HAS_SPECIES)
         || GetBoxMonDataAt(box, slot, MON_DATA_SANITY_IS_BAD_EGG))
            continue;
        mon.species = GetBoxMonDataAt(box, slot, MON_DATA_SPECIES);
        if (mon.species == SPECIES_NONE || mon.species >= NUM_SPECIES)
            continue;
        mon.egg = GetBoxMonDataAt(box, slot, MON_DATA_SANITY_IS_EGG);
        if (query->type != TYPE_NONE)
        {
            mon.types[0] = gSpeciesInfo[mon.species].types[0];
            mon.types[1] = gSpeciesInfo[mon.species].types[1];
        }
        if (query->mutationMode != PC_SEARCH_ANY || query->mutations)
            mon.mutations = GetBoxMonDataAt(box, slot, MON_DATA_SPECIAL_BADGES);
        if ((query->flags & PC_SEARCH_FAVORITE) && GetBoxMonDataAt(box, slot, MON_DATA_IS_FAVORITE))
            mon.flags |= PC_SEARCH_FAVORITE;
        if ((query->flags & PC_SEARCH_SHINY) && GetBoxMonDataAt(box, slot, MON_DATA_IS_SHINY))
            mon.flags |= PC_SEARCH_SHINY;
        if ((query->flags & PC_SEARCH_GEAR) && GetBoxMonDataAt(box, slot, MON_DATA_GEAR) != GEAR_NONE)
            mon.flags |= PC_SEARCH_GEAR;
        if (PcSearch_Matches(&mon, query))
        {
            sSearch->matches[pos / 8] |= 1 << (pos % 8);
            sSearch->count++;
        }
    }
}

static const u8 *Value(u8 row)
{
    const struct PcSearchQuery *query = &sSearch->draft;
    switch (row)
    {
    case 0: return query->species ? GetSpeciesName(query->species) : COMPOUND_STRING("ANY");
    case 1: return query->type ? gTypesInfo[query->type].name : COMPOUND_STRING("ANY");
    case 2:
        if (query->mutations)
            return HorizonMarketMutationName(query->mutations);
        return query->mutationMode == PC_SEARCH_NORMAL ? COMPOUND_STRING("NO MUTATIONS")
             : query->mutationMode == PC_SEARCH_MUTATED ? COMPOUND_STRING("ANY MUTATION") : COMPOUND_STRING("ANY");
    case 3: return query->flags & PC_SEARCH_FAVORITE ? COMPOUND_STRING("ONLY") : COMPOUND_STRING("ANY");
    case 4: return query->flags & PC_SEARCH_SHINY ? COMPOUND_STRING("ONLY") : COMPOUND_STRING("ANY");
    case 5: return query->flags & PC_SEARCH_GEAR ? COMPOUND_STRING("EQUIPPED") : COMPOUND_STRING("ANY");
    default: return COMPOUND_STRING("A");
    }
}

static void Render(void)
{
    u8 text[32];
    UnholyUi_Frame(sSearch->window, COMPOUND_STRING("PC SEARCH"));
    for (u8 row = 0; row < SEARCH_ROWS; row++)
    {
        bool8 selected = row == sSearch->row;
        u8 y = 23 + row * 12;
        u8 fill = UnholyUi_CardFill(selected);
        UnholyUi_Card(sSearch->window, 5, y, 182, 13, selected);
        UnholyUi_PrintOnFill(sSearch->window, FONT_SMALL, 10, y, 76, fill, sLabels[row]);
        UnholyUi_PrintOnFill(sSearch->window, FONT_SMALL, 91, y, 90, fill, Value(row));
    }
    u8 *cursor = StringCopy(text, COMPOUND_STRING("MATCHES "));
    ConvertUIntToDecimalStringN(cursor, sSearch->count, STR_CONV_MODE_LEFT_ALIGN, 4);
    UnholyUi_Print(sSearch->window, FONT_SMALL_NARROW, 7, 108, 178, text);
    UnholyUi_Print(sSearch->window, FONT_SMALL_NARROW, 7, 123, 178,
        sSearch->row == SEARCH_ROWS - 1 ? COMPOUND_STRING("A:FIND B:BACK")
                                      : COMPOUND_STRING("L/R:CHANGE A:NEXT B:BACK"));
    CopyWindowToVram(sSearch->window, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void Close(u8 task, u16 result)
{
    void (*onClose)(u16) = sSearch->onClose;
    if (result != PC_SEARCH_NONE)
        *sSearch->query = sSearch->draft;
    ClearWindowTilemap(sSearch->window);
    RemoveWindow(sSearch->window);
    UnholyUi_ReleasePalette(&sSearch->palette);
    Free(sSearch);
    sSearch = NULL;
    DestroyTask(task);
    ScheduleBgCopyTilemapToVram(0);
    onClose(result);
}

static void Adjust(s8 step)
{
    struct PcSearchQuery *query = &sSearch->draft;
    switch (sSearch->row)
    {
    case 0: query->species = query->species ? SPECIES_NONE : sSearch->species; break;
    case 1:
        sSearch->typeIndex = (sSearch->typeIndex + ARRAY_COUNT(sTypes) + step) % ARRAY_COUNT(sTypes);
        query->type = sTypes[sSearch->typeIndex];
        break;
    case 2:
        sSearch->mutationIndex = (sSearch->mutationIndex + ARRAY_COUNT(sMutations) + 3 + step) % (ARRAY_COUNT(sMutations) + 3);
        query->mutationMode = min(sSearch->mutationIndex, PC_SEARCH_MUTATED);
        query->mutations = sSearch->mutationIndex < 3 ? 0 : sMutations[sSearch->mutationIndex - 3];
        break;
    case 3: query->flags ^= PC_SEARCH_FAVORITE; break;
    case 4: query->flags ^= PC_SEARCH_SHINY; break;
    case 5: query->flags ^= PC_SEARCH_GEAR; break;
    default: return;
    }
    BuildMatches();
}

static void Task_Search(u8 task)
{
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Close(task, PC_SEARCH_NONE);
        return;
    }
    if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
        sSearch->row = (sSearch->row + (JOY_REPEAT(DPAD_UP) ? SEARCH_ROWS - 1 : 1)) % SEARCH_ROWS;
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
        Adjust(JOY_NEW(DPAD_LEFT | L_BUTTON) ? -1 : 1);
    else if (JOY_NEW(A_BUTTON))
    {
        if (sSearch->row != SEARCH_ROWS - 1)
            sSearch->row++;
        else
        {
            u16 result = PcSearch_FindNext(sSearch->matches, SEARCH_SLOTS, sSearch->after);
            if (result == PC_SEARCH_NONE)
            {
                PlaySE(SE_FAILURE);
                return;
            }
            PlaySE(SE_SELECT);
            Close(task, result);
            return;
        }
    }
    else
        return;
    PlaySE(SE_SELECT);
    Render();
}

bool32 PcSearch_Open(struct PcSearchQuery *query, u16 species, u16 after, void (*onClose)(u16 result))
{
    if (sSearch != NULL || query == NULL || onClose == NULL)
        return FALSE;
    sSearch = AllocZeroed(sizeof(*sSearch));
    if (sSearch == NULL)
        return FALSE;
    u8 task = CreateTask(Task_Search, 1);
    if (task == TASK_NONE)
    {
        Free(sSearch);
        sSearch = NULL;
        return FALSE;
    }
    sSearch->window = AddWindow(&sSearchWindow);
    if (sSearch->window == WINDOW_NONE)
    {
        DestroyTask(task);
        Free(sSearch);
        sSearch = NULL;
        return FALSE;
    }
    sSearch->query = query;
    sSearch->draft = *query;
    sSearch->onClose = onClose;
    sSearch->after = after;
    sSearch->species = species < NUM_SPECIES ? species : SPECIES_NONE;
    for (u8 i = 0; i < ARRAY_COUNT(sTypes); i++)
        if (sTypes[i] == query->type)
            sSearch->typeIndex = i;
    sSearch->mutationIndex = query->mutationMode;
    for (u8 i = 0; i < ARRAY_COUNT(sMutations); i++)
        if (sMutations[i] == query->mutations)
            sSearch->mutationIndex = i + 3;
    UnholyUi_AcquirePalette(&sSearch->palette, 14);
    PutWindowTilemap(sSearch->window);
    BuildMatches();
    Render();
    return TRUE;
}
