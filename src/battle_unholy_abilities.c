#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_unholy_abilities.h"
#include "battle_scripts.h"
#include "battle_script_commands.h"
#include "battle_stat_change.h"
#include "battle_stamina.h"
#include "battle_chain.h"
#include "battle_controllers.h"
#include "battle_ai_record.h"
#include "gear.h"
#include "item.h"
#include "move.h"
#include "random.h"
#include "config/unholy_abilities.h"

bool32 Unholy_SideHasAbility(enum BattlerId battler, enum Ability ability)
{
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
        if (IsBattlerAlive(i) && IsBattlerAlly(i, battler) && GetBattlerAbility(i) == ability)
            return TRUE;
    return FALSE;
}

bool32 Unholy_FieldHasAbility(enum Ability ability)
{
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
        if (IsBattlerAlive(i) && GetBattlerAbility(i) == ability)
            return TRUE;
    return FALSE;
}

bool32 Unholy_EnemyHasAntithesis(enum BattlerId battler)
{
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
        if (IsBattlerAlive(i) && !IsBattlerAlly(i, battler) && GetBattlerAbility(i) == ABILITY_ANTITHESIS)
            return TRUE;
    return FALSE;
}

enum BattlerId Unholy_EnvyTarget(enum BattlerId battler)
{
    enum BattlerId target = (battler ^ BIT_SIDE);
    if (IsBattlerAlive(target))
        return target;
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
        if (IsBattlerAlive(i) && !IsBattlerAlly(battler, i))
            return i;
    return MAX_BATTLERS_COUNT;
}

// The upstream ballistic flag already includes seeds, bombs and launched balls.
// Neither pulse/wind flags nor beam-like animations imply a projectile.
bool32 Unholy_IsProjectile(enum Move move)
{
    return IsBallisticMove(move) && !IsBattleMoveStatus(move);
}

bool32 Unholy_IsFieldMove(enum Move move)
{
    enum MoveTarget target = GetMoveTarget(move);
    return IsBattleMoveStatus(move)
        && (target == TARGET_FIELD || target == TARGET_OPPONENTS_FIELD || target == TARGET_ALL_BATTLERS
         || GetMoveEffect(move) == EFFECT_REFLECT || GetMoveEffect(move) == EFFECT_LIGHT_SCREEN
         || GetMoveEffect(move) == EFFECT_SAFEGUARD || GetMoveEffect(move) == EFFECT_AURORA_VEIL
         || GetMoveEffect(move) == EFFECT_TAILWIND);
}

enum Type Unholy_ConvertMoveType(enum Move move, enum Ability ability, enum Type type)
{
    if (GetMoveType(move) == TYPE_NORMAL)
    {
        if (ability == ABILITY_DRAGONBORN)
            return TYPE_DRAGON;
        if (ability == ABILITY_ABISAL)
            return TYPE_WATER;
        if (ability == ABILITY_MALVADO)
            return TYPE_DARK;
    }
    if (ability == ABILITY_GELIDO && type == TYPE_WATER)
        return TYPE_ICE;
    return type;
}

static bool32 TypeIsAbsorbed(enum Ability ability, enum Type type)
{
    switch (ability)
    {
    case ABILITY_VOLT_ABSORB:
    case ABILITY_MOTOR_DRIVE:
    case ABILITY_LIGHTNING_ROD:
        return type == TYPE_ELECTRIC;
    case ABILITY_WATER_ABSORB:
    case ABILITY_DRY_SKIN:
    case ABILITY_STORM_DRAIN:
    case ABILITY_MAGMA_ARMOR:
        return type == TYPE_WATER;
    case ABILITY_FLASH_FIRE:
    case ABILITY_WELL_BAKED_BODY:
        return type == TYPE_FIRE;
    case ABILITY_SAP_SIPPER:
        return type == TYPE_GRASS;
    case ABILITY_EARTH_EATER:
        return type == TYPE_GROUND;
    default:
        return FALSE;
    }
}

enum Type Unholy_BestAttackType(enum BattlerId attacker, enum BattlerId target, enum Move move)
{
    enum Type best = GetMoveType(move);
    uq4_12_t bestMod = 0;
    if (target >= gBattlersCount)
        return best;
    struct DamageContext ctx = {.battlerAtk = attacker, .battlerDef = target, .move = move};
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
    {
        ctx.abilities[i] = GetBattlerAbility(i);
        ctx.holdEffects[i] = GetBattlerHoldEffect(i);
    }
    for (enum Type type = 0; type < NUMBER_OF_MON_TYPES; type++)
    {
        if (type == TYPE_MYSTERY || type == TYPE_STELLAR || TypeIsAbsorbed(ctx.abilities[target], type))
            continue;
        ctx.moveType = type;
        uq4_12_t mod = CalcTypeEffectivenessMultiplier(&ctx);
        if (ctx.abilities[target] == ABILITY_BLACK_HOLE && Unholy_IsProjectile(move) && mod <= UQ_4_12(1.0))
            mod = 0;
        if (mod > bestMod)
        {
            bestMod = mod;
            best = type;
        }
    }
    return best;
}

enum Type Unholy_BestDefenseType(enum Type attackType)
{
    enum Type best = TYPE_NORMAL;
    uq4_12_t bestMod = UQ_4_12(8.0);
    for (enum Type type = 0; type < NUMBER_OF_MON_TYPES; type++)
    {
        if (type == TYPE_MYSTERY || type == TYPE_STELLAR)
            continue;
        uq4_12_t mod = GetTypeModifier(attackType, type);
        if (mod < bestMod)
        {
            bestMod = mod;
            best = type;
        }
    }
    return best;
}

u32 Unholy_PhysicalShare(enum Ability ability, enum Move move)
{
    if (IsBattleMoveStatus(move))
        return 100;
    if (ability == ABILITY_GODHAND)
        return 50;
    if (ability == ABILITY_MONJE && GetMoveType(move) == TYPE_PSYCHIC)
        return 45;
    if (ability == ABILITY_ONI && GetMoveType(move) == TYPE_DARK && GetMoveCategory(move) == DAMAGE_CATEGORY_PHYSICAL)
        return 50;
    return 100; // No category split. Fallen Angel instead splits the type.
}

uq4_12_t Unholy_AdjustTypeModifier(struct DamageContext *ctx, uq4_12_t modifier)
{
    // Field-wide Eclipse affects exactly these four attack types. Preserve zero.
    if (modifier != 0 && Unholy_FieldHasAbility(ABILITY_GREAT_ECLIPSE)
     && (ctx->moveType == TYPE_FAIRY || ctx->moveType == TYPE_PSYCHIC
      || ctx->moveType == TYPE_ELECTRIC || ctx->moveType == TYPE_GRASS))
        modifier = UQ_4_12(1.0);
    if (modifier > UQ_4_12(1.0) && Unholy_SideHasAbility(ctx->battlerDef, ABILITY_ANTI_REALM))
        modifier = UQ_4_12(1.0);
    return modifier;
}

bool32 Unholy_BlocksMove(struct DamageContext *ctx)
{
    if (IsBattleMoveStatus(ctx->move) || IsBattlerAlly(ctx->battlerAtk, ctx->battlerDef))
        return FALSE;
    enum MoveTarget target = GetBattlerMoveTargetType(ctx->battlerAtk, ctx->move);
    return ((target == TARGET_BOTH || target == TARGET_FOES_AND_ALLY || target == TARGET_ALL_BATTLERS)
         && Unholy_SideHasAbility(ctx->battlerDef, ABILITY_SPACE_REALM))
        || (Unholy_IsProjectile(ctx->move) && Unholy_SideHasAbility(ctx->battlerDef, ABILITY_HYPERBARIC));
}

bool32 Unholy_CanAbsorbProjectile(struct DamageContext *ctx)
{
    return ctx->abilities[ctx->battlerDef] == ABILITY_BLACK_HOLE
        && !IsBattlerAlly(ctx->battlerAtk, ctx->battlerDef)
        && Unholy_IsProjectile(ctx->move)
        && CalcTypeEffectivenessMultiplier(ctx) <= UQ_4_12(1.0);
}

enum BattlerId Unholy_ProjectileRedirector(struct BattleCalcValues *cv)
{
    if (!IsDoubleBattle() || !Unholy_IsProjectile(cv->move)
     || (GetBattlerMoveTargetType(cv->battlerAtk, cv->move) != TARGET_SELECTED
      && GetBattlerMoveTargetType(cv->battlerAtk, cv->move) != TARGET_SMART))
        return MAX_BATTLERS_COUNT;
    for (u32 order = 0; order < gBattlersCount; order++)
    {
        enum BattlerId i = gBattlerByTurnOrder[order];
        if (IsBattlerAlive(i) && !IsBattlerAlly(cv->battlerAtk, i) && cv->abilities[i] == ABILITY_BLACK_HOLE)
        {
            struct DamageContext ctx = { .battlerAtk = cv->battlerAtk, .battlerDef = i, .move = cv->move,
                .moveType = GetBattleMoveType(cv->move) };
            for (u32 j = 0; j < gBattlersCount; j++)
            {
                ctx.abilities[j] = cv->abilities[j];
                ctx.holdEffects[j] = cv->holdEffects[j];
            }
            if (Unholy_CanAbsorbProjectile(&ctx))
                return i;
        }
    }
    return MAX_BATTLERS_COUNT;
}

void Unholy_PrepareTurn(void)
{
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
    {
        gBattleStruct->unholyReactorStartedMax[i] = GetBattlerAbility(i) == ABILITY_REACTOR
            && gBattleMons[i].statStages[STAT_SPATK] == MAX_STAT_STAGE;
        gBattleStruct->unholySamuraiCharges[i] = 0;
        gBattleStruct->unholyMalvadoPriority[i] = 0;
        gBattleStruct->unholyOriginalTarget[i] = gBattleStruct->moveTarget[i];
        if (!IsBattlerAlive(i) || gChosenActionByBattler[i] != B_ACTION_USE_MOVE)
            continue;
        enum Ability ability = GetBattlerAbility(i);
        enum Move move = GetBattlerChosenMove(i);
        if (ability == ABILITY_MALVADO && GetMoveType(move) == TYPE_DARK)
            gBattleStruct->unholyMalvadoPriority[i] = RandomPercentage(RNG_UNHOLY_MALVADO, U_MALVADO_PRIORITY_CHANCE);
        u32 second = gBattleStruct->chainSecondPos[i];
        if (ability == ABILITY_MALVADO && second && second <= MAX_MON_MOVES
         && GetMoveType(gBattleMons[i].moves[second - 1]) == TYPE_DARK)
            gBattleStruct->unholyMalvadoPriority[i] |= RandomPercentage(RNG_UNHOLY_MALVADO, U_MALVADO_PRIORITY_CHANCE) << 1;
        if (ability == ABILITY_SAMURAI)
        {
            if (IsSlicingMove(move))
                gBattleStruct->unholySamuraiCharges[i] += RandomPercentage(RNG_UNHOLY_SAMURAI, 50);
            if (second && second <= MAX_MON_MOVES && IsSlicingMove(gBattleMons[i].moves[second - 1]))
                gBattleStruct->unholySamuraiCharges[i] += RandomPercentage(RNG_UNHOLY_SAMURAI, 50);
        }
    }
}

s32 Unholy_MalvadoPriority(enum BattlerId battler, enum Move move)
{
    u32 flag = Chain_IsFollowingUp(battler) ? 2 : 1;
    u32 second = gBattleStruct->chainSecondPos[battler];
    if (second && second <= MAX_MON_MOVES && GetBattlerChosenMove(battler) != move
     && gBattleMons[battler].moves[second - 1] == move)
        flag = 2;
    return GetMoveType(move) == TYPE_DARK && (gBattleStruct->unholyMalvadoPriority[battler] & flag) != 0;
}

bool32 Unholy_SamuraiReflects(enum BattlerId defender, enum BattlerId attacker, enum Move move)
{
    return GetBattlerAbility(defender) == ABILITY_SAMURAI
        && gBattleStruct->unholySamuraiCharges[defender] != 0
        && !IsBattlerAlly(attacker, defender)
        && IsMoveMakingContact(attacker, defender, GetBattlerAbility(attacker), GetBattlerHoldEffect(attacker), move);
}

void Unholy_AdjustHealing(enum BattlerId battler)
{
    if (gBattleStruct->unholyHpAdjustedMask & (1u << battler))
        return;
    gBattleStruct->unholyHpAdjustedMask |= 1u << battler;
    if (!(gBattleStruct->unholyStatusHealingMask & (1u << battler))
     && gBattleStruct->passiveHpUpdate[battler] < 0 && Unholy_EnemyHasAntithesis(battler))
        gBattleStruct->passiveHpUpdate[battler] = -gBattleStruct->passiveHpUpdate[battler];
}

s8 Unholy_AdjustStatChange(enum BattlerId battler, s8 stages)
{
    if (stages > 0 && Unholy_EnemyHasAntithesis(battler))
        stages = -stages;
    if (stages < 0 && GetBattlerAbility(battler) == ABILITY_MONJE)
        return 0;
    return stages;
}

enum Item Unholy_EffectiveItem(enum BattlerId battler)
{
    enum Ability ability = GetBattlerAbility(battler);
    if (ability == ABILITY_GREEDY)
        return ITEM_NONE;
    if (ability != ABILITY_ENVY)
        return gBattleMons[battler].item;
    enum BattlerId target = Unholy_EnvyTarget(battler);
    if (target == MAX_BATTLERS_COUNT)
        return ITEM_NONE;
    u32 owner = GetBattlerTrainer(target) * PARTY_SIZE + gBattlerPartyIndexes[target] + 1;
    if (gBattleStruct->unholyEnvySourceOwner[battler] != owner)
    {
        gBattleStruct->unholyEnvySourceOwner[battler] = owner;
        gBattleStruct->unholyEnvyConsumedItem[battler] = ITEM_NONE;
    }
    enum Item item = gBattleMons[target].item;
    return item == gBattleStruct->unholyEnvyConsumedItem[battler] ? ITEM_NONE : item;
}

bool32 Unholy_ConsumeCopiedItem(enum BattlerId battler)
{
    if (GetBattlerAbility(battler) != ABILITY_ENVY)
        return FALSE;
    enum Item item = Unholy_EffectiveItem(battler);
    if (item == ITEM_NONE)
        return FALSE;
    gBattleStruct->unholyEnvyConsumedItem[battler] = item;
    return TRUE;
}

void Unholy_AdvanceTimers(void)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (!IsBattlerAlive(battler) || !Unholy_SideHasAbility(battler, ABILITY_TIME_REALM))
            continue;
        u32 extra = Unholy_TimeStep(battler) - 1;
        // Keep one tick for native expiry processing (which clears flags and
        // emits messages). Perish Song instead checks zero before ticking.
#define ADVANCE_CLOCK(field) do { \
    u32 timer = gBattleMons[battler].volatiles.field; \
    if (timer > 1) \
        gBattleMons[battler].volatiles.field = max(1, (s32)timer - (s32)extra); \
} while (0)
        ADVANCE_CLOCK(disableTimer);
        ADVANCE_CLOCK(encoreTimer);
        ADVANCE_CLOCK(embargoTimer);
        ADVANCE_CLOCK(magnetRiseTimer);
        ADVANCE_CLOCK(telekinesisTimer);
        ADVANCE_CLOCK(healBlockTimer);
        ADVANCE_CLOCK(tauntTimer);
        ADVANCE_CLOCK(laserFocusTimer);
        ADVANCE_CLOCK(throatChopTimer);
        ADVANCE_CLOCK(chargeTimer);
        ADVANCE_CLOCK(slowStartTimer);
        ADVANCE_CLOCK(cudChewTimer);
        ADVANCE_CLOCK(wrapTurns);
        ADVANCE_CLOCK(yawn);
        ADVANCE_CLOCK(lockOn);
#undef ADVANCE_CLOCK
        if (gBattleMons[battler].volatiles.perishSong)
        {
            u32 timer = gBattleMons[battler].volatiles.perishSongTimer;
            gBattleMons[battler].volatiles.perishSongTimer = timer > extra ? timer - extra : 0;
        }
        if (gBattleMons[battler].volatiles.rechargeTimer)
            gBattleMons[battler].volatiles.rechargeTimer = 0;
        // Wish belongs to this battler slot; its native handler owns healing.
        if (gBattleStruct->wish[battler].counter > 1)
            gBattleStruct->wish[battler].counter = 1;
    }
}

u32 Unholy_TimeStep(enum BattlerId battler)
{
    return Unholy_SideHasAbility(battler, ABILITY_TIME_REALM)
        ? 1 + RandomUniform(RNG_UNHOLY_TIME, 1, 2) : 1;
}

// Count only enemy KOs: recoil, friendly fire and residual damage do not qualify.
static u32 Unholy_NumFaintedEnemies(enum BattlerId attacker)
{
    u32 count = 0;
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
        if (!IsBattlerAlly(attacker, i) && IsBattlerTurnDamaged(i, EXCLUDING_SUBSTITUTES) && !IsBattlerAlive(i))
            count++;
    return count;
}

static bool32 AbilityHeal(enum BattlerId battler, u32 amount)
{
    if (gBattleMons[battler].volatiles.healBlockTimer || (IsBattlerAtMaxHp(battler) && !Unholy_EnemyHasAntithesis(battler)))
        return FALSE;
    SetHealAmount(battler, max(1, amount));
    gBattlerAbility = gBattleScripting.battler = battler;
    BattleScriptCall(BattleScript_UnholyAbilityHeal);
    return TRUE;
}

static bool32 ApplyStatus(enum BattlerId owner, enum BattlerId target, u32 effect)
{
    if (!IsBattlerAlive(target))
        return FALSE;
    gEffectBattler = target;
    gBattleScripting.battler = gBattlerAbility = owner;
    gBattleScripting.moveEffect = effect;
    BattleScriptCall(effect == MOVE_EFFECT_LEECH_SEED || effect == MOVE_EFFECT_FLINCH
        ? BattleScript_UnholyAbilityVolatile : BattleScript_AbilityStatusEffect);
    return TRUE;
}

bool32 Unholy_AbilityEvent(u32 event, enum BattlerId battler, enum Ability ability, enum Move move, bool32 shouldTrigger)
{
    if (event == ABILITYEFFECT_ON_SWITCHIN && ability == ABILITY_KNIGHT && shouldTrigger)
    {
        gEffectBattler = gBattlerAbility = battler;
        SetStatChange(battler, STAT_DEF, 1);
        SetStatChange(battler, STAT_SPDEF, 1);
        BattleScriptCall(BattleScript_AbilityStatChange);
        return TRUE;
    }
    if (event == ABILITYEFFECT_ENDTURN && IsBattlerAlive(battler))
    {
        if (ability == ABILITY_REACTOR)
        {
            gBattleStruct->unholyReactorReady[battler] = gBattleStruct->unholyReactorStartedMax[battler]
                && gBattleMons[battler].statStages[STAT_SPATK] == MAX_STAT_STAGE;
            if (CompareStat(battler, STAT_SPATK, MAX_STAT_STAGE, CMP_LESS_THAN, ability))
            {
                gEffectBattler = gBattlerAbility = battler;
                SetStatChange(battler, STAT_SPATK, 1);
                BattleScriptCall(BattleScript_AbilityStatChange);
                return TRUE;
            }
        }
        if (ability == ABILITY_MANTHRA)
        {
            bool32 meditate = gBattleMons[battler].volatiles.truantCounter;
            gBattleMons[battler].volatiles.truantCounter ^= 1;
            if (meditate)
            {
                bool32 cured = FALSE;
                if (gBattleMons[battler].status1 & STATUS1_ANY && RandomPercentage(RNG_UNHOLY_MANTHRA, 40))
                {
                    TryDeactivateSleepClause(battler, gBattlerPartyIndexes[battler]);
                    cured = TRUE;
                    gBattleMons[battler].status1 = 0;
                    gBattleMons[battler].volatiles.nightmare = FALSE;
                    BtlController_EmitSetMonData(battler, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, 4, &gBattleMons[battler].status1);
                    MarkBattlerForControllerExec(battler);
                }
                if (AbilityHeal(battler, gBattleMons[battler].maxHP / 3))
                    return TRUE;
                if (cured)
                {
                    gBattlerAbility = gBattleScripting.battler = battler;
                    BattleScriptCall(BattleScript_UnholyStatusCure);
                    return TRUE;
                }
            }
        }
        if (ability == ABILITY_DAWN_LIGHT && gBattleMons[battler].status1 & STATUS1_ANY
         && RandomPercentage(RNG_UNHOLY_DAWN, 30))
        {
            TryDeactivateSleepClause(battler, gBattlerPartyIndexes[battler]);
            gBattleMons[battler].status1 = 0;
            gBattleMons[battler].volatiles.nightmare = FALSE;
            BtlController_EmitSetMonData(battler, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, 4, &gBattleMons[battler].status1);
            MarkBattlerForControllerExec(battler);
            gEffectBattler = gBattlerAbility = gBattleScripting.battler = battler;
            SetStatChange(battler, STAT_SPATK, 1);
            SetHealAmount(battler, max(1, gBattleMons[battler].maxHP / 3));
            BattleScriptCall(BattleScript_UnholyDawnLight);
            return TRUE;
        }
    }
    if (event == ABILITYEFFECT_MOVE_END_FOES_FAINTED && ability == ABILITY_GREEDY
     && Unholy_IsFieldMove(move) && gBattleMons[battler].item == ITEM_NONE
     && RandomPercentage(RNG_UNHOLY_GREEDY, 30))
    {
        enum BattlerId target = Unholy_EnvyTarget(battler);
        if (target != MAX_BATTLERS_COUNT && CanStealItem(battler, target, gBattleMons[target].item)
         && GetBattlerAbility(target) != ABILITY_STICKY_HOLD && gBattleMons[target].item != ITEM_NONE)
        {
            StealTargetItem(battler, target, ITEM_NONE, FALSE);
            gBattlerAbility = battler;
            gEffectBattler = target;
            BattleScriptCall(BattleScript_MagicianActivates);
            return TRUE;
        }
    }
    if (event == ABILITYEFFECT_MOVE_END_FOES_FAINTED && ability == ABILITY_REAPER && IsBattlerAlive(battler))
    {
        u32 count = Unholy_NumFaintedEnemies(battler);
        if (count)
        {
            Stamina_RestoreFraction(battler, count, 3);
            return AbilityHeal(battler, gBattleMons[battler].maxHP / 3 * count);
        }
    }
    if (event == ABILITYEFFECT_MOVE_END_ATTACKER
     && IsBattlerTurnDamaged(gBattlerTarget, EXCLUDING_SUBSTITUTES)
     && !IsBattlerAlly(battler, gBattlerTarget))
    {
        bool32 first = !(gSpecialStatuses[battler].unholyEffectTargets & (1u << gBattlerTarget));
        gSpecialStatuses[battler].unholyEffectTargets |= 1u << gBattlerTarget;
        if (first)
        {
            if (ability == ABILITY_DRAGONBORN && GetMoveType(move) == TYPE_DRAGON && RandomPercentage(RNG_UNHOLY_DRAGONBORN, 20))
                return ApplyStatus(battler, gBattlerTarget, MOVE_EFFECT_PARALYSIS);
            if (ability == ABILITY_ABISAL && GetMoveType(move) == TYPE_WATER && RandomPercentage(RNG_UNHOLY_ABISAL, U_ABISAL_FREEZE_CHANCE))
                return ApplyStatus(battler, gBattlerTarget, MOVE_EFFECT_FREEZE);
            if (ability == ABILITY_WORLD_TREE && GetBattleMoveType(move) == TYPE_GRASS)
                return ApplyStatus(battler, gBattlerTarget, MOVE_EFFECT_LEECH_SEED);
            if (ability == ABILITY_WIZARD && GetBattleMoveType(move) == TYPE_PSYCHIC && RandomPercentage(RNG_UNHOLY_WIZARD, U_WIZARD_FLINCH_CHANCE))
                return ApplyStatus(battler, gBattlerTarget, MOVE_EFFECT_FLINCH);
            if (ability == ABILITY_BELICO && GetBattleMoveType(move) == TYPE_STEEL && RandomPercentage(RNG_UNHOLY_BELICO, U_BELICO_SPIKES_CHANCE))
            {
                enum BattleSide side = GetBattlerSide(gBattlerTarget);
                if (!IsHazardOnSide(side, HAZARDS_STEELSURGE))
                {
                    PushHazardTypeToQueue(side, HAZARDS_STEELSURGE);
                    gBattlerAbility = gBattleScripting.battler = battler;
                    BattleScriptCall(BattleScript_AbilityPopUp);
                    return TRUE;
                }
            }
        }
        if (ability == ABILITY_COLERICO && !gSpecialStatuses[battler].unholyPunchRaised && IsPunchingMove(move))
        {
            gSpecialStatuses[battler].unholyPunchRaised = TRUE;
            gEffectBattler = gBattlerAbility = battler;
            SetStatChange(battler, STAT_ATK, 1);
            BattleScriptCall(BattleScript_AbilityStatChange);
            return TRUE;
        }
        if (first && ability == ABILITY_GELIDO && IsMoveMakingContact(battler, gBattlerTarget, ability, GetBattlerHoldEffect(battler), move)
         && RandomPercentage(RNG_UNHOLY_GELIDO, 20))
            return ApplyStatus(battler, gBattlerTarget, MOVE_EFFECT_FROSTBITE);
    }
    if (event == ABILITYEFFECT_MOVE_END && IsBattlerTurnDamaged(battler, EXCLUDING_SUBSTITUTES)
     && !IsBattlerAlly(battler, gBattlerAttacker)
     && IsMoveMakingContact(gBattlerAttacker, battler, GetBattlerAbility(gBattlerAttacker), GetBattlerHoldEffect(gBattlerAttacker), move))
    {
        if (ability == ABILITY_MAGMA_ARMOR && RandomPercentage(RNG_UNHOLY_MAGMA, U_MAGMA_BURN_CHANCE))
            return ApplyStatus(battler, gBattlerAttacker, MOVE_EFFECT_BURN);
        if (ability == ABILITY_GELIDO && RandomPercentage(RNG_UNHOLY_GELIDO, 20))
            return ApplyStatus(battler, gBattlerAttacker, MOVE_EFFECT_FROSTBITE);
        if (ability == ABILITY_STEEL_SCALES && IsBattlerAlive(gBattlerAttacker)
         && !CanBattlerAvoidContactEffects(gBattlerAttacker, battler, GetBattlerAbility(gBattlerAttacker), GetBattlerHoldEffect(gBattlerAttacker), move)
         && GetBattlerAbility(gBattlerAttacker) != ABILITY_MAGIC_GUARD)
        {
            enum Type types[3];
            GetBattlerTypes(gBattlerAttacker, FALSE, types);
            u32 damage = GetTypeModifier(TYPE_STEEL, types[0]);
            if (types[1] != types[0])
                damage = uq4_12_multiply(damage, GetTypeModifier(TYPE_STEEL, types[1]));
            if (types[2] != TYPE_MYSTERY && types[2] != types[0] && types[2] != types[1])
                damage = uq4_12_multiply(damage, GetTypeModifier(TYPE_STEEL, types[2]));
            damage = uq4_12_multiply_by_int_half_down(damage, max(1, gBattleMons[gBattlerAttacker].maxHP / 8));
            if (damage)
            {
                SetPassiveDamageAmount(gBattlerAttacker, damage);
                gBattlerAbility = gBattleScripting.battler = battler;
                BattleScriptCall(BattleScript_RoughSkinActivates);
                return TRUE;
            }
        }
    }
    return FALSE;
}
