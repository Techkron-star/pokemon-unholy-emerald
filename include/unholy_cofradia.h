#ifndef GUARD_UNHOLY_COFRADIA_H
#define GUARD_UNHOLY_COFRADIA_H

enum CofradiaState { COFRADIA_IDLE, COFRADIA_READY, COFRADIA_FIGHTING };
enum CofradiaResult { COFRADIA_OK, COFRADIA_INVALID, COFRADIA_NO_MONEY, COFRADIA_BUSY, COFRADIA_CASH_FULL, COFRADIA_DEBT_PENDING };
#define COFRADIA_DIFFICULTY_COUNT 6
#define COFRADIA_MAX_COUNTERS 2

struct Pokemon;

struct CofradiaView
{
    u32 stake;
    u32 fund;
    u32 wins;
    u32 bestWins;
    u32 bestStake;
    u32 bestProfit;
    u32 biggestLoss;
    u8 state;
    u8 difficulty;
    u32 borrowed;
    u32 debt;
};

#define COFRADIA_VAULT_MON_CAPACITY 8
#define COFRADIA_VAULT_ITEM_CAPACITY 24
#define COFRADIA_VAULT_PARTY 0xFF

struct CofradiaVaultView
{
    u32 totalValue;
    u8 pokemon;
    u8 itemStacks;
};

bool32 CofradiaVaultGetView(struct CofradiaVaultView *view);
u32 CofradiaAppraiseItem(enum Item item);
bool32 CofradiaVaultHoldMon(u8 box, u8 slot);
bool32 CofradiaVaultHoldItem(enum Item item, u16 count);
bool32 CofradiaVaultReturnMon(u8 index);
bool32 CofradiaVaultReturnItem(u8 index);
bool32 CofradiaVaultReturnAvailable(void);
bool32 CofradiaVaultCollect(void);
bool32 CofradiaVaultDescribeMon(u8 index, u8 *name, u32 *value);
bool32 CofradiaVaultDescribeItem(u8 index, enum Item *item, u16 *count, u32 *value);

u32 CofradiaMinimum(u8 difficulty);
u32 CofradiaCompound(u32 fund, u8 difficulty);
bool32 CofradiaGetView(struct CofradiaView *view);
enum CofradiaResult CofradiaBegin(u8 difficulty, u32 stake);
enum CofradiaResult CofradiaBeginCredit(u8 difficulty, u32 stake, u32 borrowed);
enum CofradiaResult CofradiaRepay(u32 amount);
enum CofradiaResult CofradiaRepayItem(enum Item item, u16 count);
bool32 CofradiaProcessDefault(void);
u32 CofradiaAppraisePokemon(struct Pokemon *mon);
bool32 CofradiaArm(void);
bool32 CofradiaDisarm(void);
bool32 CofradiaResolve(bool32 won);
enum CofradiaResult CofradiaCashOut(void);
bool32 CofradiaBattleActive(void);
u8 CofradiaBattleDifficulty(void);
u32 CofradiaBattleSeed(void);
u16 CofradiaPickTeamSource(u32 seed, u8 difficulty, bool32 boss, u8 ordinal);
u16 CofradiaGymPatternSource(u32 seed, u8 difficulty, u16 primary);
u8 CofradiaTeamSize(u8 difficulty, bool32 boss);
u8 CofradiaPickCounterTargets(struct Pokemon *party, u32 seed, u8 difficulty, u8 *slots);
s32 CofradiaCounterScore(struct Pokemon *candidate, struct Pokemon *target);
u16 CofradiaBattleDonor(u8 ordinal);
bool32 CofradiaBattleIsBoss(void);
void CofradiaScaleEnemyParty(void);
bool32 CofradiaPrepareBattle(void);
void CofradiaBattleEnded(u8 outcome);
void CofradiaStatusScript(void);
void CofradiaLobbyMenu(void);
void CofradiaChooseFunding(void);
void CofradiaFundingQuote(void);
void CofradiaAccountsMenu(void);
void CofradiaRepaymentMenu(void);
void CofradiaItemPaymentReset(void);
void CofradiaItemPaymentMenu(void);
void CofradiaItemPaymentChoose(void);
void CofradiaItemPaymentQuantityMenu(void);
void CofradiaItemPaymentQuote(void);
void CofradiaItemPaymentApply(void);
void CofradiaDefaultScript(void);
void CofradiaRepaymentQuote(void);
void CofradiaRepaymentApply(void);
void CofradiaCollectScript(void);
void CofradiaVaultMenu(void);
void CofradiaVaultEntryQuote(void);
void CofradiaVaultEntryReturn(void);
void CofradiaDifficultyMenu(void);
void CofradiaChooseDifficulty(void);
void CofradiaStakeMenu(void);
void CofradiaAdjustStake(void);
void CofradiaBeginScript(void);
void CofradiaArmScript(void);
void CofradiaCancelSaveScript(void);
void CofradiaStartBattleScript(void);
void CofradiaCashOutScript(void);
void CofradiaRecordsScript(void);
#endif
