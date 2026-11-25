#ifndef GUARD_UNHOLY_WALLET_H
#define GUARD_UNHOLY_WALLET_H

#define UNHOLY_RESERVE_MAX 999999999999ULL

bool32 UnholyWalletGetReserve(u64 *balance);
bool32 UnholyWalletTransfer(bool32 deposit, u32 amount);
u8 *UnholyWalletFormat(u8 *dest, u64 balance);

#endif
