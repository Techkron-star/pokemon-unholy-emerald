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
    .height = 17,
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

static void PrintText(const u8 *text, u8 x, u8 y)
{
    UnholyUi_Print(sSimulator.window, (y >= 32 && y <= 112) || y >= 120 ? FONT_SMALL_NARROW : FONT_SMALL, x, y, 200 - min(x, 200), text);
}

static void PrintNumber(u32 number, u8 x, u8 y, u8 digits)
{
    u8 text[12];
    ConvertUIntToDecimalStringN(text, number, STR_CONV_MODE_LEFT_ALIGN, digits);
    UnholyUi_Print(sSimulator.window, FONT_SMALL_NARROW, x, y, 200 - min(x, 200), text);
}

static void PrintMoney(const u8 *label, u32 amount, u8 x, u8 y)
{
    u8 text[28];
    u8 *end = StringCopy(text, label);
    ConvertUIntToDecimalStringN(end, amount, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
    UnholyUi_Print(sSimulator.window, FONT_SMALL_NARROW, x, y, 200 - min(x, 200), text);
}

static void RenderParty(void)
{
    PrintText(COMPOUND_STRING("Choose a party Pokémon."), 8, 23);
    if (sSimulator.partyCount == 0)
        PrintText(COMPOUND_STRING("Your party is empty."), 8, 42);
    for (u8 i = 0; i < sSimulator.partyCount; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sSimulator.partySlots[i]];
        const u8 *reason = GetLockReason(mon);
        u8 y = 37 + 11 * i;
        u8 name[POKEMON_NAME_LENGTH + 1];
        if (i == sSimulator.partyIndex)
            PrintText(COMPOUND_STRING(">"), 8, y);
        GetMonData(mon, MON_DATA_NICKNAME, name);
        PrintText(name, 20, y);
        if (reason != NULL)
            PrintText(reason, 107, y);
    }
    PrintText(sSimulator.notice != NULL ? sSimulator.notice : COMPOUND_STRING("IV point ¥300     EV point ¥20"), 8, 108);
    PrintText(COMPOUND_STRING("A  Edit      B  Back"), 8, 124);
}

static void RenderSpread(void)
{
    struct Pokemon *mon = SelectedMon();
    struct Pokemon preview = *mon;
    u8 name[POKEMON_NAME_LENGTH + 1];
    u16 total = TotalEvs(&sSimulator.draft);
    u8 evText[18];
    u8 *evCursor = StringCopy(evText, COMPOUND_STRING("EV "));
    evCursor = ConvertUIntToDecimalStringN(evCursor, total, STR_CONV_MODE_LEFT_ALIGN, 4);
    evCursor = StringCopy(evCursor, COMPOUND_STRING("/"));
    ConvertUIntToDecimalStringN(evCursor, MAX_TOTAL_EVS, STR_CONV_MODE_LEFT_ALIGN, 3);
    ApplyValues(&preview, &sSimulator.draft);

    GetMonData(mon, MON_DATA_NICKNAME, name);
    PrintText(name, 8, 24);
    PrintText(sSimulator.editingEvs ? COMPOUND_STRING("IV") : COMPOUND_STRING(">IV"), 80, 24);
    PrintText(sSimulator.editingEvs ? COMPOUND_STRING(">EV") : COMPOUND_STRING("EV"), 108, 24);
    UnholyUi_Print(sSimulator.window, FONT_SMALL_NARROW, 138, 26, 62, COMPOUND_STRING("OLD > NEW"));

    for (u8 row = 0; row < NUM_STATS; row++)
    {
        enum Stat stat = sStatOrder[row];
        u8 y = 39 + 10 * row;
        if (row == sSimulator.row && sSimulator.mode == SIMULATOR_EDIT)
            PrintText(COMPOUND_STRING(">"), 8, y);
        PrintText(sStatNames[stat], 20, y);
        PrintNumber(sSimulator.draft.ivs[stat], 81, y, 2);
        PrintNumber(sSimulator.draft.evs[stat], 109, y, 3);
        PrintNumber(GetMonData(mon, MON_DATA_MAX_HP + stat), 138, y, 5);
        PrintText(COMPOUND_STRING(">"), 166, y);
        PrintNumber(GetMonData(&preview, MON_DATA_MAX_HP + stat), 174, y, 5);
    }

    PrintText(evText, 8, 98);
    PrintMoney(COMPOUND_STRING("Cost ¥"), CalculateCost(mon, &sSimulator.draft), 73, 98);
    PrintMoney(COMPOUND_STRING("Have ¥"), GetMoney(&gSaveBlock1Ptr->money), 135, 98);
    if (sSimulator.notice != NULL)
        PrintText(sSimulator.notice, 8, 108);
    else if (sSimulator.mode == SIMULATOR_REVIEW)
        PrintText(COMPOUND_STRING("Apply this spread and pay?"), 8, 108);
    else
        PrintText(COMPOUND_STRING("Left/Right 1   L/R 10   SELECT IV/EV"), 8, 108);

    if (sSimulator.mode == SIMULATOR_REVIEW)
        PrintText(COMPOUND_STRING("A  Pay      B  Keep editing"), 8, 124);
    else
        PrintText(COMPOUND_STRING("A  Review      B  Discard draft"), 8, 124);
}

static void Render(void)
{
    UnholyUi_Frame(sSimulator.window, COMPOUND_STRING("POKé SIMULATOR"));
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
            sSimulator.notice = COMPOUND_STRING("Reduce EVs to the legal limit.");
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
                ? COMPOUND_STRING("Not enough money.") : COMPOUND_STRING("This spread cannot be applied.");
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
