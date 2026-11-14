#include "global.h"
#include "pokedex_variants.h"
#include "pokemon.h"
#include "random.h"
#include "test/test.h"

TEST("Pokedex variants count a combined individual once and retain each mutation membership")
{
    struct PokedexVariantCollection collection;
    PokedexVariants_Init(&collection, SPECIES_TREECKO);
    struct PokedexVariantRecord record = {.species = SPECIES_TREECKO, .inParty = TRUE,
        .mutations = POKEMON_BADGE_ALPHA | POKEMON_BADGE_HYPER};
    record.bonuses[STAT_SPEED] = -25;
    PokedexVariants_Add(&collection, &record);
    record.inParty = FALSE;
    record.bonuses[STAT_SPEED] = 25;
    PokedexVariants_Add(&collection, &record);
    record.mutations = 0;
    PokedexVariants_Add(&collection, &record);
    EXPECT_EQ(collection.total, 3);
    EXPECT_EQ(collection.normal, 1);
    EXPECT_EQ(collection.combined, 2);
    EXPECT_EQ(collection.party, 1);
    EXPECT_EQ(collection.boxes, 2);
    EXPECT_EQ(collection.variants[0].count, 2);
    EXPECT_EQ(collection.variants[1].count, 2);
    EXPECT_EQ(collection.variants[1].bonuses[STAT_SPEED].minimum, -25);
    EXPECT_EQ(collection.variants[1].bonuses[STAT_SPEED].maximum, 25);
    EXPECT_EQ(collection.variants[8].count, 0);
}

TEST("Pokedex variants exclude eggs invalid slots other species and do not call unknown mutations normal")
{
    struct PokedexVariantCollection collection;
    PokedexVariants_Init(&collection, SPECIES_TREECKO);
    struct PokedexVariantRecord record = {.species = SPECIES_TREECKO, .egg = TRUE, .mutations = POKEMON_BADGE_OMEGA};
    PokedexVariants_Add(&collection, &record);
    record.egg = FALSE;
    record.invalid = TRUE;
    PokedexVariants_Add(&collection, &record);
    record.invalid = FALSE;
    record.species = SPECIES_TORCHIC;
    PokedexVariants_Add(&collection, &record);
    EXPECT_EQ(collection.total, 0);
    record.species = SPECIES_TREECKO;
    record.mutations = 0x8000;
    PokedexVariants_Add(&collection, &record);
    EXPECT_EQ(collection.total, 1);
    EXPECT_EQ(collection.normal, 0);
    EXPECT_EQ(collection.unknown, 1);
    EXPECT_EQ(collection.variants[8].count, 0);
    EXPECT_EQ(PokedexVariants_BadgeAt(POKEDEX_VARIANT_COUNT), 0);
    PokedexVariants_Init(&collection, NUM_SPECIES);
    PokedexVariants_Add(&collection, &record);
    EXPECT_EQ(collection.total, 0);
}

TEST("Pokedex variants observe native combined bonuses without changing the Pokemon or RNG")
{
    struct Pokemon mon;
    struct PokedexVariantCollection collection;
    CreateMonWithIVs(&mon, SPECIES_TREECKO, 25, 0xABCDEF01, OTID_STRUCT_PRESET(1), 31);
    u16 badges = POKEMON_BADGE_HYPER | POKEMON_BADGE_TITAN | POKEMON_BADGE_OMEGA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    struct Pokemon before = mon;
    rng_value_t rng = gRngValue;
    PokedexVariants_Init(&collection, SPECIES_TREECKO);
    PokedexVariants_AddBoxMon(&collection, &mon.box, TRUE);
    EXPECT_EQ(collection.total, 1);
    EXPECT_EQ(collection.combined, 1);
    EXPECT_EQ(collection.variants[8].count, 1);
    for (u8 stat = 0; stat < NUM_STATS; stat++)
    {
        EXPECT_EQ(collection.variants[8].bonuses[stat].minimum, Pokemon_GetMutationStatBonusPercent(&mon, stat));
        EXPECT_EQ(collection.variants[8].bonuses[stat].maximum, Pokemon_GetMutationStatBonusPercent(&mon, stat));
    }
    EXPECT_EQ(collection.variants[8].bonuses[NUM_STATS].minimum, Pokemon_GetMutationStaminaBonusPercent(&mon));
    EXPECT_EQ(memcmp(&before, &mon, sizeof(mon)), 0);
    EXPECT_EQ(memcmp(&rng, &gRngValue, sizeof(rng)), 0);
}

TEST("Pokedex variants reject a damaged encrypted checksum without marking the source as a Bad Egg")
{
    struct Pokemon mon;
    struct PokedexVariantCollection collection;
    CreateMonWithIVs(&mon, SPECIES_TREECKO, 25, 0xABCDEF01, OTID_STRUCT_PRESET(1), 31);
    mon.box.checksum ^= 1;
    struct Pokemon before = mon;
    rng_value_t rng = gRngValue;
    PokedexVariants_Init(&collection, SPECIES_TREECKO);
    PokedexVariants_AddBoxMon(&collection, &mon.box, FALSE);
    EXPECT_EQ(collection.total, 0);
    EXPECT_EQ((u32)mon.box.isBadEgg, FALSE);
    EXPECT_EQ(memcmp(&before, &mon, sizeof(mon)), 0);
    EXPECT_EQ(memcmp(&rng, &gRngValue, sizeof(rng)), 0);
}

TEST("Pokedex variants bounded scans resume after each occupied slot without changing the party or RNG")
{
    struct Pokemon saved[PARTY_SIZE];
    memcpy(saved, gParties[B_TRAINER_PLAYER], sizeof(saved));
    u8 savedCount = gPartiesCount[B_TRAINER_PLAYER];
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], SPECIES_TREECKO, 25, 1, OTID_STRUCT_PRESET(1), 31);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][1], SPECIES_TORCHIC, 25, 2, OTID_STRUCT_PRESET(1), 31);
    gPartiesCount[B_TRAINER_PLAYER] = 2;
    struct Pokemon before[2];
    memcpy(before, gParties[B_TRAINER_PLAYER], sizeof(before));
    rng_value_t rng = gRngValue;
    struct PokedexVariantCollection collection;
    PokedexVariants_Init(&collection, SPECIES_TREECKO);
    u16 cursor = 0;
    EXPECT_EQ(PokedexVariants_ScanStep(&collection, &cursor, 1), FALSE);
    EXPECT_EQ(cursor, 1);
    EXPECT_EQ(collection.total, 1);
    EXPECT_EQ(PokedexVariants_ScanStep(&collection, &cursor, 1), FALSE);
    EXPECT_EQ(cursor, 2);
    EXPECT_EQ(collection.total, 1);
    EXPECT_EQ(PokedexVariants_ScanStep(&collection, &cursor, 0), FALSE);
    EXPECT_EQ(cursor, 2);
    EXPECT_EQ(memcmp(before, gParties[B_TRAINER_PLAYER], sizeof(before)), 0);
    EXPECT_EQ(memcmp(&rng, &gRngValue, sizeof(rng)), 0);
    memcpy(gParties[B_TRAINER_PLAYER], saved, sizeof(saved));
    gPartiesCount[B_TRAINER_PLAYER] = savedCount;
}
