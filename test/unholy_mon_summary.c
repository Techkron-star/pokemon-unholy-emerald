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
    EXPECT_EQ(summary.mutationStaminaBonusPercent, Pokemon_GetMutationStaminaBonusPercent(&mon));
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

TEST("Alpha boosts actual HP and five regular stats by 20 percent without modifying genetics")
{
    struct Pokemon regular, alpha;
    u32 originalIv, originalEv;

    CreateMonWithIVs(&regular, SPECIES_SCEPTILE, 50, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    alpha = regular;
    originalIv = GetMonData(&alpha, MON_DATA_HP_IV);
    originalEv = GetMonData(&alpha, MON_DATA_HP_EV);
    Pokemon_SetBadge(&alpha, POKEMON_BADGE_ALPHA);
    CalculateMonStats(&alpha);

    for (u32 stat = 0; stat < NUM_STATS; stat++)
    {
        u32 base = GetMonData(&regular, MON_DATA_MAX_HP + stat);
        EXPECT_EQ(GetMonData(&alpha, MON_DATA_MAX_HP + stat), (base * 120 + 50) / 100);
    }
    EXPECT_EQ(GetMonData(&alpha, MON_DATA_HP_IV), originalIv);
    EXPECT_EQ(GetMonData(&alpha, MON_DATA_HP_EV), originalEv);
    CalculateMonStats(&alpha);
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(GetMonData(&alpha, MON_DATA_MAX_HP + stat),
                  (GetMonData(&regular, MON_DATA_MAX_HP + stat) * 120 + 50) / 100);
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
    EXPECT_EQ(summary.mutationStaminaBonusPercent, Pokemon_GetMutationStaminaBonusPercent(&mon));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(memcmp(&mon, &before, sizeof(mon)), 0);
}

TEST("Alpha grants 20 percent to all six stats, stacking additively with Hyper and Titan")
{
    struct Pokemon mon;
    struct UnholyMonSummary summary;

    CreateMonWithIVs(&mon, SPECIES_MIGHTYENA, 30, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_ALPHA);
    EXPECT(UnholyMonSummary_Build(&mon, &summary));
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(summary.mutationBonusPercent[stat], 20);
    EXPECT_EQ(summary.mutationStaminaBonusPercent, 0);

    Pokemon_SetBadge(&mon, POKEMON_BADGE_HYPER);
    EXPECT(UnholyMonSummary_Build(&mon, &summary));
    for (u32 stat = 0; stat < NUM_STATS; stat++)
        EXPECT_EQ(summary.mutationBonusPercent[stat], 45);
    EXPECT_EQ(summary.mutationStaminaBonusPercent, 25);

    Pokemon_SetBadge(&mon, POKEMON_BADGE_TITAN);
    EXPECT(UnholyMonSummary_Build(&mon, &summary));
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPEED], -5);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_ATK], 45);
    EXPECT_GE(summary.mutationBonusPercent[STAT_HP], 65);
    EXPECT_LE(summary.mutationBonusPercent[STAT_HP], 105);
    EXPECT_EQ(summary.mutationStaminaBonusPercent, 25);
}

TEST("Enraged boosts physical and special attacks but reduces both defenses as level rises")
{
    struct Pokemon lowLevelMon, highLevelMon, normalMon;
    struct UnholyMonSummary summary;

    CreateMonWithIVs(&normalMon, SPECIES_MIGHTYENA, 20, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    CreateMonWithIVs(&lowLevelMon, SPECIES_MIGHTYENA, 20, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    Pokemon_SetBadge(&lowLevelMon, POKEMON_BADGE_ENRAGED);
    CalculateMonStats(&lowLevelMon);
    EXPECT_GT(GetMonData(&lowLevelMon, MON_DATA_ATK), GetMonData(&normalMon, MON_DATA_ATK));
    EXPECT_GT(GetMonData(&lowLevelMon, MON_DATA_SPATK), GetMonData(&normalMon, MON_DATA_SPATK));
    EXPECT_LT(GetMonData(&lowLevelMon, MON_DATA_DEF), GetMonData(&normalMon, MON_DATA_DEF));
    EXPECT_LT(GetMonData(&lowLevelMon, MON_DATA_SPDEF), GetMonData(&normalMon, MON_DATA_SPDEF));
    EXPECT_EQ(GetMonData(&lowLevelMon, MON_DATA_MAX_HP), GetMonData(&normalMon, MON_DATA_MAX_HP));
    EXPECT_EQ(GetMonData(&lowLevelMon, MON_DATA_SPEED), GetMonData(&normalMon, MON_DATA_SPEED));
    EXPECT(UnholyMonSummary_Build(&lowLevelMon, &summary));
    EXPECT_EQ(summary.mutationBonusPercent[STAT_ATK], 12);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPATK], 12);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_DEF], -6);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPDEF], -6);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_HP], 0);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPEED], 0);
    EXPECT_EQ(summary.mutationStaminaBonusPercent, 0);

    CreateMonWithIVs(&highLevelMon, SPECIES_MIGHTYENA, 100, 0x12345678,
                     OTID_STRUCT_PRESET(0xABCDEF01), 20);
    Pokemon_SetBadge(&highLevelMon, POKEMON_BADGE_ENRAGED);
    EXPECT_EQ(Pokemon_GetMutationStatBonusPercent(&highLevelMon, STAT_ATK), 60);
    EXPECT_EQ(Pokemon_GetMutationStatBonusPercent(&highLevelMon, STAT_SPATK), 60);
    EXPECT_EQ(Pokemon_GetMutationStatBonusPercent(&highLevelMon, STAT_DEF), -35);
    EXPECT_EQ(Pokemon_GetMutationStatBonusPercent(&highLevelMon, STAT_SPDEF), -35);

    Pokemon_SetBadge(&highLevelMon, POKEMON_BADGE_HYPER);
    EXPECT(UnholyMonSummary_Build(&highLevelMon, &summary));
    EXPECT_EQ(summary.mutationBonusPercent[STAT_ATK], 85);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPATK], 85);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_DEF], -10);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPDEF], -10);
    EXPECT_EQ(summary.mutationBonusPercent[STAT_SPEED], 25);
    EXPECT_EQ(summary.mutationStaminaBonusPercent, 25);
}
