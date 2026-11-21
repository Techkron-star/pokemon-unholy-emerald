#include "global.h"
#include "hf_wallet.h"
#include "money.h"
#include "unholy_save.h"

// Reserve the final 48 bytes of SaveBlock5's payload for the wallet.
// Other SaveBlock5 payload consumers must not use this region.
#define HF_WALLET_MAGIC 0x48574631u // HWF1
#define HF_WALLET_OFFSET (UNHOLY_SAVE5_PAYLOAD_BYTES - sizeof(struct HfWalletData))

struct HfWalletData
{
    u32 magic;
    u8 unlocked;
    u8 reserved[3];
    u32 balances[HF_WALLET_MAX_SLOTS];
};

static struct HfWalletData *HfWallet_GetData(void)
{
    struct SaveBlock5 *save = UnholySave5_Get();
    struct HfWalletData *wallet;

    if (save == NULL)
        return NULL;
    wallet = (struct HfWalletData *)&save->payload[HF_WALLET_OFFSET];
    if (wallet->magic != HF_WALLET_MAGIC)
    {
        // Only initialize an unused wallet region; never erase unknown data.
        u32 i;
        const u8 *bytes = (const u8 *)wallet;
        for (i = 0; i < sizeof(*wallet); i++)
            if (bytes[i] != 0 && bytes[i] != 0xFF)
                return NULL;
        memset(wallet, 0, sizeof(*wallet));
        wallet->magic = HF_WALLET_MAGIC;
        wallet->unlocked = 1;
        UnholySave5_Commit();
    }
    if (wallet->unlocked == 0 || wallet->unlocked > HF_WALLET_MAX_SLOTS)
        return NULL;
    for (u8 i = 0; i < wallet->unlocked; i++)
        if (wallet->balances[i] > HF_WALLET_SLOT_CAP)
            return NULL;
    return wallet;
}

bool32 HfWallet_IsAvailable(void)
{
    return HfWallet_GetData() != NULL;
}

u8 HfWallet_GetUnlockedSlots(void)
{
    struct HfWalletData *wallet = HfWallet_GetData();
    return wallet == NULL ? 0 : wallet->unlocked;
}

u32 HfWallet_GetBalance(u8 slot)
{
    struct HfWalletData *wallet = HfWallet_GetData();
    return wallet == NULL || slot >= wallet->unlocked ? 0 : wallet->balances[slot];
}

u32 HfWallet_GetNextSlotPrice(void)
{
    u8 unlocked = HfWallet_GetUnlockedSlots();
    return unlocked == 0 || unlocked >= HF_WALLET_MAX_SLOTS ? 0 : unlocked * 250000u;
}

bool32 HfWallet_BuySlot(void)
{
    struct HfWalletData *wallet = HfWallet_GetData();
    u32 price = HfWallet_GetNextSlotPrice();

    if (wallet == NULL || price == 0 || !IsEnoughMoney(&gSaveBlock1Ptr->money, price))
        return FALSE;
    RemoveMoney(&gSaveBlock1Ptr->money, price);
    wallet->balances[wallet->unlocked++] = 0;
    return UnholySave5_Commit();
}

bool32 HfWallet_Deposit(u8 slot, u32 amount)
{
    struct HfWalletData *wallet = HfWallet_GetData();

    if (wallet == NULL || slot >= wallet->unlocked || amount == 0
     || wallet->balances[slot] > HF_WALLET_SLOT_CAP
     || amount > HF_WALLET_SLOT_CAP - wallet->balances[slot]
     || !IsEnoughMoney(&gSaveBlock1Ptr->money, amount))
        return FALSE;
    RemoveMoney(&gSaveBlock1Ptr->money, amount);
    wallet->balances[slot] += amount;
    return UnholySave5_Commit();
}

bool32 HfWallet_Withdraw(u8 slot, u32 amount)
{
    struct HfWalletData *wallet = HfWallet_GetData();
    u32 carried = GetMoney(&gSaveBlock1Ptr->money);

    if (wallet == NULL || slot >= wallet->unlocked || amount == 0
     || wallet->balances[slot] < amount || carried > MAX_MONEY
     || amount > MAX_MONEY - carried)
        return FALSE;
    wallet->balances[slot] -= amount;
    AddMoney(&gSaveBlock1Ptr->money, amount);
    return UnholySave5_Commit();
}

// Transfers between unlocked vaults never touch the player's carried money.
bool32 HfWallet_Transfer(u8 from, u8 to, u32 amount)
{
    struct HfWalletData *wallet = HfWallet_GetData();

    if (wallet == NULL || from >= wallet->unlocked || to >= wallet->unlocked
     || from == to || amount == 0 || wallet->balances[from] < amount
     || amount > HF_WALLET_SLOT_CAP - wallet->balances[to])
        return FALSE;
    wallet->balances[from] -= amount;
    wallet->balances[to] += amount;
    return UnholySave5_Commit();
}

// A 64-bit accumulator prevents overflow when several vaults are full.
u64 HfWallet_GetTotalBalance(void)
{
    struct HfWalletData *wallet = HfWallet_GetData();
    u64 total = 0;
    u8 i;

    if (wallet == NULL)
        return 0;
    for (i = 0; i < wallet->unlocked; i++)
        total += wallet->balances[i];
    return total;
}
