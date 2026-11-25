#include "global.h"
#include "money.h"
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
