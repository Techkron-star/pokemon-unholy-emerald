#include "global.h"
#include "unholy_ui.h"
#include "battle.h"
#include "hfc_simulator.h"
#include "menu.h"
#include "money.h"
#include "palette.h"
#include "pokemon.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

#define SIMULATOR_IV_COST 300
#define SIMULATOR_EV_COST 20

enum SimulatorMode
{
    SIMULATOR_PARTY,
    SIMULATOR_EDIT,
    SIMULATOR_REVIEW,
};

static EWRAM_DATA struct
{
    struct HfcSimulatorValues draft;
    void (*returnToWatch)(void);
    const u8 *notice;
    u8 partySlots[PARTY_SIZE];
    u8 partyCount;
    u8 partyIndex;
    u8 row;
    u8 window;
    bool8 editingEvs;
    enum SimulatorMode mode;
} sSimulator;

static EWRAM_DATA struct UnholyUiPalette sUiPalette;

static const struct WindowTemplate sSimulatorWindow =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 26,
    .height = 18,
    .paletteNum = 14,
    .baseBlock = 0x20,
};

static const enum Stat sStatOrder[NUM_STATS] =
{
    STAT_HP, STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED,
};

static const u8 *const sStatNames[NUM_STATS] =
{
    [STAT_HP] = COMPOUND_STRING("HP"),
    [STAT_ATK] = COMPOUND_STRING("Attack"),
    [STAT_DEF] = COMPOUND_STRING("Defense"),
    [STAT_SPEED] = COMPOUND_STRING("Speed"),
    [STAT_SPATK] = COMPOUND_STRING("Sp. Atk"),
    [STAT_SPDEF] = COMPOUND_STRING("Sp. Def"),
};

static const u8 *GetLockReason(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    if (species == SPECIES_EGG)
        return COMPOUND_STRING("Egg");
    if (species == SPECIES_NONE || species >= NUM_SPECIES || GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG))
        return COMPOUND_STRING("Unavailable");
    if (GetMonData(mon, MON_DATA_SPECIAL_BADGES) & (POKEMON_BADGE_ASCENDED | POKEMON_BADGE_OMEGA))
        return COMPOUND_STRING("Locked genetics");
    return NULL;
}

static u16 TotalEvs(const struct HfcSimulatorValues *values)
{
    u16 total = 0;
    for (u8 stat = 0; stat < NUM_STATS; stat++)
        total += values->evs[stat];
    return total;
}

static bool32 IsLegalSpread(const struct HfcSimulatorValues *values)
{
    if (TotalEvs(values) > MAX_TOTAL_EVS)
        return FALSE;
    for (u8 stat = 0; stat < NUM_STATS; stat++)
    {
        if (values->ivs[stat] > MAX_PER_STAT_IVS || values->evs[stat] > MAX_PER_STAT_EVS)
            return FALSE;
    }
    return TRUE;
}

void HfcSimulator_ReadValues(struct Pokemon *mon, struct HfcSimulatorValues *values)
{
    for (u8 stat = 0; stat < NUM_STATS; stat++)
    {
        values->ivs[stat] = GetMonData(mon, MON_DATA_HP_IV + stat);
        values->evs[stat] = GetMonData(mon, MON_DATA_HP_EV + stat);
    }
}

static u32 CalculateCost(struct Pokemon *mon, const struct HfcSimulatorValues *values)
{
    struct HfcSimulatorValues original;
    u32 cost = 0;
    HfcSimulator_ReadValues(mon, &original);
    for (u8 stat = 0; stat < NUM_STATS; stat++)
    {
        cost += abs(values->ivs[stat] - original.ivs[stat]) * SIMULATOR_IV_COST;
        cost += abs(values->evs[stat] - original.evs[stat]) * SIMULATOR_EV_COST;
    }
    return cost;
}

static void ApplyValues(struct Pokemon *mon, const struct HfcSimulatorValues *values)
{
    u8 savedLevelUpHp = gBattleScripting.levelUpHP;
    for (u8 stat = 0; stat < NUM_STATS; stat++)
    {
        SetMonData(mon, MON_DATA_HP_IV + stat, &values->ivs[stat]);
        SetMonData(mon, MON_DATA_HP_EV + stat, &values->evs[stat]);
    }
    CalculateMonStats(mon);
    gBattleScripting.levelUpHP = savedLevelUpHp;
}

enum HfcSimulatorResult HfcSimulator_Apply(struct Pokemon *mon, const struct HfcSimulatorValues *values, u32 *money)
{
    if (GetLockReason(mon) != NULL)
        return HFC_SIMULATOR_LOCKED;
    if (!IsLegalSpread(values))
        return HFC_SIMULATOR_INVALID_SPREAD;

    u32 cost = CalculateCost(mon, values);
    if (cost == 0)
        return HFC_SIMULATOR_NO_CHANGES;
    if (!IsEnoughMoney(money, cost))
        return HFC_SIMULATOR_INSUFFICIENT_FUNDS;

    ApplyValues(mon, values);
    RemoveMoney(money, cost);
    return HFC_SIMULATOR_APPLIED;
}

static struct Pokemon *SelectedMon(void)
{
    return &gParties[B_TRAINER_PLAYER][sSimulator.partySlots[sSimulator.partyIndex]];
}

static void PrintMoney(const u8 *label, u32 amount, u8 x)
{
    u8 text[28];
    u8 *end = StringCopy(text, label);
    ConvertUIntToDecimalStringN(end, amount, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
    u8 font = GetFontIdToFit(text, FONT_SMALL_NARROW, 0, 62);
    UnholyUi_Print(sSimulator.window, font, x, 104, 62, text);
}

static void RenderParty(void)
{
    u8 window = sSimulator.window;
    const u8 *reason = NULL;
    if (sSimulator.partyCount == 0)
        UnholyUi_Print(window, FONT_SMALL, 8, 25, 192, COMPOUND_STRING("Your party is empty."));

    for (u8 i = 0; i < sSimulator.partyCount; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sSimulator.partySlots[i]];
        const u8 *lock = GetLockReason(mon);
        bool8 selected = i == sSimulator.partyIndex;
        u8 name[POKEMON_NAME_LENGTH + 1];
        GetMonData(mon, MON_DATA_NICKNAME, name);
        UnholyUi_ChoiceRow(window, 23 + 13 * i, name, lock, selected);
        if (selected)
            reason = lock;
    }
    const u8 *notice = sSimulator.notice != NULL ? sSimulator.notice : COMPOUND_STRING("IV point ¥300    EV point ¥20");
    UnholyUi_Print(window, FONT_SMALL_NARROW, 8, 105, 192, notice);
    UnholyUi_Footer(window, sSimulator.partyCount != 0 && reason == NULL
        ? COMPOUND_STRING("A:EDIT  B:BACK") : COMPOUND_STRING("B:BACK"));
}

static void RenderStatRow(struct Pokemon *mon, struct Pokemon *preview, u8 row)
{
    u8 window = sSimulator.window;
    enum Stat stat = sStatOrder[row];
    bool8 selected = row == sSimulator.row && sSimulator.mode == SIMULATOR_EDIT;
    u8 fill = UnholyUi_CardFill(selected);
    u8 y = 32 + 12 * row;
    const u8 columns[] = {70, 99, 132, 174};
    const u32 values[] = {
        sSimulator.draft.ivs[stat], sSimulator.draft.evs[stat],
        GetMonData(mon, MON_DATA_MAX_HP + stat), GetMonData(preview, MON_DATA_MAX_HP + stat),
    };

    FillWindowPixelRect(window, PIXEL_FILL(fill), 6, y, 196, 12);
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 10, y, 52, fill, sStatNames[stat]);
    for (u8 i = 0; i < ARRAY_COUNT(values); i++)
    {
        u8 text[8];
        ConvertUIntToDecimalStringN(text, values[i], STR_CONV_MODE_LEFT_ALIGN, 5);
        UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, columns[i], y, 25, fill, text);
    }
}

static void RenderSpread(void)
{
    u8 window = sSimulator.window;
    bool8 reviewing = sSimulator.mode == SIMULATOR_REVIEW;
    struct Pokemon *mon = SelectedMon();
    struct Pokemon preview = *mon;
    u8 name[POKEMON_NAME_LENGTH + 1];
    u8 evText[18];
    u8 *evCursor = StringCopy(evText, COMPOUND_STRING("EV "));
    evCursor = ConvertUIntToDecimalStringN(evCursor, TotalEvs(&sSimulator.draft), STR_CONV_MODE_LEFT_ALIGN, 4);
    evCursor = StringCopy(evCursor, COMPOUND_STRING("/"));
    ConvertUIntToDecimalStringN(evCursor, MAX_TOTAL_EVS, STR_CONV_MODE_LEFT_ALIGN, 3);
    ApplyValues(&preview, &sSimulator.draft);

    GetMonData(mon, MON_DATA_NICKNAME, name);
    UnholyUi_Print(window, FONT_SMALL_NARROW, 112, 5, 88, name);
    u8 activeX = sSimulator.editingEvs ? 94 : 65;
    if (!reviewing)
        UnholyUi_Card(window, activeX, 20, 23, 12, TRUE);
    UnholyUi_Print(window, FONT_SMALL_NARROW, 10, 20, 52, COMPOUND_STRING("STAT"));
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 70, 20, 18,
        UnholyUi_CardFill(!reviewing && !sSimulator.editingEvs), COMPOUND_STRING("IV"));
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 99, 20, 18,
        UnholyUi_CardFill(!reviewing && sSimulator.editingEvs), COMPOUND_STRING("EV"));
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 124, 20, 39, COMPOUND_STRING("CURRENT"));
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 165, 20, 37, COMPOUND_STRING("PREVIEW"));

    for (u8 row = 0; row < NUM_STATS; row++)
        RenderStatRow(mon, &preview, row);

    UnholyUi_Print(window, FONT_SMALL_NARROW, 8, 104, 62, evText);
    PrintMoney(COMPOUND_STRING("Cost ¥"), CalculateCost(mon, &sSimulator.draft), 72);
    PrintMoney(COMPOUND_STRING("Have ¥"), GetMoney(&gSaveBlock1Ptr->money), 138);

    const u8 *notice = reviewing ? COMPOUND_STRING("Pay to apply these changes?")
                                : COMPOUND_STRING("LEFT/RIGHT:1  L/R:10  SEL:IV/EV");
    UnholyUi_Print(window, FONT_SMALL_NARROW, 8, 113, 192, sSimulator.notice != NULL ? sSimulator.notice : notice);
    UnholyUi_Footer(window, reviewing ? COMPOUND_STRING("A:PAY  B:KEEP EDITING")
                                    : COMPOUND_STRING("A:REVIEW  B:DISCARD"));
}

static void Render(void)
{
    UnholyUi_Frame(sSimulator.window, sSimulator.mode == SIMULATOR_REVIEW
        ? COMPOUND_STRING("REVIEW PAYMENT") : COMPOUND_STRING("POKé SIMULATOR"));
    if (sSimulator.mode == SIMULATOR_PARTY)
        RenderParty();
    else
        RenderSpread();
    CopyWindowToVram(sSimulator.window, COPYWIN_FULL);
}

static void Close(u8 taskId)
{
    void (*returnToWatch)(void) = sSimulator.returnToWatch;
    ClearStdWindowAndFrameToTransparent(sSimulator.window, TRUE);
    RemoveWindow(sSimulator.window);
    UnholyUi_ReleasePalette(&sUiPalette);
    DestroyTask(taskId);
    returnToWatch();
}

static bool32 HandlePartyInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Close(taskId);
        return FALSE;
    }
    if (sSimulator.partyCount == 0)
        return FALSE;

    if (JOY_REPEAT(DPAD_UP))
        sSimulator.partyIndex = (sSimulator.partyIndex + sSimulator.partyCount - 1) % sSimulator.partyCount;
    else if (JOY_REPEAT(DPAD_DOWN))
        sSimulator.partyIndex = (sSimulator.partyIndex + 1) % sSimulator.partyCount;
    else if (JOY_NEW(A_BUTTON))
    {
        const u8 *reason = GetLockReason(SelectedMon());
        if (reason != NULL)
        {
            sSimulator.notice = reason;
            PlaySE(SE_FAILURE);
            return TRUE;
        }

        HfcSimulator_ReadValues(SelectedMon(), &sSimulator.draft);
        sSimulator.editingEvs = FALSE;
        sSimulator.row = 0;
        sSimulator.mode = SIMULATOR_EDIT;
    }
    else
        return FALSE;

    sSimulator.notice = NULL;
    PlaySE(SE_SELECT);
    return TRUE;
}

static void AdjustDraft(s8 delta)
{
    enum Stat stat = sStatOrder[sSimulator.row];
    u8 *value = sSimulator.editingEvs ? &sSimulator.draft.evs[stat] : &sSimulator.draft.ivs[stat];
    s32 limit = MAX_PER_STAT_IVS;

    if (sSimulator.editingEvs)
        limit = min(MAX_PER_STAT_EVS, MAX_TOTAL_EVS - (s32)TotalEvs(&sSimulator.draft) + *value);
    if (delta > 0)
        *value = min(*value + delta, max(*value, limit));
    else
        *value = max(*value + delta, 0);
}

static bool32 HandleEditInput(void)
{
    if (!JOY_NEW(A_BUTTON | B_BUTTON | SELECT_BUTTON)
     && !JOY_REPEAT(DPAD_ANY | L_BUTTON | R_BUTTON))
        return FALSE;

    sSimulator.notice = NULL;
    if (JOY_NEW(B_BUTTON))
        sSimulator.mode = SIMULATOR_PARTY;
    else if (JOY_NEW(A_BUTTON))
    {
        if (CalculateCost(SelectedMon(), &sSimulator.draft) == 0)
            sSimulator.notice = COMPOUND_STRING("Change a value first.");
        else if (!IsLegalSpread(&sSimulator.draft))
            sSimulator.notice = COMPOUND_STRING("Too many EVs. Lower the total.");
        else
            sSimulator.mode = SIMULATOR_REVIEW;
    }
    else if (JOY_NEW(SELECT_BUTTON))
        sSimulator.editingEvs = !sSimulator.editingEvs;
    else if (JOY_REPEAT(DPAD_UP))
        sSimulator.row = (sSimulator.row + NUM_STATS - 1) % NUM_STATS;
    else if (JOY_REPEAT(DPAD_DOWN))
        sSimulator.row = (sSimulator.row + 1) % NUM_STATS;
    else
    {
        s8 step = JOY_REPEAT(L_BUTTON | R_BUTTON) ? 10 : 1;
        AdjustDraft(JOY_REPEAT(DPAD_LEFT | L_BUTTON) ? -step : step);
    }
    PlaySE(sSimulator.notice == NULL ? SE_SELECT : SE_FAILURE);
    return TRUE;
}

static bool32 HandleReviewInput(void)
{
    if (JOY_NEW(B_BUTTON))
    {
        sSimulator.notice = NULL;
        sSimulator.mode = SIMULATOR_EDIT;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        enum HfcSimulatorResult result = HfcSimulator_Apply(SelectedMon(), &sSimulator.draft, &gSaveBlock1Ptr->money);
        if (result == HFC_SIMULATOR_APPLIED)
        {
            sSimulator.mode = SIMULATOR_PARTY;
            sSimulator.notice = COMPOUND_STRING("Changes applied.");
            PlaySE(SE_SHOP);
        }
        else
        {
            sSimulator.notice = result == HFC_SIMULATOR_INSUFFICIENT_FUNDS
                ? COMPOUND_STRING("Not enough money.") : COMPOUND_STRING("Could not apply changes.");
            PlaySE(SE_FAILURE);
        }
    }
    else
        return FALSE;

    return TRUE;
}

static void Task_Simulator(u8 taskId)
{
    bool32 redraw;

    if (gPaletteFade.active)
        return;
    if (sSimulator.mode == SIMULATOR_PARTY)
        redraw = HandlePartyInput(taskId);
    else if (sSimulator.mode == SIMULATOR_EDIT)
        redraw = HandleEditInput();
    else
        redraw = HandleReviewInput();

    if (redraw)
        Render();
}

void HfcSimulator_Open(void (*returnToWatch)(void))
{
    u8 taskId = CreateTask(Task_Simulator, 8);
    if (gTasks[taskId].func != Task_Simulator)
    {
        returnToWatch();
        return;
    }

    memset(&sSimulator, 0, sizeof(sSimulator));
    sSimulator.window = AddWindow(&sSimulatorWindow);
    if (sSimulator.window == WINDOW_NONE)
    {
        DestroyTask(taskId);
        returnToWatch();
        return;
    }

    sSimulator.returnToWatch = returnToWatch;
    for (u8 slot = 0; slot < PARTY_SIZE; slot++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SANITY_HAS_SPECIES))
            sSimulator.partySlots[sSimulator.partyCount++] = slot;
    }

    LoadMessageBoxAndBorderGfx();
    UnholyUi_AcquirePalette(&sUiPalette, 14);
    PutWindowTilemap(sSimulator.window);
    Render();
}

void HfcSimulator_OpenForParty(u8 partySlot, void (*onReturn)(void))
{
    HfcSimulator_Open(onReturn);
    if (FindTaskIdByFunc(Task_Simulator) == TASK_NONE) return;
    for (u8 i = 0; i < sSimulator.partyCount; i++)
        if (sSimulator.partySlots[i] == partySlot)
        {
            sSimulator.partyIndex = i;
            sSimulator.notice = GetLockReason(SelectedMon());
            if (sSimulator.notice == NULL)
            {
                HfcSimulator_ReadValues(SelectedMon(), &sSimulator.draft);
                sSimulator.mode = SIMULATOR_EDIT;
            }
            Render();
            return;
        }
}
