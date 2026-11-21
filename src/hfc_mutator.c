#include "global.h"
#include "unholy_ui.h"
#include "battle.h"
#include "event_data.h"
#include "hfc_mutator.h"
#include "hfc_watch_services.h"
#include "horizon_market.h"
#include "menu.h"
#include "money.h"
#include "mutation_tiers.h"
#include "palette.h"
#include "random.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/flags.h"
#include "constants/songs.h"

static const struct
{
    const u8 *name;
    u32 price;
    u8 success;
    u8 lossPercent;
} sPlans[HFC_MUTATOR_PLAN_COUNT] =
{
    [HFC_MUTATOR_BASIC]     = {COMPOUND_STRING("Basic"),     12000, 35,  5},
    [HFC_MUTATOR_ENHANCED]  = {COMPOUND_STRING("Enhanced"),  30000, 60, 10},
    [HFC_MUTATOR_INTENSIVE] = {COMPOUND_STRING("Intensive"), 60000, 85, 15},
};

static const struct
{
    enum PokemonBadge badge;
    u8 weight;
} sMutations[] =
{
    {POKEMON_BADGE_ALPHA,      25},
    {POKEMON_BADGE_HYPER,      20},
    {POKEMON_BADGE_SUPER,      15},
    {POKEMON_BADGE_OVERLOADED, 12},
    {POKEMON_BADGE_ENRAGED,    11},
    {POKEMON_BADGE_ANCESTRAL,   9},
    {POKEMON_BADGE_TITAN,       5},
    {POKEMON_BADGE_ASCENDED,    3},
};

enum MutatorScreen
{
    MUTATOR_PARTY,
    MUTATOR_PLANS,
    MUTATOR_CONFIRM,
    MUTATOR_RESULT,
};

static EWRAM_DATA struct
{
    void (*returnToWatch)(void);
    enum MutatorScreen screen;
    enum HfcMutatorPlan plan;
    enum HfcMutatorResult result;
    enum PokemonBadge mutation;
    u8 slots[PARTY_SIZE];
    u8 count;
    u8 cursor;
    u8 window;
} sMutator;

static EWRAM_DATA struct UnholyUiPalette sUiPalette;

static const struct WindowTemplate sMutatorWindow =
{
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 26,
    .height = 17, .paletteNum = 14, .baseBlock = 0x20,
};

static const u8 *const sMessages[] =
{
    [HFC_MUTATOR_INVALID_MON] = COMPOUND_STRING("This Pokémon is unavailable."),
    [HFC_MUTATOR_EGG] = COMPOUND_STRING("Eggs cannot enter the Mutator."),
    [HFC_MUTATOR_OMEGA] = COMPOUND_STRING("Omega is protected."),
    [HFC_MUTATOR_NEEDS_SUPER_LEAGUE] = COMPOUND_STRING("Rerolls require the Super League."),
    [HFC_MUTATOR_NO_ALTERNATIVE] = COMPOUND_STRING("No new mutations are available."),
    [HFC_MUTATOR_INVALID_PLAN] = COMPOUND_STRING("Choose another treatment."),
    [HFC_MUTATOR_INSUFFICIENT_FUNDS] = COMPOUND_STRING("Not enough money."),
    [HFC_MUTATOR_ROLL_FAILED] = COMPOUND_STRING("The treatment failed."),
};

static enum HfcMutatorResult GetEligibility(struct Pokemon *mon, bool32 canReroll)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
    if (GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG))
        return HFC_MUTATOR_INVALID_MON;
    if (species == SPECIES_EGG)
        return HFC_MUTATOR_EGG;
    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsSpeciesEnabled(species)
     || GetMonData(mon, MON_DATA_LEVEL) == 0)
        return HFC_MUTATOR_INVALID_MON;

    u16 badges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
    if (badges & POKEMON_BADGE_OMEGA)
        return HFC_MUTATOR_OMEGA;
    if (badges != 0 && !canReroll)
        return HFC_MUTATOR_NEEDS_SUPER_LEAGUE;
    if (badges == (POKEMON_BADGES_ALL & ~POKEMON_BADGE_OMEGA))
        return HFC_MUTATOR_NO_ALTERNATIVE;
    return HFC_MUTATOR_READY;
}

static u8 GetResultLevel(u8 level, enum HfcMutatorPlan plan)
{
    u8 loss = (level * sPlans[plan].lossPercent + 99) / 100;
    return level - min(level - 1, min(loss, 15));
}

static enum PokemonBadge GetPrimaryMutation(u16 badges)
{
    enum PokemonBadge primary = 0;
    enum MutationTier highestTier = MUTATION_TIER_NONE;
    for (u8 i = 0; i < ARRAY_COUNT(sMutations); i++)
    {
        enum PokemonBadge badge = sMutations[i].badge;
        enum MutationTier tier = MutationTierFromBadge(badge);
        if ((badges & badge) && tier > highestTier)
        {
            primary = badge;
            highestTier = tier;
        }
    }
    return primary;
}

static enum PokemonBadge RollMutation(u16 existing)
{
    u16 weights[ARRAY_COUNT(sMutations)];
    u16 total = 0;
    for (u8 i = 0; i < ARRAY_COUNT(sMutations); i++)
    {
        weights[i] = existing & sMutations[i].badge ? 0 : sMutations[i].weight;
        total += weights[i];
    }

    u16 roll = Random32() % total;
    for (u8 i = 0; i < ARRAY_COUNT(sMutations); i++)
    {
        if (roll < weights[i])
            return sMutations[i].badge;
        roll -= weights[i];
    }
    return 0;
}

static void NormalizeEvs(struct Pokemon *mon)
{
    u16 total = 0;
    for (u8 i = 0; i < NUM_STATS; i++)
        total += GetMonData(mon, MON_DATA_HP_EV + i);
    if (total <= MAX_TOTAL_EVS)
        return;

    for (u8 i = 0; i < NUM_STATS; i++)
    {
        u8 ev = GetMonData(mon, MON_DATA_HP_EV + i) * MAX_TOTAL_EVS / total;
        SetMonData(mon, MON_DATA_HP_EV + i, &ev);
    }
}

enum HfcMutatorResult HfcMutator_Attempt(struct Pokemon *mon, enum HfcMutatorPlan plan,
                                        bool32 canReroll, u32 *money, enum PokemonBadge *mutation)
{
    enum HfcMutatorResult eligibility = GetEligibility(mon, canReroll);
    *mutation = 0;
    if (eligibility != HFC_MUTATOR_READY)
        return eligibility;
    if ((u32)plan >= HFC_MUTATOR_PLAN_COUNT)
        return HFC_MUTATOR_INVALID_PLAN;
    if (!IsEnoughMoney(money, sPlans[plan].price))
        return HFC_MUTATOR_INSUFFICIENT_FUNDS;

    RemoveMoney(money, sPlans[plan].price);
    if (Random32() % 100 >= sPlans[plan].success)
        return HFC_MUTATOR_ROLL_FAILED;

    u16 badges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
    enum PokemonBadge replaced = GetPrimaryMutation(badges);
    *mutation = RollMutation(badges);
    u16 newBadges = (badges & ~replaced) | *mutation;
    SetMonData(mon, MON_DATA_SPECIAL_BADGES, &newBadges);
    if (replaced == POKEMON_BADGE_ASCENDED)
        NormalizeEvs(mon);

    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    u8 resultLevel = GetResultLevel(level, plan);
    if (resultLevel != level)
    {
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        SetMonData(mon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][resultLevel]);
    }
    u8 savedLevelUpHp = gBattleScripting.levelUpHP;
    CalculateMonStats(mon);
    gBattleScripting.levelUpHP = savedLevelUpHp;
    return HFC_MUTATOR_APPLIED;
}

static struct Pokemon *SelectedMon(void)
{
    return &gParties[B_TRAINER_PLAYER][sMutator.slots[sMutator.cursor]];
}

static void Print(const u8 *text, u8 x, u8 y)
{
    UnholyUi_Print(sMutator.window, y >= 32 ? FONT_SMALL_NARROW : FONT_SMALL, x, y, 200 - min(x, 200), text);
}

static void PrintAmount(const u8 *label, u32 amount, const u8 *suffix, u8 x, u8 y, u8 digits)
{
    u8 text[40];
    u8 *end = StringCopy(text, label);
    end = ConvertUIntToDecimalStringN(end, amount, STR_CONV_MODE_LEFT_ALIGN, digits);
    if (suffix != NULL)
        StringCopy(end, suffix);
    Print(text, x, y);
}

static void RenderParty(void)
{
    u8 window = sMutator.window;
    bool32 canReroll = FlagGet(FLAG_DEFEATED_SUPER_LEAGUE);
    enum HfcMutatorResult eligibility = HFC_MUTATOR_READY;
    if (sMutator.count == 0)
        Print(COMPOUND_STRING("Your party is empty."), 8, 25);

    for (u8 i = 0; i < sMutator.count; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sMutator.slots[i]];
        enum HfcMutatorResult result = GetEligibility(mon, canReroll);
        bool8 selected = i == sMutator.cursor;
        const u8 *detail = NULL;
        if (result != HFC_MUTATOR_READY)
            detail = COMPOUND_STRING("LOCKED");
        else if (GetMonData(mon, MON_DATA_SPECIAL_BADGES) != 0)
            detail = COMPOUND_STRING("REROLL");
        u8 name[POKEMON_NAME_LENGTH + 1];
        GetMonData(mon, MON_DATA_NICKNAME, name);
        UnholyUi_ChoiceRow(window, 23 + 13 * i, name, detail, selected);
        if (selected)
            eligibility = result;
    }

    enum HfcMutatorResult notice = sMutator.result != HFC_MUTATOR_READY ? sMutator.result : eligibility;
    if (notice != HFC_MUTATOR_READY)
        Print(sMessages[notice], 8, 105);
    UnholyUi_Footer(window, sMutator.count != 0 && eligibility == HFC_MUTATOR_READY
        ? COMPOUND_STRING("A:CHOOSE  B:BACK") : COMPOUND_STRING("B:BACK"));
}

static void RenderPlanRow(enum HfcMutatorPlan plan, u8 level)
{
    u8 window = sMutator.window;
    u8 y = 49 + 15 * plan;
    bool8 selected = plan == sMutator.plan;
    u8 fill = UnholyUi_CardFill(selected);
    u8 amount[16];

    UnholyUi_Card(window, 6, y, 196, 14, selected);
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 11, y + 1, 61, fill, sPlans[plan].name);
    u8 *end = StringCopy(amount, COMPOUND_STRING("¥"));
    ConvertUIntToDecimalStringN(end, sPlans[plan].price, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 75, y + 1, 46, fill, amount);
    end = ConvertUIntToDecimalStringN(amount, sPlans[plan].success, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(end, COMPOUND_STRING("%"));
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 126, y + 1, 32, fill, amount);
    ConvertUIntToDecimalStringN(amount, GetResultLevel(level, plan), STR_CONV_MODE_LEFT_ALIGN, 3);
    UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 172, y + 1, 25, fill, amount);
}

static void RenderPlans(void)
{
    u8 window = sMutator.window;
    struct Pokemon *mon = SelectedMon();
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    u16 badges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
    u8 text[32];
    GetMonData(mon, MON_DATA_NICKNAME, text);
    u8 *end = StringAppend(text, COMPOUND_STRING("  Lv."));
    ConvertUIntToDecimalStringN(end, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    UnholyUi_Print(window, FONT_SMALL_NARROW, 8, 22, 100, text);
    if (badges != 0)
    {
        end = StringCopy(text, COMPOUND_STRING("Reroll "));
        StringCopy(end, HorizonMarketMutationName(GetPrimaryMutation(badges)));
        UnholyUi_Print(window, FONT_SMALL_NARROW, 111, 22, 89, text);
    }
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 11, 37, 61, COMPOUND_STRING("PLAN"));
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 75, 37, 46, COMPOUND_STRING("COST"));
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 122, 37, 42, COMPOUND_STRING("SUCCESS"));
    UnholyUi_Print(window, FONT_SMALL_NARROWER, 167, 37, 35, COMPOUND_STRING("NEW LV."));

    for (enum HfcMutatorPlan plan = 0; plan < HFC_MUTATOR_PLAN_COUNT; plan++)
        RenderPlanRow(plan, level);

    PrintAmount(COMPOUND_STRING("Have ¥"), GetMoney(&gSaveBlock1Ptr->money), NULL, 8, 93, MAX_MONEY_DIGITS);
    Print(sMutator.result == HFC_MUTATOR_INSUFFICIENT_FUNDS ? sMessages[sMutator.result]
          : COMPOUND_STRING("The fee applies even if it fails."), 8, 105);
    UnholyUi_Footer(window, sMutator.screen == MUTATOR_CONFIRM ? COMPOUND_STRING("A:PAY  B:CANCEL")
                                                             : COMPOUND_STRING("UP/DOWN:PLAN  A:REVIEW  B:BACK"));
}

static void RenderResult(void)
{
    u8 window = sMutator.window;
    bool8 applied = sMutator.result == HFC_MUTATOR_APPLIED;
    u8 fill = UnholyUi_CardFill(applied);
    UnholyUi_Card(window, 6, 23, 196, applied ? 52 : 30, applied);
    UnholyUi_PrintOnFill(window, FONT_SMALL, 12, 28, 183, fill,
        applied ? COMPOUND_STRING("Mutation applied.") : sMessages[sMutator.result]);
    if (applied)
    {
        UnholyUi_PrintOnFill(window, FONT_SMALL, 12, 44, 183, fill, HorizonMarketMutationName(sMutator.mutation));
        u8 text[18];
        u8 *end = StringCopy(text, COMPOUND_STRING("New level "));
        ConvertUIntToDecimalStringN(end, GetMonData(SelectedMon(), MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
        UnholyUi_PrintOnFill(window, FONT_SMALL_NARROW, 12, 59, 183, fill, text);
    }
    PrintAmount(COMPOUND_STRING("Paid ¥"), sPlans[sMutator.plan].price, NULL, 8, 89, MAX_MONEY_DIGITS);
    UnholyUi_Footer(window, COMPOUND_STRING("A/B:CONTINUE"));
}

static void Render(void)
{
    UnholyUi_Frame(sMutator.window, sMutator.screen == MUTATOR_CONFIRM ? COMPOUND_STRING("CONFIRM MUTATION") : COMPOUND_STRING("MUTATOR"));
    UnholyUi_Icon(sMutator.window, GetWindowAttribute(sMutator.window, WINDOW_WIDTH) * TILE_WIDTH - 18, 6, UNHOLY_UI_ICON_MUTATOR, UnholyUi_AccentColor());
    switch (sMutator.screen)
    {
    case MUTATOR_PARTY:
        RenderParty();
        break;
    case MUTATOR_PLANS:
    case MUTATOR_CONFIRM:
        RenderPlans();
        break;
    case MUTATOR_RESULT:
        RenderResult();
        break;
    }
    CopyWindowToVram(sMutator.window, COPYWIN_FULL);
}

static void ConfirmMutation(void)
{
    struct Pokemon *mon = SelectedMon();
    struct BoxPokemon original = mon->box;
    sMutator.result = HfcMutator_Attempt(mon, sMutator.plan, FlagGet(FLAG_DEFEATED_SUPER_LEAGUE),
                                        &gSaveBlock1Ptr->money, &sMutator.mutation);
    switch (sMutator.result)
    {
    case HFC_MUTATOR_APPLIED:
        HfcWatch_RegisterMutation(&original);
        HfcWatch_RegisterMutation(&mon->box);
        sMutator.screen = MUTATOR_RESULT;
        PlaySE(SE_SHOP);
        break;
    case HFC_MUTATOR_ROLL_FAILED:
        sMutator.screen = MUTATOR_RESULT;
        PlaySE(SE_FAILURE);
        break;
    case HFC_MUTATOR_INSUFFICIENT_FUNDS:
        PlaySE(SE_FAILURE);
        break;
    default:
        sMutator.screen = MUTATOR_PARTY;
        PlaySE(SE_FAILURE);
        break;
    }
}

static void Task_Mutator(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    UnholyUi_Tick(14);

    if (JOY_NEW(B_BUTTON))
    {
        if (sMutator.screen == MUTATOR_PARTY)
        {
            ClearStdWindowAndFrameToTransparent(sMutator.window, TRUE);
            RemoveWindow(sMutator.window);
            UnholyUi_ReleasePalette(&sUiPalette);
            DestroyTask(taskId);
            PlaySE(SE_SELECT);
            sMutator.returnToWatch();
            return;
        }
        sMutator.screen = sMutator.screen == MUTATOR_CONFIRM ? MUTATOR_PLANS : MUTATOR_PARTY;
        sMutator.result = HFC_MUTATOR_READY;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        switch (sMutator.screen)
        {
        case MUTATOR_PARTY:
            if (sMutator.count == 0)
                return;
            sMutator.result = GetEligibility(SelectedMon(), FlagGet(FLAG_DEFEATED_SUPER_LEAGUE));
            if (sMutator.result == HFC_MUTATOR_READY)
            {
                sMutator.plan = HFC_MUTATOR_BASIC;
                sMutator.screen = MUTATOR_PLANS;
            }
            break;
        case MUTATOR_PLANS:
            sMutator.screen = MUTATOR_CONFIRM;
            break;
        case MUTATOR_CONFIRM:
            ConfirmMutation();
            Render();
            return;
        case MUTATOR_RESULT:
            sMutator.screen = MUTATOR_PARTY;
            sMutator.result = HFC_MUTATOR_READY;
            break;
        }
    }
    else if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        s8 step = JOY_REPEAT(DPAD_UP) ? -1 : 1;
        if (sMutator.screen == MUTATOR_PARTY && sMutator.count != 0)
            sMutator.cursor = (sMutator.cursor + sMutator.count + step) % sMutator.count;
        else if (sMutator.screen == MUTATOR_PLANS)
            sMutator.plan = (sMutator.plan + HFC_MUTATOR_PLAN_COUNT + step) % HFC_MUTATOR_PLAN_COUNT;
        else
            return;
        sMutator.result = HFC_MUTATOR_READY;
    }
    else
        return;

    PlaySE(sMutator.result == HFC_MUTATOR_READY ? SE_SELECT : SE_FAILURE);
    Render();
}

void HfcMutator_Open(void (*returnToWatch)(void))
{
    u8 taskId = CreateTask(Task_Mutator, 8);
    if (gTasks[taskId].func != Task_Mutator)
    {
        returnToWatch();
        return;
    }

    memset(&sMutator, 0, sizeof(sMutator));
    sMutator.window = AddWindow(&sMutatorWindow);
    if (sMutator.window == WINDOW_NONE)
    {
        DestroyTask(taskId);
        returnToWatch();
        return;
    }

    sMutator.returnToWatch = returnToWatch;
    for (u8 slot = 0; slot < PARTY_SIZE; slot++)
        if (GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_SANITY_HAS_SPECIES))
            sMutator.slots[sMutator.count++] = slot;

    LoadMessageBoxAndBorderGfx();
    UnholyUi_AcquirePalette(&sUiPalette, 14);
    PutWindowTilemap(sMutator.window);
    Render();
}
