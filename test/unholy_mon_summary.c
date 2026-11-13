#include "global.h"
#include "pokemon.h"
#include "battle_stamina.h"
#include "unholy_mon_summary.h"
#include "gear.h"
#include "test/test.h"

TEST("Unholy summary exposes Omega perfect genetics and actual battle stamina")
{
    struct Pokemon mon;
    struct UnholyMonSummary summary;
    u32 originalStamina;

    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 100, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), MAX_PER_STAT_IVS);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_OMEGA);
    CalculateMonStats(&mon);
    originalStamina = Stamina_CalcMaxForMon(&mon);

    EXPECT_EQ(UnholyMonSummary_Build(&mon, &summary), TRUE);
    EXPECT_EQ(summary.species, SPECIES_SCEPTILE);
    EXPECT_EQ(summary.level, 100);
    EXPECT_EQ(summary.staminaMax, originalStamina);
    EXPECT_EQ(summary.mutationBadges & POKEMON_BADGE_OMEGA, POKEMON_BADGE_OMEGA);
    EXPECT_EQ(summary.gearId, GetMonData(&mon, MON_DATA_GEAR));

    for (u32 i = 0; i < NUM_STATS; i++)
    {
        EXPECT_EQ(summary.ivs[i], MAX_PER_STAT_IVS);
        EXPECT_EQ(summary.evs[i], 252);
        EXPECT_EQ(summary.stats[i], GetMonData(&mon, MON_DATA_MAX_HP + i));
        EXPECT_EQ(summary.mutationBonusPercent[i], Pokemon_GetMutationStatBonusPercent(&mon, i));
    }

    // A Summary read must never change the Pokémon or its current battle stamina.
    EXPECT_EQ(UnholyMonSummary_Build(&mon, &summary), TRUE);
    EXPECT_EQ(Stamina_CalcMaxForMon(&mon), originalStamina);
}

TEST("Unholy summary rejects missing output and null Pokémon")
{
    struct UnholyMonSummary summary;

    EXPECT_EQ(UnholyMonSummary_Build(NULL, &summary), FALSE);
    EXPECT_EQ(Stamina_CalcMaxForMon(NULL), 0);
}

TEST("Unholy summary reads combined mutation bonuses and independent item and Gear without changing the mon")
{
    struct Pokemon mon, before;
    struct UnholyMonSummary summary;
    u16 item = ITEM_LEFTOVERS;
    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 50, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_ALPHA);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_TITAN);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    Gear_SetEquipped(&mon, GEAR_NONE);
    CalculateMonStats(&mon);
    before = mon;
    EXPECT(UnholyMonSummary_Build(&mon, &summary));
    EXPECT_EQ(summary.mutationBadges, POKEMON_BADGE_ALPHA | POKEMON_BADGE_TITAN);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(summary.mutationBonusPercent[stat], Pokemon_GetMutationStatBonusPercent(&mon, stat));
    EXPECT_LT(summary.mutationBonusPercent[STAT_SPEED], summary.mutationBonusPercent[STAT_ATK]);
    EXPECT_EQ(summary.gearId, GEAR_NONE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(memcmp(&mon, &before, sizeof(mon)), 0);
}
