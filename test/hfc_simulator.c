#include "global.h"
#include "hfc_simulator.h"
#include "money.h"
#include "pokemon.h"
#include "test/test.h"

TEST("HFC Simulator applies the draft, recalculates stats and charges for both increases and reductions")
{
    struct Pokemon mon;
    struct HfcSimulatorValues spread;
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 100, 0, OTID_STRUCT_PRESET(0), 10);
    HfcSimulator_ReadValues(&mon, &spread);

    u32 attack = GetMonData(&mon, MON_DATA_ATK);
    u32 defense = GetMonData(&mon, MON_DATA_DEF);
    spread.ivs[STAT_ATK] += 2;
    spread.ivs[STAT_DEF] -= 2;
    spread.evs[STAT_ATK] += 4;
    SetMoney(&money, 1300);
    u16 hp = 0;
    SetMonData(&mon, MON_DATA_HP, &hp);

    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_APPLIED);
    EXPECT_EQ(GetMoney(&money), 20);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ATK_IV), 12);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DEF_IV), 8);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ATK_EV), 4);
    EXPECT(GetMonData(&mon, MON_DATA_ATK) > attack);
    EXPECT(GetMonData(&mon, MON_DATA_DEF) < defense);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP), 0);

    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_NO_CHANGES);
    EXPECT_EQ(GetMoney(&money), 20);
}

TEST("HFC Simulator rejects illegal spreads and insufficient funds without committing any edits")
{
    struct Pokemon mon;
    struct HfcSimulatorValues spread;
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 100, 0, OTID_STRUCT_PRESET(0), 10);
    HfcSimulator_ReadValues(&mon, &spread);
    SetMoney(&money, 299);

    spread.ivs[STAT_HP] += 1;
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_INSUFFICIENT_FUNDS);
    EXPECT_EQ(GetMoney(&money), 299);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), 10);

    SetMoney(&money, 100000);
    spread.ivs[STAT_HP] = MAX_PER_STAT_IVS + 1;
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_INVALID_SPREAD);
    spread.ivs[STAT_HP] = MAX_PER_STAT_IVS;
    if (MAX_PER_STAT_EVS < 255)
    {
        spread.evs[STAT_HP] = 255;
        EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_INVALID_SPREAD);
    }
    spread.evs[STAT_HP] = MAX_PER_STAT_EVS;
    spread.evs[STAT_ATK] = MAX_PER_STAT_EVS;
    spread.evs[STAT_DEF] = MAX_TOTAL_EVS - 2 * MAX_PER_STAT_EVS + 1;
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_INVALID_SPREAD);
    EXPECT_EQ(GetMoney(&money), 100000);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), 10);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), 0);

    spread.evs[STAT_DEF]--;
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_APPLIED);
    EXPECT_EQ(GetMoney(&money), 83500);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), MAX_PER_STAT_IVS);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), MAX_PER_STAT_EVS);
}

TEST("HFC Simulator preserves Ascended and Omega genetics, and never edits eggs")
{
    struct Pokemon mon;
    struct HfcSimulatorValues spread = {0};
    u32 money;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 100, 0, OTID_STRUCT_PRESET(0), 10);
    SetMoney(&money, 100000);

    u16 badge = POKEMON_BADGE_ASCENDED;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badge);
    CalculateMonStats(&mon);
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_LOCKED);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), 252);

    badge = POKEMON_BADGE_OMEGA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badge);
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_LOCKED);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), MAX_PER_STAT_IVS);

    u8 isEgg = TRUE;
    badge = 0;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badge);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    EXPECT_EQ(HfcSimulator_Apply(&mon, &spread, &money), HFC_SIMULATOR_LOCKED);
    EXPECT_EQ(GetMoney(&money), 100000);
}
