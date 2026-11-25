#include "global.h"
#include "money.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "data.h"
#include "constants/trainers.h"
#include "unholy_save.h"
#include "unholy_cofradia.h"
#include "test/test.h"

TEST("Cofradia compounds all six difficulties without unsigned overflow")
{
    const u32 expected[] = {101000, 103000, 107000, 112000, 120000, 135000};
    const u32 minimum[] = {5000, 15000, 35000, 75000, 150000, 300000};
    for (u8 i = 0; i < COFRADIA_DIFFICULTY_COUNT; i++)
    {
        EXPECT_EQ(CofradiaMinimum(i), minimum[i]);
        EXPECT_EQ(CofradiaCompound(100000, i), expected[i]);
        EXPECT_EQ(CofradiaCompound(MAX_MONEY, i), MAX_MONEY);
    }
    EXPECT_EQ(CofradiaCompound(1, 0), 1);
}

TEST("Cofradia escrow is paid once, defeat forfeits it, cancelled first round refunds")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    struct CofradiaView view;
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 1000000);
    EXPECT_EQ(CofradiaBegin(0, 4999), COFRADIA_INVALID);
    EXPECT_EQ(CofradiaBegin(0, 1000001), COFRADIA_NO_MONEY);
    EXPECT_EQ(CofradiaBegin(5, 500000), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 500000);
    EXPECT_EQ(CofradiaBegin(0, 5000), COFRADIA_BUSY);
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_INVALID);
    EXPECT(CofradiaArm());
    EXPECT(!CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT(!CofradiaResolve(TRUE));
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.fund, 675000);
    EXPECT_EQ(view.wins, 1);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 1411250);
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_INVALID);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.bestWins, 2);
    EXPECT_EQ(view.bestProfit, 411250);
    EXPECT_EQ(CofradiaBegin(0, 100000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaDisarm());
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 1411250);
    EXPECT_EQ(CofradiaBegin(0, 100000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(FALSE));
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 1311250);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.biggestLoss, 101000);
    EXPECT_EQ(view.state, COFRADIA_IDLE);
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    gSaveBlock3Ptr->unholySave4 = oldSave;
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
}

TEST("Cofradia retains a payout that would overflow cash")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    struct CofradiaView view;
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
    EXPECT_EQ(CofradiaBegin(5, MAX_MONEY), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT(!CofradiaArm());
    AddMoney(&gSaveBlock1Ptr->money, 1);
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_CASH_FULL);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.fund, MAX_MONEY);
    EXPECT_EQ(view.state, COFRADIA_READY);
    RemoveMoney(&gSaveBlock1Ptr->money, 1);
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), MAX_MONEY);
    gSaveBlock3Ptr->unholySave4 = oldSave;
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
}

TEST("Cofradia derived teams have bounded difficulty sizes and seeded source selection")
{
    const u8 expected[] = {3, 4, 4, 5, 6, 6};
    for (u8 tier = 0; tier < COFRADIA_DIFFICULTY_COUNT; tier++)
    {
        EXPECT_EQ(CofradiaTeamSize(tier, FALSE), expected[tier]);
        EXPECT_EQ(CofradiaTeamSize(tier, TRUE), min(PARTY_SIZE, expected[tier] + 1));
        for (u8 ordinal = 0; ordinal < 5; ordinal++)
        {
            u16 id = CofradiaPickTeamSource(0x12345678u, tier, FALSE, ordinal);
            EXPECT(id > TRAINER_NONE && id < TRAINERS_COUNT);
            EXPECT_EQ(id, CofradiaPickTeamSource(0x12345678u, tier, FALSE, ordinal));
            const struct Trainer *trainer = GetTrainerStructFromId(id);
            if (ordinal == 0)
                EXPECT_EQ(trainer->battleType, TRAINER_BATTLE_TYPE_SINGLES);
            if (trainer->overrideTrainer)
                trainer = GetTrainerStructFromId(trainer->overrideTrainer);
            EXPECT(trainer->party != NULL);
            EXPECT(trainer->partySize > 0);
        }
    }
}

TEST("Cofradia boss source varies across persisted round seeds")
{
    u16 first = CofradiaPickTeamSource(1, 5, TRUE, 0);
    bool32 varied = FALSE;
    for (u32 seed = 2; seed < 32; seed++)
        if (CofradiaPickTeamSource(seed, 5, TRUE, 0) != first)
            varied = TRUE;
    EXPECT(varied);
}

TEST("Cofradia donor slots avoid repeated trainers and rematch aliases")
{
    for (u8 tier = 0; tier < COFRADIA_DIFFICULTY_COUNT; tier++)
    {
        for (u8 boss = 0; boss < 2; boss++)
        {
            for (u32 seed = 1; seed <= 8; seed++)
            {
                u16 identities[5];
                for (u8 slot = 0; slot < ARRAY_COUNT(identities); slot++)
                {
                    u16 id = CofradiaPickTeamSource(seed, tier, boss, slot);
                    EXPECT(id > TRAINER_NONE && id < TRAINERS_COUNT);
                    if (id == TRAINER_NONE || id >= TRAINERS_COUNT)
                        break;
                    const struct Trainer *trainer = GetTrainerStructFromId(id);
                    identities[slot] = trainer->overrideTrainer ? trainer->overrideTrainer : id;
                    for (u8 previous = 0; previous < slot; previous++)
                        EXPECT(identities[slot] != identities[previous]);
                    EXPECT_EQ(id, CofradiaPickTeamSource(seed, tier, boss, slot));
                }
            }
        }
    }
}

TEST("Cofradia gym blueprints are reproducible, varied and preserve a gym anchor")
{
    EXPECT_EQ(CofradiaGymPatternSource(123, 0, TRAINER_ROXANNE_1), TRAINER_ROXANNE_1);
    EXPECT_EQ(CofradiaGymPatternSource(456, 5, TRAINER_JUAN_1), TRAINER_JUAN_1);
    for (u8 tier = 0; tier < COFRADIA_DIFFICULTY_COUNT; tier++)
    {
        u16 first = CofradiaGymPatternSource(1, tier, TRAINER_NONE);
        bool32 varied = FALSE;
        for (u32 seed = 1; seed <= 32; seed++)
        {
            u16 source = CofradiaGymPatternSource(seed, tier, TRAINER_NONE);
            EXPECT(source > TRAINER_NONE && source < TRAINERS_COUNT);
            EXPECT_EQ(source, CofradiaGymPatternSource(seed, tier, TRAINER_NONE));
            if (source == TRAINER_NONE || source >= TRAINERS_COUNT)
                continue;
            const struct Trainer *trainer = GetTrainerStructFromId(source);
            if (trainer->overrideTrainer)
                trainer = GetTrainerStructFromId(trainer->overrideTrainer);
            EXPECT(trainer->party != NULL);
            EXPECT(trainer->partySize > 0);
            varied |= source != first;
        }
        EXPECT(varied);
    }
}

TEST("Cofradia scouts distinct living party slots reproducibly, at most two")
{
    struct Pokemon party[PARTY_SIZE] = {0};
    CreateRandomMon(&party[0], SPECIES_PIKACHU, 50);
    CreateRandomMon(&party[2], SPECIES_CHARIZARD, 50);
    CreateRandomMon(&party[4], SPECIES_BLASTOISE, 50);
    u16 hp = 0;
    SetMonData(&party[4], MON_DATA_HP, &hp);
    bool32 varied = FALSE;
    u8 first[COFRADIA_MAX_COUNTERS];
    CofradiaPickCounterTargets(party, 1, 5, first);
    for (u32 seed = 1; seed <= 64; seed++)
    {
        u8 slots[COFRADIA_MAX_COUNTERS], again[COFRADIA_MAX_COUNTERS];
        u8 count = CofradiaPickCounterTargets(party, seed, 5, slots);
        EXPECT(count >= 1 && count <= COFRADIA_MAX_COUNTERS);
        EXPECT_EQ(count, CofradiaPickCounterTargets(party, seed, 5, again));
        for (u8 i = 0; i < count; i++)
        {
            EXPECT(slots[i] == 0 || slots[i] == 2);
            EXPECT_EQ(slots[i], again[i]);
            if (i)
                EXPECT(slots[i] != slots[0]);
        }
        varied |= slots[0] != first[0];
    }
    EXPECT(varied);
    ZeroPartyMons(party);
    EXPECT_EQ(CofradiaPickCounterTargets(party, 1, 5, first), 0);
}

TEST("Cofradia counters need real damaging coverage rather than typing alone")
{
    struct Pokemon attacker, defender;
    CreateRandomMon(&attacker, SPECIES_BLASTOISE, 50);
    CreateRandomMon(&defender, SPECIES_CHARIZARD, 50);
    for (u8 i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&attacker, MOVE_NONE, i);
    SetMonMoveSlot(&attacker, MOVE_TACKLE, 0);
    EXPECT_EQ(CofradiaCounterScore(&attacker, &defender), 0);
    SetMonMoveSlot(&attacker, MOVE_RAIN_DANCE, 0);
    EXPECT_EQ(CofradiaCounterScore(&attacker, &defender), 0);
    SetMonMoveSlot(&attacker, MOVE_SURF, 0);
    EXPECT(CofradiaCounterScore(&attacker, &defender) > 0);
    EXPECT_EQ(CofradiaCounterScore(NULL, &defender), 0);
}

TEST("Cofradia mixed credit refunds only own contribution and repays principal once")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    struct CofradiaView view;
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 100000);
    EXPECT_EQ(CofradiaBeginCredit(0, 5000, 5001), COFRADIA_INVALID);
    EXPECT_EQ(CofradiaBeginCredit(0, 200000, 50000), COFRADIA_NO_MONEY);
    EXPECT_EQ(CofradiaBeginCredit(0, 100000, 60000), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 60000);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaDisarm());
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 100000);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.borrowed, 0);
    EXPECT_EQ(view.debt, 0);
    EXPECT_EQ(CofradiaBeginCredit(0, 100000, 60000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 101000);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.debt, 0);
    EXPECT_EQ(view.borrowed, 0);
    EXPECT_EQ(CofradiaCashOut(), COFRADIA_INVALID);
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    gSaveBlock3Ptr->unholySave4 = oldSave;
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
}

TEST("Cofradia defeat creates only principal debt and permits installments")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    struct CofradiaView view;
    UnholySave4_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 100000);
    EXPECT_EQ(CofradiaBeginCredit(0, 100000, 60000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(TRUE));
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(FALSE));
    EXPECT(!CofradiaResolve(FALSE));
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.debt, 60000);
    EXPECT_EQ(view.borrowed, 0);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 60000);
    EXPECT_EQ(CofradiaBeginCredit(0, 5000, 1), COFRADIA_DEBT_PENDING);
    EXPECT_EQ(CofradiaRepay(60001), COFRADIA_INVALID);
    EXPECT_EQ(CofradiaRepay(0), COFRADIA_INVALID);
    EXPECT_EQ(CofradiaRepay(20000), COFRADIA_OK);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.debt, 40000);
    EXPECT_EQ(CofradiaRepay(40000), COFRADIA_OK);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 0);
    EXPECT(CofradiaGetView(&view));
    EXPECT_EQ(view.debt, 0);
    EXPECT_EQ(CofradiaBeginCredit(0, 5000, 5000), COFRADIA_OK);
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    gSaveBlock3Ptr->unholySave4 = oldSave;
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
}

TEST("Cofradia appraisal excludes empty slots and values mutations")
{
    struct Pokemon mon = {0};
    EXPECT_EQ(CofradiaAppraisePokemon(&mon), 0);
    EXPECT_EQ(CofradiaAppraisePokemon(NULL), 0);
    CreateRandomMon(&mon, SPECIES_CHARIZARD, 50);
    u16 badges = 0;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    u32 plain = CofradiaAppraisePokemon(&mon);
    badges = POKEMON_BADGE_ALPHA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    u32 alpha = CofradiaAppraisePokemon(&mon);
    badges |= POKEMON_BADGE_OMEGA;
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    EXPECT(plain > 0);
    EXPECT(alpha > plain);
    EXPECT(CofradiaAppraisePokemon(&mon) > alpha);
}
