#ifndef GUARD_BATTLE_UNHOLY_ABILITIES_H
#define GUARD_BATTLE_UNHOLY_ABILITIES_H

struct DamageContext;
struct BattleCalcValues;

bool32 Unholy_SideHasAbility(enum BattlerId battler, enum Ability ability);
bool32 Unholy_FieldHasAbility(enum Ability ability);
bool32 Unholy_EnemyHasAntithesis(enum BattlerId battler);
enum BattlerId Unholy_EnvyTarget(enum BattlerId battler);
bool32 Unholy_IsProjectile(enum Move move);
bool32 Unholy_IsFieldMove(enum Move move);
enum Type Unholy_ConvertMoveType(enum Move move, enum Ability ability, enum Type type);
enum Type Unholy_BestAttackType(enum BattlerId attacker, enum BattlerId target, enum Move move);
enum Type Unholy_BestDefenseType(enum Type attackType);
u32 Unholy_PhysicalShare(enum Ability ability, enum Move move);
uq4_12_t Unholy_AdjustTypeModifier(struct DamageContext *ctx, uq4_12_t modifier);
bool32 Unholy_BlocksMove(struct DamageContext *ctx);
bool32 Unholy_CanAbsorbProjectile(struct DamageContext *ctx);
enum BattlerId Unholy_ProjectileRedirector(struct BattleCalcValues *cv);
bool32 Unholy_AbilityEvent(u32 event, enum BattlerId battler, enum Ability ability, enum Move move, bool32 shouldTrigger);
void Unholy_PrepareTurn(void);
void Unholy_AdvanceTimers(void);
enum Item Unholy_EffectiveItem(enum BattlerId battler);
bool32 Unholy_ConsumeCopiedItem(enum BattlerId battler);
s32 Unholy_MalvadoPriority(enum BattlerId battler, enum Move move);
bool32 Unholy_SamuraiReflects(enum BattlerId defender, enum BattlerId attacker, enum Move move);
void Unholy_AdjustHealing(enum BattlerId battler);
s8 Unholy_AdjustStatChange(enum BattlerId battler, s8 stages);
u32 Unholy_TimeStep(enum BattlerId battler);

#endif
