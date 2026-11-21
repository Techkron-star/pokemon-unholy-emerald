#ifndef GUARD_UNHOLY_BOUNTY_H
#define GUARD_UNHOLY_BOUNTY_H

#define UNHOLY_BOUNTY_RANK_COUNT 5
#define LOCALID_BOUNTY_TARGET 248

enum UnholyBountyResult
{
    BOUNTY_OK,
    BOUNTY_LOCKED,
    BOUNTY_NO_MONEY,
    BOUNTY_BUSY,
    BOUNTY_BAG_FULL,
    BOUNTY_UNAVAILABLE,
};

u32 UnholyBounty_GetFee(u8 rank);
u8 UnholyBounty_GetRequiredBadges(u8 rank);
u32 UnholyBounty_GetReward(u8 rank, u32 seed);
u16 UnholyBounty_GetMapSection(void);
const struct ObjectEventTemplate *UnholyBounty_GetTemplate(void);
void UnholyBounty_TrySpawn(s16 cameraX, s16 cameraY);
enum UnholyBountyResult UnholyBounty_Purchase(u8 rank, u32 seed);
enum UnholyBountyResult UnholyBounty_Claim(void);
void UnholyBounty_OnBattleEnd(u8 outcome);
void UnholyBounty_OpenMenu(void);
void UnholyBounty_PreparePurchase(void);
void UnholyBounty_Buy(void);
void UnholyBounty_Status(void);
void UnholyBounty_Abandon(void);
void UnholyBounty_Interact(void);
void UnholyBounty_ClaimScript(void);

#endif
