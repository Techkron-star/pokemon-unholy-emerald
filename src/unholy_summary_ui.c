#include "global.h"
#include "mutation_icons.h"
#include "unholy_summary_ui.h"
#include "unholy_mon_summary.h"
#include "unholy_evolution.h"
#include "evolution_scene.h"
#include "move_relearner.h"
#include "naming_screen.h"
#include "tv.h"
#include "caps.h"
#include "unholy_ui.h"
#include "battle.h"
#include "bg.h"
#include "data.h"
#include "gear.h"
#include "gpu_regs.h"
#include "horizon_market.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "constants/party_menu.h"

enum { PAGE_INFO, PAGE_STATS, PAGE_MOVES, PAGE_GEAR, PAGE_COUNT };
enum { QOL_NONE, QOL_MENU, QOL_EVOLUTIONS, QOL_REVIEW, QOL_CONFIRM };
#define EVO_VISIBLE 5
#define EVO_DEPTH 8

static EWRAM_DATA struct NativeSummary
{
    void *mons;
    void (*callback)(void);
    struct Pokemon mon;
    struct UnholyMonSummary values;
    u16 tilemap[32 * 32];
    u16 portrait;
    u8 mutationIcons[10];
    u8 window;
    u8 mode;
    u8 index;
    u8 last;
    u8 page;
    u8 detail;
    u8 move;
    u8 qol, tool, depth;
    u16 viewSpecies, evoCursor, evoScroll, requirementPage;
    u16 parents[EVO_DEPTH], parentCursors[EVO_DEPTH];
    struct UnholyEvolutionOption evolution;
    struct UnholyEvolutionPlan plan;
    const u8 *notice;
    u8 rename[POKEMON_NAME_LENGTH + 1];
    void (*previousEvoCallback)(void);
} *sSummary;

static const struct BgTemplate sBg = {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0};
static const struct WindowTemplate sWindows[] =
{
    {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 18, .paletteNum = 14, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};
static const u8 *const sTabs[] =
{
    COMPOUND_STRING("INFO"), COMPOUND_STRING("STATS"), COMPOUND_STRING("MOVES"), COMPOUND_STRING("GEAR"),
};
static const u8 *const sStats[NUM_STATS] =
{
    COMPOUND_STRING("HP"), COMPOUND_STRING("ATTACK"), COMPOUND_STRING("DEFENSE"),
    COMPOUND_STRING("SPEED"), COMPOUND_STRING("SP. ATK"), COMPOUND_STRING("SP. DEF"),
};
static const u8 sStatOrder[] = {STAT_HP, STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED};
static const enum PokemonBadge sBadges[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER, POKEMON_BADGE_OVERLOADED,
    POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL, POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED,
    POKEMON_BADGE_OMEGA,
};

static void Init(void);
static void Cleanup(void);
static bool32 QolInput(u8 taskId);
static void DrawQol(void);

static void Print(u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_Print(sSummary->window, FONT_SMALL, x, y, width, text);
}

static void Number(u16 x, u16 y, u32 value)
{
    u8 text[16];
    ConvertUIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, 5);
    Print(x, y, 60, text);
}

static void Paragraph(const u8 *source, u16 y, u8 rows)
{
    u8 text[256];
    u8 *end = StringCopyN(text, source, sizeof(text) - 1);
    *end = EOS;
    WrapFontIdToFit(text, end, FONT_SMALL, 204);
    const u8 *line = text;
    for (u8 row = 0; row < rows && *line != EOS; row++)
    {
        Print(9, y + row * 12, 204, line);
        while (*line != EOS && *line != CHAR_NEWLINE)
            line++;
        if (*line == CHAR_NEWLINE)
            line++;
    }
}

static bool8 IsBox(void)
{
    return sSummary->mode == SUMMARY_MODE_BOX;
}

static void ReadMon(void)
{
    if (IsBox())
        BoxMonToMon(&((struct BoxPokemon *)sSummary->mons)[sSummary->index], &sSummary->mon);
    else
        sSummary->mon = ((struct Pokemon *)sSummary->mons)[sSummary->index];
    memset(&sSummary->values, 0, sizeof(sSummary->values));
    UnholyMonSummary_Build(&sSummary->mon, &sSummary->values);
}

static void DestroyPortrait(void)
{
    if (sSummary->portrait < MAX_SPRITES)
        FreeAndDestroyMonPicSprite(sSummary->portrait);
    sSummary->portrait = SPRITE_NONE;
}

static void DestroyMutationIcons(void)
{
    for (u8 i = 0; i < ARRAY_COUNT(sSummary->mutationIcons); i++)
    {
        MutationIcon_Destroy(sSummary->mutationIcons[i]);
        sSummary->mutationIcons[i] = MAX_SPRITES;
    }
}

static void Info(void)
{
    struct Pokemon *mon = &sSummary->mon;
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    u8 text[64];

    if (sSummary->portrait == SPRITE_NONE)
        sSummary->portrait = CreateMonPicSprite(GetMonData(mon, MON_DATA_IS_EGG) ? SPECIES_EGG : species,
            GetMonData(mon, MON_DATA_IS_SHINY), GetMonData(mon, MON_DATA_PERSONALITY), TRUE, 49, 84, 0, TAG_NONE);

    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        Print(86, 47, 130, COMPOUND_STRING("EGG"));
        Print(86, 65, 130, COMPOUND_STRING("A: HATCH DETAILS"));
        return;
    }
    Print(85, 43, 132, GetSpeciesName(species));
    StringCopy(text, gTypesInfo[gSpeciesInfo[species].types[0]].name);
    if (gSpeciesInfo[species].types[0] != gSpeciesInfo[species].types[1])
    {
        StringAppend(text, COMPOUND_STRING(" / "));
        StringAppend(text, gTypesInfo[gSpeciesInfo[species].types[1]].name);
    }
    Print(85, 57, 132, text);
    Print(85, 72, 132, gNaturesInfo[GetNature(mon)].name);
    Print(85, 87, 132, gAbilitiesInfo[GetMonAbility(mon)].name);
    StringCopy(text, COMPOUND_STRING("STA MAX "));
    ConvertUIntToDecimalStringN(text + StringLength(text), sSummary->values.staminaMax, STR_CONV_MODE_LEFT_ALIGN, 3);
    Print(85, 103, 132, text);
    u8 *cursor = StringCopy(text, COMPOUND_STRING("HP "));
    cursor = ConvertUIntToDecimalStringN(cursor, GetMonData(mon, MON_DATA_HP), STR_CONV_MODE_LEFT_ALIGN, 4);
    cursor = StringCopy(cursor, COMPOUND_STRING("/"));
    ConvertUIntToDecimalStringN(cursor, GetMonData(mon, MON_DATA_MAX_HP), STR_CONV_MODE_LEFT_ALIGN, 4);
    Print(9, 117, 104, text);
    UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, 118, 119, 96,
        sSummary->values.mutationBadges ? COMPOUND_STRING("MUTATIONS: GEAR TAB") : COMPOUND_STRING("NO MUTATIONS"));
}

static void Stats(void)
{
    struct UnholyMonSummary *values = &sSummary->values;
    const u8 *mode = sSummary->detail == 1 ? COMPOUND_STRING("IVs") : sSummary->detail == 2 ? COMPOUND_STRING("EVs") : COMPOUND_STRING("STATS");
    FillWindowPixelRect(sSummary->window, PIXEL_FILL(1), 173, 3, 43, 14);
    UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, 177, 6, 39, mode);
    for (u8 row = 0; row < NUM_STATS; row++)
    {
        u8 stat = sStatOrder[row];
        u16 y = 42 + row * 12;
        u32 value = sSummary->detail == 1 ? GetAdjustedIvData(&sSummary->mon, stat)
                  : sSummary->detail == 2 ? values->evs[stat] : values->stats[stat];
        Print(9, y, 72, sStats[stat]);
        Number(91, y, value);
        UnholyUi_Bar(sSummary->window, 146, y + 8, 66, value,
            sSummary->detail == 1 ? MAX_PER_STAT_IVS : sSummary->detail == 2 ? MAX_PER_STAT_EVS : max(255, value), 5);
    }
    Print(9, 115, 105, COMPOUND_STRING("STAMINA MAX"));
    Number(112, 115, values->staminaMax);
}

static void Moves(void)
{
    struct Pokemon *mon = &sSummary->mon;
    u8 text[32];
    for (u8 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + i);
        u8 fill = UnholyUi_CardFill(i == sSummary->move);
        u8 y = 40 + i * 17;
        UnholyUi_Card(sSummary->window, 6, y, 212, 16, i == sSummary->move);
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL, 12, y + 1, 142, fill, GetMoveName(move));
        ConvertUIntToDecimalStringN(text, GetMonData(mon, MON_DATA_PP1 + i), STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(text, COMPOUND_STRING(" PP"));
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL, 162, y + 1, 51, fill, text);
    }
    enum Move selected = GetMonData(mon, MON_DATA_MOVE1 + sSummary->move);
    Print(9, 113, 204, selected == MOVE_NONE ? COMPOUND_STRING("EMPTY MOVE SLOT") : GetMoveDescription(selected));
}

static void Gear(void)
{
    struct UnholyMonSummary *values = &sSummary->values;
    if (sSummary->detail == 0)
    {
        UnholyUi_Card(sSummary->window, 7, 41, 210, 29, FALSE);
        UnholyUi_Card(sSummary->window, 7, 73, 210, 29, FALSE);
        u8 fill = UnholyUi_CardFill(FALSE);
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 13, 42, 198, fill, COMPOUND_STRING("HELD ITEM"));
        u16 item = GetMonData(&sSummary->mon, MON_DATA_HELD_ITEM);
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 13, 55, 198, fill, item == ITEM_NONE ? COMPOUND_STRING("EMPTY ITEM SLOT") : GetItemName(item));
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 13, 74, 198, fill, COMPOUND_STRING("GEAR - INDEPENDENT SLOT"));
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 13, 87, 198, fill, values->gearId > GEAR_NONE && values->gearId < GEAR_COUNT ? gGearsInfo[values->gearId].name : COMPOUND_STRING("EMPTY GEAR SLOT"));
        if (values->gearId > GEAR_NONE && values->gearId < GEAR_COUNT)
            Paragraph(gGearsInfo[values->gearId].description, 103, 2);
        else
            UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, 9, 108, 204, COMPOUND_STRING("BAG > GEAR: SELECT A GEAR TO EQUIP"));
    }
    else if (sSummary->detail == 1)
    {
        for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
        {
            if (values->mutationBadges & sBadges[i])
            {
                u8 x = 9 + (i % 3) * 70;
                u8 y = 40 + (i / 3) * 28;
                const u8 *name = HorizonMarketMutationName(sBadges[i]);
                u16 width = GetStringWidth(FONT_SMALL_NARROW, name, 0);
                sSummary->mutationIcons[i] = MutationIcon_Create(sBadges[i], x + 42, y + 15, 0);
                UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, x + (68 - min(width, 68)) / 2, y + 16, 68, name);
            }
        }
        if (values->mutationBadges == 0)
        {
            UnholyUi_Card(sSummary->window, 7, 46, 210, 62, FALSE);
            u8 fill = UnholyUi_CardFill(FALSE);
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 17, 52, 190, fill, COMPOUND_STRING("NO MUTATIONS"));
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 17, 69, 190, fill, COMPOUND_STRING("THIS POKéMON HAS NO MUTATION BADGES."));
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 17, 86, 190, fill, COMPOUND_STRING("L/R: EQUIPMENT / ACTUAL BONUSES"));
        }
    }
    else
    {
        for (u8 row = 0; row < NUM_STATS; row++)
        {
            u8 stat = sStatOrder[row];
            s32 bonus = values->mutationBonusPercent[stat];
            u8 text[24];
            StringCopy(text, bonus < 0 ? COMPOUND_STRING("-") : COMPOUND_STRING("+"));
            ConvertUIntToDecimalStringN(text + StringLength(text), abs(bonus), STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(text, COMPOUND_STRING("%"));
            Print(9, 43 + row * 12, 110, sStats[stat]);
            Print(143, 43 + row * 12, 65, text);
        }
        Print(9, 116, 204, COMPOUND_STRING("ACTUAL MUTATION BONUSES"));
    }
}

static void Render(void)
{
    if (sSummary->qol != QOL_NONE)
    {
        DestroyMutationIcons();
        DestroyPortrait();
        DrawQol();
        PutWindowTilemap(sSummary->window);
        CopyWindowToVram(sSummary->window, COPYWIN_FULL);
        return;
    }
    u8 name[POKEMON_NAME_LENGTH + 1];
    u8 text[32];
    GetMonData(&sSummary->mon, MON_DATA_NICKNAME, name);
    DestroyMutationIcons();
    u16 badges = GetMonData(&sSummary->mon, MON_DATA_IS_EGG) ? 0 : sSummary->values.mutationBadges;
    UnholyUi_Frame(sSummary->window, COMPOUND_STRING(""));
    Print(7, 4, badges ? 136 : 160, name);
    if (badges)
    {
        sSummary->mutationIcons[9] = MutationIcon_Create(MutationIcon_BadgeAt(badges, 0), 163, 18, 0);
        if (badges & (badges - 1))
            Print(166, 4, 8, COMPOUND_STRING("+"));
    }
    StringCopy(text, COMPOUND_STRING("Lv "));
    ConvertUIntToDecimalStringN(text + StringLength(text), GetMonData(&sSummary->mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    Print(174, 4, 42, text);
    for (u8 tab = 0; tab < PAGE_COUNT; tab++)
    {
        u8 x = 6 + tab * 54;
        UnholyUi_Card(sSummary->window, x, 23, 51, 15, tab == sSummary->page);
        UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, x + 6, 25, 43,
            UnholyUi_CardFill(tab == sSummary->page), sTabs[tab]);
    }
    if (sSummary->page != PAGE_INFO)
        DestroyPortrait();
    if (GetMonData(&sSummary->mon, MON_DATA_IS_EGG) && sSummary->page != PAGE_INFO)
        Print(9, 58, 204, COMPOUND_STRING("EGG DATA IS NOT AVAILABLE YET."));
    else
    {
        switch (sSummary->page)
        {
        case PAGE_INFO: Info(); break;
        case PAGE_STATS: Stats(); break;
        case PAGE_MOVES: Moves(); break;
        case PAGE_GEAR: Gear(); break;
        }
    }
    const u8 *help = sSummary->page == PAGE_STATS ? COMPOUND_STRING("<>:TAB L/R:IV/EV START:TOOLS B:BACK")
                   : sSummary->page == PAGE_GEAR ? COMPOUND_STRING("<>:TAB L/R:VIEW START:TOOLS B:BACK")
                   : sSummary->page == PAGE_MOVES ? COMPOUND_STRING("<>:TAB A:DETAIL START:TOOLS B:BACK")
                   : COMPOUND_STRING("<>:TAB A:DETAIL START:TOOLS B:BACK");
    UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, 7, 131, 210, help);
    PutWindowTilemap(sSummary->window);
    CopyWindowToVram(sSummary->window, COPYWIN_FULL);
}

static void VBlank(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Cleanup(void)
{
    SetVBlankCallback(NULL);
    DestroyPortrait();
    DestroyMutationIcons();
    gReservedSpritePaletteCount = 0;
    FreeAllWindowBuffers();
    UnsetBgTilemapBuffer(0);
}

static void ReturnFromDetail(void)
{
    sSummary->index = gLastViewedMonIndex;
    sSummary->portrait = SPRITE_NONE;
    SetMainCallback2(Init);
}

static void Input(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    if (QolInput(taskId)) return;
    if (JOY_NEW(B_BUTTON))
    {
        void (*callback)(void) = sSummary->callback;
        gLastViewedMonIndex = sSummary->index;
        Cleanup();
        Free(sSummary);
        sSummary = NULL;
        DestroyTask(taskId);
        SetMainCallback2(callback);
        return;
    }
    if (JOY_NEW(A_BUTTON) && (sSummary->page == PAGE_INFO || sSummary->page == PAGE_MOVES))
    {
        Cleanup();
        DestroyTask(taskId);
        ShowPokemonSummaryScreenLegacyPage(sSummary->mode, sSummary->mons, sSummary->index, sSummary->last,
            ReturnFromDetail, sSummary->page == PAGE_MOVES ? PSS_PAGE_BATTLE_MOVES : PSS_PAGE_INFO);
        return;
    }
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        sSummary->page = (sSummary->page + (JOY_NEW(DPAD_LEFT) ? PAGE_COUNT - 1 : 1)) % PAGE_COUNT;
        sSummary->detail = 0;
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON) && (sSummary->page == PAGE_STATS || sSummary->page == PAGE_GEAR))
        sSummary->detail = (sSummary->detail + (JOY_NEW(L_BUTTON) ? 2 : 1)) % 3;
    else if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        s8 delta = JOY_REPEAT(DPAD_UP) ? -1 : 1;
        if (sSummary->page == PAGE_MOVES)
            sSummary->move = (sSummary->move + MAX_MON_MOVES + delta) % MAX_MON_MOVES;
        else
        {
            u8 original = sSummary->index;
            do
            {
                sSummary->index = (sSummary->index + sSummary->last + 1 + delta) % (sSummary->last + 1);
                ReadMon();
            } while (GetMonData(&sSummary->mon, MON_DATA_SPECIES) == SPECIES_NONE && sSummary->index != original);
            DestroyPortrait();
        }
    }
    else
        return;
    PlaySE(SE_SELECT);
    Render();
}

static bool32 EditableParty(void)
{
    return sSummary->mode == SUMMARY_MODE_NORMAL && sSummary->mons == gParties[B_TRAINER_PLAYER]
        && sSummary->index < gPartiesCount[B_TRAINER_PLAYER] && sSummary->index < PARTY_SIZE;
}

static bool32 QolValid(void)
{
    return GetMonData(&sSummary->mon, MON_DATA_SPECIES) != SPECIES_NONE
        && !GetMonData(&sSummary->mon, MON_DATA_IS_EGG)
        && !GetMonData(&sSummary->mon, MON_DATA_SANITY_IS_BAD_EGG);
}

static void ReturnFromQol(void)
{
    if (EditableParty()) sSummary->last = gPartiesCount[B_TRAINER_PLAYER] - 1;
    sSummary->portrait = SPRITE_NONE;
    sSummary->qol = QOL_MENU;
    sSummary->notice = NULL;
    SetMainCallback2(Init);
}

static void ReturnFromEvolution(void)
{
    gCB2_AfterEvolution = sSummary->previousEvoCallback;
    ReturnFromQol();
}

static void ReturnFromRename(void)
{
    if (IsBox()) SetBoxMonData(&((struct BoxPokemon *)sSummary->mons)[sSummary->index], MON_DATA_NICKNAME, sSummary->rename);
    else if (EditableParty()) SetMonData(&((struct Pokemon *)sSummary->mons)[sSummary->index], MON_DATA_NICKNAME, sSummary->rename);
    ReturnFromQol();
}

static void QolText(u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_Print(sSummary->window, FONT_SMALL_NARROW, x, y, width, text);
}

static void QolFooter(const u8 *first, const u8 *second)
{
    FillWindowPixelRect(sSummary->window, PIXEL_FILL(1), 5, 125, 214, 1);
    FillWindowPixelRect(sSummary->window, PIXEL_FILL(3), 5, 117, 214, 1);
    QolText(7, 118, 210, first);
    QolText(7, 130, 210, second);
}

static bool32 SelectedEvolution(void)
{
    struct Pokemon *mon = EditableParty() ? &gParties[B_TRAINER_PLAYER][sSummary->index] : &sSummary->mon;
    return UnholyEvolution_GetOption(mon, sSummary->viewSpecies, sSummary->evoCursor,
        EditableParty() ? sSummary->index : PARTY_SIZE, &sSummary->evolution);
}

static void DrawQol(void)
{
    u8 text[128];
    UnholyUi_Frame(sSummary->window, sSummary->qol == QOL_MENU ? COMPOUND_STRING("TRAINER TOOLS") : COMPOUND_STRING("EVOLUTION PATHS"));
    if (sSummary->qol == QOL_MENU)
    {
        static const u8 *const names[] = {COMPOUND_STRING("EVOLUTION PATHS / EVOLVE"), COMPOUND_STRING("REMEMBER MOVES"), COMPOUND_STRING("CHANGE NICKNAME"), COMPOUND_STRING("RETURN TO SUMMARY")};
        for (u8 i = 0; i < ARRAY_COUNT(names); i++)
        {
            u8 y = 25 + i * 22;
            UnholyUi_Card(sSummary->window, 6, y, 212, 20, i == sSummary->tool);
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 13, y + 4, 198,
                UnholyUi_CardFill(i == sSummary->tool), names[i]);
        }
        QolFooter(sSummary->notice != NULL ? sSummary->notice : COMPOUND_STRING("NO EXTRA LEVEL NEEDED TO EVOLVE"), COMPOUND_STRING("UP/DN:CHOOSE A:OPEN B:SUMMARY"));
        return;
    }
    QolText(119, 6, 98, GetSpeciesName(sSummary->viewSpecies));
    if (sSummary->qol == QOL_EVOLUTIONS)
    {
        u16 count = UnholyEvolution_Count(sSummary->viewSpecies);
        if (!count)
        {
            QolText(12, 48, 200, COMPOUND_STRING("NO FURTHER EVOLUTION"));
            QolText(12, 68, 200, COMPOUND_STRING("L / B: PREVIOUS STAGE"));
        }
        for (u8 i = 0; i < EVO_VISIBLE && sSummary->evoScroll + i < count; i++)
        {
            struct UnholyEvolutionOption option;
            u16 row = sSummary->evoScroll + i;
            struct Pokemon *mon = EditableParty() ? &gParties[B_TRAINER_PLAYER][sSummary->index] : &sSummary->mon;
            UnholyEvolution_GetOption(mon, sSummary->viewSpecies, row,
                EditableParty() ? sSummary->index : PARTY_SIZE, &option);
            bool8 selected = row == sSummary->evoCursor;
            u8 y = 24 + i * 16;
            u8 fill = UnholyUi_CardFill(selected);
            UnholyUi_Card(sSummary->window, 6, y, 212, 15, selected);
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 12, y + 1, 130, fill, GetSpeciesName(option.target));
            const u8 *state = option.status == UNHOLY_EVO_READY ? COMPOUND_STRING("READY") : option.status == UNHOLY_EVO_ITEM ? COMPOUND_STRING("ITEM")
                : option.status == UNHOLY_EVO_EVERSTONE ? COMPOUND_STRING("EVERSTONE") : option.status == UNHOLY_EVO_PREVIEW ? COMPOUND_STRING("PREVIEW") : COMPOUND_STRING("LOCKED");
            if (option.status == UNHOLY_EVO_LEVEL)
            {
                u8 *end = StringCopy(text, COMPOUND_STRING("Lv "));
                ConvertIntToDecimalStringN(end, option.minimumLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
                state = text;
            }
            UnholyUi_PrintOnFill(sSummary->window, FONT_SMALL_NARROW, 149, y + 1, 64, fill, state);
        }
        u8 *end = ConvertIntToDecimalStringN(text, count ? sSummary->evoCursor + 1 : 0, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, COMPOUND_STRING("/"));
        end = ConvertIntToDecimalStringN(end, count, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, COMPOUND_STRING("  BADGE CAP: "));
        ConvertIntToDecimalStringN(end, GetCurrentLevelCap(), STR_CONV_MODE_LEFT_ALIGN, 3);
        QolText(8, 104, 208, text);
        QolFooter(sSummary->notice != NULL ? sSummary->notice : COMPOUND_STRING("A:REVIEW REQUIREMENTS R:NEXT STAGE"), COMPOUND_STRING("UP/DN:BRANCH L:PREVIOUS B:BACK"));
        return;
    }
    const struct UnholyEvolutionOption *option = &sSummary->evolution;
    QolText(8, 23, 208, GetSpeciesName(option->target));
    UnholyEvolution_StatusText(option, text);
    QolText(8, 36, 208, text);
    sSummary->portrait = CreateMonPicSprite(option->target, GetMonData(&sSummary->mon, MON_DATA_IS_SHINY),
        GetMonData(&sSummary->mon, MON_DATA_PERSONALITY), TRUE, 182, 80, 0, TAG_NONE);
    if (sSummary->qol == QOL_CONFIRM)
    {
        if (!option->costCount)
        {
            QolText(8, 55, 134, COMPOUND_STRING("NO ITEMS REQUIRED"));
            QolText(8, 72, 134, COMPOUND_STRING("NO EXTRA LEVEL OR EXP"));
        }
        for (u8 i = 0; i < option->costCount; i++)
        {
            const struct UnholyEvolutionCost *cost = &option->costs[i];
            u8 *end = StringCopy(text, GetItemName(cost->item));
            end = StringAppend(end, COMPOUND_STRING(" x"));
            end = ConvertIntToDecimalStringN(end, cost->count, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(end, cost->held ? COMPOUND_STRING(" (HELD)") : COMPOUND_STRING(" (BAG)"));
            QolText(8, 51 + 13 * i, 134, text);
        }
        QolText(8, 105, 208, option->costCount ? COMPOUND_STRING("CONFIRM USES ALL LISTED ITEMS") : COMPOUND_STRING("B CAN STOP THE EVOLUTION ANIMATION"));
        QolFooter(COMPOUND_STRING("A:EVOLVE  B:REVIEW"), COMPOUND_STRING("ONLY THIS EVOLUTION STAGE"));
        return;
    }
    u16 lines = UnholyEvolution_RequirementCount(option);
    for (u8 i = 0; i < 3 && sSummary->requirementPage * 3 + i < lines; i++)
    {
        UnholyEvolution_RequirementText(option, sSummary->requirementPage * 3 + i, text);
        QolText(8, 51 + i * 13, 134, text);
    }
    if (option->costCount)
    {
        const struct UnholyEvolutionCost *cost = &option->costs[0];
        u8 *end = StringCopy(text, COMPOUND_STRING("USES: "));
        end = StringAppend(end, GetItemName(cost->item));
        end = StringAppend(end, COMPOUND_STRING(" x"));
        ConvertIntToDecimalStringN(end, cost->count, STR_CONV_MODE_LEFT_ALIGN, 3);
        QolText(8, 93, 134, text);
        QolText(8, 105, 208, COMPOUND_STRING("A REVIEWS ALL COSTS BEFORE EVOLVING"));
    }
    else QolText(8, 104, 208, COMPOUND_STRING("LEVEL AND MUTATIONS ARE PRESERVED"));
    bool32 canEvolve = EditableParty() && sSummary->viewSpecies == GetMonData(&sSummary->mon, MON_DATA_SPECIES)
                   && option->status == UNHOLY_EVO_READY;
    QolFooter(!EditableParty() ? COMPOUND_STRING("MOVE TO YOUR PARTY TO EVOLVE") : canEvolve ? COMPOUND_STRING("A:REVIEW EVOLUTION B:CANCEL") : COMPOUND_STRING("REQUIREMENTS NOT MET - B:BACK"),
        COMPOUND_STRING("L/R:REQUIREMENTS  B:PATHS"));
}

static bool32 QolInput(u8 taskId)
{
    if (sSummary->qol == QOL_NONE)
    {
        if (!JOY_NEW(START_BUTTON)) return FALSE;
        sSummary->qol = QOL_MENU;
        sSummary->notice = NULL;
        PlaySE(SE_SELECT); Render(); return TRUE;
    }
    if (JOY_NEW(B_BUTTON))
    {
        if (sSummary->qol == QOL_MENU) sSummary->qol = QOL_NONE;
        else if (sSummary->qol == QOL_CONFIRM) sSummary->qol = QOL_REVIEW;
        else if (sSummary->qol == QOL_REVIEW) sSummary->qol = QOL_EVOLUTIONS;
        else if (sSummary->depth)
        {
            sSummary->viewSpecies = sSummary->parents[--sSummary->depth];
            sSummary->evoCursor = sSummary->parentCursors[sSummary->depth];
            sSummary->evoScroll = sSummary->evoCursor / EVO_VISIBLE * EVO_VISIBLE;
        }
        else sSummary->qol = QOL_MENU;
        sSummary->notice = NULL;
        PlaySE(SE_SELECT); Render(); return TRUE;
    }
    if (sSummary->qol == QOL_MENU)
    {
        if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
        { sSummary->tool = (sSummary->tool + (JOY_REPEAT(DPAD_UP) ? 3 : 1)) % 4; sSummary->notice = NULL; }
        else if (JOY_NEW(A_BUTTON))
        {
            if (sSummary->tool == 3) sSummary->qol = QOL_NONE;
            else if (!QolValid()) sSummary->notice = COMPOUND_STRING("EGG / INVALID POKéMON - UNAVAILABLE");
            else if (sSummary->tool == 0)
            {
                sSummary->qol = QOL_EVOLUTIONS;
                sSummary->viewSpecies = GetMonData(&sSummary->mon, MON_DATA_SPECIES);
                sSummary->evoCursor = sSummary->evoScroll = sSummary->depth = 0;
                sSummary->notice = NULL;
            }
            else if (sSummary->tool == 1)
            {
                if (!EditableParty()) sSummary->notice = COMPOUND_STRING("MOVE TO YOUR PARTY TO RELEARN");
                else if (MoveRelearner_OpenForParty(sSummary->index, ReturnFromQol))
                { Cleanup(); DestroyTask(taskId); return TRUE; }
                else sSummary->notice = COMPOUND_STRING("NO AVAILABLE MOVES TO RELEARN");
            }
            else if ((!IsBox() && !EditableParty()) || sSummary->mode == SUMMARY_MODE_BOX_CURSOR)
                sSummary->notice = COMPOUND_STRING("NICKNAME CANNOT BE CHANGED HERE");
            else if (GetMonData(&sSummary->mon, MON_DATA_OT_ID) != GetPlayerIDAsU32())
                sSummary->notice = COMPOUND_STRING("ONLY THE ORIGINAL TRAINER CAN RENAME");
            else
            {
                GetMonData(&sSummary->mon, MON_DATA_NICKNAME, sSummary->rename);
                Cleanup(); DestroyTask(taskId);
                DoNamingScreen(NAMING_SCREEN_NICKNAME, sSummary->rename, GetMonData(&sSummary->mon, MON_DATA_SPECIES),
                    GetMonGender(&sSummary->mon), GetMonData(&sSummary->mon, MON_DATA_PERSONALITY), ReturnFromRename);
                return TRUE;
            }
        }
        else return TRUE;
        PlaySE(SE_SELECT); Render(); return TRUE;
    }
    if (sSummary->qol == QOL_EVOLUTIONS)
    {
        u16 count = UnholyEvolution_Count(sSummary->viewSpecies);
        if (count && JOY_REPEAT(DPAD_UP | DPAD_DOWN))
        {
            sSummary->evoCursor = (sSummary->evoCursor + count + (JOY_REPEAT(DPAD_UP) ? -1 : 1)) % count;
            sSummary->evoScroll = sSummary->evoCursor / EVO_VISIBLE * EVO_VISIBLE;
        }
        else if (JOY_NEW(L_BUTTON) && sSummary->depth)
        {
            sSummary->viewSpecies = sSummary->parents[--sSummary->depth];
            sSummary->evoCursor = sSummary->parentCursors[sSummary->depth];
            sSummary->evoScroll = sSummary->evoCursor / EVO_VISIBLE * EVO_VISIBLE;
        }
        else if (JOY_NEW(R_BUTTON) && count && sSummary->depth < EVO_DEPTH && SelectedEvolution())
        {
            sSummary->parents[sSummary->depth] = sSummary->viewSpecies;
            sSummary->parentCursors[sSummary->depth++] = sSummary->evoCursor;
            sSummary->viewSpecies = sSummary->evolution.target;
            sSummary->evoCursor = sSummary->evoScroll = 0;
        }
        else if (JOY_NEW(A_BUTTON) && count && SelectedEvolution())
        {
            if (EditableParty() && sSummary->viewSpecies == GetMonData(&sSummary->mon, MON_DATA_SPECIES))
            {
                UnholyEvolution_Prepare(sSummary->index, sSummary->evolution.entry, &sSummary->plan);
                sSummary->evolution = sSummary->plan.option;
            }
            sSummary->requirementPage = 0;
            sSummary->qol = QOL_REVIEW;
        }
        else return TRUE;
        sSummary->notice = NULL;
        PlaySE(SE_SELECT); Render(); return TRUE;
    }
    if (sSummary->qol == QOL_REVIEW && JOY_NEW(L_BUTTON | R_BUTTON))
    {
        u16 pages = (UnholyEvolution_RequirementCount(&sSummary->evolution) + 2) / 3;
        sSummary->requirementPage = (sSummary->requirementPage + pages + (JOY_NEW(L_BUTTON) ? -1 : 1)) % pages;
        PlaySE(SE_SELECT); Render();
    }
    else if (JOY_NEW(A_BUTTON) && EditableParty() && sSummary->evolution.status == UNHOLY_EVO_READY
          && sSummary->viewSpecies == GetMonData(&sSummary->mon, MON_DATA_SPECIES))
    {
        if (sSummary->qol == QOL_REVIEW)
        { sSummary->qol = QOL_CONFIRM; PlaySE(SE_SELECT); Render(); return TRUE; }
        u8 status = UnholyEvolution_Consume(&sSummary->plan);
        if (status != UNHOLY_EVO_READY)
        { sSummary->evolution.status = status; sSummary->qol = QOL_REVIEW; PlaySE(SE_FAILURE); Render(); return TRUE; }
        u16 target = sSummary->plan.option.target;
        bool32 canStop = sSummary->plan.option.costCount == 0;
        sSummary->previousEvoCallback = gCB2_AfterEvolution;
        Cleanup(); DestroyTask(taskId);
        // BeginEvolutionScene fades the current palette before installing its
        // own VBlank handler. Keep palette transfers running during that fade.
        SetVBlankCallback(VBlank);
        gCB2_AfterEvolution = ReturnFromEvolution;
        BeginEvolutionScene(&gParties[B_TRAINER_PLAYER][sSummary->index], target, canStop, sSummary->index);
    }
    return TRUE;
}

static void Main(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Init(void)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, &sBg, 1);
    ResetAllBgsCoordinates();
    memset(sSummary->tilemap, 0, sizeof(sSummary->tilemap));
    SetBgTilemapBuffer(0, sSummary->tilemap);
    ResetSpriteData();
    FreeAllSpritePalettes();
    // The portrait helper draws directly into OBJ bank 0.
    gReservedSpritePaletteCount = 1;
    ResetTasks();
    ResetPaletteFade();
    if (!InitWindows(sWindows))
    {
        void (*callback)(void) = sSummary->callback;
        gReservedSpritePaletteCount = 0;
        FreeAllWindowBuffers();
        UnsetBgTilemapBuffer(0);
        Free(sSummary);
        sSummary = NULL;
        SetMainCallback2(callback);
        return;
    }
    DeactivateAllTextPrinters();
    sSummary->window = 0;
    sSummary->portrait = SPRITE_NONE;
    for (u8 i = 0; i < ARRAY_COUNT(sSummary->mutationIcons); i++)
        sSummary->mutationIcons[i] = MAX_SPRITES;
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    LoadPalette(GetUnholyUiPalette() + 1, BG_PLTT_ID(0), PLTT_SIZEOF(1));
    ReadMon();
    Render();
    CopyBgTilemapBufferToVram(0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    CreateTask(Input, 8);
    SetVBlankCallback(VBlank);
    SetMainCallback2(Main);
}

bool32 UnholySummary_TryOpen(u8 mode, void *mons, u8 index, u8 last, void (*callback)(void))
{
    if (sSummary != NULL || gMain.inBattle || MenuHelpers_IsLinkActive() || index == PC_MON_CHOSEN || mons == NULL
     || index > last || (mode != SUMMARY_MODE_NORMAL && mode != SUMMARY_MODE_LOCK_MOVES
                     && mode != SUMMARY_MODE_BOX && mode != SUMMARY_MODE_BOX_CURSOR))
        return FALSE;
    sSummary = AllocZeroed(sizeof(*sSummary));
    if (sSummary == NULL)
        return FALSE;
    sSummary->mons = mons;
    sSummary->callback = callback;
    sSummary->mode = mode;
    sSummary->index = index;
    sSummary->last = last;
    sSummary->portrait = SPRITE_NONE;
    SetMainCallback2(Init);
    return TRUE;
}
