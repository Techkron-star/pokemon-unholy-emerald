#include "global.h"
#include "constants/characters.h"
#include "battle.h"
#include "battle_chain.h"
#include "battle_controllers.h"
#include "battle_scripts.h"
#include "gear.h"
#include "move.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "unholy_localization.h"
#include "unholy_status.h"
#include "test_runner.h"

STATIC_ASSERT(sizeof(struct PokemonSubstruct0) == 12, ExileMetadataPreservesSubstructureSize);

static const u8 sDurations[UNHOLY_STATUS_COUNT] = {3, 3, 2, 3, 2, 2, 0, 4, 4};
static const enum UnholyStatus sNegative[] = {
    UNHOLY_STATUS_ROOTED, UNHOLY_STATUS_CONFINED, UNHOLY_STATUS_BLINDED,
    UNHOLY_STATUS_TERRIFIED, UNHOLY_STATUS_CHAINED,
};

const u8 *UnholyStatus_Name(enum UnholyStatus status)
{
    static const u8 *const en[] = {
        COMPOUND_STRING("Rooted"), COMPOUND_STRING("Weightless"), COMPOUND_STRING("Confined"),
        COMPOUND_STRING("Blinded"), COMPOUND_STRING("Terrified"), COMPOUND_STRING("Chained"),
        COMPOUND_STRING("Banished"), COMPOUND_STRING("Sanctified"), COMPOUND_STRING("Demonified"),
    };
    static const u8 *const es[] = {
        COMPOUND_STRING("Enraizado"), COMPOUND_STRING("Ingravidez"), COMPOUND_STRING("Confinado"),
        COMPOUND_STRING("Cegado"), COMPOUND_STRING("Aterrado"), COMPOUND_STRING("Encadenado"),
        COMPOUND_STRING("Desterrado"), COMPOUND_STRING("Santificado"), COMPOUND_STRING("Demonificado"),
    };
    return (u32)status < UNHOLY_STATUS_COUNT ? UnholyLocalize(en[status], es[status]) : COMPOUND_STRING("");
}

bool32 UnholyStatus_PartyExiled(u32 trainer, u32 slot)
{
    return gBattleStruct != NULL && trainer < MAX_BATTLE_TRAINERS && slot < PARTY_SIZE
        && gBattleStruct->partyState[trainer][slot].exileTurns != 0;
}

u8 UnholyStatus_GetTurns(u32 battler, enum UnholyStatus status)
{
    if (gBattleStruct == NULL || battler >= gBattlersCount || (u32)status >= UNHOLY_STATUS_COUNT)
        return 0;
    if (status == UNHOLY_STATUS_BANISHED)
        return GetBattlerPartyState(battler)->exileTurns;
    if (status == UNHOLY_STATUS_CHAINED)
        return gBattleStruct->shackleTurns[battler];
    return gBattleStruct->customStatusTurns[battler][status];
}

bool32 UnholyStatus_Has(u32 battler, enum UnholyStatus status)
{
    return UnholyStatus_GetTurns(battler, status) != 0;
}

bool32 UnholyStatus_CanApply(u32 battler, enum UnholyStatus status)
{
    enum Ability ability;
    if (gBattleStruct == NULL || battler >= gBattlersCount || (u32)status >= UNHOLY_STATUS_COUNT
     || !IsBattlerAlive(battler) || UnholyStatus_Has(battler, UNHOLY_STATUS_BANISHED)
     || UnholyStatus_Has(battler, status))
        return FALSE;
    ability = GetBattlerAbility(battler);
    if (status == UNHOLY_STATUS_TERRIFIED && ability == ABILITY_BELICO)
        return FALSE;
    if (status == UNHOLY_STATUS_BLINDED && (ability == ABILITY_KEEN_EYE || ability == ABILITY_MINDS_EYE))
        return FALSE;
    if (status == UNHOLY_STATUS_WEIGHTLESS && (UnholyStatus_Has(battler, UNHOLY_STATUS_ROOTED)
        || gBattleMons[battler].volatiles.root || (gFieldStatuses & STATUS_FIELD_GRAVITY)))
        return FALSE;
    if (status == UNHOLY_STATUS_BANISHED)
    {
        if (GetMonData(GetBattlerMon(battler), MON_DATA_HAS_BEEN_EXILED))
            return FALSE;
        // No impossible party screens; link/Arena need their own format rules.
        if ((gBattleTypeFlags & BATTLE_TYPE_ARENA) || (!IsDoubleBattle() && !CanBattlerSwitch(battler)))
            return FALSE;
        if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
        {
            // The emulator harness uses recorded-link controllers for local
            // battle tests; real link battles remain explicitly unsupported.
#if TESTING
            if (!gTestRunnerEnabled)
#endif
                return FALSE;
        }
    }
    return TRUE;
}

void UnholyStatus_Clear(u32 battler)
{
    if (gBattleStruct == NULL || battler >= MAX_BATTLERS_COUNT)
        return;
    memset(gBattleStruct->customStatusTurns[battler], 0, sizeof(gBattleStruct->customStatusTurns[battler]));
    gBattleStruct->customStatusFresh[battler] = 0;
    gBattleStruct->shackleTurns[battler] = 0;
    // Party exile timers deliberately survive the forced switch in singles.
}

bool32 UnholyStatus_Apply(u32 battler, enum UnholyStatus status, u8 turns)
{
    if (!UnholyStatus_CanApply(battler, status))
        return FALSE;
    if (turns == 0)
        turns = status == UNHOLY_STATUS_BANISHED ? RandomUniform(RNG_UNHOLY_BANISH, 1, 5) : sDurations[status];
    turns = min(turns, 5);
    if (status == UNHOLY_STATUS_BANISHED)
    {
        u8 permanent = TRUE;
        struct Pokemon *mon = GetBattlerMon(battler);
        UnholyStatus_Clear(battler);
        CancelMultiTurnMoves(battler);
        memset(&gBattleMons[battler].volatiles, 0, sizeof(gBattleMons[battler].volatiles));
        gBattleMons[battler].status1 = 0;
        TryDeactivateSleepClause(battler, gBattlerPartyIndexes[battler]);
        SetMonData(mon, MON_DATA_STATUS, &gBattleMons[battler].status1);
        SetMonData(mon, MON_DATA_HAS_BEEN_EXILED, &permanent);
        GetBattlerPartyState(battler)->exileTurns = turns;
        Chain_InvalidateFollowUps();
        BtlController_EmitSetMonData(battler, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, 4, &gBattleMons[battler].status1);
        MarkBattlerForControllerExec(battler);
    }
    else if (status == UNHOLY_STATUS_CHAINED)
        gBattleStruct->shackleTurns[battler] = turns; // Existing Grillete semantics.
    else
    {
        if (status == UNHOLY_STATUS_ROOTED)
            gBattleStruct->customStatusTurns[battler][UNHOLY_STATUS_WEIGHTLESS] = 0;
        if (status == UNHOLY_STATUS_SANCTIFIED || status == UNHOLY_STATUS_DEMONIFIED)
        {
            gBattleStruct->customStatusTurns[battler][UNHOLY_STATUS_SANCTIFIED] = 0;
            gBattleStruct->customStatusTurns[battler][UNHOLY_STATUS_DEMONIFIED] = 0;
        }
        gBattleStruct->customStatusTurns[battler][status] = turns;
        gBattleStruct->customStatusFresh[battler] |= 1 << status;
    }
    return TRUE;
}

void UnholyStatus_EndTurn(void)
{
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        for (u32 status = 0; status < UNHOLY_STATUS_COUNT; status++)
            if (gBattleStruct->customStatusTurns[battler][status] != 0
             && !(gBattleStruct->customStatusFresh[battler] & (1 << status)))
                gBattleStruct->customStatusTurns[battler][status]--;
        gBattleStruct->customStatusFresh[battler] = 0;
    }
    for (u32 trainer = 0; trainer < MAX_BATTLE_TRAINERS; trainer++)
    {
        for (u32 slot = 0; slot < PARTY_SIZE; slot++)
            if (gBattleStruct->partyState[trainer][slot].exileTurns != 0)
                gBattleStruct->partyState[trainer][slot].exileTurns--;
    }
}

bool32 UnholyStatus_HasNegative(u32 battler)
{
    for (u32 i = 0; i < ARRAY_COUNT(sNegative); i++)
        if (UnholyStatus_Has(battler, sNegative[i]))
            return TRUE;
    return FALSE;
}

bool32 UnholyStatus_CureNegative(u32 battler)
{
    if (UnholyStatus_Has(battler, UNHOLY_STATUS_BANISHED))
        return FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(sNegative); i++)
    {
        enum UnholyStatus status = sNegative[i];
        if (UnholyStatus_Has(battler, status))
        {
            if (status == UNHOLY_STATUS_CHAINED)
                gBattleStruct->shackleTurns[battler] = 0;
            else
                gBattleStruct->customStatusTurns[battler][status] = 0;
            StringCopy(gBattleTextBuff1, UnholyStatus_Name(status));
            StringCopy(gBattleTextBuff2, UnholyLocalize(COMPOUND_STRING("cured"), COMPOUND_STRING("curado")));
            return TRUE; // One removal per Healer activation, not a full cleanse.
        }
    }
    return FALSE;
}

u32 UnholyStatus_AdjustAccuracy(u32 battler, u32 accuracy)
{
    return accuracy != 0 && UnholyStatus_Has(battler, UNHOLY_STATUS_BLINDED) ? max(1, accuracy * 75 / 100) : accuracy;
}

u32 UnholyStatus_AdjustAttack(u32 battler, u32 attack)
{
    return UnholyStatus_Has(battler, UNHOLY_STATUS_TERRIFIED) ? max(1, attack * 80 / 100) : attack;
}

uq4_12_t UnholyStatus_AdjustStab(struct DamageContext *ctx, uq4_12_t original)
{
    bool32 positive, negative, favorable;
    if (original <= UQ_4_12(1.0))
        return original;
    positive = ctx->moveType == TYPE_PSYCHIC || ctx->moveType == TYPE_FAIRY
            || ctx->moveType == TYPE_NORMAL || ctx->moveType == TYPE_FIGHTING;
    negative = ctx->moveType == TYPE_POISON || ctx->moveType == TYPE_GHOST
            || ctx->moveType == TYPE_DRAGON || ctx->moveType == TYPE_FIRE;
    if ((!positive && !negative) || (!UnholyStatus_Has(ctx->battlerAtk, UNHOLY_STATUS_SANCTIFIED)
        && !UnholyStatus_Has(ctx->battlerAtk, UNHOLY_STATUS_DEMONIFIED)))
        return original;
    favorable = UnholyStatus_Has(ctx->battlerAtk, UNHOLY_STATUS_SANCTIFIED) ? positive : negative;
    return favorable ? original + UQ_4_12(0.25) : UQ_4_12(0.75);
}

void UnholyStatus_FormatSummary(u32 battler, u8 *text)
{
    static const enum UnholyStatus priority[] = {
        UNHOLY_STATUS_BANISHED, UNHOLY_STATUS_CHAINED, UNHOLY_STATUS_CONFINED,
        UNHOLY_STATUS_ROOTED, UNHOLY_STATUS_BLINDED, UNHOLY_STATUS_TERRIFIED,
        UNHOLY_STATUS_WEIGHTLESS, UNHOLY_STATUS_SANCTIFIED, UNHOLY_STATUS_DEMONIFIED,
    };
    text[0] = EOS;
    for (u32 i = 0; i < ARRAY_COUNT(priority); i++)
    {
        u8 turns = UnholyStatus_GetTurns(battler, priority[i]);
        if (turns)
        {
            u8 *cursor = StringCopy(text, UnholyStatus_Name(priority[i]));
            *cursor++ = CHAR_SPACE;
            ConvertUIntToDecimalStringN(cursor, turns, STR_CONV_MODE_LEFT_ALIGN, 1);
            return;
        }
    }
}

void UnholyStatus_ApplyMove(void)
{
    enum UnholyStatus status = gMovesInfo[gCurrentMove].argument.status;
    gBattleCommunication[MULTISTRING_CHOOSER] = 0;
    if (!UnholyStatus_CanApply(gBattlerTarget, status)
     || DoesSubstituteBlockMove(gBattlerAttacker, gBattlerTarget, gCurrentMove))
        return;
    if (!IsBattlerAlly(gBattlerAttacker, gBattlerTarget) && Gear_TryPaladinBlock(gBattlerTarget))
        return;
    if (!UnholyStatus_Apply(gBattlerTarget, status, 0))
        return;
    gBattleCommunication[MULTISTRING_CHOOSER] = status == UNHOLY_STATUS_BANISHED && !IsDoubleBattle() ? 2 : 1;
    u8 *cursor = StringCopy(gBattleTextBuff1, UnholyStatus_Name(status));
    cursor = StringCopy(cursor, COMPOUND_STRING(" ("));
    cursor = ConvertUIntToDecimalStringN(cursor, UnholyStatus_GetTurns(gBattlerTarget, status), STR_CONV_MODE_LEFT_ALIGN, 1);
    StringCopy(cursor, COMPOUND_STRING(")"));
}

u8 UnholyStatus_MoveLevel(enum Species species, enum Move move)
{
    bool32 grass, psychic, ghost, dark, poison, dragon, steel, fairy, fighting, fire;
    enum Type a = GetSpeciesType(species, 0), b = GetSpeciesType(species, 1);
    grass = a == TYPE_GRASS || b == TYPE_GRASS;
    psychic = a == TYPE_PSYCHIC || b == TYPE_PSYCHIC;
    ghost = a == TYPE_GHOST || b == TYPE_GHOST;
    dark = a == TYPE_DARK || b == TYPE_DARK;
    poison = a == TYPE_POISON || b == TYPE_POISON;
    dragon = a == TYPE_DRAGON || b == TYPE_DRAGON;
    steel = a == TYPE_STEEL || b == TYPE_STEEL;
    fairy = a == TYPE_FAIRY || b == TYPE_FAIRY;
    fighting = a == TYPE_FIGHTING || b == TYPE_FIGHTING;
    fire = a == TYPE_FIRE || b == TYPE_FIRE;
    switch (move)
    {
    case MOVE_UNHOLY_ROOT: return grass ? 24 : 0;
    case MOVE_UNHOLY_WEIGHTLESS: return psychic || ghost ? 28 : 0;
    case MOVE_UNHOLY_CONFINE: return psychic || ghost || dark ? 28 : 0;
    case MOVE_UNHOLY_BLIND: return poison || dark ? 24 : 0;
    case MOVE_UNHOLY_TERROR: return ghost || dark || dragon ? 32 : 0;
    case MOVE_UNHOLY_SHACKLE: return steel ? 30 : 0;
    case MOVE_UNHOLY_BANISH: return ghost || psychic ? 50 : 0;
    case MOVE_UNHOLY_SANCTIFY: return fairy || fighting ? 36 : 0;
    case MOVE_UNHOLY_DEMONIFY: return dark || fire ? 36 : 0;
    default: return 0;
    }
}
