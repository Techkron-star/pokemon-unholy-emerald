#ifndef GUARD_UNHOLY_STATUS_H
#define GUARD_UNHOLY_STATUS_H

enum UnholyStatus
{
    UNHOLY_STATUS_ROOTED,
    UNHOLY_STATUS_WEIGHTLESS,
    UNHOLY_STATUS_CONFINED,
    UNHOLY_STATUS_BLINDED,
    UNHOLY_STATUS_TERRIFIED,
    UNHOLY_STATUS_CHAINED,
    UNHOLY_STATUS_BANISHED,
    UNHOLY_STATUS_SANCTIFIED,
    UNHOLY_STATUS_DEMONIFIED,
    UNHOLY_STATUS_COUNT,
};

struct DamageContext;
u8 UnholyStatus_MoveLevel(enum Species species, enum Move move);
u8 UnholyStatus_GetTurns(u32 battler, enum UnholyStatus status);
bool32 UnholyStatus_Has(u32 battler, enum UnholyStatus status);
bool32 UnholyStatus_CanApply(u32 battler, enum UnholyStatus status);
bool32 UnholyStatus_Apply(u32 battler, enum UnholyStatus status, u8 turns);
void UnholyStatus_Clear(u32 battler);
void UnholyStatus_EndTurn(void);
bool32 UnholyStatus_PartyExiled(u32 trainer, u32 slot);
bool32 UnholyStatus_HasNegative(u32 battler);
bool32 UnholyStatus_CureNegative(u32 battler);
u32 UnholyStatus_AdjustAccuracy(u32 battler, u32 accuracy);
u32 UnholyStatus_AdjustAttack(u32 battler, u32 attack);
uq4_12_t UnholyStatus_AdjustStab(struct DamageContext *ctx, uq4_12_t original);
const u8 *UnholyStatus_Name(enum UnholyStatus status);
void UnholyStatus_FormatSummary(u32 battler, u8 *text);
void UnholyStatus_ApplyMove(void);

#endif
