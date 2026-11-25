#include "global.h"
#include "unholy_cofradia.h"
#include "unholy_save.h"
#include "unholy_localization.h"
#include "money.h"
#include "battle_setup.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script_pokemon_util.h"
#include "list_menu.h"
#include "script.h"
#include "event_data.h"
#include "follower_npc.h"
#include "hfc_watch.h"
#include "malloc.h"
#include "random.h"
#include "script_menu.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/trainers.h"
#include "constants/script_menu.h"

#define COFRADIA_MAGIC 0x434F4631u
#define COFRADIA_VERSION 1
#define C_TEXT(en, es) UnholyLocalize(COMPOUND_STRING(en), COMPOUND_STRING(es))

struct CofradiaRecord
{
    u32 magic;
    u16 version;
    u8 state;
    u8 difficulty;
    u32 stake;
    u32 fund;
    u32 wins;
    u32 seed;
    u32 bestWins;
    u32 bestStake;
    u32 bestProfit;
    u32 biggestLoss;
};
STATIC_ASSERT(UNHOLY_SAVE4_COFRADIA_OFFSET + sizeof(struct CofradiaRecord) <= UNHOLY_SAVE4_PAYLOAD_BYTES, CofradiaFitsSave4);

struct CofradiaDifficulty
{
    const u8 *en;
    const u8 *es;
    u32 minimum;
    u8 multiplier;
    u8 badges;
};
static const struct CofradiaDifficulty sDifficulties[] =
{
    {COMPOUND_STRING("Normal"), COMPOUND_STRING("Normal"), 5000, 101, 3},
    {COMPOUND_STRING("Hard"), COMPOUND_STRING("Difícil"), 15000, 103, 4},
    {COMPOUND_STRING("Expert"), COMPOUND_STRING("Experto"), 35000, 107, 5},
    {COMPOUND_STRING("Master"), COMPOUND_STRING("Maestro"), 75000, 112, 6},
    {COMPOUND_STRING("Infernal"), COMPOUND_STRING("Infernal"), 150000, 120, 7},
    {COMPOUND_STRING("Abyss"), COMPOUND_STRING("Abismo"), 300000, 135, 8},
};
static EWRAM_DATA struct Pokemon sPartyBackup[PARTY_SIZE] = {0};
static EWRAM_DATA bool8 sActive = FALSE;
static EWRAM_DATA u32 sCashBackup = 0;
static EWRAM_DATA u8 sDifficulty = 0;
static EWRAM_DATA u32 sStake = 0;
static EWRAM_DATA u32 sStep = 1000;

static struct CofradiaRecord *GetRecord(void)
{
    struct SaveBlock4 *block = UnholySave4_Get();
    if (block == NULL)
        return NULL;
    struct CofradiaRecord *r = (struct CofradiaRecord *)(block->payload + UNHOLY_SAVE4_COFRADIA_OFFSET);
    if (r->magic == 0)
    {
        for (u32 i = 0; i < sizeof(*r); i++)
            if (((u8 *)r)[i] != 0)
                return NULL;
        r->magic = COFRADIA_MAGIC;
        r->version = COFRADIA_VERSION;
        if (!UnholySave4_Commit())
            return NULL;
    }
    if (r->magic != COFRADIA_MAGIC || r->version != COFRADIA_VERSION
     || r->state > COFRADIA_FIGHTING || r->difficulty >= COFRADIA_DIFFICULTY_COUNT)
        return NULL;
    if (r->state != COFRADIA_IDLE && (r->stake < CofradiaMinimum(r->difficulty) || r->fund < r->stake))
        return NULL;
    return r;
}

static void ClearRun(struct CofradiaRecord *r)
{
    r->state = COFRADIA_IDLE;
    r->stake = r->fund = r->wins = r->seed = 0;
}

u32 CofradiaMinimum(u8 difficulty)
{
    return difficulty < COFRADIA_DIFFICULTY_COUNT ? sDifficulties[difficulty].minimum : 0;
}

u32 CofradiaCompound(u32 fund, u8 difficulty)
{
    if (difficulty >= COFRADIA_DIFFICULTY_COUNT)
        return fund;
    u64 result = (u64)fund * sDifficulties[difficulty].multiplier / 100;
    return result > MAX_MONEY ? MAX_MONEY : (u32)result;
}

bool32 CofradiaGetView(struct CofradiaView *view)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || view == NULL)
        return FALSE;
    *view = (struct CofradiaView){r->stake, r->fund, r->wins, r->bestWins,
        r->bestStake, r->bestProfit, r->biggestLoss, r->state, r->difficulty};
    return TRUE;
}

enum CofradiaResult CofradiaBegin(u8 difficulty, u32 stake)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || difficulty >= COFRADIA_DIFFICULTY_COUNT || stake < CofradiaMinimum(difficulty))
        return COFRADIA_INVALID;
    if (r->state != COFRADIA_IDLE)
        return COFRADIA_BUSY;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, stake))
        return COFRADIA_NO_MONEY;
    r->difficulty = difficulty;
    r->state = COFRADIA_READY;
    r->stake = r->fund = stake;
    r->wins = 0;
    r->seed = Random32();
    RemoveMoney(&gSaveBlock1Ptr->money, stake);
    UnholySave4_Commit();
    return COFRADIA_OK;
}

bool32 CofradiaArm(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_READY || (r->wins && r->fund == MAX_MONEY))
        return FALSE;
    r->state = COFRADIA_FIGHTING;
    return UnholySave4_Commit();
}

bool32 CofradiaDisarm(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING || sActive)
        return FALSE;
    if (!r->wins)
    {
        u32 cash = GetMoney(&gSaveBlock1Ptr->money);
        if (r->stake > MAX_MONEY - cash)
            return FALSE;
        AddMoney(&gSaveBlock1Ptr->money, r->stake);
        ClearRun(r);
    }
    else
        r->state = COFRADIA_READY;
    return UnholySave4_Commit();
}

bool32 CofradiaResolve(bool32 won)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING)
        return FALSE;
    r->bestStake = max(r->bestStake, r->stake);
    if (won)
    {
        r->fund = CofradiaCompound(r->fund, r->difficulty);
        if (r->wins < MAX_MONEY)
            r->wins++;
        r->bestWins = max(r->bestWins, r->wins);
        r->state = COFRADIA_READY;
    }
    else
    {
        r->biggestLoss = max(r->biggestLoss, r->fund);
        ClearRun(r);
    }
    return UnholySave4_Commit();
}

enum CofradiaResult CofradiaCashOut(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_READY || !r->wins)
        return COFRADIA_INVALID;
    if (r->fund > MAX_MONEY - GetMoney(&gSaveBlock1Ptr->money))
        return COFRADIA_CASH_FULL;
    AddMoney(&gSaveBlock1Ptr->money, r->fund);
    r->bestProfit = max(r->bestProfit, r->fund - r->stake);
    ClearRun(r);
    UnholySave4_Commit();
    return COFRADIA_OK;
}

// Existing trainer pools are a first integration step; custom tower teams follow.
static const u16 sOpponents[6][4] =
{
    {TRAINER_UNHOLY_BOUNTY_BRONZE, TRAINER_UNHOLY_BOUNTY_SILVER, TRAINER_UNHOLY_BOUNTY_BRONZE, TRAINER_UNHOLY_BOUNTY_SILVER},
    {TRAINER_UNHOLY_BOUNTY_SILVER, TRAINER_UNHOLY_BOUNTY_GOLD, TRAINER_UNHOLY_BOUNTY_SILVER, TRAINER_UNHOLY_BOUNTY_GOLD},
    {TRAINER_UNHOLY_BOUNTY_GOLD, TRAINER_UNHOLY_BOUNTY_PLATINUM, TRAINER_UNHOLY_BOUNTY_GOLD, TRAINER_UNHOLY_BOUNTY_PLATINUM},
    {TRAINER_UNHOLY_BOUNTY_PLATINUM, TRAINER_UNHOLY_BOUNTY_LEGENDARY, TRAINER_UNHOLY_BOUNTY_PLATINUM, TRAINER_UNHOLY_BOUNTY_LEGENDARY},
    {TRAINER_UNHOLY_BOUNTY_LEGENDARY, TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA},
    {TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA, TRAINER_DRAKE},
};
static const u16 sBosses[] =
{
    TRAINER_UNHOLY_BOUNTY_SILVER, TRAINER_UNHOLY_BOUNTY_GOLD,
    TRAINER_UNHOLY_BOUNTY_PLATINUM, TRAINER_UNHOLY_BOUNTY_LEGENDARY,
    TRAINER_WALLACE, TRAINER_STEVEN,
};
static u16 PickOpponent(const struct CofradiaRecord *r)
{
    if (r->wins % 5 == 4)
        return sBosses[r->difficulty];
    u32 seed = r->seed ^ (r->wins * 0x9E3779B9u);
    seed ^= seed >> 16;
    seed *= 0x7FEB352Du;
    seed ^= seed >> 15;
    return sOpponents[r->difficulty][seed % 4];
}

bool32 CofradiaBattleActive(void) { return sActive; }

bool32 CofradiaPrepareBattle(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING || sActive
     || CountPartyAliveNonEggMonsExcept(PARTY_SIZE) == 0 || IsNPCFollowerWildBattle() || FollowerNPCIsBattlePartner())
        return FALSE;
    memcpy(sPartyBackup, gParties[B_TRAINER_PLAYER], sizeof(sPartyBackup));
    sCashBackup = GetMoney(&gSaveBlock1Ptr->money);
    sActive = TRUE;
    BattleSetup_StartCofradiaBattle(PickOpponent(r));
    return TRUE;
}

void CofradiaBattleEnded(u8 outcome)
{
    if (!sActive)
        return;
    // Full party copy restores held items, PP, custom mutations, gear, identity and status.
    memcpy(gParties[B_TRAINER_PLAYER], sPartyBackup, sizeof(sPartyBackup));
    CalculatePlayerPartyCount();
    HealPlayerParty();
    SetMoney(&gSaveBlock1Ptr->money, sCashBackup);
    sActive = FALSE;
    CofradiaResolve(outcome == B_OUTCOME_WON);
}

static void Message(const u8 *s, u16 result)
{
    StringCopy(gStringVar4, s);
    gSpecialVar_Result = result;
}
static u8 *Number(u8 *p, u32 n)
{
    return ConvertUIntToDecimalStringN(p, n, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
}

void CofradiaStatusScript(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL)
    {
        Message(C_TEXT("Ledger unavailable.", "Libro de cuentas no disponible."), 3);
        return;
    }
    if (r->state == COFRADIA_FIGHTING && !sActive)
    {
        CofradiaResolve(FALSE);
        Message(C_TEXT("Interrupted run. Stake lost.", "Racha interrumpida. Fondo perdido."), 4);
        return;
    }
    if (r->state == COFRADIA_READY && r->wins)
    {
        u8 *p = StringCopy(gStringVar4, C_TEXT("Wins: ", "Victorias: "));
        p = Number(p, r->wins);
        p = StringCopy(p, C_TEXT("\nFund: ¥", "\nFondo: ¥"));
        p = Number(p, r->fund);
        StringCopy(p, C_TEXT("\pYES: fight again. NO: cash out.", "\pSÍ: otro combate. NO: retirar."));
        gSpecialVar_Result = 1;
    }
    else if (r->state == COFRADIA_READY)
        gSpecialVar_Result = 2;
    else
        gSpecialVar_Result = 0;
}

static bool32 OpenMenu(const u8 *const *labels, u8 count)
{
    struct ListMenuItem *items = AllocZeroed(count * sizeof(*items));
    if (items == NULL)
        return FALSE;
    for (u8 i = 0; i < count; i++)
    {
        u8 *s = Alloc(64);
        if (s == NULL)
        {
            while (i)
                Free((void *)items[--i].name);
            Free(items);
            return FALSE;
        }
        StringCopy(s, labels[i]);
        items[i] = (struct ListMenuItem){s, i};
    }
    return ScriptMenu_MultichoiceDynamic(0, 0, count, items, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE);
}

void CofradiaDifficultyMenu(void)
{
    u8 texts[6][48];
    const u8 *labels[6];
    for (u8 i = 0; i < 6; i++)
    {
        u8 *p = StringCopy(texts[i], UnholyLocalize(sDifficulties[i].en, sDifficulties[i].es));
        p = StringCopy(p, COMPOUND_STRING(" ¥"));
        p = Number(p, sDifficulties[i].minimum);
        if (HfcWatch_CountBadges() < sDifficulties[i].badges)
        {
            p = StringCopy(p, COMPOUND_STRING(" ["));
            p = Number(p, sDifficulties[i].badges);
            StringCopy(p, COMPOUND_STRING("]"));
        }
        labels[i] = texts[i];
    }
    gSpecialVar_Result = OpenMenu(labels, 6);
}

void CofradiaChooseDifficulty(void)
{
    u16 difficulty = gSpecialVar_0x8004;
    if (difficulty >= 6 || HfcWatch_CountBadges() < sDifficulties[difficulty].badges)
    {
        Message(C_TEXT("More badges required.", "Necesitas más medallas."), FALSE);
        return;
    }
    sDifficulty = difficulty;
    sStake = sDifficulties[difficulty].minimum;
    sStep = 1000;
    gSpecialVar_Result = TRUE;
}

void CofradiaStakeMenu(void)
{
    u8 value[40], plus[40], minus[40], step[40];
    u8 *p = StringCopy(value, C_TEXT("Bet ¥", "Apostar ¥"));
    Number(p, sStake);
    p = StringCopy(plus, COMPOUND_STRING("+ ")); Number(p, sStep);
    p = StringCopy(minus, COMPOUND_STRING("- ")); Number(p, sStep);
    p = StringCopy(step, C_TEXT("Step: ", "Paso: ")); Number(p, sStep);
    const u8 *labels[] = {value, plus, minus, step, C_TEXT("All cash", "Todo el efectivo"), C_TEXT("Minimum", "Mínimo")};
    gSpecialVar_Result = OpenMenu(labels, ARRAY_COUNT(labels));
}

void CofradiaAdjustStake(void)
{
    u32 cash = GetMoney(&gSaveBlock1Ptr->money);
    u32 floor = CofradiaMinimum(sDifficulty);
    switch (gSpecialVar_0x8004)
    {
    case 1: sStake = sStep > MAX_MONEY - sStake ? MAX_MONEY : sStake + sStep; break;
    case 2: sStake = sStep > sStake - floor ? floor : sStake - sStep; break;
    case 3: sStep = sStep == 1000000000 ? 1 : sStep * 10; break;
    case 4: sStake = max(cash, floor); break;
    case 5: sStake = floor; break;
    }
    if (cash >= floor)
        sStake = min(sStake, cash);
}

void CofradiaBeginScript(void)
{
    enum CofradiaResult result = CofradiaBegin(sDifficulty, sStake);
    if (result != COFRADIA_OK)
        Message(C_TEXT("Cannot fund this bet.", "No puedes financiar esta apuesta."), FALSE);
    else
        gSpecialVar_Result = TRUE;
}

void CofradiaArmScript(void)
{
    if (CountPartyAliveNonEggMonsExcept(PARTY_SIZE) == 0 || IsNPCFollowerWildBattle() || FollowerNPCIsBattlePartner())
    {
        Message(C_TEXT("Prepare your team.", "Prepara tu equipo."), FALSE);
        return;
    }
    if (!CofradiaArm())
        Message(C_TEXT("Cannot start this round.", "No puedes iniciar esta ronda."), FALSE);
    else
        gSpecialVar_Result = TRUE;
}
void CofradiaCancelSaveScript(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r != NULL && r->state == COFRADIA_READY && !r->wins)
    {
        if (CofradiaArm())
            CofradiaDisarm();
    }
    else
        CofradiaDisarm();
}
void CofradiaStartBattleScript(void)
{
    if (!CofradiaPrepareBattle())
    {
        CofradiaDisarm();
        Message(C_TEXT("Battle unavailable.", "Combate no disponible."), FALSE);
    }
    else
        gSpecialVar_Result = TRUE;
}
void CofradiaCashOutScript(void)
{
    enum CofradiaResult result = CofradiaCashOut();
    Message(result == COFRADIA_OK ? C_TEXT("Fund paid in full.", "Fondo abonado por completo.")
        : C_TEXT("No room for payout. Use reserve.", "Sin espacio. Usa tu reserva."), result == COFRADIA_OK);
}

void CofradiaRecordsScript(void)
{
    struct CofradiaView view;
    if (!CofradiaGetView(&view))
    {
        Message(C_TEXT("Ledger unavailable.", "Libro no disponible."), FALSE);
        return;
    }
    u8 *p = StringCopy(gStringVar4, C_TEXT("Best streak: ", "Mejor racha: "));
    p = Number(p, view.bestWins);
    p = StringCopy(p, C_TEXT("\nBiggest stake: ¥", "\nMayor apuesta: ¥"));
    p = Number(p, view.bestStake);
    p = StringCopy(p, C_TEXT("\pBest profit: ¥", "\pMayor ganancia: ¥"));
    p = Number(p, view.bestProfit);
    p = StringCopy(p, C_TEXT("\nBiggest loss: ¥", "\nMayor pérdida: ¥"));
    Number(p, view.biggestLoss);
    gSpecialVar_Result = TRUE;
}
