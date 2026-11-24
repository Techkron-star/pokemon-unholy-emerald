#include "global.h"
#include "horizon_eggs.h"
#include "mutation_tiers.h"
#include "config/special_badges.h"
#include "daycare.h"
#include "event_data.h"
#include "money.h"
#include "pokemon.h"
#include "random.h"
#include "strings.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"

// Five visible EGGS, not revealed Pokemon. Each grade is rolled independently
// at market refresh and saved. Species and genetics are rolled on purchase.
#define HORIZON_EGG_SOLD_BIT(tier) (1 << (10 + (tier)))
#define HORIZON_EGG_GRADE_SHIFT(tier) ((tier) * 2)
#define HORIZON_EGG_MAX_MUTATIONS 3

struct HorizonEggMutationOdds
{
    enum PokemonBadge badge;
    u16 denominator;
};

static const struct HorizonEggMutationOdds sEggMutationOdds[] =
{
    {POKEMON_BADGE_ALPHA, SPECIAL_BADGE_ALPHA_DENOMINATOR},
    {POKEMON_BADGE_SUPER, SPECIAL_BADGE_SUPER_DENOMINATOR},
    {POKEMON_BADGE_ENRAGED, SPECIAL_BADGE_ENRAGED_DENOMINATOR},
    {POKEMON_BADGE_OMEGA, SPECIAL_BADGE_OMEGA_DENOMINATOR},
    {POKEMON_BADGE_HYPER, SPECIAL_BADGE_HYPER_DENOMINATOR},
    {POKEMON_BADGE_OVERLOADED, SPECIAL_BADGE_OVERLOADED_DENOMINATOR},
    {POKEMON_BADGE_TITAN, SPECIAL_BADGE_TITAN_DENOMINATOR},
    {POKEMON_BADGE_ANCESTRAL, SPECIAL_BADGE_ANCESTRAL_DENOMINATOR},
    {POKEMON_BADGE_ASCENDED, SPECIAL_BADGE_ASCENDED_DENOMINATOR},
};

static const enum Species sEggBasic[] =
{
    SPECIES_ZIGZAGOON, SPECIES_POOCHYENA, SPECIES_WURMPLE,
    SPECIES_TAILLOW, SPECIES_WHISMUR, SPECIES_LOTAD, SPECIES_SEEDOT,
};
static const enum Species sEggMedium[] =
{
    SPECIES_RALTS, SPECIES_SHROOMISH, SPECIES_MAKUHITA,
    SPECIES_ELECTRIKE, SPECIES_NUMEL, SPECIES_SWABLU, SPECIES_TRAPINCH,
};
static const enum Species sEggHigh[] =
{
    SPECIES_DRATINI, SPECIES_LARVITAR, SPECIES_BAGON,
    SPECIES_BELDUM, SPECIES_GIBLE, SPECIES_ABSOL, SPECIES_FEEBAS,
};
static const enum Species sEggMythic[] =
{
    SPECIES_MEW, SPECIES_CELEBI, SPECIES_JIRACHI,
    SPECIES_MANAPHY, SPECIES_DARKRAI, SPECIES_VICTINI,
};
static const enum Species sEggLegendary[] =
{
    SPECIES_ARTICUNO, SPECIES_ZAPDOS, SPECIES_MOLTRES,
    SPECIES_RAIKOU, SPECIES_ENTEI, SPECIES_SUICUNE,
    SPECIES_LATIAS, SPECIES_LATIOS, SPECIES_LUGIA,
    SPECIES_KYOGRE, SPECIES_GROUDON, SPECIES_RAYQUAZA,
};

static const u8 sEggTierBasic[] = _("BASIC");
static const u8 sEggTierMedium[] = _("MEDIUM");
static const u8 sEggTierHigh[] = _("HIGH");
static const u8 sEggTierMythic[] = _("MYTHIC");
static const u8 sEggTierLegendary[] = _("LEGENDARY");
static const u8 *const sEggTierNames[] =
{
    sEggTierBasic, sEggTierMedium, sEggTierHigh, sEggTierMythic, sEggTierLegendary,
};

static const u8 sEggQualityStandard[] = _("STANDARD");
static const u8 sEggQualityRadiant[] = _("RADIANT");
static const u8 sEggQualityPrismatic[] = _("PRISMATIC");
static const u8 *const sEggQualityNames[] =
{
    sEggQualityStandard, sEggQualityRadiant, sEggQualityPrismatic,
};

// Special visual quality is a *separate* RNG roll for each visible egg.
// Values are denominators; lower numbers are more frequent.
static const u16 sRadiantOdds[HORIZON_EGG_TIER_COUNT] = {32, 26, 20, 15, 12};
static const u16 sPrismaticOdds[HORIZON_EGG_TIER_COUNT] = {400, 320, 240, 160, 120};
static const u32 sBaseEggPrices[HORIZON_EGG_TIER_COUNT] = {1500, 6000, 20000, 75000, 140000};

// Chance per 10,000 to win one of the two highest species categories.
// Even a basic egg has a minuscule chance to hatch a legendary.
static const u16 sLegendaryChances[HORIZON_EGG_TIER_COUNT] = {1, 3, 15, 1400, 7000};
static const u16 sMythicChances[HORIZON_EGG_TIER_COUNT] = {2, 10, 60, 6500, 3000};

const u8 *HorizonEggTierName(u8 tier)
{
    if (tier >= HORIZON_EGG_TIER_COUNT)
        return sEggTierNames[0];
    return sEggTierNames[tier];
}

const u8 *HorizonEggQualityName(u8 quality)
{
    if (quality > HORIZON_EGG_PRISMATIC)
        quality = HORIZON_EGG_STANDARD;
    return sEggQualityNames[quality];
}

void HorizonEggsRollStock(void)
{
    u16 stock = 0;

    for (u8 tier = 0; tier < HORIZON_EGG_TIER_COUNT; tier++)
    {
        u8 quality = HORIZON_EGG_STANDARD;

        if (Random32() % sPrismaticOdds[tier] == 0)
            quality = HORIZON_EGG_PRISMATIC;
        else if (Random32() % sRadiantOdds[tier] == 0)
            quality = HORIZON_EGG_RADIANT;
        stock |= (u16)quality << HORIZON_EGG_GRADE_SHIFT(tier);
    }
    VarSet(VAR_HORIZON_EGG_QUALITIES, stock);
}

u8 HorizonEggGetQuality(u8 tier)
{
    if (tier >= HORIZON_EGG_TIER_COUNT)
        return HORIZON_EGG_STANDARD;

    return (VarGet(VAR_HORIZON_EGG_QUALITIES) >> HORIZON_EGG_GRADE_SHIFT(tier)) & 3;
}

bool8 HorizonEggIsSold(u8 tier)
{
    return tier >= HORIZON_EGG_TIER_COUNT
        || (VarGet(VAR_HORIZON_EGG_QUALITIES) & HORIZON_EGG_SOLD_BIT(tier)) != 0;
}

u32 HorizonEggGetPrice(u8 tier)
{
    u8 quality;

    if (HorizonEggIsSold(tier))
        return 0;

    quality = HorizonEggGetQuality(tier);
    if (quality == HORIZON_EGG_PRISMATIC)
        return sBaseEggPrices[tier] * 4;
    if (quality == HORIZON_EGG_RADIANT)
        return sBaseEggPrices[tier] * 2;
    return sBaseEggPrices[tier];
}

static enum Species HorizonEggRollSpecies(u8 tier, u8 quality)
{
    u32 factor = quality == HORIZON_EGG_PRISMATIC ? 5
               : quality == HORIZON_EGG_RADIANT ? 2 : 1;
    u32 legendary = sLegendaryChances[tier] * factor;
    u32 mythical = sMythicChances[tier] * factor;
    u32 roll;

    if (legendary + mythical > 10000)
    {
        legendary = legendary * 10000 / (legendary + mythical);
        mythical = 10000 - legendary;
    }

    roll = Random32() % 10000;
    if (roll < legendary)
        return sEggLegendary[Random32() % ARRAY_COUNT(sEggLegendary)];
    if (roll < legendary + mythical)
        return sEggMythic[Random32() % ARRAY_COUNT(sEggMythic)];

    switch (tier)
    {
    case HORIZON_EGG_LEGENDARY:
        return sEggLegendary[Random32() % ARRAY_COUNT(sEggLegendary)];
    case HORIZON_EGG_MYTHIC:
        return sEggMythic[Random32() % ARRAY_COUNT(sEggMythic)];
    case HORIZON_EGG_HIGH:
        return sEggHigh[Random32() % ARRAY_COUNT(sEggHigh)];
    case HORIZON_EGG_MEDIUM:
        // Small chance to advance into the high species pool.
        if (Random32() % 10 == 0)
            return sEggHigh[Random32() % ARRAY_COUNT(sEggHigh)];
        return sEggMedium[Random32() % ARRAY_COUNT(sEggMedium)];
    default:
        // Basic eggs can sometimes hatch a medium-tier species.
        if (Random32() % 20 == 0)
            return sEggMedium[Random32() % ARRAY_COUNT(sEggMedium)];
        return sEggBasic[Random32() % ARRAY_COUNT(sEggBasic)];
    }
}

static u16 HorizonEggRollMutations(u8 tier, u8 quality)
{
    u16 badges = 0;
    u32 factor = 1 + tier + quality * 2;
    u8 count = 0;

    for (u8 i = 0; i < ARRAY_COUNT(sEggMutationOdds); i++)
    {
        const struct HorizonEggMutationOdds *entry = &sEggMutationOdds[i];
        u32 denominator = max(1, entry->denominator / factor);

        // Eggs may roll *every* known tier, including S and Omega. Never
        // confuse a high egg quality with a guaranteed mutation tier.
        if (MutationTierFromBadge(entry->badge) == MUTATION_TIER_UNKNOWN)
            continue;
        if (Random32() % denominator != 0)
            continue;

        // Combinations are allowed, but additional badges pass the same
        // progressively harsher gates as regular encounter mutation stacks.
        if (count > 0)
        {
            u32 gate = count == 1 ? SPECIAL_BADGE_SECOND_GATE : SPECIAL_BADGE_THIRD_GATE;
            if (Random32() % gate != 0)
                continue;
        }

        badges |= entry->badge;
        if (++count >= HORIZON_EGG_MAX_MUTATIONS)
            break;
    }
    return badges;
}

enum HorizonMarketPurchaseResult HorizonEggPurchase(u8 tier)
{
    struct Pokemon egg;
    enum Species species;
    u8 quality;
    u16 mutations;
    u16 stock;
    u32 price;
    u32 cycles;

    if (HorizonEggIsSold(tier))
        return HORIZON_MARKET_PURCHASE_SOLD_OUT;
    if (IsPlayerPartyAndPokemonStorageFull())
        return HORIZON_MARKET_PURCHASE_NO_SPACE;

    price = HorizonEggGetPrice(tier);
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
        return HORIZON_MARKET_PURCHASE_NO_MONEY;

    quality = HorizonEggGetQuality(tier);
    species = HorizonEggRollSpecies(tier, quality);
    mutations = HorizonEggRollMutations(tier, quality);
    CreateEgg(&egg, species, TRUE);

    // Non-breedable mythical/legendary species still need a viable hatch timer.
    cycles = GetMonData(&egg, MON_DATA_FRIENDSHIP);
    if (cycles == 0)
    {
        cycles = 35;
        SetMonData(&egg, MON_DATA_FRIENDSHIP, &cycles);
    }

    if (mutations)
    {
        SetMonData(&egg, MON_DATA_SPECIAL_BADGES, &mutations);
        CalculateMonStats(&egg);
    }

    if (GiveCapturedMonToPlayer(&egg) == MON_CANT_GIVE)
        return HORIZON_MARKET_PURCHASE_FAILED;

    stock = VarGet(VAR_HORIZON_EGG_QUALITIES);
    VarSet(VAR_HORIZON_EGG_QUALITIES, stock | HORIZON_EGG_SOLD_BIT(tier));
    RemoveMoney(&gSaveBlock1Ptr->money, price);
    return HORIZON_MARKET_PURCHASE_OK;
}
