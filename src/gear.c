#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_z_move.h"
#include "battle_util.h"
#include "battle_ai_record.h"
#include "move.h"
#include "pokemon.h"
#include "gear.h"
#include "battle_stamina.h"
#include "battle_chain.h"
#include "graphics.h"
#include "item.h"
#include "data/gears.h"

STATIC_ASSERT(GEAR_COUNT <= GEAR_MAX_COUNT, TooManyGears);

// El ciclo vive en gBattleStruct (no toca el formato de guardado). Se recuerda
// al cambiar un Pokémon durante un combate para evitar recargas gratis.

enum Gear Gear_GetEquipped(struct Pokemon *mon)
{
    u32 gear = GetMonData(mon, MON_DATA_GEAR);

    return gear < GEAR_COUNT ? gear : GEAR_NONE;
}

void Gear_SetEquipped(struct Pokemon *mon, enum Gear gear)
{
    u32 value = gear < GEAR_COUNT ? gear : GEAR_NONE;

    SetMonData(mon, MON_DATA_GEAR, &value);
}

// Explicit mapping: item IDs and Gear IDs are independently append-only.
enum Gear Gear_FromItem(enum Item item)
{
    switch (item)
    {
    case ITEM_GEAR_VISOR_COSMICO: return GEAR_VISOR_COSMICO;
    case ITEM_GEAR_CORAZON_TITAN: return GEAR_CORAZON_TITAN;
    case ITEM_GEAR_SKI_MASK: return GEAR_SKI_MASK;
    case ITEM_GEAR_CETRO_SINIESTRO: return GEAR_CETRO_SINIESTRO;
    case ITEM_GEAR_IMPULSO_COSMICO: return GEAR_IMPULSO_COSMICO;
    case ITEM_GEAR_PERMAFROST_FRAGMENT: return GEAR_PERMAFROST_FRAGMENT;
    case ITEM_GEAR_STORM_STAFF: return GEAR_STORM_STAFF;
    case ITEM_GEAR_SHACKLE: return GEAR_SHACKLE;
    case ITEM_GEAR_ARMADURA_METEORICA: return GEAR_ARMADURA_METEORICA;
    case ITEM_GEAR_ESCUDO_PALADIN: return GEAR_ESCUDO_PALADIN;
#define EXTRA_ARTIFACT(id) case ITEM_GEAR_##id: return GEAR_##id;
    EXTRA_ARTIFACT(ESTANDARTE_GUERRA)
    EXTRA_ARTIFACT(TORRE_VIGILANCIA)
    EXTRA_ARTIFACT(FARO)
    EXTRA_ARTIFACT(ROMPEOLAS)
    EXTRA_ARTIFACT(RED_SILOS)
    EXTRA_ARTIFACT(TABLERO_TRIFASICO)
    EXTRA_ARTIFACT(CRT)
    EXTRA_ARTIFACT(DESHUMIDIFICADOR)
    EXTRA_ARTIFACT(BOMBA_ARIETE)
    EXTRA_ARTIFACT(GLICERINA)
    EXTRA_ARTIFACT(ABONO)
#undef EXTRA_ARTIFACT
#define RECENT_GEAR(id, gfx, path, name, description) case ITEM_GEAR_##id: return GEAR_##id;
#include "constants/recent_gears.h"
#undef RECENT_GEAR
#define NEW_GEAR(id, gfx, name, description) case ITEM_GEAR_##id: return GEAR_##id;
#include "constants/gear_batch8.h"
#undef NEW_GEAR
#define GEAR_BLOOD_VARIANT(type, name, moveTypeName, bagIcon) case ITEM_GEAR_BLOOD_##type: return GEAR_BLOOD_##type;
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
    default: return GEAR_NONE;
    }
}

enum Item Gear_ToItem(enum Gear gear)
{
    switch (gear)
    {
    case GEAR_VISOR_COSMICO: return ITEM_GEAR_VISOR_COSMICO;
    case GEAR_CORAZON_TITAN: return ITEM_GEAR_CORAZON_TITAN;
    case GEAR_SKI_MASK: return ITEM_GEAR_SKI_MASK;
    case GEAR_CETRO_SINIESTRO: return ITEM_GEAR_CETRO_SINIESTRO;
    case GEAR_IMPULSO_COSMICO: return ITEM_GEAR_IMPULSO_COSMICO;
    case GEAR_PERMAFROST_FRAGMENT: return ITEM_GEAR_PERMAFROST_FRAGMENT;
    case GEAR_STORM_STAFF: return ITEM_GEAR_STORM_STAFF;
    case GEAR_SHACKLE: return ITEM_GEAR_SHACKLE;
    case GEAR_ARMADURA_METEORICA: return ITEM_GEAR_ARMADURA_METEORICA;
    case GEAR_ESCUDO_PALADIN: return ITEM_GEAR_ESCUDO_PALADIN;
#define EXTRA_ARTIFACT(id) case GEAR_##id: return ITEM_GEAR_##id;
    EXTRA_ARTIFACT(ESTANDARTE_GUERRA)
    EXTRA_ARTIFACT(TORRE_VIGILANCIA)
    EXTRA_ARTIFACT(FARO)
    EXTRA_ARTIFACT(ROMPEOLAS)
    EXTRA_ARTIFACT(RED_SILOS)
    EXTRA_ARTIFACT(TABLERO_TRIFASICO)
    EXTRA_ARTIFACT(CRT)
    EXTRA_ARTIFACT(DESHUMIDIFICADOR)
    EXTRA_ARTIFACT(BOMBA_ARIETE)
    EXTRA_ARTIFACT(GLICERINA)
    EXTRA_ARTIFACT(ABONO)
#undef EXTRA_ARTIFACT
#define RECENT_GEAR(id, gfx, path, name, description) case GEAR_##id: return ITEM_GEAR_##id;
#include "constants/recent_gears.h"
#undef RECENT_GEAR
#define NEW_GEAR(id, gfx, name, description) case GEAR_##id: return ITEM_GEAR_##id;
#include "constants/gear_batch8.h"
#undef NEW_GEAR
#define GEAR_BLOOD_VARIANT(type, name, moveTypeName, bagIcon) case GEAR_BLOOD_##type: return ITEM_GEAR_BLOOD_##type;
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
    default: return ITEM_NONE;
    }
}

enum GearEquipResult Gear_TryEquipFromBag(struct Pokemon *mon, enum Item item)
{
    enum Gear requested = Gear_FromItem(item);
    enum Gear current;
    enum Item previousItem;

    if (mon == NULL || requested == GEAR_NONE)
        return GEAR_EQUIP_NOT_GEAR;
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return GEAR_EQUIP_NOT_GEAR;
    current = Gear_GetEquipped(mon);
    if (current == requested)
        return GEAR_EQUIP_ALREADY;
    if (!CheckBagHasItem(item, 1))
        return GEAR_EQUIP_NOT_OWNED;

    previousItem = Gear_ToItem(current);
    // No Pokémon mutation until the inventory exchange is complete.
    if (!RemoveBagItem(item, 1))
        return GEAR_EQUIP_NOT_OWNED;

    if (current != GEAR_NONE)
    {
        if (previousItem == ITEM_NONE || !AddBagItem(previousItem, 1))
        {
            AddBagItem(item, 1); // Restore the incoming Gear on failure.
            return GEAR_EQUIP_BAG_FULL;
        }
    }

    Gear_SetEquipped(mon, requested);
    return GEAR_EQUIP_OK;
}

s8 Gear_ModifyStatStageByAbility(enum Ability ability, s8 stages)
{
    if (ability == ABILITY_CONTRARY)
        return -stages;
    if (ability == ABILITY_SIMPLE)
        return 2 * stages;
    return stages;
}

bool32 Gear_ApplySelfStatChange(enum BattlerId battler, enum Stat stat, s8 stages)
{
    enum Ability ability;
    s32 oldStage, newStage;

    if (!GEARS_ENABLED || stat < STAT_ATK || stat > STAT_EVASION || stages == 0)
        return FALSE;

    // Self-inflicted Gear stat changes are not bypassed by the opponent's Mold Breaker.
    // Mirror Armor / Clear Body / Defiant / Competitive are foe-source interactions,
    // so they do not reflect or block the wearer's own Gear recoil.
    ability = GetBattlerAbilityIgnoreMoldBreaker(battler);
    stages = Gear_ModifyStatStageByAbility(ability, stages);
    oldStage = gBattleMons[battler].statStages[stat];
    newStage = min(MAX_STAT_STAGE, max(MIN_STAT_STAGE, oldStage + stages));
    if (newStage == oldStage)
        return FALSE;

    gBattleMons[battler].statStages[stat] = newStage;
    if (ability == ABILITY_CONTRARY || ability == ABILITY_SIMPLE)
        RecordAbilityBattle(battler, ability);
    return TRUE;
}

static void StartCycle(struct GearCycle *cycle, const struct GearInfo *info)
{
    if (info->trigger == GEAR_TRIGGER_TIMED)
    {
        cycle->phase = GEAR_PHASE_OVERLOAD;
        cycle->turns = max(info->overloadTurns, 1);
    }
    else
    {
        cycle->phase = GEAR_PHASE_ARMED;
        cycle->turns = 0;
    }
}

static void StartRecharge(struct GearCycle *cycle, const struct GearInfo *info)
{
    cycle->phase = GEAR_PHASE_RECHARGING;
    cycle->turns = max(info->cooldownTurns, 1);
}

static struct GearCycle *Prepare(enum BattlerId battler)
{
    u32 slot = gBattlerPartyIndexes[battler];

    if (slot >= PARTY_SIZE)
    {
        gBattleStruct->gearOwner[battler] = 0;
        return NULL;
    }

    // Dos puestos del mismo entrenador comparten equipo. Entrenadores
    // distintos pueden tener el mismo índice de equipo sin compartir el ciclo.
    enum BattleTrainer trainer = GetBattlerTrainer(battler);
    u8 owner = trainer * PARTY_SIZE + slot + 1;
    struct GearCycle *cycle = &gBattleStruct->gearCycles[trainer][slot];
    if (gBattleStruct->gearOwner[battler] != owner)
    {
        enum Gear gear = Gear_GetEquipped(&GetTrainerParty(trainer)[slot]);
        gBattleStruct->gearOwner[battler] = owner;
        gBattleStruct->gearActivations[battler] = 0;
        gBattleStruct->artifactLastAnnounced[battler] = 0;
        gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;
        gBattleStruct->meteorArmorPending[battler] = 0;

        if (cycle->gear != gear)
        {
            cycle->gear = gear;
            cycle->activations = 0;
            cycle->paladinLastBlockTurn = 0;
            StartCycle(cycle, &gGearsInfo[gear]);
        }
    }
    return cycle->gear == GEAR_NONE ? NULL : cycle;
}

// Solo los movimientos de daño que cumplan los filtros del Gear lo activan.
static bool32 IsCompatible(const struct GearInfo *info, enum Move move, enum Type moveType)
{
    enum DamageCategory category = GetMoveCategory(move);

    if (category == DAMAGE_CATEGORY_STATUS)
        return FALSE;
    if (info->typeMask != 0 && !(info->typeMask & GEAR_TYPE(moveType)))
        return FALSE;
    if (info->categoryMask != 0 && !(info->categoryMask & (1 << category)))
        return FALSE;
    return TRUE;
}

static u32 CurrentPercent(const struct GearCycle *cycle, enum Move move, enum Type moveType)
{
    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    u32 percent = 0;

    if (IsCompatible(info, move, moveType))
        percent = cycle->phase == GEAR_PHASE_RECHARGING ? info->residualPowerPercent : info->overloadPowerPercent;
    return percent != 0 ? percent : 100;
}

enum Gear Gear_GetBattlerGear(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return GEAR_NONE;
    const struct GearCycle *cycle = Prepare(battler);
    return cycle == NULL ? GEAR_NONE : cycle->gear;
}

enum GearPhase Gear_GetPhase(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return GEAR_PHASE_ARMED;

    const struct GearCycle *cycle = Prepare(battler);
    return cycle == NULL ? GEAR_PHASE_ARMED : cycle->phase;
}

u8 Gear_GetTurnsRemaining(enum BattlerId battler)
{
    if (!GEARS_ENABLED)
        return 0;
    const struct GearCycle *cycle = Prepare(battler);
    return cycle == NULL ? 0 : cycle->turns;
}

bool32 Gear_ActivateEvent(enum BattlerId battler)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED)
        return FALSE;
    cycle = Prepare(battler);
    if (cycle == NULL || cycle->phase != GEAR_PHASE_ARMED
     || gGearsInfo[cycle->gear].trigger != GEAR_TRIGGER_EVENT)
        return FALSE;

    StartRecharge(cycle, &gGearsInfo[cycle->gear]);
    gBattleStruct->gearActivations[battler]++;
    return TRUE;
}

uq4_12_t Gear_GetPowerModifier(enum BattlerId battler, enum Move move, enum Type moveType)
{
    const struct GearCycle *cycle;
    u32 percent;

    if (!GEARS_ENABLED)
        return UQ_4_12(1.0);
    cycle = Prepare(battler);
    if (cycle == NULL)
        return UQ_4_12(1.0);

    // The accuracy lens uses this snapshot for its accuracy window only.
    if (cycle->gear == GEAR_LENTE_CERTEZA)
        return UQ_4_12(1.0);

    // Si el movimiento ya se está ejecutando, vale lo que se fijó al empezar
    // (la sobrecarga ya se gastó y la fase actual es la de recarga).
    if (gBattleStruct->gearSnapshotMove[battler] == move)
        percent = gBattleStruct->gearSnapshotPercent[battler];
    else
        percent = CurrentPercent(cycle, move, moveType);
    return (percent * 4096 + 50) / 100;
}

bool32 Gear_StormStaffRepeatsMove(enum BattlerId battler, enum Move move)
{
    if (!IsBattleMoveSpecial(move)
     || Gear_GetBattlerGear(battler) != GEAR_STORM_STAFF
     || IsMultiHitMove(move)
     || GetMoveStrikeCount(move) > 1
     || GetMovePower(move) == 0
     || IsMoveParentalBondBanned(move)
     || IsZMove(move)
     || IsMaxMove(move))
        return FALSE;

    enum BattleMoveEffects effect = GetMoveEffect(move);
    if (gBattleMoveEffects[effect].twoTurnEffect)
        return FALSE;

    switch (effect)
    {
    case EFFECT_OHKO:
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_FIXED_HP_DAMAGE:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_ENDEAVOR:
    case EFFECT_BIDE:
    case EFFECT_FUTURE_SIGHT:
        return FALSE;
    default:
        return TRUE;
    }
}

bool32 Gear_SuppressesAdditionalEffect(enum BattlerId battler, enum Move move, const struct AdditionalEffect *effect)
{
    if (gBattleStruct->artifactReplicaKind[battler] != 0 && gSpecialStatuses[battler].multiHitOn)
        return TRUE;
    if (gSpecialStatuses[battler].stormStaffStrike && gSpecialStatuses[battler].multiHitOn)
        return TRUE;

    return effect->chance != 0 && IsBattleMoveSpecial(move)
        && Gear_GetBattlerGear(battler) == GEAR_STORM_STAFF;
}

static u32 GetWarBannerControlMask(enum BattlerId battler)
{
    const struct Volatiles *v = &gBattleMons[battler].volatiles;
    return (!!v->torment << 0) | (!!v->wrapped << 1) | (!!v->infatuation << 2)
         | (!!v->escapePrevention << 3) | (!!v->nightmare << 4)
         | (!!v->cursed << 5) | (!!v->saltCure << 6)
         | (!!v->syrupBomb << 7) | (!!v->leechSeed << 8)
         | (!!v->yawn << 9) | (!!v->disabledMove << 10)
         | (!!v->encoredMove << 11) | (!!v->healBlockTimer << 12)
         | (!!v->tauntTimer << 13) | (!!v->perishSong << 14);
}

// The engine already has a single-target multi-strike path. Artifact echoes
// borrow it instead of issuing new moves (which would spend PP and Stamina,
// replay chains, or trigger another Artifact recursively).
u8 Gear_TryStartExtraStrikes(enum BattlerId battler, enum Move move, enum BattlerId target, bool32 singleTarget)
{
    struct GearCycle *cycle = GEARS_ENABLED ? Prepare(battler) : NULL;
    enum BattleMoveEffects effect = GetMoveEffect(move);
    enum Gear gear;
    u8 count = 0, kind = 0;

    gBattleStruct->artifactReplicaTotal[battler] = 0;
    gBattleStruct->artifactReplicaKind[battler] = 0;
    if (!singleTarget || cycle == NULL || target >= gBattlersCount || IsBattlerAlly(battler, target)
     || IsBattleMoveStatus(move) || GetMovePower(move) == 0
     || IsMultiHitMove(move) || GetMoveStrikeCount(move) > 1 || IsMoveParentalBondBanned(move)
     || IsZMove(move) || IsMaxMove(move) || gBattleMoveEffects[effect].twoTurnEffect)
        return 0;
    switch (effect)
    {
    case EFFECT_OHKO:
    case EFFECT_LEVEL_DAMAGE:
    case EFFECT_PSYWAVE:
    case EFFECT_FIXED_HP_DAMAGE:
    case EFFECT_FIXED_PERCENT_DAMAGE:
    case EFFECT_FINAL_GAMBIT:
    case EFFECT_REFLECT_DAMAGE:
    case EFFECT_ENDEAVOR:
    case EFFECT_BIDE:
    case EFFECT_FUTURE_SIGHT:
        return 0;
    default:
        break;
    }

    gear = cycle->gear;
    if (gear == GEAR_RED_SILOS && IsBallisticMove(move)
     && move != MOVE_PIN_MISSILE && move != MOVE_SPIKE_CANNON
     && move != MOVE_BULLET_SEED)
    {
        count = RandomUniform(RNG_GEAR_SILOS, 1, 4);
        kind = 1;
    }
    else if (gear == GEAR_BOMBA_ARIETE && GetBattleMoveType(move) == TYPE_WATER && cycle->turns >= 2)
    {
        // A linked pair fires the extra Water strike on its LAST move, not the
        // opener. If the follow-up is cancelled, the charge is preserved.
        if (gBattleStruct->chainSecondPos[battler] != 0 && !Chain_IsFollowingUp(battler))
            return 0;
        count = 1;
        kind = 2;
        cycle->turns = 0;
    }
    else if (gear == GEAR_TABLERO_TRIFASICO && GetBattleMoveType(move) == TYPE_ELECTRIC
          && (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
          && target < gBattlersCount && IsBattlerAlive(GetPartnerBattler(target))
          && !IsBattlerAlly(battler, target))
    {
        count = 1;
        kind = 3;
    }

    if (count != 0)
    {
        gBattleStruct->artifactReplicaTotal[battler] = count;
        gBattleStruct->artifactReplicaKind[battler] = kind;
        gBattleStruct->gearActivations[battler]++;
    }
    return count;
}

u32 Gear_AdjustResolvedDamage(enum BattlerId attacker, enum BattlerId defender,
                             enum Move move, u32 damage, bool32 spread)
{
    u8 kind = gBattleStruct->artifactReplicaKind[attacker];
    u8 count = gBattleStruct->artifactReplicaTotal[attacker];

    if (damage == 0)
        return 0;
    if (kind != 0 && count != 0 && gSpecialStatuses[attacker].multiHitOn)
    {
        u8 index = count + 1 - gMultiHitCounter;
        u32 percent = kind == 1 ? (index <= 2 ? 15 : 10) : 25;
        damage = max(1, damage * percent / 100);
    }

    if (GetBattleMoveType(move) == TYPE_WATER && Gear_GetBattlerGear(defender) == GEAR_ROMPEOLAS)
    {
        struct GearCycle *cycle = Prepare(defender);
        if (cycle != NULL && cycle->turns >= 2)
        {
            damage = max(1, damage * (spread ? 60 : 75) / 100);
            cycle->turns = 0;
            gBattleStruct->gearActivations[defender]++;
        }
    }
    return damage;
}

enum BattlerId Gear_GetReboundTarget(enum BattlerId attacker, enum BattlerId originalTarget)
{
    enum BattlerId partner;
    if (gBattleStruct->artifactReplicaKind[attacker] != 3 || originalTarget >= gBattlersCount)
        return originalTarget;
    partner = GetPartnerBattler(originalTarget);
    return partner < gBattlersCount && IsBattlerAlive(partner) ? partner : originalTarget;
}

static void ArtifactSetStatus(enum BattlerId target, u32 status)
{
    gBattleMons[target].status1 = status;
    BtlController_EmitSetMonData(target, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE,
                                 0, sizeof(gBattleMons[target].status1), &gBattleMons[target].status1);
    MarkBattlerForControllerExec(target);
}

void Gear_OnExtraStrike(enum BattlerId attacker, enum BattlerId defender, enum Move move)
{
    if (gBattleStruct->artifactReplicaKind[attacker] == 3
     && gSpecialStatuses[attacker].multiHitOn
     && IsBattlerAlive(defender) && !IsBattlerAlly(attacker, defender)
     && CanBeParalyzed(attacker, defender, GetBattlerAbility(defender))
     && RandomPercentage(RNG_GEAR_TABLERO, 10))
        ArtifactSetStatus(defender, STATUS1_PARALYSIS);
}

void Gear_OnFireMoveHit(enum BattlerId attacker, enum BattlerId defender, enum Move move)
{
    if (Gear_GetBattlerGear(attacker) == GEAR_GLICERINA && GetBattleMoveType(move) == TYPE_FIRE
     && !gSpecialStatuses[attacker].multiHitOn
     && IsBattlerAlive(defender) && !IsBattlerAlly(attacker, defender)
     && CanBeBurned(attacker, defender, GetBattlerAbility(defender))
     && RandomPercentage(RNG_GEAR_GLICERINA, 20))
    {
        ArtifactSetStatus(defender, STATUS1_BURN);
        gBattleStruct->gearActivations[attacker]++;
    }
}

enum BattlerId Gear_TryWarBannerSupport(enum BattlerId attacker, enum BattlerId defender)
{
    enum BattlerId partner;
    bool32 control = FALSE;

    if (Gear_GetBattlerGear(attacker) != GEAR_ESTANDARTE_GUERRA || defender >= gBattlersCount
     || IsBattlerAlly(attacker, defender))
        return MAX_BATTLERS_COUNT;

    if ((gBattleMons[defender].status1 & ~gBattleStruct->warBannerInitialStatus[defender]) != 0
     || (gBattleMons[defender].volatiles.flinched && !gBattleStruct->warBannerInitialFlinch[defender])
     || (gBattleMons[defender].volatiles.confusionTimer > 0 && !gBattleStruct->warBannerInitialConfusion[defender])
     || (GetWarBannerControlMask(defender) & ~gBattleStruct->warBannerInitialControlMask[defender]))
        control = TRUE;
    for (enum Stat stat = STAT_ATK; stat <= STAT_EVASION; stat++)
    {
        if (gBattleMons[defender].statStages[stat] < gBattleStruct->warBannerInitialStages[defender][stat])
            control = TRUE;
    }
    // Consume this exact change even when several MoveEnd callbacks occur.
    gBattleStruct->warBannerInitialStatus[defender] = gBattleMons[defender].status1;
    memcpy(gBattleStruct->warBannerInitialStages[defender], gBattleMons[defender].statStages,
           sizeof(gBattleMons[defender].statStages));
    gBattleStruct->warBannerInitialFlinch[defender] = gBattleMons[defender].volatiles.flinched;
    gBattleStruct->warBannerInitialConfusion[defender] = gBattleMons[defender].volatiles.confusionTimer;
    gBattleStruct->warBannerInitialControlMask[defender] = GetWarBannerControlMask(defender);

    if (!control)
        return MAX_BATTLERS_COUNT;

    partner = (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? GetPartnerBattler(attacker) : attacker;
    if (partner >= gBattlersCount || !IsBattlerAlive(partner))
        return MAX_BATTLERS_COUNT;
    if (gBattleMons[partner].status1 != STATUS1_NONE && RandomPercentage(RNG_GEAR_ESTANDARTE, 30))
    {
        ArtifactSetStatus(partner, STATUS1_NONE);
    }
    else if (gBattleMons[partner].volatiles.confusionTimer
          && RandomPercentage(RNG_GEAR_ESTANDARTE, 30))
    {
        gBattleMons[partner].volatiles.confusionTimer = 0;
    }
    if (IsBattlerAtMaxHp(partner))
    {
        gBattleStruct->gearActivations[attacker]++;
        return MAX_BATTLERS_COUNT;
    }
    gBattleStruct->gearActivations[attacker]++;
    return partner;
}

void Gear_OnMoveUsed(enum BattlerId battler, enum Move move, enum Type moveType)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED)
        return;
    cycle = Prepare(battler);
    if (cycle == NULL)
        return;

    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    if (cycle->gear == GEAR_ESTANDARTE_GUERRA)
    {
        for (enum BattlerId other = 0; other < gBattlersCount; other++)
        {
            gBattleStruct->warBannerInitialStatus[other] = gBattleMons[other].status1;
            memcpy(gBattleStruct->warBannerInitialStages[other], gBattleMons[other].statStages,
                   sizeof(gBattleMons[other].statStages));
            gBattleStruct->warBannerInitialFlinch[other] = gBattleMons[other].volatiles.flinched;
            gBattleStruct->warBannerInitialConfusion[other] = gBattleMons[other].volatiles.confusionTimer;
            gBattleStruct->warBannerInitialControlMask[other] = GetWarBannerControlMask(other);
        }
    }
    if (cycle->gear == GEAR_AMPLIFICADOR)
    {
        bool32 charged = cycle->phase == GEAR_PHASE_ARMED
                       && !IsBattleMoveStatus(move) && IsSoundMove(move);
        gBattleStruct->gearSnapshotMove[battler] = move;
        gBattleStruct->gearSnapshotPercent[battler] = charged ? 125 : 100;
        if (charged)
            Gear_ActivateEvent(battler);
        return;
    }
    if (info->trigger == GEAR_TRIGGER_EVENT && cycle->gear == GEAR_LENTE_CERTEZA)
    {
        if (!IsBattleMoveStatus(move) && GetMoveAccuracy(move) != 0 && GetMoveEffect(move) != EFFECT_OHKO)
        {
            bool32 charged = cycle->phase == GEAR_PHASE_ARMED;
            gBattleStruct->gearSnapshotMove[battler] = move;
            gBattleStruct->gearSnapshotPercent[battler] = charged ? 101 : 110;
            if (charged)
                Gear_ActivateEvent(battler);
        }
        return;
    }
    if (info->trigger == GEAR_TRIGGER_PASSIVE || info->trigger == GEAR_TRIGGER_EVENT)
    {
        if (cycle->gear == GEAR_PIEDRA_AFILAR && IsSlicingMove(move)
         && !IsBattleMoveStatus(move) && Gear_ApplySelfStatChange(battler, STAT_ATK, 1))
        {
            // One stage per move execution, not per individual multihit.
            gBattleStruct->gearActivations[battler]++;
        }
        return;
    }

    gBattleStruct->gearSnapshotMove[battler] = move;
    gBattleStruct->gearSnapshotPercent[battler] = CurrentPercent(cycle, move, moveType);

    if (cycle->phase != GEAR_PHASE_RECHARGING && IsCompatible(info, move, moveType))
    {
        gBattleStruct->gearActivations[battler]++;
        if (info->trigger == GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE && cycle->phase == GEAR_PHASE_ARMED)
            StartRecharge(cycle, info);
    }
}

void Gear_OnDamaged(enum BattlerId battler, u32 damage)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED || damage == 0 || !IsBattlerAlive(battler))
        return;
    cycle = Prepare(battler);
    if (cycle == NULL)
        return;

    if (cycle->gear == GEAR_BOTON_PANICO && cycle->activations == 0
     && gBattleMons[battler].hp * 100 < gBattleMons[battler].maxHP * 30)
    {
        cycle->activations = 1;
        Stamina_RestorePercent(battler, 18);
        Gear_ApplySelfStatChange(battler, STAT_SPEED, 1);
        // Cancel before the queued second action can spend PP or Stamina.
        gBattleStruct->chainSecondPos[battler] = 0;
        gBattleStruct->gearActivations[battler]++;
    }
    else if (cycle->gear == GEAR_NUCLEO_HYDRA && cycle->activations < 2
          && cycle->phase == GEAR_PHASE_ARMED
          && gBattleMons[battler].hp * 2 < gBattleMons[battler].maxHP)
    {
        cycle->activations++;
        cycle->phase = GEAR_PHASE_OVERLOAD;
        // Triggering turn does not heal. The following three turns do.
        cycle->turns = 4;
        gBattleStruct->gearActivations[battler]++;
    }
}

bool32 Gear_TryMeteorArmorSurvive(enum BattlerId defender, enum BattlerId attacker,
                                 enum Move move, s32 damage, u16 *remainingHp)
{
    struct GearCycle *cycle;
    enum BattleMoveEffects effect;
    u32 overkill;
    u16 maxHp;

    if (!GEARS_ENABLED || remainingHp == NULL || damage <= 0 || defender == attacker
     || !IsBattlerAlive(defender)
     || IsBattlerAlly(attacker, defender) || gBattleMons[defender].hp > damage
     || IsBattleMoveStatus(move) || IsMultiHitMove(move) || GetMoveStrikeCount(move) > 1
     || gSpecialStatuses[attacker].multiHitOn || gSpecialStatuses[attacker].attackerInParty
     || GetBattlerAbility(attacker) == ABILITY_PARENTAL_BOND)
        return FALSE;

    effect = GetMoveEffect(move);
    if (gBattleMoveEffects[effect].twoTurnEffect || effect == EFFECT_FUTURE_SIGHT
     || effect == EFFECT_OHKO || effect == EFFECT_BIDE)
        return FALSE;

    cycle = Prepare(defender);
    if (cycle == NULL || cycle->gear != GEAR_ARMADURA_METEORICA
     || cycle->phase != GEAR_PHASE_ARMED || cycle->activations != 0)
        return FALSE;

    maxHp = gBattleMons[defender].maxHP;
    if (maxHp == 0)
        return FALSE;
    overkill = (u32)damage - gBattleMons[defender].hp;
    gBattleStruct->meteorArmorPending[defender] = overkill >= (maxHp * 3 + 3) / 4 ? 3
                                               : overkill >= (maxHp + 3) / 4 ? 2 : 1;
    *remainingHp = max(1, (maxHp * 30 + 99) / 100);

    // One activation for the entire fight, including switching and entering again.
    // Keep its HUD discharged rather than advertising another activation in 3 turns.
    Gear_ActivateEvent(defender);
    cycle->activations = 1;
    cycle->turns = 0;
    return TRUE;
}

static void MeteorArmorBurn(enum BattlerId defender, enum BattlerId target)
{
    if (!IsBattlerAlive(target)
     || !CanBeBurned(defender, target, GetBattlerAbility(target)))
        return;

    gBattleMons[target].status1 = STATUS1_BURN;
    BtlController_EmitSetMonData(target, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE,
                                 0, sizeof(gBattleMons[target].status1), &gBattleMons[target].status1);
    MarkBattlerForControllerExec(target);
}

bool32 Gear_ResolveMeteorArmorAsh(enum BattlerId defender, enum BattlerId attacker)
{
    u8 tier = gBattleStruct->meteorArmorPending[defender];

    if (tier == 0)
        return FALSE;
    gBattleStruct->meteorArmorPending[defender] = 0;
    Gear_ApplySelfStatChange(defender, STAT_DEF, -(s8)tier);
    Gear_ApplySelfStatChange(defender, STAT_SPDEF, -(s8)tier);

    MeteorArmorBurn(defender, attacker);
    if (tier == 3 && (gBattleTypeFlags & BATTLE_TYPE_DOUBLE))
        MeteorArmorBurn(defender, GetPartnerBattler(attacker));
    return TRUE;
}

u16 Gear_GetPaladinChanceTenths(enum BattlerId battler)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED || battler >= gBattlersCount || !IsBattlerAlive(battler))
        return 0;
    cycle = Prepare(battler);
    if (cycle == NULL || cycle->gear != GEAR_ESCUDO_PALADIN
     || cycle->paladinLastBlockTurn == gBattleStruct->paladinTurnSerial + 1)
        return 0;
    if (cycle->turns >= 8)
        return 1000;
    return 125 + min(cycle->turns, 5) * 25;
}

bool32 Gear_TryPaladinBlock(enum BattlerId defender)
{
    u16 chance = Gear_GetPaladinChanceTenths(defender);
    struct GearCycle *cycle;

    // No RNG consumption when it cannot activate; guaranteed pity uses no roll.
    if (chance == 0 || (chance < 1000 && RandomUniform(RNG_GEAR_PALADIN, 0, 999) >= chance))
        return FALSE;
    cycle = Prepare(defender);
    cycle->paladinLastBlockTurn = gBattleStruct->paladinTurnSerial + 1;
    cycle->turns = 0;
    gBattleStruct->gearActivations[defender]++;
    return TRUE;
}

u32 Gear_ApplyPaladinChainPenalty(enum BattlerId attacker, enum BattlerId defender, u32 damage)
{
    if (attacker < gBattlersCount && defender < gBattlersCount
     && gBattleStruct->paladinWeakenedChainTarget[attacker] == defender + 1)
        return damage * 75 / 100;
    return damage;
}

void Gear_OnProtectSuccess(enum BattlerId battler)
{
    if ((gBattleTypeFlags & BATTLE_TYPE_DOUBLE) || Gear_GetBattlerGear(battler) != GEAR_CASCO_HOPLITA)
        return;
    if (Gear_ActivateEvent(battler))
        Stamina_RestorePercent(battler, 12);
}

bool32 Gear_WeatherBonusesSuppressed(enum BattlerId battler)
{
    if (!GEARS_ENABLED || gBattleStruct == NULL || battler >= gBattlersCount)
        return FALSE;
    return gBattleStruct->faroSuppressionTurn[GetBattlerSide(battler)] == gBattleStruct->paladinTurnSerial + 1;
}

void Gear_OnScreenSet(enum BattlerId battler, u16 *duration)
{
    if (duration != NULL && Gear_GetBattlerGear(battler) == GEAR_CRT)
    {
        *duration = min(16, *duration * 2);
        gBattleStruct->gearActivations[battler]++;
    }
}

void Gear_OnProtectExtended(enum BattlerId battler)
{
    enum BattlerId partner;
    if (!(gBattleTypeFlags & BATTLE_TYPE_DOUBLE) || Gear_GetBattlerGear(battler) != GEAR_CRT)
        return;
    partner = GetPartnerBattler(battler);
    if (partner >= gBattlersCount || !IsBattlerAlive(partner))
        return;
    gProtectStructs[partner].protected = gProtectStructs[battler].protected;
    gBattleStruct->gearActivations[battler]++;
}

u32 Gear_GetEndTurnHeal(enum BattlerId battler)
{
    struct GearCycle *cycle = Prepare(battler);

    if (cycle == NULL || !IsBattlerAlive(battler) || IsBattlerAtMaxHp(battler))
        return 0;
    if (cycle->gear == GEAR_NUCLEO_HYDRA && cycle->phase == GEAR_PHASE_OVERLOAD && cycle->turns <= 3)
        return max(1, gBattleMons[battler].maxHP * 8 / 100);
    if (cycle->gear == GEAR_ABONO && IS_BATTLER_OF_TYPE(battler, TYPE_GRASS)
     && (gFieldTimers.terrain == B_TERRAIN_NONE || gFieldTimers.terrain == B_TERRAIN_GRASSY))
        return max(1, gBattleMons[battler].maxHP / 16) * (gBattleMons[battler].volatiles.root ? 2 : 1);
    return 0;
}

void Gear_EndOfTurn(enum BattlerId battler)
{
    struct GearCycle *cycle;

    if (!GEARS_ENABLED)
        return;
    gBattleStruct->gearSnapshotMove[battler] = MOVE_NONE;
    cycle = Prepare(battler);
    if (cycle == NULL)
        return;

    if (cycle->gear == GEAR_ESCUDO_PALADIN)
    {
        if (cycle->paladinLastBlockTurn != gBattleStruct->paladinTurnSerial + 1)
            cycle->turns = min(cycle->turns + 1, 8);
        return;
    }

    // The meteorite has broken: no reactivation until a new battle.
    if (cycle->gear == GEAR_ARMADURA_METEORICA && cycle->activations != 0)
        return;

    if (cycle->gear == GEAR_NIHIL_ENGINE)
    {
        // Nihil normalizes ONE opposing hazard every third active turn.
        // The timer belongs to the trainer's party slot, not the field slot.
        if (++cycle->turns >= 3)
        {
            enum BattleSide otherSide = GetBattlerSide(battler) == B_SIDE_PLAYER ? B_SIDE_OPPONENT : B_SIDE_PLAYER;
            cycle->turns = 0;
            for (enum Hazards hazard = HAZARDS_SPIKES; hazard < HAZARDS_MAX_COUNT; hazard++)
            {
                if (IsHazardOnSide(otherSide, hazard))
                {
                    RemoveHazardFromField(otherSide, hazard);
                    gBattleStruct->gearActivations[battler]++;
                    break;
                }
            }
        }
        return;
    }

    if (cycle->gear == GEAR_TORRE_VIGILANCIA && ++cycle->turns >= 3)
    {
        cycle->turns = 0;
        if (gBattleMons[battler].statStages[STAT_ACC] < DEFAULT_STAT_STAGE)
            gBattleMons[battler].statStages[STAT_ACC] = DEFAULT_STAT_STAGE;
        if (Gear_ApplySelfStatChange(battler, STAT_ACC, 1))
            gBattleStruct->gearActivations[battler]++;
        return;
    }
    if (cycle->gear == GEAR_FARO && ++cycle->turns >= 2)
    {
        enum BattleSide other = GetBattlerSide(battler) == B_SIDE_PLAYER ? B_SIDE_OPPONENT : B_SIDE_PLAYER;
        cycle->turns = 0;
        if (GetWeather() != B_WEATHER_NONE)
        {
            gBattleStruct->faroSuppressionTurn[other] = gBattleStruct->paladinTurnSerial + 2;
            gBattleStruct->gearActivations[battler]++;
        }
        return;
    }
    if (cycle->gear == GEAR_DESHUMIDIFICADOR && ++cycle->turns >= 2)
    {
        cycle->turns = 0;
        if (GetWeather() != B_WEATHER_NONE && gBattleStruct->weatherDuration > 1)
        {
            u8 reduction = RandomUniform(RNG_GEAR_DESHUMIDIFICADOR, 1, 2);
            gBattleStruct->weatherDuration = max(1, gBattleStruct->weatherDuration - reduction);
            gBattleStruct->gearActivations[battler]++;
        }
        return;
    }
    if (cycle->gear == GEAR_BOMBA_ARIETE || cycle->gear == GEAR_ROMPEOLAS)
    {
        cycle->turns = min(2, cycle->turns + 1);
        return;
    }

    const struct GearInfo *info = &gGearsInfo[cycle->gear];
    switch (cycle->phase)
    {
    case GEAR_PHASE_OVERLOAD:
        if (--cycle->turns == 0)
            StartRecharge(cycle, info);
        break;
    case GEAR_PHASE_RECHARGING:
        if (--cycle->turns == 0)
            StartCycle(cycle, info);
        break;
    default:
        break;
    }
}
