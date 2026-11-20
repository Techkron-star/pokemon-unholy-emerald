#include "global.h"
#include "event_data.h"
#include "random.h"
#include "unholy_starters.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"

#define STARTERS_PER_TYPE 8
#define STARTER_OFFER_COUNT 3

// Deliberately curated stage-one Pokémon, not every Pokémon of a type.
// Their initial power is modest and none is legendary, mythical, evolved or
// has an unusually high base-stage stat total. Dragon includes some slow-
// growing pseudo-legendary first stages, but never their final forms.
// Order follows SCROLL_MULTI_UNHOLY_STARTER_TYPES in field_specials.c.
static const u16 sStarterTypes[UNHOLY_STARTER_TYPE_COUNT] =
{
    TYPE_NORMAL, TYPE_FIRE, TYPE_WATER, TYPE_ELECTRIC, TYPE_GRASS, TYPE_ICE,
    TYPE_FIGHTING, TYPE_POISON, TYPE_GROUND, TYPE_FLYING, TYPE_PSYCHIC,
    TYPE_BUG, TYPE_ROCK, TYPE_GHOST, TYPE_DRAGON, TYPE_DARK, TYPE_STEEL,
    TYPE_FAIRY,
};

static const u16 sStarterPools[UNHOLY_STARTER_TYPE_COUNT][STARTERS_PER_TYPE] =
{
    // NORMAL
    {SPECIES_RATTATA, SPECIES_SENTRET, SPECIES_ZIGZAGOON, SPECIES_BIDOOF,
     SPECIES_LILLIPUP, SPECIES_BUNNELBY, SPECIES_SKWOVET, SPECIES_EEVEE},
    // FIRE
    {SPECIES_CHARMANDER, SPECIES_CYNDAQUIL, SPECIES_TORCHIC, SPECIES_CHIMCHAR,
     SPECIES_TEPIG, SPECIES_FENNEKIN, SPECIES_LITTEN, SPECIES_SCORBUNNY},
    // WATER
    {SPECIES_SQUIRTLE, SPECIES_TOTODILE, SPECIES_MUDKIP, SPECIES_PIPLUP,
     SPECIES_OSHAWOTT, SPECIES_FROAKIE, SPECIES_POPPLIO, SPECIES_SOBBLE},
    // ELECTRIC
    {SPECIES_PICHU, SPECIES_ELEKID, SPECIES_MAREEP, SPECIES_SHINX,
     SPECIES_ELECTRIKE, SPECIES_JOLTIK, SPECIES_TYNAMO, SPECIES_PAWMI},
    // GRASS
    {SPECIES_BULBASAUR, SPECIES_CHIKORITA, SPECIES_TREECKO, SPECIES_TURTWIG,
     SPECIES_SNIVY, SPECIES_CHESPIN, SPECIES_ROWLET, SPECIES_GROOKEY},
    // ICE
    {SPECIES_SWINUB, SPECIES_SPHEAL, SPECIES_SNORUNT, SPECIES_VANILLITE,
     SPECIES_CUBCHOO, SPECIES_BERGMITE, SPECIES_SNOM, SPECIES_AMAURA},
    // FIGHTING
    {SPECIES_MACHOP, SPECIES_MAKUHITA, SPECIES_MEDITITE, SPECIES_RIOLU,
     SPECIES_TIMBURR, SPECIES_MIENFOO, SPECIES_PANCHAM, SPECIES_CRABRAWLER},
    // POISON
    {SPECIES_EKANS, SPECIES_ZUBAT, SPECIES_ODDISH, SPECIES_KOFFING,
     SPECIES_GRIMER, SPECIES_STUNKY, SPECIES_CROAGUNK, SPECIES_SKRELP},
    // GROUND
    {SPECIES_SANDSHREW, SPECIES_DIGLETT, SPECIES_WOOPER, SPECIES_PHANPY,
     SPECIES_TRAPINCH, SPECIES_BALTOY, SPECIES_SANDILE, SPECIES_MUDBRAY},
    // FLYING
    {SPECIES_PIDOVE, SPECIES_STARLY, SPECIES_FLETCHLING, SPECIES_ROOKIDEE,
     SPECIES_WINGULL, SPECIES_HOPPIP, SPECIES_RUFFLET, SPECIES_NOIBAT},
    // PSYCHIC
    {SPECIES_ABRA, SPECIES_RALTS, SPECIES_MEDITITE, SPECIES_SPOINK,
     SPECIES_NATU, SPECIES_SOLOSIS, SPECIES_MUNNA, SPECIES_ESPURR},
    // BUG
    {SPECIES_CATERPIE, SPECIES_WEEDLE, SPECIES_WURMPLE, SPECIES_SEWADDLE,
     SPECIES_VENIPEDE, SPECIES_JOLTIK, SPECIES_GRUBBIN, SPECIES_NYMBLE},
    // ROCK
    {SPECIES_GEODUDE, SPECIES_ONIX, SPECIES_NOSEPASS, SPECIES_ROGGENROLA,
     SPECIES_DWEBBLE, SPECIES_ROCKRUFF, SPECIES_ROLYCOLY, SPECIES_NACLI},
    // GHOST
    {SPECIES_GASTLY, SPECIES_DUSKULL, SPECIES_SHUPPET, SPECIES_LITWICK,
     SPECIES_PHANTUMP, SPECIES_PUMPKABOO, SPECIES_GOLETT, SPECIES_FRILLISH},
    // DRAGON (strong evolutions take more effort, but no static legendaries)
    {SPECIES_FRIGIBAX, SPECIES_AXEW, SPECIES_APPLIN, SPECIES_TYRUNT,
     SPECIES_DRATINI, SPECIES_GIBLE, SPECIES_BAGON, SPECIES_JANGMO_O},
    // DARK
    {SPECIES_HOUNDOUR, SPECIES_POOCHYENA, SPECIES_STUNKY, SPECIES_SANDILE,
     SPECIES_PAWNIARD, SPECIES_INKAY, SPECIES_NICKIT, SPECIES_IMPIDIMP},
    // STEEL
    {SPECIES_MAGNEMITE, SPECIES_ARON, SPECIES_BRONZOR, SPECIES_PAWNIARD,
     SPECIES_KLINK, SPECIES_HONEDGE, SPECIES_FERROSEED, SPECIES_TINKATINK},
    // FAIRY
    {SPECIES_RALTS, SPECIES_SPRITZEE, SPECIES_SNUBBULL, SPECIES_CLEFFA,
     SPECIES_IGGLYBUFF, SPECIES_CUTIEFLY, SPECIES_MILCERY, SPECIES_TINKATINK},
};

static const u16 sStarterOfferVars[STARTER_OFFER_COUNT] =
{
    VAR_UNHOLY_STARTER_OFFER_0,
    VAR_UNHOLY_STARTER_OFFER_1,
    VAR_UNHOLY_STARTER_OFFER_2,
};

u16 UnholyGetStarterOption(u16 slot)
{
    u16 typeChoice = VarGet(VAR_UNHOLY_STARTER_TYPE);
    u16 species;
    u32 i;

    if (slot >= STARTER_OFFER_COUNT || typeChoice == 0 || typeChoice > UNHOLY_STARTER_TYPE_COUNT)
        return SPECIES_NONE;

    species = VarGet(sStarterOfferVars[slot]);
    // Reject stale or corrupt offer data on older saves instead of passing an
    // invalid species ID into the starter sprite and party creation paths.
    for (i = 0; i < STARTERS_PER_TYPE; i++)
    {
        if (species == sStarterPools[typeChoice - 1][i])
            return species;
    }
    return SPECIES_NONE;
}

void UnholyStarterOptionsReady(void)
{
    u16 a = UnholyGetStarterOption(0);
    u16 b = UnholyGetStarterOption(1);
    u16 c = UnholyGetStarterOption(2);

    gSpecialVar_Result = (a != SPECIES_NONE && b != SPECIES_NONE && c != SPECIES_NONE
                          && a != b && a != c && b != c);
}

void UnholySetFavoriteStarterType(void)
{
    u16 typeChoice = gSpecialVar_Result;
    u16 selected[STARTER_OFFER_COUNT];
    u32 i, j;

    if (typeChoice >= UNHOLY_STARTER_TYPE_COUNT)
    {
        gSpecialVar_Result = FALSE;
        return;
    }

    for (i = 0; i < STARTER_OFFER_COUNT; i++)
    {
        u16 index;
        bool8 duplicate;

        do
        {
            index = Random() % STARTERS_PER_TYPE;
            duplicate = FALSE;
            for (j = 0; j < i; j++)
            {
                if (selected[j] == sStarterPools[typeChoice][index])
                    duplicate = TRUE;
            }
        } while (duplicate);

        selected[i] = sStarterPools[typeChoice][index];
        VarSet(sStarterOfferVars[i], selected[i]);
    }

    // Commit the selector last so a partially written trio is never used.
    // Save-vars persist before and after the Route 101 rescue battle.
    VarSet(VAR_UNHOLY_STARTER_TYPE, typeChoice + 1);
    gSpecialVar_Result = TRUE;
}

// Keep the expected type indexes explicit for review and the host validator.
STATIC_ASSERT(ARRAY_COUNT(sStarterTypes) == UNHOLY_STARTER_TYPE_COUNT, StarterTypeCountMustBe18);
STATIC_ASSERT(ARRAY_COUNT(sStarterPools) == UNHOLY_STARTER_TYPE_COUNT, StarterPoolCountMustBe18);
