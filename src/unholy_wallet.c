#include "global.h"
#include "money.h"
#include "string_util.h"
#include "unholy_save.h"
#include "unholy_wallet.h"

#define WALLET_MAGIC 0x57414C31u
struct WalletRecord
{
    u32 magic;
    u32 low;
    u32 high;
};
STATIC_ASSERT(UNHOLY_SAVE4_WALLET_OFFSET + sizeof(struct WalletRecord) <= UNHOLY_SAVE4_GENESIS_OFFSET, WalletFitsSave4);

static struct WalletRecord *GetWalletRecord(void)
{
    struct SaveBlock4 *block = UnholySave4_Get();
    if (block == NULL)
        return NULL;
    struct WalletRecord *record = (struct WalletRecord *)(block->payload + UNHOLY_SAVE4_WALLET_OFFSET);
    if (record->magic == 0 && record->low == 0 && record->high == 0)
    {
        record->magic = WALLET_MAGIC;
        if (!UnholySave4_Commit())
            return NULL;
    }
    if (record->magic != WALLET_MAGIC || (((u64)record->high << 32) | record->low) > UNHOLY_RESERVE_MAX)
        return NULL;
    return record;
}

bool32 UnholyWalletGetReserve(u64 *balance)
{
    struct WalletRecord *record = GetWalletRecord();
    if (record == NULL)
        return FALSE;
    *balance = ((u64)record->high << 32) | record->low;
    return TRUE;
}

bool32 UnholyWalletTransfer(bool32 deposit, u32 amount)
{
    struct WalletRecord *record = GetWalletRecord();
    if (record == NULL || amount == 0)
        return FALSE;
    u64 reserve = ((u64)record->high << 32) | record->low;
    u32 cash = GetMoney(&gSaveBlock1Ptr->money);
    if (deposit)
    {
        if (amount > cash || amount > UNHOLY_RESERVE_MAX - reserve)
            return FALSE;
        reserve += amount;
        cash -= amount;
    }
    else
    {
        if (amount > reserve || amount > MAX_MONEY - cash)
            return FALSE;
        reserve -= amount;
        cash += amount;
    }
    struct WalletRecord old = *record;
    record->low = (u32)reserve;
    record->high = (u32)(reserve >> 32);
    if (!UnholySave4_Commit())
    {
        *record = old;
        return FALSE;
    }
    SetMoney(&gSaveBlock1Ptr->money, cash);
    return TRUE;
}

u8 *UnholyWalletFormat(u8 *dest, u64 balance)
{
    // Render in 9-digit chunks using the existing unsigned formatter.
    if (balance >= 1000000000ULL)
    {
        dest = ConvertUIntToDecimalStringN(dest, balance / 1000000000ULL, STR_CONV_MODE_LEFT_ALIGN, 4);
        return ConvertUIntToDecimalStringN(dest, balance % 1000000000ULL, STR_CONV_MODE_LEADING_ZEROS, 9);
    }
    return ConvertUIntToDecimalStringN(dest, (u32)balance, STR_CONV_MODE_LEFT_ALIGN, 9);
}
