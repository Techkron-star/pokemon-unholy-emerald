#include "global.h"
#include "horizon_market.h"
#include "config/special_badges.h"
#include "event_data.h"
#include "money.h"
#include "random.h"
#include "rtc.h"
#include "strings.h"
#include "constants/flags.h"
#include "constants/pokemon.h"
#include "constants/vars.h"

// Stock occupies eight previously unused save variables; no SaveBlock expansion.
// A mutation code is one nibble per slot; bits 8 and 9 mark sold offers.
// A new period never rolls again just because the player reopens the menu.
#define HORIZON_STOCK_VERSION       0x484D
#define HORIZON_MARKET_SOLD(slot)   (1 << (8 + (slot)))
#define HORIZON_MARKET_MUT_SHIFT(slot) ((slot) * 4)

struct HorizonSpeciesOffer
{
    enum Species species;
    u8 minBadges;
    u8 timeRestriction; // 0 = any, 1 = day, 2 = night
};

struct HorizonMutationRoll
{
    enum PokemonBadge mutation;
    u16 denominator;
    u8 minBadges;
};

static const struct HorizonSpeciesOffer sHorizonSpeciesPool[] =
{
    {SPECIES_ZIGZAGOON, 0, 0},
    {SPECIES_POOCHYENA, 0, 2},
    {SPECIES_WURMPLE, 0, 1},
    {SPECIES_TAILLOW, 0, 1},
    {SPECIES_WHISMUR, 0, 0},
    {SPECIES_SHROOMISH, 1, 1},
    {SPECIES_RALTS, 1, 0},
    {SPECIES_MAKUHITA, 1, 0},
    {SPECIES_ELECTRIKE, 2, 1},
    {SPECIES_NUMEL, 2, 0},
    {SPECIES_SABLEYE, 3, 2},
    {SPECIES_MAWILE, 3, 1},
    {SPECIES_SWABLU, 3, 1},
    {SPECIES_TRAPINCH, 4, 0},
    {SPECIES_ABSOL, 5, 2},
    {SPECIES_BAGON, 7, 0},
    {SPECIES_BELDUM, 8, 0},
};

// Configured wild encounter odds are also the baseline for market rarity.
// Market availability is stricter: the rarer categories need more badges.
// One mutation per marketed individual in this first stock format.
static const struct HorizonMutationRoll sHorizonMutationRolls[] =
{
    {POKEMON_BADGE_ALPHA,      SPECIAL_BADGE_ALPHA_DENOMINATOR, 0},
    {POKEMON_BADGE_SUPER,      SPECIAL_BADGE_SUPER_DENOMINATOR, 2},
    {POKEMON_BADGE_ENRAGED,    SPECIAL_BADGE_ENRAGED_DENOMINATOR, 4},
    {POKEMON_BADGE_OMEGA,      SPECIAL_BADGE_OMEGA_DENOMINATOR, 8},
    {POKEMON_BADGE_HYPER,      SPECIAL_BADGE_HYPER_DENOMINATOR, 1},
    {POKEMON_BADGE_OVERLOADED, SPECIAL_BADGE_OVERLOADED_DENOMINATOR, 2},
    {POKEMON_BADGE_TITAN,      SPECIAL_BADGE_TITAN_DENOMINATOR, 3},
    {POKEMON_BADGE_ANCESTRAL,  SPECIAL_BADGE_ANCESTRAL_DENOMINATOR, 5},
    {POKEMON_BADGE_ASCENDED,   SPECIAL_BADGE_ASCENDED_DENOMINATOR, 7},
};

static const u8 sHorizonNameNormal[] = _("NORMAL");
static const u8 sHorizonNameAlpha[] = _("ALPHA");
static const u8 sHorizonNameSuper[] = _("SUPER");
static const u8 sHorizonNameEnraged[] = _("ENRAGED");
static const u8 sHorizonNameOmega[] = _("OMEGA");
static const u8 sHorizonNameHyper[] = _("HYPER");
static const u8 sHorizonNameOverloaded[] = _("OVERLOADED");
static const u8 sHorizonNameTitan[] = _("TITAN");
static const u8 sHorizonNameAncestral[] = _("ANCESTRAL");
static const u8 sHorizonNameAscended[] = _("ASCENDED");
static const u8 *const sHorizonMutationNames[] =
{
    sHorizonNameNormal, sHorizonNameAlpha, sHorizonNameSuper,
    sHorizonNameEnraged, sHorizonNameOmega, sHorizonNameHyper,
    sHorizonNameOverloaded, sHorizonNameTitan,
    sHorizonNameAncestral, sHorizonNameAscended,
};

static u8 HorizonMarketBadgeCount(void)
{
    u8 count = 0;

    for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        if (FlagGet(flag))
            count++;
    return count;
}

static u8 HorizonMarketPeriod(void)
{
    enum TimeOfDay time = GetTimeOfDay();
    return time == TIME_NIGHT || time == TIME_EVENING ? 1 : 0;
}

static enum Species HorizonMarketRollSpecies(u8 badges, u8 period)
{
    u16 count = 0;
    u16 chosen;

    for (u16 i = 0; i < ARRAY_COUNT(sHorizonSpeciesPool); i++)
    {
        const struct HorizonSpeciesOffer *entry = &sHorizonSpeciesPool[i];
        if (entry->minBadges <= badges
         && (entry->timeRestriction == 0 || entry->timeRestriction == period + 1))
            count++;
    }

    // The pool always contains at least one unrestricted starter-route species.
    if (count == 0)
        return SPECIES_ZIGZAGOON;

    chosen = Random32() % count;
    for (u16 i = 0; i < ARRAY_COUNT(sHorizonSpeciesPool); i++)
    {
        const struct HorizonSpeciesOffer *entry = &sHorizonSpeciesPool[i];
        if (entry->minBadges <= badges
         && (entry->timeRestriction == 0 || entry->timeRestriction == period + 1))
        {
            if (chosen-- == 0)
                return entry->species;
        }
    }
    return SPECIES_ZIGZAGOON;
}

static u8 HorizonMarketRollMutation(u8 badges)
{
    u8 result = 0;
    u16 highestRarity = 0;

    for (u8 i = 0; i < ARRAY_COUNT(sHorizonMutationRolls); i++)
    {
        const struct HorizonMutationRoll *entry = &sHorizonMutationRolls[i];
        if (badges >= entry->minBadges && Random32() % entry->denominator == 0
         && entry->denominator > highestRarity)
        {
            result = i + 1;
            highestRarity = entry->denominator;
        }
    }
    return result;
}

static u16 HorizonMarketSpeciesVar(u8 slot)
{
    return slot == 0 ? VAR_HORIZON_MARKET_SPECIES_0 : VAR_HORIZON_MARKET_SPECIES_1;
}

void HorizonMarketRefresh(void)
{
    u16 day = (u16)RtcGetLocalDayCount();
    u8 period = HorizonMarketPeriod();
    u8 badges = HorizonMarketBadgeCount();

    if (VarGet(VAR_HORIZON_MARKET_VERSION) == HORIZON_STOCK_VERSION
     && VarGet(VAR_HORIZON_MARKET_BADGES) == badges)
    {
        u16 lastDay = VarGet(VAR_HORIZON_MARKET_DAY);
        u16 lastPeriod = VarGet(VAR_HORIZON_MARKET_PERIOD);

        // Never allow a backwards clock adjustment to farm mutation rolls.
        if (day < lastDay || (day == lastDay && period <= lastPeriod))
            return;
    }

    u16 mutationCodes = 0;
    enum Species first = SPECIES_NONE;

    for (u8 i = 0; i < HORIZON_MARKET_OFFER_COUNT; i++)
    {
        enum Species species = HorizonMarketRollSpecies(badges, period);
        u8 mutation = HorizonMarketRollMutation(badges);

        if (i == 1 && species == first)
        {
            for (u8 retry = 0; retry < 4 && species == first; retry++)
                species = HorizonMarketRollSpecies(badges, period);
        }
        if (i == 0)
            first = species;

        VarSet(HorizonMarketSpeciesVar(i), species);
        mutationCodes |= mutation << HORIZON_MARKET_MUT_SHIFT(i);
    }

    VarSet(VAR_HORIZON_MARKET_MUTATIONS, mutationCodes);
    VarSet(VAR_HORIZON_MARKET_DAY, day);
    VarSet(VAR_HORIZON_MARKET_PERIOD, period);
    VarSet(VAR_HORIZON_MARKET_BADGES, badges);
    VarSet(VAR_HORIZON_MARKET_SEEN, 0);
    VarSet(VAR_HORIZON_MARKET_VERSION, HORIZON_STOCK_VERSION);
}

enum Species HorizonMarketGetSpecies(u8 slot)
{
    if (slot >= HORIZON_MARKET_OFFER_COUNT)
        return SPECIES_NONE;
    return VarGet(HorizonMarketSpeciesVar(slot));
}

bool8 HorizonMarketOfferSold(u8 slot)
{
    if (slot >= HORIZON_MARKET_OFFER_COUNT)
        return TRUE;
    return (VarGet(VAR_HORIZON_MARKET_MUTATIONS) & HORIZON_MARKET_SOLD(slot)) != 0;
}

static u8 HorizonMarketGetMutationCode(u8 slot)
{
    u16 codes;

    if (slot >= HORIZON_MARKET_OFFER_COUNT)
        return 0;

    codes = VarGet(VAR_HORIZON_MARKET_MUTATIONS);
    if (codes & HORIZON_MARKET_SOLD(slot))
        return 0;
    return (codes >> HORIZON_MARKET_MUT_SHIFT(slot)) & 0xF;
}

enum PokemonBadge HorizonMarketGetMutation(u8 slot)
{
    u8 code = HorizonMarketGetMutationCode(slot);

    if (code == 0 || code > ARRAY_COUNT(sHorizonMutationRolls))
        return (enum PokemonBadge)0;
    return sHorizonMutationRolls[code - 1].mutation;
}

const u8 *HorizonMarketMutationName(enum PokemonBadge mutation)
{
    for (u8 i = 0; i < ARRAY_COUNT(sHorizonMutationRolls); i++)
    {
        if (sHorizonMutationRolls[i].mutation == mutation)
            return sHorizonMutationNames[i + 1];
    }
    return sHorizonMutationNames[0];
}

bool8 HorizonMarketHasUnseenMutation(void)
{
    HorizonMarketRefresh();
    if (VarGet(VAR_HORIZON_MARKET_SEEN))
        return FALSE;

    for (u8 i = 0; i < HORIZON_MARKET_OFFER_COUNT; i++)
    {
        if (HorizonMarketGetSpecies(i) != SPECIES_NONE && HorizonMarketGetMutationCode(i) != 0)
            return TRUE;
    }
    return FALSE;
}

void HorizonMarketMarkPokemonSeen(void)
{
    VarSet(VAR_HORIZON_MARKET_SEEN, 1);
}

u32 HorizonMarketGetPrice(u8 slot)
{
    enum PokemonBadge badge;

    if (HorizonMarketGetSpecies(slot) == SPECIES_NONE)
        return 0;

    badge = HorizonMarketGetMutation(slot);
    u32 base = 2000 + HorizonMarketBadgeCount() * 1200;

    if (badge == POKEMON_BADGE_OMEGA)
        return base + 180000;
    if (badge == POKEMON_BADGE_ASCENDED)
        return base + 45000;
    if (badge == POKEMON_BADGE_ENRAGED || badge == POKEMON_BADGE_ANCESTRAL)
        return base + 20000;
    if (badge != 0)
        return base + 7000;
    return base;
}

enum HorizonMarketPurchaseResult HorizonMarketPurchase(u8 slot)
{
    struct Pokemon mon;
    enum Species species = HorizonMarketGetSpecies(slot);
    u32 cost = HorizonMarketGetPrice(slot);
    enum PokemonBadge mutation;
    u16 stock;

    if (slot >= HORIZON_MARKET_OFFER_COUNT || species == SPECIES_NONE || cost == 0)
        return HORIZON_MARKET_PURCHASE_SOLD_OUT;
    stock = VarGet(VAR_HORIZON_MARKET_MUTATIONS);
    if (stock & HORIZON_MARKET_SOLD(slot))
        return HORIZON_MARKET_PURCHASE_SOLD_OUT;
    if (IsPlayerPartyAndPokemonStorageFull())
        return HORIZON_MARKET_PURCHASE_NO_SPACE;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, cost))
        return HORIZON_MARKET_PURCHASE_NO_MONEY;

    mutation = HorizonMarketGetMutation(slot);
    CreateRandomMon(&mon, species, 8 + HorizonMarketBadgeCount() * 4);
    if (mutation != 0)
    {
        Pokemon_SetBadge(&mon, mutation);
        CalculateMonStats(&mon);
    }

    if (GiveCapturedMonToPlayer(&mon) == MON_CANT_GIVE)
        return HORIZON_MARKET_PURCHASE_FAILED;

    RemoveMoney(&gSaveBlock1Ptr->money, cost);
    VarSet(VAR_HORIZON_MARKET_MUTATIONS, stock | HORIZON_MARKET_SOLD(slot));
    return HORIZON_MARKET_PURCHASE_OK;
}
