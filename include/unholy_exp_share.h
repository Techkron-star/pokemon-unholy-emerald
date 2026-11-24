#ifndef GUARD_UNHOLY_EXP_SHARE_H
#define GUARD_UNHOLY_EXP_SHARE_H

struct Pokemon;

#define OVERLOADED_EXP_BONUS_PERCENT 25
#define OVERLOADED_EXP_PER_EGG_CYCLE 256

enum Item UnholyExpShare_GetActiveItem(void);
void UnholyExpShare_AdvanceEggs(u32 experience);
void UnholyExpShare_TrainIvs(struct Pokemon *mon, u8 previousLevel);
bool32 UnholyExpShare_FindReadyEgg(void);

#endif // GUARD_UNHOLY_EXP_SHARE_H
