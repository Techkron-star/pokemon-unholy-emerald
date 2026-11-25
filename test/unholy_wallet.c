#include "global.h"
#include "money.h"
#include "string_util.h"
#include "unholy_save.h"
#include "unholy_wallet.h"
#include "test/test.h"

TEST("Cash can cross the signed boundary and saturates at the full u32 limit")
{
    u32 old = GetMoney(&gSaveBlock1Ptr->money);
    SetMoney(&gSaveBlock1Ptr->money, 2147483647u);
    AddMoney(&gSaveBlock1Ptr->money, 1);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 2147483648u);
    AddMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), MAX_MONEY);
    u8 digits[16];
    ConvertUIntToDecimalStringN(digits, MAX_MONEY, STR_CONV_MODE_LEFT_ALIGN, MAX_MONEY_DIGITS);
    EXPECT_EQ(StringCompare(digits, COMPOUND_STRING("4294967295")), 0);
    RemoveMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 0);
    SetMoney(&gSaveBlock1Ptr->money, old);
}

TEST("Reserve transfers preserve cash and reject an overflowing withdrawal")
{
    struct SaveBlock4 oldSave = gSaveBlock3Ptr->unholySave4;
    u32 oldMoney = GetMoney(&gSaveBlock1Ptr->money);
    u64 reserve;
    UnholySave4_Reset();
    EXPECT(UnholyWalletGetReserve(&reserve));
    EXPECT_EQ(reserve, 0);
    SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
    EXPECT(UnholyWalletTransfer(TRUE, MAX_MONEY));
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 0);
    EXPECT(UnholyWalletGetReserve(&reserve));
    EXPECT_EQ(reserve, (u64)MAX_MONEY);
    EXPECT(!UnholyWalletTransfer(TRUE, 1));
    SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY - 1);
    EXPECT(!UnholyWalletTransfer(FALSE, 2));
    EXPECT(UnholyWalletTransfer(FALSE, 1));
    EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), MAX_MONEY);
    EXPECT(UnholyWalletGetReserve(&reserve));
    EXPECT_EQ(reserve, (u64)MAX_MONEY - 1);
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    u8 digits[16];
    UnholyWalletFormat(digits, UNHOLY_RESERVE_MAX);
    EXPECT_EQ(StringCompare(digits, COMPOUND_STRING("999999999999")), 0);
    SetMoney(&gSaveBlock1Ptr->money, oldMoney);
    gSaveBlock3Ptr->unholySave4 = oldSave;
}
