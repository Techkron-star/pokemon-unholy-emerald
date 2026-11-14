#include "global.h"
#include "pokedex_variants.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"

STATIC_ASSERT(NUM_STATS + 1 == POKEDEX_VARIANT_STATS, DexVariantStatsIncludeStamina);

static const u16 sBadges[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER,
    POKEMON_BADGE_OVERLOADED, POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
    POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
};
STATIC_ASSERT(ARRAY_COUNT(sBadges) == POKEDEX_VARIANT_COUNT, DexVariantCountMatchesBadges);

u16 PokedexVariants_BadgeAt(u8 index)
{
    return index < ARRAY_COUNT(sBadges) ? sBadges[index] : 0;
}

void PokedexVariants_Init(struct PokedexVariantCollection *collection, u16 species)
{
    memset(collection, 0, sizeof(*collection));
    if (species > SPECIES_NONE && species < NUM_SPECIES)
        collection->species = species;
}

void PokedexVariants_Add(struct PokedexVariantCollection *collection, const struct PokedexVariantRecord *record)
{
    if (!collection->species || record->species != collection->species || record->egg || record->invalid
     || collection->total == 0xFFFF)
        return;

    collection->total++;
    if (record->inParty) collection->party++;
    else collection->boxes++;
    if (record->mutations == 0) collection->normal++;
    if (record->mutations & ~POKEMON_BADGES_ALL) collection->unknown++;
    u8 count = 0;
    for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
    {
        if (!(record->mutations & sBadges[i]))
            continue;
        struct PokedexVariantBucket *bucket = &collection->variants[i];
        for (u8 stat = 0; stat < POKEDEX_VARIANT_STATS; stat++)
        {
            struct PokedexVariantRange *range = &bucket->bonuses[stat];
            if (bucket->count == 0)
                range->minimum = range->maximum = record->bonuses[stat];
            else
            {
                range->minimum = min(range->minimum, record->bonuses[stat]);
                range->maximum = max(range->maximum, record->bonuses[stat]);
            }
        }
        bucket->count++;
        count++;
    }
    if (count > 1) collection->combined++;
}

void PokedexVariants_AddBoxMon(struct PokedexVariantCollection *collection, const struct BoxPokemon *source, bool8 inParty)
{
    if (collection->species == SPECIES_NONE || !source->hasSpecies || source->isBadEgg || source->isEgg)
        return;
    struct Pokemon copy = {0};
    copy.box = *source;
    struct PokedexVariantRecord record = {.inParty = inParty};
    record.species = GetMonData(&copy, MON_DATA_SPECIES);
    // The getter validates the copied encrypted checksum. Never decrypt the real slot.
    if (copy.box.isBadEgg || record.species != collection->species || GetMonData(&copy, MON_DATA_IS_EGG))
        return;
    record.mutations = GetMonData(&copy, MON_DATA_SPECIAL_BADGES);
    for (u8 stat = 0; stat < NUM_STATS; stat++)
        record.bonuses[stat] = Pokemon_GetMutationStatBonusPercent(&copy, stat);
    record.bonuses[NUM_STATS] = Pokemon_GetMutationStaminaBonusPercent(&copy);
    PokedexVariants_Add(collection, &record);
}

u16 PokedexVariants_ScanTotal(void)
{
    return PARTY_SIZE + TOTAL_BOXES_COUNT * IN_BOX_COUNT;
}

bool32 PokedexVariants_ScanStep(struct PokedexVariantCollection *collection, u16 *cursor, u16 budget)
{
    if (!collection->species)
        return TRUE;
    while (*cursor < PokedexVariants_ScanTotal() && budget != 0)
    {
        u16 position = (*cursor)++;
        bool8 inParty = position < PARTY_SIZE;
        const struct BoxPokemon *source;
        if (inParty)
        {
            if (position >= min(gPartiesCount[B_TRAINER_PLAYER], PARTY_SIZE))
                continue;
            source = &gParties[B_TRAINER_PLAYER][position].box;
        }
        else
        {
            position -= PARTY_SIZE;
            source = GetBoxedMonPtr(position / IN_BOX_COUNT, position % IN_BOX_COUNT);
        }
        if (!source->hasSpecies)
            continue;
        PokedexVariants_AddBoxMon(collection, source, inParty);
        budget--;
    }
    return *cursor >= PokedexVariants_ScanTotal();
}

void PokedexVariants_ReadCollection(struct PokedexVariantCollection *collection, u16 species)
{
    PokedexVariants_Init(collection, species);
    u16 cursor = 0;
    PokedexVariants_ScanStep(collection, &cursor, 0xFFFF);
}
