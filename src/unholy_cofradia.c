#include "global.h"
#include "data.h"
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
static EWRAM_DATA u8 sBattleDifficulty = 0;
static EWRAM_DATA u32 sBattleWins = 0;
static EWRAM_DATA u32 sBattleSeed = 0;
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

// The actual trainer database is the source of all species, moves, abilities,
 // items and gear; no separate speculative species list or trainer flag allocation.
static const u16 sGymTeams[8][5] =
{
    {TRAINER_ROXANNE_1, TRAINER_ROXANNE_2, TRAINER_ROXANNE_3, TRAINER_ROXANNE_4, TRAINER_ROXANNE_5},
    {TRAINER_BRAWLY_1, TRAINER_BRAWLY_2, TRAINER_BRAWLY_3, TRAINER_BRAWLY_4, TRAINER_BRAWLY_5},
    {TRAINER_WATTSON_1, TRAINER_WATTSON_2, TRAINER_WATTSON_3, TRAINER_WATTSON_4, TRAINER_WATTSON_5},
    {TRAINER_FLANNERY_1, TRAINER_FLANNERY_2, TRAINER_FLANNERY_3, TRAINER_FLANNERY_4, TRAINER_FLANNERY_5},
    {TRAINER_NORMAN_1, TRAINER_NORMAN_2, TRAINER_NORMAN_3, TRAINER_NORMAN_4, TRAINER_NORMAN_5},
    {TRAINER_WINONA_1, TRAINER_WINONA_2, TRAINER_WINONA_3, TRAINER_WINONA_4, TRAINER_WINONA_5},
    {TRAINER_TATE_AND_LIZA_1, TRAINER_TATE_AND_LIZA_2, TRAINER_TATE_AND_LIZA_3, TRAINER_TATE_AND_LIZA_4, TRAINER_TATE_AND_LIZA_5},
    {TRAINER_JUAN_1, TRAINER_JUAN_2, TRAINER_JUAN_3, TRAINER_JUAN_4, TRAINER_JUAN_5},
};
static const u16 sLeagueTeams[] = {TRAINER_SIDNEY, TRAINER_PHOEBE, TRAINER_GLACIA, TRAINER_DRAKE, TRAINER_WALLACE, TRAINER_STEVEN};
static const u16 sBountyTeams[] =
{
    TRAINER_UNHOLY_BOUNTY_BRONZE, TRAINER_UNHOLY_BOUNTY_SILVER, TRAINER_UNHOLY_BOUNTY_GOLD,
    TRAINER_UNHOLY_BOUNTY_PLATINUM, TRAINER_UNHOLY_BOUNTY_LEGENDARY,
};

static bool32 SourceValid(u16 id, bool32 primary)
{
    if (id == TRAINER_NONE || id >= TRAINERS_COUNT)
        return FALSE;
    const struct Trainer *t = GetTrainerStructFromId(id);
    if (primary && t->battleType != TRAINER_BATTLE_TYPE_SINGLES)
        return FALSE;
    if (t->overrideTrainer)
    {
        if (t->overrideTrainer >= TRAINERS_COUNT)
            return FALSE;
        const struct Trainer *original = GetTrainerStructFromId(t->overrideTrainer);
        return original->party != NULL && original->partySize > 0 && original->partySize <= PARTY_SIZE
            && t->partySize <= PARTY_SIZE;
    }
    return t->party != NULL && t->partySize > 0 && t->partySize <= PARTY_SIZE;
}

static bool32 OrdinaryClass(u8 cls, u8 difficulty)
{
    switch (cls)
    {
    case TRAINER_CLASS_COOLTRAINER:
    case TRAINER_CLASS_COOLTRAINER_2:
    case TRAINER_CLASS_EXPERT:
    case TRAINER_CLASS_GENTLEMAN:
    case TRAINER_CLASS_DRAGON_TAMER:
    case TRAINER_CLASS_PKMN_RANGER:
    case TRAINER_CLASS_PSYCHIC:
    case TRAINER_CLASS_BLACK_BELT:
    case TRAINER_CLASS_BIRD_KEEPER:
    case TRAINER_CLASS_BATTLE_GIRL:
    case TRAINER_CLASS_PKMN_BREEDER:
    case TRAINER_CLASS_COLLECTOR:
        return TRUE;
    case TRAINER_CLASS_HIKER:
    case TRAINER_CLASS_GUITARIST:
    case TRAINER_CLASS_RUIN_MANIAC:
    case TRAINER_CLASS_NINJA_BOY:
    case TRAINER_CLASS_SCHOOL_KID:
    case TRAINER_CLASS_YOUNGSTER:
    case TRAINER_CLASS_LASS:
        return difficulty <= 2;
    default:
        return FALSE;
    }
}

static u16 PickTeamSourceRaw(u32 seed, u8 difficulty, bool32 boss, u8 ordinal)
{
    difficulty = min(difficulty, COFRADIA_DIFFICULTY_COUNT - 1);
    rng_value_t rng = LocalRandomSeed(seed ^ ((u32)ordinal * 0x9E3779B9u));
    bool32 primary = ordinal == 0;
    u32 roll = LocalRandom32(&rng) % 100;
    // Bosses primarily use gym or League archetypes, ordinary rounds vary
    // between existing route trainers, leaders and bounty teams.
    if (difficulty >= 4 && roll < (boss ? 70 : 20))
    {
        u8 start = LocalRandom32(&rng) % ARRAY_COUNT(sLeagueTeams);
        for (u8 i = 0; i < ARRAY_COUNT(sLeagueTeams); i++)
        {
            u16 id = sLeagueTeams[(start + i) % ARRAY_COUNT(sLeagueTeams)];
            if (SourceValid(id, primary))
                return id;
        }
    }
    if (boss || roll < 35 + difficulty * 5)
    {
        u8 gymCount = min(8, difficulty + 3);
        u8 gymStart = LocalRandom32(&rng) % gymCount;
        u8 maxRematch = min(4, difficulty + (boss ? 1 : 0));
        for (u8 i = 0; i < gymCount; i++)
        {
            u8 gym = (gymStart + i) % gymCount;
            u8 rematch = LocalRandom32(&rng) % (maxRematch + 1);
            u16 id = sGymTeams[gym][rematch];
            if (SourceValid(id, primary))
                return id;
        }
    }
    if (roll >= 90)
    {
        u8 tier = min(difficulty, ARRAY_COUNT(sBountyTeams) - 1);
        if (SourceValid(sBountyTeams[tier], primary))
            return sBountyTeams[tier];
    }
    // Reservoir sample the actual trainer database without allocating a list.
    // No defeated flags or story status are modified.
    u16 picked = TRAINER_NONE;
    u16 count = 0;
    for (u16 id = 1; id < TRAINERS_COUNT; id++)
    {
        if (!OrdinaryClass(GetTrainerClassFromId(id), difficulty) || !SourceValid(id, primary))
            continue;
        if (LocalRandom32(&rng) % ++count == 0)
            picked = id;
    }
    if (picked != TRAINER_NONE)
        return picked;
    return sBountyTeams[min(difficulty, ARRAY_COUNT(sBountyTeams) - 1)];
}

// Trainer rematch aliases can point at the same underlying party.
static u16 TeamSourceIdentity(u16 id)
{
    if (id == TRAINER_NONE || id >= TRAINERS_COUNT)
        return TRAINER_NONE;
    const struct Trainer *trainer = GetTrainerStructFromId(id);
    return trainer->overrideTrainer ? trainer->overrideTrainer : id;
}

static bool32 TeamSourceUnused(u16 id, const u16 *selected, u8 count)
{
    u16 identity = TeamSourceIdentity(id);
    if (identity == TRAINER_NONE)
        return FALSE;
    for (u8 i = 0; i < count; i++)
        if (TeamSourceIdentity(selected[i]) == identity)
            return FALSE;
    return TRUE;
}

u16 CofradiaPickTeamSource(u32 seed, u8 difficulty, bool32 boss, u8 ordinal)
{
    // Keep the lead archetype unchanged, but do not waste donor slots on
    // identical trainers or aliases of their party.
    u16 selected[5];
    if (ordinal >= ARRAY_COUNT(selected))
        return PickTeamSourceRaw(seed, difficulty, boss, ordinal);
    difficulty = min(difficulty, COFRADIA_DIFFICULTY_COUNT - 1);
    selected[0] = PickTeamSourceRaw(seed, difficulty, boss, 0);
    for (u8 slot = 1; slot <= ordinal; slot++)
    {
        u16 candidate = PickTeamSourceRaw(seed, difficulty, boss, slot);
        for (u8 attempt = 1; attempt <= 8 && !TeamSourceUnused(candidate, selected, slot); attempt++)
            candidate = PickTeamSourceRaw(seed ^ ((u32)attempt * 0xA511E9B3u), difficulty, boss, slot);
        if (!TeamSourceUnused(candidate, selected, slot))
        {
            // Bounded deterministic fallback to a compatible ordinary trainer.
            for (u16 id = 1; id < TRAINERS_COUNT; id++)
            {
                if (OrdinaryClass(GetTrainerClassFromId(id), difficulty)
                    && SourceValid(id, FALSE) && TeamSourceUnused(id, selected, slot))
                {
                    candidate = id;
                    break;
                }
            }
        }
        selected[slot] = candidate;
    }
    return selected[ordinal];
}

u8 CofradiaTeamSize(u8 difficulty, bool32 boss)
{
    static const u8 sizes[] = {3, 4, 4, 5, 6, 6};
    return min(PARTY_SIZE, sizes[min(difficulty, COFRADIA_DIFFICULTY_COUNT - 1)] + (boss ? 1 : 0));
}

u32 CofradiaBattleSeed(void) { return sBattleSeed; }
bool32 CofradiaBattleIsBoss(void) { return sBattleWins % 5 == 4; }
u16 CofradiaBattleDonor(u8 ordinal)
{
    return CofradiaPickTeamSource(sBattleSeed, sBattleDifficulty, CofradiaBattleIsBoss(), ordinal);
}

bool32 CofradiaBattleActive(void) { return sActive; }
u8 CofradiaBattleDifficulty(void) { return sBattleDifficulty; }

void CofradiaScaleEnemyParty(void)
{
    if (!sActive)
        return;
    static const s8 offsets[] = {-5, -2, 0, 2, 4, 7};
    u8 playerLevel = 1;
    for (u8 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
            playerLevel = max(playerLevel, GetMonData(mon, MON_DATA_LEVEL));
    }
    bool32 boss = sBattleWins % 5 == 4;
    s32 target = (s32)playerLevel + offsets[sBattleDifficulty] + (s32)min(sBattleWins / 5, 15u) + (boss ? 2 : 0);
    target = max(1, min(MAX_LEVEL, target));
    for (u8 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        if (species == SPECIES_NONE)
            continue;
        // Source trainers span the entire game; normalize the generated team
        // so random postgame donors cannot make Normal absurdly overlevelled.
        u8 level = target;
        u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        u16 badges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
        if (sBattleDifficulty >= 2 && (boss || sBattleWins >= 9))
            badges |= POKEMON_BADGE_ALPHA;
        if (sBattleDifficulty >= 3 && sBattleWins >= 14 && (boss || i % 2 == 0))
            badges |= POKEMON_BADGE_HYPER;
        if (sBattleDifficulty >= 4 && sBattleWins >= 24 && (boss || i % 2 == 0))
            badges |= POKEMON_BADGE_ANCESTRAL;
        if (sBattleDifficulty == 5 && sBattleWins >= 49 && boss && i == 0)
            badges |= POKEMON_BADGE_OMEGA;
        SetMonData(mon, MON_DATA_SPECIAL_BADGES, &badges);
        CalculateMonStats(mon);
        u16 hp = GetMonData(mon, MON_DATA_MAX_HP);
        SetMonData(mon, MON_DATA_HP, &hp);
    }
}

bool32 CofradiaPrepareBattle(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING || sActive
     || CountPartyAliveNonEggMonsExcept(PARTY_SIZE) == 0 || IsNPCFollowerWildBattle() || FollowerNPCIsBattlePartner())
        return FALSE;
    memcpy(sPartyBackup, gParties[B_TRAINER_PLAYER], sizeof(sPartyBackup));
    sCashBackup = GetMoney(&gSaveBlock1Ptr->money);
    sBattleDifficulty = r->difficulty;
    sBattleWins = r->wins;
    sBattleSeed = r->seed ^ (r->wins * 0x85EBCA6Bu);
    sActive = TRUE;
    BattleSetup_StartCofradiaBattle(CofradiaBattleDonor(0));
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
