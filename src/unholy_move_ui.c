#include "global.h"
#include "unholy_move_ui.h"
#include "unholy_moves.h"
#include "unholy_ui.h"
#include "battle.h"
#include "bg.h"
#include "gpu_regs.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "move.h"
#include "palette.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum { MOVE_LIST, MOVE_DETAIL, MOVE_SLOT, MOVE_CONFIRM, MOVE_ORDER, MOVE_DELETE };
#define VISIBLE_MOVES 5

static EWRAM_DATA struct MoveUi
{
    struct Pokemon mon;
    struct UnholyMoveCatalog catalog;
    struct UnholyMovePlan plan;
    void (*onReturn)(void);
    const u8 *notice;
    u16 tilemap[32 * 32];
    u16 cursor, move;
    u8 partySlot, mode, category, slot, marked;
    bool8 editable;
} *sMoves;

static const u8 sSources[] = {UNHOLY_MOVE_ALL, UNHOLY_MOVE_LEVEL, UNHOLY_MOVE_PRE_EVO, UNHOLY_MOVE_EGG, UNHOLY_MOVE_MACHINE, UNHOLY_MOVE_TUTOR};
static const struct BgTemplate sBg = {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .priority = 0};
static const struct WindowTemplate sWindows[] = {
    {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 18, .paletteNum = 14, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};

static void Print(u16 x, u16 y, u16 width, const u8 *text)
{
    UnholyUi_Print(0, FONT_SMALL_NARROW, x, y, width, text);
}

static void Footer(const u8 *first, const u8 *second)
{
    FillWindowPixelRect(0, PIXEL_FILL(1), 5, 115, 214, 1);
    Print(7, 118, 210, first);
    Print(7, 130, 210, second);
}

static struct Pokemon *ActualMon(void)
{
    return sMoves->editable ? &gParties[B_TRAINER_PLAYER][sMoves->partySlot] : &sMoves->mon;
}

static void Refresh(void)
{
    if (sMoves->editable) sMoves->mon = *ActualMon();
    UnholyMoves_Build(&sMoves->mon, &sMoves->catalog);
}

static const u8 *Status(u8 status)
{
    switch (status)
    {
    case UNHOLY_MOVE_READY: return COMPOUND_STRING("READY");
    case UNHOLY_MOVE_KNOWN: return COMPOUND_STRING("KNOWN");
    case UNHOLY_MOVE_NEEDS_LEVEL: return COMPOUND_STRING("LEVEL");
    case UNHOLY_MOVE_NEEDS_DISC: return COMPOUND_STRING("NEED DISC");
    case UNHOLY_MOVE_CANNOT_FORGET: return COMPOUND_STRING("FIELD MOVE: USE THE MOVE DELETER");
    case UNHOLY_MOVE_CHANGED: return COMPOUND_STRING("POKéMON CHANGED - REVIEW AGAIN");
    default: return COMPOUND_STRING("UNAVAILABLE");
    }
}

static void CostText(u8 *text)
{
    u16 item;
    u8 state = UnholyMoves_Status(&sMoves->mon, &sMoves->catalog, sMoves->move, &item);
    if (state == UNHOLY_MOVE_KNOWN) { StringCopy(text, COMPOUND_STRING("ALREADY KNOWN")); return; }
    u8 *end;
    if (item != ITEM_NONE)
    {
        end = StringCopy(text, GetItemName(item));
        end = StringAppend(end, COMPOUND_STRING("  HAVE "));
        end = ConvertUIntToDecimalStringN(end, CountTotalItemQuantityInBag(item), STR_CONV_MODE_LEFT_ALIGN, 5);
        StringAppend(end, (I_REUSABLE_TMS || GetItemImportance(item)) ? COMPOUND_STRING(" / NEED 1 - KEEP") : COMPOUND_STRING(" / USE 1"));
    }
    else if (state == UNHOLY_MOVE_NEEDS_LEVEL)
    {
        end = StringCopy(text, COMPOUND_STRING("LEVEL "));
        end = ConvertUIntToDecimalStringN(end, GetMonData(&sMoves->mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
        end = StringAppend(end, COMPOUND_STRING(" / NEED "));
        ConvertUIntToDecimalStringN(end, sMoves->catalog.level[sMoves->move], STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    else StringCopy(text, COMPOUND_STRING("COMPATIBLE ROUTE - NO ITEM COST"));
}

static void Description(enum Move move)
{
    u8 text[256];
    u8 *end = StringCopyN(text, GetMoveDescription(move), sizeof(text) - 1);
    *end = EOS;
    WrapFontIdToFit(text, end, FONT_SMALL_NARROW, 204);
    const u8 *line = text;
    for (u8 row = 0; row < 2 && *line != EOS; row++)
    {
        Print(8, 77 + row * 12, 204, line);
        while (*line != EOS && *line != CHAR_NEWLINE) line++;
        if (*line == CHAR_NEWLINE) line++;
    }
}

static void Render(void)
{
    u8 text[160];
    UnholyUi_Frame(0, COMPOUND_STRING("MOVE MANAGER"));
    Print(113, 6, 103, GetSpeciesName(GetMonData(&sMoves->mon, MON_DATA_SPECIES)));
    if (sMoves->mode == MOVE_LIST)
    {
        u8 source = sSources[sMoves->category];
        u16 count = UnholyMoves_Count(&sMoves->catalog, source);
        Print(8, 24, 152, UnholyMoves_SourceName(source));
        u8 *end = ConvertUIntToDecimalStringN(text, count ? sMoves->cursor + 1 : 0, STR_CONV_MODE_LEFT_ALIGN, 3);
        end = StringAppend(end, COMPOUND_STRING("/"));
        ConvertUIntToDecimalStringN(end, count, STR_CONV_MODE_LEFT_ALIGN, 3);
        Print(164, 24, 50, text);
        u16 first = sMoves->cursor / VISIBLE_MOVES * VISIBLE_MOVES;
        for (u8 i = 0; i < VISIBLE_MOVES && first + i < count; i++)
        {
            enum Move move = UnholyMoves_At(&sMoves->catalog, source, first + i);
            u16 item;
            u8 state = UnholyMoves_Status(&sMoves->mon, &sMoves->catalog, move, &item);
            bool8 selected = first + i == sMoves->cursor;
            u8 y = 40 + i * 14, fill = UnholyUi_CardFill(selected);
            UnholyUi_Card(0, 6, y, 212, 14, selected);
            UnholyUi_PrintOnFill(0, FONT_SMALL_NARROW, 11, y + 1, 141, fill, GetMoveName(move));
            UnholyUi_PrintOnFill(0, FONT_SMALL_NARROW, 156, y + 1, 57, fill, Status(state));
        }
        if (!count) Print(12, 64, 200, COMPOUND_STRING("NO MOVES IN THIS CATEGORY"));
        Footer(sMoves->notice != NULL ? sMoves->notice : COMPOUND_STRING("LEFT/RIGHT OR L/R: SOURCE A:DETAIL"),
            sMoves->editable ? COMPOUND_STRING("SELECT:MOVE SLOTS B:BACK") : COMPOUND_STRING("READ ONLY - B:BACK"));
    }
    else if (sMoves->mode == MOVE_DETAIL)
    {
        Print(8, 24, 207, GetMoveName(sMoves->move));
        u8 *end = StringCopy(text, gTypesInfo[GetMoveType(sMoves->move)].name);
        end = StringAppend(end, COMPOUND_STRING(" / "));
        StringAppend(end, GetMoveCategory(sMoves->move) == DAMAGE_CATEGORY_PHYSICAL ? COMPOUND_STRING("PHYSICAL")
                        : GetMoveCategory(sMoves->move) == DAMAGE_CATEGORY_SPECIAL ? COMPOUND_STRING("SPECIAL") : COMPOUND_STRING("STATUS"));
        Print(8, 38, 207, text);
        end = StringCopy(text, COMPOUND_STRING("PWR "));
        if (GetMovePower(sMoves->move) > 1) end = ConvertUIntToDecimalStringN(end, GetMovePower(sMoves->move), STR_CONV_MODE_LEFT_ALIGN, 3);
        else end = StringAppend(end, COMPOUND_STRING("--"));
        end = StringAppend(end, COMPOUND_STRING("  ACC "));
        if (GetMoveAccuracy(sMoves->move)) end = ConvertUIntToDecimalStringN(end, GetMoveAccuracy(sMoves->move), STR_CONV_MODE_LEFT_ALIGN, 3);
        else end = StringAppend(end, COMPOUND_STRING("--"));
        end = StringAppend(end, COMPOUND_STRING("  PP "));
        ConvertUIntToDecimalStringN(end, GetMovePP(sMoves->move), STR_CONV_MODE_LEFT_ALIGN, 3);
        Print(8, 51, 207, text);
        StringCopy(text, COMPOUND_STRING(""));
        for (u8 i = 1; i < ARRAY_COUNT(sSources); i++)
            if (sMoves->catalog.sources[sMoves->move] & sSources[i])
            {
                StringAppend(text, i == 4 ? COMPOUND_STRING("TM/HM ") : i == 5 ? COMPOUND_STRING("TUTOR ") : UnholyMoves_SourceName(sSources[i]));
                StringAppend(text, COMPOUND_STRING(" "));
            }
        Print(8, 64, 207, text);
        Description(sMoves->move);
        CostText(text);
        Print(8, 103, 207, text);
        Footer(sMoves->notice != NULL ? sMoves->notice : sMoves->editable ? COMPOUND_STRING("A:CHOOSE A SLOT B:CATALOG") : COMPOUND_STRING("READ ONLY - MOVE TO PARTY TO TEACH"),
            COMPOUND_STRING("ONLY MOVES COMPATIBLE WITH THIS SPECIES"));
    }
    else if (sMoves->mode == MOVE_SLOT || sMoves->mode == MOVE_ORDER)
    {
        bool8 ordering = sMoves->mode == MOVE_ORDER;
        Print(8, 24, 207, ordering ? COMPOUND_STRING("CURRENT MOVE SLOTS") : COMPOUND_STRING("CHOOSE THE SLOT TO REPLACE"));
        for (u8 i = 0; i < MAX_MON_MOVES; i++)
        {
            enum Move move = GetMonData(&sMoves->mon, MON_DATA_MOVE1 + i);
            bool8 selected = i == sMoves->slot;
            u8 y = 43 + i * 15, fill = UnholyUi_CardFill(selected);
            UnholyUi_Card(0, 6, y, 212, 15, selected);
            UnholyUi_PrintOnFill(0, FONT_SMALL_NARROW, 11, y + 1, 139, fill, move == MOVE_NONE ? COMPOUND_STRING("-- EMPTY --") : GetMoveName(move));
            u8 *end = StringCopy(text, sMoves->marked == i ? COMPOUND_STRING("> PP ") : COMPOUND_STRING("PP "));
            ConvertUIntToDecimalStringN(end, GetMonData(&sMoves->mon, MON_DATA_PP1 + i), STR_CONV_MODE_LEFT_ALIGN, 3);
            UnholyUi_PrintOnFill(0, FONT_SMALL_NARROW, 164, y + 1, 47, fill, text);
        }
        Footer(sMoves->notice != NULL ? sMoves->notice : ordering ? COMPOUND_STRING("A:MARK / SWAP START:FORGET B:BACK") : COMPOUND_STRING("A:REVIEW REPLACEMENT B:MOVE DETAILS"),
            ordering ? COMPOUND_STRING("PP AND PP UPS MOVE WITH THEIR SLOT") : GetMoveName(sMoves->move));
    }
    else
    {
        bool8 deleting = sMoves->mode == MOVE_DELETE;
        Print(8, 24, 207, deleting ? COMPOUND_STRING("FORGET THIS MOVE?") : COMPOUND_STRING("CONFIRM MOVE LEARNING"));
        Print(8, 43, 207, GetMoveName(GetMonData(&sMoves->mon, MON_DATA_MOVE1 + sMoves->slot)));
        if (!deleting)
        {
            Print(8, 60, 207, COMPOUND_STRING("BECOMES:"));
            Print(8, 74, 207, GetMoveName(sMoves->move));
            CostText(text);
            Print(8, 95, 207, text);
        }
        else Print(8, 76, 207, COMPOUND_STRING("THE LAST KNOWN MOVE CANNOT BE DELETED"));
        Footer(COMPOUND_STRING("A:CONFIRM B:CANCEL"), deleting ? COMPOUND_STRING("OTHER MOVES KEEP THEIR PP AND PP UPS") : COMPOUND_STRING("COSTS ARE CHECKED AGAIN BEFORE LEARNING"));
    }
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void VBlank(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Close(u8 taskId)
{
    void (*callback)(void) = sMoves->onReturn;
    SetVBlankCallback(NULL);
    FreeAllWindowBuffers();
    UnsetBgTilemapBuffer(0);
    Free(sMoves);
    sMoves = NULL;
    DestroyTask(taskId);
    SetMainCallback2(callback);
}

static void Input(u8 taskId)
{
    if (gPaletteFade.active) return;
    if (JOY_NEW(B_BUTTON))
    {
        if (sMoves->mode == MOVE_LIST) { Close(taskId); return; }
        if (sMoves->mode == MOVE_ORDER && sMoves->marked < MAX_MON_MOVES) sMoves->marked = MAX_MON_MOVES;
        else sMoves->mode = sMoves->mode == MOVE_SLOT ? MOVE_DETAIL : sMoves->mode == MOVE_CONFIRM ? MOVE_SLOT : sMoves->mode == MOVE_DELETE ? MOVE_ORDER : MOVE_LIST;
        sMoves->notice = NULL;
    }
    else if (sMoves->mode == MOVE_LIST)
    {
        u16 count = UnholyMoves_Count(&sMoves->catalog, sSources[sMoves->category]);
        if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
        {
            sMoves->category = (sMoves->category + (JOY_NEW(DPAD_LEFT | L_BUTTON) ? ARRAY_COUNT(sSources) - 1 : 1)) % ARRAY_COUNT(sSources);
            sMoves->cursor = 0;
        }
        else if (count && JOY_REPEAT(DPAD_UP | DPAD_DOWN))
            sMoves->cursor = (sMoves->cursor + count + (JOY_REPEAT(DPAD_UP) ? -1 : 1)) % count;
        else if (count && JOY_NEW(A_BUTTON))
        { sMoves->move = UnholyMoves_At(&sMoves->catalog, sSources[sMoves->category], sMoves->cursor); sMoves->mode = MOVE_DETAIL; }
        else if (JOY_NEW(SELECT_BUTTON) && sMoves->editable)
        { sMoves->mode = MOVE_ORDER; sMoves->slot = 0; sMoves->marked = MAX_MON_MOVES; }
        else return;
        sMoves->notice = NULL;
    }
    else if (sMoves->mode == MOVE_DETAIL && JOY_NEW(A_BUTTON))
    {
        u16 item;
        u8 state = UnholyMoves_Status(ActualMon(), &sMoves->catalog, sMoves->move, &item);
        if (!sMoves->editable) sMoves->notice = COMPOUND_STRING("MOVE TO YOUR PARTY TO TEACH");
        else if (state != UNHOLY_MOVE_READY) sMoves->notice = Status(state);
        else
        {
            sMoves->slot = 0;
            for (u8 i = 0; i < MAX_MON_MOVES; i++)
                if (GetMonData(&sMoves->mon, MON_DATA_MOVE1 + i) == MOVE_NONE) { sMoves->slot = i; break; }
            sMoves->mode = MOVE_SLOT;
            sMoves->notice = NULL;
        }
    }
    else if (sMoves->mode == MOVE_SLOT || sMoves->mode == MOVE_ORDER)
    {
        if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
        { sMoves->slot = (sMoves->slot + (JOY_REPEAT(DPAD_UP) ? MAX_MON_MOVES - 1 : 1)) % MAX_MON_MOVES; sMoves->notice = NULL; }
        else if (JOY_NEW(A_BUTTON))
        {
            if (sMoves->mode == MOVE_ORDER)
            {
                if (sMoves->marked == MAX_MON_MOVES) sMoves->marked = sMoves->slot;
                else
                {
                    UnholyMoves_Swap(sMoves->partySlot, sMoves->marked, sMoves->slot);
                    sMoves->marked = MAX_MON_MOVES;
                    Refresh();
                }
            }
            else
            {
                u8 state = UnholyMoves_Prepare(sMoves->partySlot, sMoves->move, sMoves->slot, &sMoves->plan);
                if (state == UNHOLY_MOVE_READY) { sMoves->mode = MOVE_CONFIRM; sMoves->notice = NULL; }
                else sMoves->notice = Status(state);
            }
        }
        else if (sMoves->mode == MOVE_ORDER && JOY_NEW(START_BUTTON))
        {
            sMoves->plan.before = *ActualMon();
            sMoves->marked = MAX_MON_MOVES;
            sMoves->mode = MOVE_DELETE;
        }
        else return;
    }
    else if (sMoves->mode == MOVE_CONFIRM && JOY_NEW(A_BUTTON))
    {
        u8 state = UnholyMoves_Apply(&sMoves->plan);
        Refresh();
        sMoves->mode = MOVE_LIST;
        sMoves->notice = state == UNHOLY_MOVE_READY ? COMPOUND_STRING("MOVE LEARNED") : Status(state);
        if (state == UNHOLY_MOVE_READY) PlayFanfare(MUS_LEVEL_UP);
    }
    else if (sMoves->mode == MOVE_DELETE && JOY_NEW(A_BUTTON))
    {
        enum Move move = GetMonData(&sMoves->mon, MON_DATA_MOVE1 + sMoves->slot);
        bool32 done = !memcmp(ActualMon(), &sMoves->plan.before, sizeof(struct Pokemon)) && UnholyMoves_Forget(sMoves->partySlot, sMoves->slot, move);
        Refresh();
        sMoves->mode = MOVE_ORDER;
        sMoves->notice = done ? COMPOUND_STRING("MOVE FORGOTTEN") : COMPOUND_STRING("CANNOT DELETE AN EMPTY / LAST MOVE");
    }
    else return;
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
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, &sBg, 1);
    ResetAllBgsCoordinates();
    SetBgTilemapBuffer(0, sMoves->tilemap);
    ResetSpriteData();
    FreeAllSpritePalettes();
    gReservedSpritePaletteCount = 0;
    ResetTasks();
    ResetPaletteFade();
    if (!InitWindows(sWindows))
    {
        void (*callback)(void) = sMoves->onReturn;
        FreeAllWindowBuffers();
        Free(sMoves); sMoves = NULL;
        UnsetBgTilemapBuffer(0);
        SetMainCallback2(callback);
        return;
    }
    DeactivateAllTextPrinters();
    LoadPalette(GetUnholyUiPalette(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    LoadPalette(GetUnholyUiPalette() + 1, BG_PLTT_ID(0), PLTT_SIZEOF(1));
    Render();
    CopyBgTilemapBufferToVram(0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    CreateTask(Input, 8);
    SetVBlankCallback(VBlank);
    SetMainCallback2(Main);
}

void UnholyMoveUi_Open(struct Pokemon *mon, u8 partySlot, void (*onReturn)(void))
{
    if (onReturn == NULL || sMoves != NULL) return;
    if (mon == NULL || gMain.inBattle || GetMonData(mon, MON_DATA_IS_EGG)
     || GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG) || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
    { SetMainCallback2(onReturn); return; }
    sMoves = AllocZeroed(sizeof(*sMoves));
    if (sMoves == NULL) { SetMainCallback2(onReturn); return; }
    sMoves->mon = *mon;
    sMoves->partySlot = partySlot;
    sMoves->onReturn = onReturn;
    sMoves->marked = MAX_MON_MOVES;
    sMoves->editable = !gMain.inBattle && partySlot < gPartiesCount[B_TRAINER_PLAYER] && partySlot < PARTY_SIZE && mon == &gParties[B_TRAINER_PLAYER][partySlot];
    Refresh();
    SetMainCallback2(Init);
}
