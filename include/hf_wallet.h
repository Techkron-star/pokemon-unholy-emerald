#ifndef GUARD_HF_WALLET_H
#define GUARD_HF_WALLET_H

#define HF_WALLET_MAX_SLOTS 10
#define HF_WALLET_SLOT_CAP 2147483647u

bool32 HfWallet_IsAvailable(void);
u8 HfWallet_GetUnlockedSlots(void);
u32 HfWallet_GetBalance(u8 slot);
u32 HfWallet_GetNextSlotPrice(void);
bool32 HfWallet_BuySlot(void);
bool32 HfWallet_Deposit(u8 slot, u32 amount);
bool32 HfWallet_Withdraw(u8 slot, u32 amount);
bool32 HfWallet_Transfer(u8 from, u8 to, u32 amount);
u64 HfWallet_GetTotalBalance(void);

#endif
