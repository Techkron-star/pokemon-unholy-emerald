#include "global.h"
#include "mutation_icons.h"
#include "unholy_summary_ui.h"
#include "unholy_mon_summary.h"
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
    const u8 *help = sSummary->page == PAGE_STATS ? COMPOUND_STRING("<>:TAB L/R:IV/EV B:BACK")
                   : sSummary->page == PAGE_GEAR ? COMPOUND_STRING("<>:TAB L/R:DETAIL B:BACK")
                   : sSummary->page == PAGE_MOVES ? COMPOUND_STRING("<>:TAB UP/DN:MOVE A:DETAIL B:BACK")
                   : COMPOUND_STRING("<>:TAB UP/DN:PKMN A:DETAIL B:BACK");
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
