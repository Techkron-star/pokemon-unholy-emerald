#include "global.h"
#include "data.h"
#include "unholy_cofradia.h"
#include "unholy_save.h"
#include "unholy_localization.h"
#include "money.h"
#include "item.h"
#include "constants/items.h"
#include "constants/item.h"
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
    // Append-only fields: original P5C saves have zeroed reserved bytes.
    u32 borrowed;
    u32 debt;
    u8 collectionPending; // processed in the lobby, after party restoration
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
static EWRAM_DATA u8 sFunding = 0; // cash, shortfall credit, full credit
static EWRAM_DATA u32 sPayment = 0;
static EWRAM_DATA u8 sVaultSelection = 0;
static EWRAM_DATA u8 sVaultIds[COFRADIA_VAULT_MON_CAPACITY + COFRADIA_VAULT_ITEM_CAPACITY] = {0};
static EWRAM_DATA u8 sVaultCount = 0;

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
    if (r->state == COFRADIA_IDLE && (r->stake || r->fund || r->wins || r->seed))
        return NULL;
    if (r->borrowed > r->stake || (r->state == COFRADIA_IDLE && r->borrowed != 0)
        || (r->borrowed && r->debt) || r->collectionPending > 1
        || (r->collectionPending && !r->debt))
        return NULL;
    return r;
}

static void ClearRun(struct CofradiaRecord *r)
{
    r->state = COFRADIA_IDLE;
    r->stake = r->fund = r->wins = r->seed = r->borrowed = 0;
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
        r->bestStake, r->bestProfit, r->biggestLoss, r->state, r->difficulty, r->borrowed, r->debt};
    return TRUE;
}

enum CofradiaResult CofradiaBegin(u8 difficulty, u32 stake)
{
    return CofradiaBeginCredit(difficulty, stake, 0);
}

// Enforce credit policy here as well as in the lobby UI.
enum CofradiaResult CofradiaBeginCredit(u8 difficulty, u32 stake, u32 borrowed)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || difficulty >= COFRADIA_DIFFICULTY_COUNT || stake < CofradiaMinimum(difficulty))
        return COFRADIA_INVALID;
    if (r->state != COFRADIA_IDLE)
        return COFRADIA_BUSY;
    if (borrowed > stake || borrowed > CofradiaMinimum(difficulty) * 10u)
        return COFRADIA_INVALID;
    if (r->collectionPending)
        return COFRADIA_DEBT_PENDING;
    if (borrowed && r->debt)
        return COFRADIA_DEBT_PENDING;
    if (borrowed)
    {
        struct CofradiaVaultView vault;
        if (!CofradiaVaultGetView(&vault))
            return COFRADIA_INVALID;
        if (vault.pokemon || vault.itemStacks)
            return COFRADIA_DEBT_PENDING; // retrieve previous assets first
    }
    u32 own = stake - borrowed;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, own))
        return COFRADIA_NO_MONEY;
    struct CofradiaRecord old = *r;
    r->borrowed = borrowed;
    r->difficulty = difficulty;
    r->state = COFRADIA_READY;
    r->stake = r->fund = stake;
    r->wins = 0;
    r->seed = Random32();
    if (!UnholySave4_Commit())
    {
        *r = old;
        return COFRADIA_INVALID;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, own);
    return COFRADIA_OK;
}

bool32 CofradiaArm(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_READY || (r->wins && r->fund == MAX_MONEY))
        return FALSE;
    r->state = COFRADIA_FIGHTING;
    if (!UnholySave4_Commit())
    {
        r->state = COFRADIA_READY;
        return FALSE;
    }
    return TRUE;
}

bool32 CofradiaDisarm(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING || sActive)
        return FALSE;
    struct CofradiaRecord old = *r;
    u32 refund = 0;
    if (!r->wins)
    {
        u32 cash = GetMoney(&gSaveBlock1Ptr->money);
        refund = r->stake - r->borrowed;
        if (refund > MAX_MONEY - cash)
            return FALSE;
        ClearRun(r);
    }
    else
        r->state = COFRADIA_READY;
    if (!UnholySave4_Commit())
    {
        *r = old;
        return FALSE;
    }
    AddMoney(&gSaveBlock1Ptr->money, refund);
    return TRUE;
}

bool32 CofradiaResolve(bool32 won)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_FIGHTING)
        return FALSE;
    struct CofradiaRecord old = *r;
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
        // Only borrowed principal becomes debt. Own money was already paid.
        if (r->borrowed)
            r->collectionPending = TRUE;
        r->debt += r->borrowed;
        ClearRun(r);
    }
    if (!UnholySave4_Commit())
    {
        *r = old;
        return FALSE;
    }
    return TRUE;
}

enum CofradiaResult CofradiaCashOut(void)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || r->state != COFRADIA_READY || !r->wins)
        return COFRADIA_INVALID;
    u32 payout = r->fund - r->borrowed;
    if (payout > MAX_MONEY - GetMoney(&gSaveBlock1Ptr->money))
        return COFRADIA_CASH_FULL;
    struct CofradiaRecord old = *r;
    r->bestProfit = max(r->bestProfit, r->fund - r->stake);
    ClearRun(r);
    if (!UnholySave4_Commit())
    {
        *r = old;
        return COFRADIA_INVALID;
    }
    AddMoney(&gSaveBlock1Ptr->money, payout);
    return COFRADIA_OK;
}

enum CofradiaResult CofradiaRepay(u32 amount)
{
    struct CofradiaRecord *r = GetRecord();
    if (r == NULL || amount == 0 || amount > r->debt)
        return COFRADIA_INVALID;
    if (r->state != COFRADIA_IDLE)
        return COFRADIA_BUSY;
    if (sActive || !IsEnoughMoney(&gSaveBlock1Ptr->money, amount))
        return COFRADIA_NO_MONEY;
    struct CofradiaRecord old = *r;
    r->debt -= amount;
    if (!r->debt)
        r->collectionPending = FALSE;
    if (!UnholySave4_Commit())
    {
        *r = old;
        return COFRADIA_INVALID;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, amount);
    return COFRADIA_OK;
}

// Resolve the default after the battle party is restored and before further
// lobby choices. A partial collection is not an additional debt or sale.
bool32 CofradiaProcessDefault(void)
{
    struct CofradiaRecord *r = GetRecord();
    struct CofradiaVaultView vault;
    if (r == NULL || !r->collectionPending || r->state != COFRADIA_IDLE || sActive)
        return FALSE;
    if (!CofradiaVaultGetView(&vault))
        return FALSE;
    CofradiaVaultCollect();
    if (!CofradiaVaultGetView(&vault))
        return FALSE;
    r = GetRecord();
    if (r == NULL)
        return FALSE;
    r->collectionPending = FALSE;
    if (!UnholySave4_Commit())
    {
        r->collectionPending = TRUE;
        return FALSE;
    }
    return TRUE;
}

enum CofradiaResult CofradiaRepayItem(enum Item item, u16 count)
{
    struct CofradiaRecord *r = GetRecord();
    u32 unit = CofradiaAppraiseItem(item);
    u64 value = (u64)unit * count;
    if (r == NULL || !unit || !count || value > r->debt)
        return COFRADIA_INVALID;
    if (r->state != COFRADIA_IDLE || sActive)
        return COFRADIA_BUSY;
    if (!CheckBagHasItem(item, count))
        return COFRADIA_INVALID;
    if (GetItemPocket(item) == POCKET_KEY_ITEMS && count != 1)
        return COFRADIA_INVALID;
    struct CofradiaRecord old = *r;
    r->debt -= (u32)value;
    if (!r->debt)
        r->collectionPending = FALSE;
    if (!UnholySave4_Commit())
    {
        *r = old;
        return COFRADIA_INVALID;
    }
    if (!RemoveBagItem(item, count))
    {
        *r = old;
        UnholySave4_Commit();
        return COFRADIA_INVALID;
    }
    return COFRADIA_OK;
}

// Provisional appraisal: normalize training potential, mutation rarity,
// species rarity and level independently before applying the design weights.
u32 CofradiaAppraisePokemon(struct Pokemon *mon)
{
    if (mon == NULL || GetMonData(mon, MON_DATA_IS_EGG))
        return 0;
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || species >= NUM_SPECIES || GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG))
        return 0;
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 bst = info->baseHP + info->baseAttack + info->baseDefense
            + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
    static const u8 ivFields[] = {MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
        MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV};
    static const u8 evFields[] = {MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV,
        MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV};
    u32 iv = 0, ev = 0;
    for (u8 i = 0; i < ARRAY_COUNT(ivFields); i++)
    {
        iv += GetMonData(mon, ivFields[i]);
        ev += GetMonData(mon, evFields[i]);
    }
    u32 training = min(bst, 900u) * 60 / 900
        + min(iv, 186u) * 25 / 186 + min(ev, 510u) * 15 / 510;
    u32 badges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
    u32 mutation = 0;
    static const u8 rarity[] = {15, 20, 14, 65, 25, 25, 30, 35, 45};
    for (u8 i = 0; i < ARRAY_COUNT(rarity); i++)
        if (badges & (1u << i))
            mutation += rarity[i];
    mutation = min(mutation, 100u);
    u32 speciesRarity = info->isMythical ? 100 : info->isRestrictedLegendary ? 90
        : info->isSubLegendary ? 75 : 25;
    u32 level = min(100u, GetMonData(mon, MON_DATA_LEVEL));
    u32 quality = 40 * training + 35 * mutation + 15 * speciesRarity + 10 * level;
    u32 value = 1000 + quality * 25;
    // Shiny status is a real rarity signal. V2 identifies the palette format,
    // not its probability; do not reward arbitrary color seeds as rarity.
    if (GetMonData(mon, MON_DATA_IS_SHINY))
        value = value * 125 / 100;
    return value;
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

u16 CofradiaGymPatternSource(u32 seed, u8 difficulty, u16 primary)
{
    // If a gym leader is already the anchor, read their real generated roster.
    // Otherwise select a gym blueprint independently of the battle portrait.
    for (u8 gym = 0; gym < ARRAY_COUNT(sGymTeams); gym++)
        for (u8 rematch = 0; rematch < ARRAY_COUNT(sGymTeams[0]); rematch++)
            if (sGymTeams[gym][rematch] == primary && SourceValid(primary, FALSE))
                return primary;

    difficulty = min(difficulty, COFRADIA_DIFFICULTY_COUNT - 1);
    rng_value_t rng = LocalRandomSeed(seed ^ 0xD1B54A35u);
    u8 gymCount = min(8, difficulty + 3);
    u8 start = LocalRandom32(&rng) % gymCount;
    u8 rematchCount = min(5, difficulty + 2);
    for (u8 i = 0; i < gymCount; i++)
    {
        u8 gym = (start + i) % gymCount;
        u8 rematchStart = LocalRandom32(&rng) % rematchCount;
        for (u8 j = 0; j < rematchCount; j++)
        {
            u16 id = sGymTeams[gym][(rematchStart + j) % rematchCount];
            if (SourceValid(id, FALSE))
                return id;
        }
    }
    return primary;
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
        if (!CofradiaResolve(FALSE))
        {
            Message(C_TEXT("Ledger unavailable.", "Libro no disponible."), 3);
            return;
        }
        Message(C_TEXT("Interrupted run. Stake lost.", "Racha interrumpida. Fondo perdido."), 4);
        return;
    }
    if (r->state == COFRADIA_IDLE && r->collectionPending)
    {
        gSpecialVar_Result = 5;
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
    const u8 *labels[] = {value, plus, minus, step, sFunding == 2 ? C_TEXT("Max credit", "Crédito máximo") : sFunding == 1 ? C_TEXT("Max funding", "Financiación máxima") : C_TEXT("All cash", "Todo el efectivo"), C_TEXT("Minimum", "Mínimo")};
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
    case 4:
        sStake = sFunding == 2 ? floor * 10 : sFunding == 1
            ? (cash > MAX_MONEY - floor * 10 ? MAX_MONEY : cash + floor * 10) : max(cash, floor);
        break;
    case 5: sStake = floor; break;
    }
    u32 limit = cash;
    u32 credit = floor * 10; // provisional ceiling per difficulty
    if (sFunding == 1)
        limit = cash > MAX_MONEY - credit ? MAX_MONEY : cash + credit;
    else if (sFunding == 2)
        limit = credit;
    sStake = min(sStake, max(floor, limit));
}

void CofradiaBeginScript(void)
{
    u32 cash = GetMoney(&gSaveBlock1Ptr->money);
    u32 borrowed = sFunding == 2 ? sStake : sFunding == 1 && sStake > cash ? sStake - cash : 0;
    if (borrowed > CofradiaMinimum(sDifficulty) * 10)
    {
        Message(C_TEXT("Credit limit exceeded.", "Superas el límite de crédito."), FALSE);
        return;
    }
    enum CofradiaResult result = CofradiaBeginCredit(sDifficulty, sStake, borrowed);
    if (result != COFRADIA_OK)
        Message(result == COFRADIA_DEBT_PENDING
            ? C_TEXT("Settle debt and reclaim assets first.", "Salda la deuda y recupera tus bienes.")
            : C_TEXT("Cannot fund this bet.", "No puedes financiar esta apuesta."), FALSE);
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
    Message(result == COFRADIA_OK ? C_TEXT("Net fund paid; loan settled.", "Fondo neto abonado. Préstamo saldado.")
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

void CofradiaLobbyMenu(void)
{
    const u8 *labels[] = {C_TEXT("Cash wager", "Apuesta en efectivo"),
        C_TEXT("Mixed credit", "Crédito mixto"), C_TEXT("Full credit", "Crédito total"),
        C_TEXT("Accounts / vault", "Cuentas / bóveda"), C_TEXT("Exit", "Salir")};
    gSpecialVar_Result = OpenMenu(labels, ARRAY_COUNT(labels));
}

void CofradiaChooseFunding(void)
{
    u16 choice = gSpecialVar_0x8004;
    if (choice > 2)
    {
        Message(C_TEXT("Invalid choice.", "Opción inválida."), FALSE);
        return;
    }
    if (choice)
    {
        struct CofradiaView ledger;
        struct CofradiaVaultView vault;
        if (!CofradiaGetView(&ledger) || !CofradiaVaultGetView(&vault)
            || ledger.debt || vault.pokemon || vault.itemStacks)
        {
            Message(C_TEXT("Settle debt and reclaim assets first.", "Salda la deuda y recupera tus bienes."), FALSE);
            return;
        }
    }
    sFunding = choice;
    gSpecialVar_Result = TRUE;
}

void CofradiaFundingQuote(void)
{
    u32 cash = GetMoney(&gSaveBlock1Ptr->money);
    u32 borrowed = sFunding == 2 ? sStake : sFunding == 1 && sStake > cash ? sStake - cash : 0;
    u8 *p = StringCopy(gStringVar4, C_TEXT("Own cash: ¥", "Tu efectivo: ¥"));
    p = Number(p, sStake - borrowed);
    p = StringCopy(p, C_TEXT("\nLoan: ¥", "\nPréstamo: ¥"));
    p = Number(p, borrowed);
    if (borrowed)
        StringCopy(p, C_TEXT("\pDefeat or reset creates debt.\nAssets will be held until paid.\pNo interest. No bag in battle.\pConfirm?", "\pPerder o reiniciar crea deuda.\nRetendrán bienes hasta pagar.\pSin intereses. Sin mochila en combate.\p¿Confirmas?"));
    else
        StringCopy(p, C_TEXT("\pLose or reset: fund lost.\nNo bag in battle. Confirm?", "\pSi pierdes o reinicias, pierdes\nel fondo. Sin mochila en combate.\p¿Confirmas?"));
    gSpecialVar_Result = TRUE;
}

void CofradiaAccountsMenu(void)
{
    struct CofradiaView ledger;
    struct CofradiaVaultView vault;
    if (!CofradiaGetView(&ledger) || !CofradiaVaultGetView(&vault))
    {
        Message(C_TEXT("Ledger unavailable.", "Libro no disponible."), FALSE);
        return;
    }
    u8 debt[40], held[40];
    Number(StringCopy(debt, C_TEXT("Debt: ¥", "Deuda: ¥")), ledger.debt);
    Number(StringCopy(held, C_TEXT("Held: ¥", "Retenido: ¥")), vault.totalValue);
    const u8 *labels[] = {debt, held, C_TEXT("Repay", "Pagar deuda"),
        C_TEXT("Secure debt", "Entregar garantías"), C_TEXT("Inspect vault", "Ver bóveda"), C_TEXT("Back", "Volver")};
    gSpecialVar_Result = OpenMenu(labels, ARRAY_COUNT(labels));
}

void CofradiaRepaymentMenu(void)
{
    const u8 *labels[] = {C_TEXT("25 percent", "25 por ciento"), C_TEXT("50 percent", "50 por ciento"),
        C_TEXT("Full debt", "Deuda completa"), C_TEXT("Available cash", "Efectivo disponible"),
        C_TEXT("Pay with items", "Pagar con objetos"), C_TEXT("Back", "Volver")};
    gSpecialVar_Result = OpenMenu(labels, ARRAY_COUNT(labels));
}

void CofradiaRepaymentQuote(void)
{
    struct CofradiaView ledger;
    u32 cash = GetMoney(&gSaveBlock1Ptr->money);
    if (!CofradiaGetView(&ledger) || ledger.state != COFRADIA_IDLE || !ledger.debt || !cash || gSpecialVar_0x8004 > 3)
    {
        Message(C_TEXT("No payment available.", "No hay pago disponible."), FALSE);
        return;
    }
    switch (gSpecialVar_0x8004)
    {
    case 0: sPayment = max(1u, ledger.debt / 4); break;
    case 1: sPayment = max(1u, ledger.debt / 2); break;
    case 2: sPayment = ledger.debt; break;
    default: sPayment = min(cash, ledger.debt); break;
    }
    if (sPayment > cash)
    {
        Message(C_TEXT("Not enough cash.", "Efectivo insuficiente."), FALSE);
        return;
    }
    u8 *p = Number(StringCopy(gStringVar4, C_TEXT("Pay ¥", "Pagar ¥")), sPayment);
    p = Number(StringCopy(p, C_TEXT("\nRemaining: ¥", "\nRestante: ¥")), ledger.debt - sPayment);
    StringCopy(p, C_TEXT("\pConfirm?", "\p¿Confirmas?"));
    gSpecialVar_Result = TRUE;
}

void CofradiaRepaymentApply(void)
{
    bool32 ok = CofradiaRepay(sPayment) == COFRADIA_OK;
    sPayment = 0;
    Message(ok ? C_TEXT("Payment recorded.", "Pago registrado.")
        : C_TEXT("Payment unavailable.", "Pago no disponible."), ok);
}

void CofradiaCollectScript(void)
{
    struct CofradiaVaultView before, after;
    if (!CofradiaVaultGetView(&before))
    {
        Message(C_TEXT("Vault unavailable.", "Bóveda no disponible."), FALSE);
        return;
    }
    bool32 covered = CofradiaVaultCollect();
    if (!CofradiaVaultGetView(&after))
    {
        Message(C_TEXT("Vault unavailable.", "Bóveda no disponible."), FALSE);
        return;
    }
    bool32 changed = before.totalValue != after.totalValue || before.pokemon != after.pokemon || before.itemStacks != after.itemStacks;
    Message(covered ? C_TEXT("Debt secured. Assets held.", "Deuda garantizada. Bienes retenidos.")
        : C_TEXT("Coverage incomplete. Debt remains.", "Garantía parcial. La deuda continúa."), changed);
}

void CofradiaVaultMenu(void)
{
    u8 texts[COFRADIA_VAULT_MON_CAPACITY + COFRADIA_VAULT_ITEM_CAPACITY + 1][48];
    const u8 *labels[ARRAY_COUNT(texts)];
    sVaultCount = 0;
    for (u8 i = 0; i < COFRADIA_VAULT_MON_CAPACITY; i++)
    {
        u8 name[POKEMON_NAME_LENGTH + 1];
        u32 value;
        if (CofradiaVaultDescribeMon(i, name, &value))
        {
            StringCopy(texts[sVaultCount], name);
            labels[sVaultCount] = texts[sVaultCount];
            sVaultIds[sVaultCount++] = i;
        }
    }
    for (u8 i = 0; i < COFRADIA_VAULT_ITEM_CAPACITY; i++)
    {
        enum Item item;
        u16 count;
        u32 value;
        if (CofradiaVaultDescribeItem(i, &item, &count, &value))
        {
            // Keep list labels short; full name/quantity/value in the detail view.
            CopyItemName(item, texts[sVaultCount]);
            labels[sVaultCount] = texts[sVaultCount];
            sVaultIds[sVaultCount++] = COFRADIA_VAULT_MON_CAPACITY + i;
        }
    }
    labels[sVaultCount] = C_TEXT("Back", "Volver");
    gSpecialVar_Result = OpenMenu(labels, sVaultCount + 1);
}

void CofradiaVaultEntryQuote(void)
{
    if (gSpecialVar_0x8004 >= sVaultCount)
    {
        gSpecialVar_Result = 2;
        return;
    }
    sVaultSelection = sVaultIds[gSpecialVar_0x8004];
    u32 value;
    u8 *p;
    if (sVaultSelection < COFRADIA_VAULT_MON_CAPACITY)
    {
        u8 name[POKEMON_NAME_LENGTH + 1];
        if (!CofradiaVaultDescribeMon(sVaultSelection, name, &value))
        {
            gSpecialVar_Result = 2;
            return;
        }
        p = StringCopy(gStringVar4, name);
    }
    else
    {
        enum Item item;
        u16 count;
        if (!CofradiaVaultDescribeItem(sVaultSelection - COFRADIA_VAULT_MON_CAPACITY, &item, &count, &value))
        {
            gSpecialVar_Result = 2;
            return;
        }
        p = CopyItemName(item, gStringVar4);
        p = Number(StringCopy(p, C_TEXT("\nQuantity: ", "\nCantidad: ")), count);
    }
    p = Number(StringCopy(p, C_TEXT("\nValue: ¥", "\nValor: ¥")), value);
    bool32 available = CofradiaVaultReturnAvailable();
    StringCopy(p, available ? C_TEXT("\pReclaim this asset?", "\p¿Recuperar este bien?")
        : C_TEXT("\pPay debt to reclaim.", "\pSalda la deuda para recuperar."));
    gSpecialVar_Result = available;
}

void CofradiaVaultEntryReturn(void)
{
    bool32 ok = sVaultSelection < COFRADIA_VAULT_MON_CAPACITY
        ? CofradiaVaultReturnMon(sVaultSelection)
        : CofradiaVaultReturnItem(sVaultSelection - COFRADIA_VAULT_MON_CAPACITY);
    Message(ok ? C_TEXT("Asset returned.", "Bien recuperado.")
        : C_TEXT("Return blocked. Asset stays safe.", "Sin espacio o duplicado. Bien retenido."), ok);
}

#define COFRADIA_PAYMENT_PAGE 8
static EWRAM_DATA u16 sPayPageStart = 1;
static EWRAM_DATA u16 sPayPageNext = 1;
static EWRAM_DATA u16 sPayItemIds[COFRADIA_PAYMENT_PAGE] = {0};
static EWRAM_DATA u8 sPayItemRows = 0;
static EWRAM_DATA enum Item sPayItem = ITEM_NONE;
static EWRAM_DATA u16 sPayCount = 0;

static u16 ItemPaymentMax(enum Item item)
{
    struct CofradiaView ledger;
    u32 unit = CofradiaAppraiseItem(item);
    if (!unit || !CofradiaGetView(&ledger) || ledger.state != COFRADIA_IDLE)
        return 0;
    u32 count = min(ledger.debt / unit, CountTotalItemQuantityInBag(item));
    if (GetItemPocket(item) == POCKET_KEY_ITEMS)
        count = min(count, 1u);
    return count;
}

void CofradiaItemPaymentReset(void)
{
    sPayPageStart = 1;
    sPayItem = ITEM_NONE;
    sPayCount = 0;
}

void CofradiaItemPaymentMenu(void)
{
    u8 names[COFRADIA_PAYMENT_PAGE][48];
    const u8 *labels[COFRADIA_PAYMENT_PAGE + 2];
    sPayItemRows = 0;
    struct CofradiaView ledger;
    if (!CofradiaGetView(&ledger) || ledger.state != COFRADIA_IDLE || !ledger.debt)
    {
        Message(C_TEXT("No payment available.", "No hay pago disponible."), FALSE);
        return;
    }
    u16 id = sPayPageStart;
    for (; id < ITEMS_COUNT && sPayItemRows < COFRADIA_PAYMENT_PAGE; id++)
    {
        u32 unit = CofradiaAppraiseItem(id);
        if (!unit || unit > ledger.debt || !CheckBagHasItem(id, 1))
            continue;
        CopyItemName(id, names[sPayItemRows]);
        labels[sPayItemRows] = names[sPayItemRows];
        sPayItemIds[sPayItemRows++] = id;
    }
    sPayPageNext = id < ITEMS_COUNT ? id : 1;
    labels[sPayItemRows] = C_TEXT("Next / restart", "Siguiente / inicio");
    labels[sPayItemRows + 1] = C_TEXT("Back", "Volver");
    gSpecialVar_Result = OpenMenu(labels, sPayItemRows + 2);
}

void CofradiaItemPaymentChoose(void)
{
    u16 row = gSpecialVar_0x8004;
    if (row == sPayItemRows)
    {
        sPayPageStart = sPayPageNext;
        gSpecialVar_Result = 2;
    }
    else if (row < sPayItemRows)
    {
        sPayItem = sPayItemIds[row];
        gSpecialVar_Result = ItemPaymentMax(sPayItem) != 0;
    }
    else
        gSpecialVar_Result = FALSE;
}

void CofradiaItemPaymentQuantityMenu(void)
{
    const u8 *labels[] = {C_TEXT("One unit", "Una unidad"), C_TEXT("Maximum", "Máximo sin exceder deuda"), C_TEXT("Back", "Volver")};
    gSpecialVar_Result = OpenMenu(labels, ARRAY_COUNT(labels));
}

void CofradiaItemPaymentQuote(void)
{
    u16 maxCount = ItemPaymentMax(sPayItem);
    if (!maxCount || gSpecialVar_0x8004 > 1)
    {
        Message(C_TEXT("No payment available.", "No hay pago disponible."), FALSE);
        return;
    }
    sPayCount = gSpecialVar_0x8004 == 0 ? 1 : maxCount;
    u8 *p = CopyItemName(sPayItem, gStringVar4);
    p = Number(StringCopy(p, C_TEXT("\nQuantity: ", "\nCantidad: ")), sPayCount);
    p = Number(StringCopy(p, C_TEXT("\pDebt reduction: ¥", "\pAbono: ¥")), CofradiaAppraiseItem(sPayItem) * sPayCount);
    StringCopy(p, C_TEXT("\nPermanent payment, no return.\pConfirm?", "\nPago definitivo, sin devolución.\p¿Confirmas?"));
    gSpecialVar_Result = TRUE;
}

void CofradiaItemPaymentApply(void)
{
    bool32 ok = CofradiaRepayItem(sPayItem, sPayCount) == COFRADIA_OK;
    sPayItem = ITEM_NONE;
    sPayCount = 0;
    Message(ok ? C_TEXT("Item payment recorded.", "Pago con objetos registrado.")
        : C_TEXT("Payment unavailable.", "Pago no disponible."), ok);
}

void CofradiaDefaultScript(void)
{
    bool32 ok = CofradiaProcessDefault();
    Message(ok ? C_TEXT("Collateral held if available.\pInspect the vault and debt.", "Garantías retenidas si disponibles.\pConsulta la bóveda y la deuda.")
        : C_TEXT("No new collection.", "Sin nueva retención."), ok);
}
