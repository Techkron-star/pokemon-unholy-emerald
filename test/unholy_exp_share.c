#include "global.h"
#include "daycare.h"
#include "event_data.h"
#include "item.h"
#include "test/battle.h"
#include "unholy_exp_share.h"
#include "constants/flags.h"
#include "constants/vars.h"

WILD_BATTLE_TEST("Overloaded Exp Share grants full EXP once, extra EVs and level-up IVs")
{
    enum Item share = ITEM_NONE;
    u16 flag = 0;
    u16 percent = 0;
    u8 evMultiplier = 0;

    PARAMETRIZE { share = ITEM_SUPER_EXP_SHARE; flag = FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED; percent = 100; evMultiplier = 1; }
    PARAMETRIZE { share = ITEM_OVERLOADED_EXP_SHARE; flag = FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED; percent = 125; evMultiplier = 2; }

    GIVEN {
        WITH_CONFIG(B_SCALED_EXP, GEN_3);
        WITH_CONFIG(B_SPLIT_EXP, GEN_3);
        WITH_CONFIG(B_UNEVOLVED_EXP_MULTIPLIER, GEN_3);
        FLAG_SET(flag);
        GIVE_PLAYER_ITEM(share, 1);
        PLAYER(SPECIES_WOBBUFFET) { Level(5); HPIV(0); AttackIV(10); DefenseIV(10); SpeedIV(10); SpAttackIV(10); SpDefenseIV(10); }
        PLAYER(SPECIES_WOBBUFFET) { Level(5); HPIV(0); AttackIV(10); DefenseIV(10); SpeedIV(10); SpAttackIV(10); SpDefenseIV(10); }
        PLAYER(SPECIES_WOBBUFFET) { Level(5); HP(0); HPIV(0); }
        OPPONENT(SPECIES_CATERPIE) { Level(21); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } THEN {
        u32 baseExp = gSpeciesInfo[SPECIES_CATERPIE].expYield * 21 / 7;
        u32 startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][5];
        for (u8 slot = 0; slot < 2; slot++)
        {
            struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
            EXPECT_EQ(GetMonData(mon, MON_DATA_EXP), startExp + baseExp * percent / 100);
            EXPECT_EQ(GetMonData(mon, MON_DATA_HP_EV), gSpeciesInfo[SPECIES_CATERPIE].evYield_HP * evMultiplier);
            EXPECT_EQ(GetMonData(mon, MON_DATA_HP_IV),
                      share == ITEM_OVERLOADED_EXP_SHARE ? GetMonData(mon, MON_DATA_LEVEL) - 5 : 0);
        }
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][2], MON_DATA_EXP), startExp);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][2], MON_DATA_HP_EV), 0);
    }
}

TEST("Overloaded Exp Share carries partial EXP and queues only ready party eggs")
{
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    u8 cycles = 1;

    CreateEgg(&party[0], SPECIES_WYNAUT, FALSE);
    SetMonData(&party[0], MON_DATA_FRIENDSHIP, &cycles);
    CreateEgg(&party[1], SPECIES_WYNAUT, FALSE);
    SetMonData(&party[1], MON_DATA_FRIENDSHIP, &cycles);
    CreateMonWithIVs(&party[2], SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0), 10);
    gPartiesCount[B_TRAINER_PLAYER] = 3;
    VarSet(VAR_UNHOLY_EGG_EXP, 0);

    UnholyExpShare_AdvanceEggs(255);
    EXPECT(!UnholyExpShare_FindReadyEgg());
    EXPECT_EQ(VarGet(VAR_UNHOLY_EGG_EXP), 255);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_FRIENDSHIP), 1);

    UnholyExpShare_AdvanceEggs(1);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_FRIENDSHIP), 0);
    EXPECT_EQ(GetMonData(&party[1], MON_DATA_FRIENDSHIP), 0);
    EXPECT(ShouldEggHatch());
    EXPECT_EQ(gSpecialVar_0x8004, 0);

    u8 noEgg = FALSE;
    SetMonData(&party[0], MON_DATA_IS_EGG, &noEgg);
    EXPECT(ShouldEggHatch());
    EXPECT_EQ(gSpecialVar_0x8004, 1);

    SetMonData(&party[1], MON_DATA_IS_EGG, &noEgg);
    EXPECT(!UnholyExpShare_FindReadyEgg());
    EXPECT_EQ(VarGet(VAR_UNHOLY_EGG_EXP), 0);
}

TEST("Overloaded Exp Share keeps EV caps and protects trained IVs")
{
    struct Pokemon mon;
    CreateMonWithIVs(&mon, SPECIES_WOBBUFFET, 20, 0, OTID_STRUCT_PRESET(0), 10);
    u8 hpEv = MAX_PER_STAT_EVS - 1;
    u8 attackEv = MAX_PER_STAT_EVS;
    u8 defenseEv = MAX_TOTAL_EVS - hpEv - attackEv - 1;
    SetMonData(&mon, MON_DATA_HP_EV, &hpEv);
    SetMonData(&mon, MON_DATA_ATK_EV, &attackEv);
    SetMonData(&mon, MON_DATA_DEF_EV, &defenseEv);
    MonGainEVs(&mon, SPECIES_CATERPIE, TRUE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV), MAX_PER_STAT_EVS);
    EXPECT_EQ(GetMonEVCount(&mon), MAX_TOTAL_EVS);

    u8 trained = TRUE;
    SetMonData(&mon, MON_DATA_HYPER_TRAINED_HP, &trained);
    UnholyExpShare_TrainIvs(&mon, 18);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), 10);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ATK_IV), 11);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DEF_IV), 11);

    u16 badge = POKEMON_BADGE_ASCENDED;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badge);
    UnholyExpShare_TrainIvs(&mon, 18);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ATK_IV), 11);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DEF_IV), 11);
}

TEST("Overloaded Exp Share requires possession and its own toggle")
{
    SetBagItemsPointers();
    ClearBag();
    FlagClear(FLAG_UNHOLY_EXP_SHARE_ENABLED);
    FlagClear(FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED);
    FlagSet(FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED);
    EXPECT_EQ(UnholyExpShare_GetActiveItem(), ITEM_NONE);
    EXPECT(AddBagItem(ITEM_OVERLOADED_EXP_SHARE, 1));
    EXPECT_EQ(UnholyExpShare_GetActiveItem(), ITEM_OVERLOADED_EXP_SHARE);
    FlagClear(FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED);
    EXPECT_EQ(UnholyExpShare_GetActiveItem(), ITEM_NONE);
}
