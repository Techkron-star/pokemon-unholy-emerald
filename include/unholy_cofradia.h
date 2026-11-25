#ifndef GUARD_UNHOLY_COFRADIA_H
#define GUARD_UNHOLY_COFRADIA_H

enum CofradiaState { COFRADIA_IDLE, COFRADIA_READY, COFRADIA_FIGHTING };
enum CofradiaResult { COFRADIA_OK, COFRADIA_INVALID, COFRADIA_NO_MONEY, COFRADIA_BUSY, COFRADIA_CASH_FULL };
#define COFRADIA_DIFFICULTY_COUNT 6

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
};

u32 CofradiaMinimum(u8 difficulty);
u32 CofradiaCompound(u32 fund, u8 difficulty);
bool32 CofradiaGetView(struct CofradiaView *view);
enum CofradiaResult CofradiaBegin(u8 difficulty, u32 stake);
bool32 CofradiaArm(void);
bool32 CofradiaDisarm(void);
bool32 CofradiaResolve(bool32 won);
enum CofradiaResult CofradiaCashOut(void);
bool32 CofradiaBattleActive(void);
u8 CofradiaBattleDifficulty(void);
u32 CofradiaBattleSeed(void);
u16 CofradiaPickTeamSource(u32 seed, u8 difficulty, bool32 boss, u8 ordinal);
u8 CofradiaTeamSize(u8 difficulty, bool32 boss);
u16 CofradiaBattleDonor(u8 ordinal);
bool32 CofradiaBattleIsBoss(void);
void CofradiaScaleEnemyParty(void);
bool32 CofradiaPrepareBattle(void);
void CofradiaBattleEnded(u8 outcome);
void CofradiaStatusScript(void);
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
