#include "global.h"
#include "battle.h"
#include "battle_chain.h"
#include "battle_stamina.h"
#include "move.h"
#include "malloc.h"
#include "unholy_moves.h"
#include "unholy_status.h"
#include "test/battle.h"

TEST("Unholy statuses: exile immunity survives boxed copies without changing the Pokemon save layout")
{
    struct Pokemon mon;
    struct BoxPokemon copy;
    u8 value = TRUE;
    u16 badges = POKEMON_BADGE_ALPHA | POKEMON_BADGE_TITAN;
    CreateRandomMon(&mon, SPECIES_GENGAR, 50);
    SetMonData(&mon, MON_DATA_SPECIAL_BADGES, &badges);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HAS_BEEN_EXILED), FALSE);
    SetMonData(&mon, MON_DATA_HAS_BEEN_EXILED, &value);
    copy = mon.box;
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_HAS_BEEN_EXILED), TRUE);
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_SPECIAL_BADGES), badges);
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_SPECIES), SPECIES_GENGAR);
    EXPECT_EQ(GetBoxMonData(&copy, MON_DATA_SANITY_IS_BAD_EGG), FALSE);
}

TEST("Unholy statuses: thematic tutor catalog enforces levels for new moves")
{
    struct Pokemon mon;
    struct UnholyMoveCatalog *catalog = Alloc(sizeof(*catalog));
    u16 item;
    CreateRandomMon(&mon, SPECIES_GENGAR, 49);
    UnholyMoves_Build(&mon, catalog);
    EXPECT(catalog->sources[MOVE_UNHOLY_BANISH] & UNHOLY_MOVE_TUTOR);
    EXPECT_EQ(UnholyMoves_Status(&mon, catalog, MOVE_UNHOLY_BANISH, &item), UNHOLY_MOVE_NEEDS_LEVEL);
    EXPECT_EQ(UnholyStatus_MoveLevel(SPECIES_GENGAR, MOVE_UNHOLY_BANISH), 50);
    EXPECT_EQ(UnholyStatus_MoveLevel(SPECIES_AGGRON, MOVE_UNHOLY_SHACKLE), 30);
    EXPECT_EQ(UnholyStatus_MoveLevel(SPECIES_MAGIKARP, MOVE_UNHOLY_BANISH), 0);
    Free(catalog);
}

SINGLE_BATTLE_TEST("Unholy statuses: custom moves apply separate volatile effects", u8 unused)
{
    enum Move move;
    enum UnholyStatus status;
    u8 turns;
    PARAMETRIZE { move = MOVE_UNHOLY_ROOT; status = UNHOLY_STATUS_ROOTED; turns = 3; }
    PARAMETRIZE { move = MOVE_UNHOLY_CONFINE; status = UNHOLY_STATUS_CONFINED; turns = 2; }
    PARAMETRIZE { move = MOVE_UNHOLY_BLIND; status = UNHOLY_STATUS_BLINDED; turns = 3; }
    PARAMETRIZE { move = MOVE_UNHOLY_TERROR; status = UNHOLY_STATUS_TERRIFIED; turns = 2; }
    PARAMETRIZE { move = MOVE_UNHOLY_SHACKLE; status = UNHOLY_STATUS_CHAINED; turns = 1; }
    PARAMETRIZE { move = MOVE_UNHOLY_SANCTIFY; status = UNHOLY_STATUS_SANCTIFIED; turns = 4; }
    PARAMETRIZE { move = MOVE_UNHOLY_DEMONIFY; status = UNHOLY_STATUS_DEMONIFIED; turns = 4; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); HP(2000); Speed(100); }
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(UnholyStatus_GetTurns(B_BATTLER_1, status), turns);
        EXPECT(opponent->status1 & STATUS1_BURN);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Rooted grounds a Flying type and restricts withdrawal")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_FARFETCHD) { HP(2000); Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_ROOT); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_EARTHQUAKE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent);
    } THEN {
        EXPECT(IsBattlerGrounded(B_BATTLER_1, GetBattlerAbility(B_BATTLER_1), GetBattlerHoldEffect(B_BATTLER_1)));
        EXPECT(!CanBattlerEscape(B_BATTLER_1));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Weightlessness avoids Ground attacks")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_WEIGHTLESS, target: player); MOVE(opponent, MOVE_EARTHQUAKE); }
    } SCENE {
        NOT HP_BAR(player);
    } THEN {
        EXPECT(UnholyStatus_Has(B_BATTLER_0, UNHOLY_STATUS_WEIGHTLESS));
        EXPECT(!IsBattlerGrounded(B_BATTLER_0, GetBattlerAbility(B_BATTLER_0), GetBattlerHoldEffect(B_BATTLER_0)));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Gravity suppresses Weightlessness")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); HP(2000); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_WEIGHTLESS, target: player); MOVE(opponent, MOVE_GRAVITY); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_EARTHQUAKE); }
    } SCENE {
        HP_BAR(player);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Belico is immune to Terrified")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_BELICO); Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_TERROR); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_TERRIFIED));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Blinded respects Keen Eye immunity")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_KEEN_EYE); Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_BLIND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_BLINDED));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Terrified weakens physical and special damage", s16 damage)
{
    bool32 fear;
    enum Move move;
    PARAMETRIZE { fear = FALSE; move = MOVE_TACKLE; }
    PARAMETRIZE { fear = TRUE; move = MOVE_TACKLE; }
    PARAMETRIZE { fear = FALSE; move = MOVE_PSYCHIC; }
    PARAMETRIZE { fear = TRUE; move = MOVE_PSYCHIC; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { HP(2000); Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, fear ? MOVE_UNHOLY_TERROR : MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, move); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
        EXPECT_LT(results[3].damage, results[2].damage);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Chained allows one move and blocks chaining")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_SHACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!Chain_IsAvailable(B_BATTLER_1));
        EXPECT_EQ(opponent->pp[0], GetMovePP(MOVE_CELEBRATE) - 1);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Substitute blocks hostile custom statuses")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(200); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_UNHOLY_TERROR); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_TERRIFIED));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: custom effects expire in full turns and clear on switch")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(90); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_TERROR); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); SWITCH(opponent, 1); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_TERRIFIED));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: opposing affinities replace each other and preserve unknown types")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_SANCTIFY); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_UNHOLY_DEMONIFY); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct DamageContext ctx = {.battlerAtk = B_BATTLER_1, .moveType = TYPE_PSYCHIC};
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_SANCTIFIED));
        EXPECT(UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_DEMONIFIED));
        EXPECT_EQ(UnholyStatus_AdjustStab(&ctx, UQ_4_12(1.5)), UQ_4_12(0.75));
        ctx.moveType = TYPE_GHOST;
        EXPECT_EQ(UnholyStatus_AdjustStab(&ctx, UQ_4_12(1.5)), UQ_4_12(1.75));
        ctx.moveType = TYPE_WATER;
        EXPECT_EQ(UnholyStatus_AdjustStab(&ctx, UQ_4_12(1.5)), UQ_4_12(1.5));
        EXPECT_EQ(UnholyStatus_AdjustStab(&ctx, UQ_4_12(1.0)), UQ_4_12(1.0));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Banishment fails without a replacement and does not consume lifetime immunity")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_BANISH); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_BANISHED));
        EXPECT_EQ(GetMonData(GetBattlerMon(B_BATTLER_1), MON_DATA_HAS_BEEN_EXILED), FALSE);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Banishment forces a replacement in singles")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(90); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_BANISH, WITH_RNG(RNG_UNHOLY_BANISH, 2)); MOVE(opponent, MOVE_CELEBRATE); SEND_OUT(opponent, 1); }
    } THEN {
        EXPECT_EQ(gBattlerPartyIndexes[B_BATTLER_1], 1);
        EXPECT(UnholyStatus_PartyExiled(B_TRAINER_OPPONENT_A, 0));
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_BANISHED));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_STATUS), 0);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HAS_BEEN_EXILED), TRUE);
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: Banishment purges statuses blocks spread damage and prevents acting")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); Speed(20); HP(2000); }
        OPPONENT(SPECIES_WYNAUT) { Speed(10); HP(2000); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft, WITH_RNG(RNG_UNHOLY_BANISH, 2));
            MOVE(playerRight, MOVE_SWIFT);
            MOVE(opponentLeft, MOVE_TACKLE, target: playerLeft);
            MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        EXPECT_EQ(opponentLeft->hp, 2000);
        EXPECT_EQ(playerLeft->hp, playerLeft->maxHP);
        EXPECT(opponentRight->hp < opponentRight->maxHP);
        EXPECT_EQ(UnholyStatus_GetTurns(B_BATTLER_1, UNHOLY_STATUS_BANISHED), 1);
        EXPECT_EQ(opponentLeft->status1, 0);
        EXPECT(!CanBattlerEscape(B_BATTLER_1));
        EXPECT(!CanBattlerSwitch(B_BATTLER_1));
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: Banishment expires and permanent immunity prevents a second exile")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(90); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft, WITH_RNG(RNG_UNHOLY_BANISH, 1)); }
        TURN { }
        TURN { MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_BANISHED));
        EXPECT_EQ(GetMonData(GetBattlerMon(B_BATTLER_1), MON_DATA_HAS_BEEN_EXILED), TRUE);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Blinded changes real hit chance to seventy five percent")
{
    PASSES_RANDOMLY(75, 100, RNG_ACCURACY);
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); HP(2000); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_BLIND, WITH_RNG(RNG_ACCURACY, TRUE)); MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: Terrified expires after two following full turns")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_TERROR); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_TERRIFIED));
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: affinities alter actual positive and negative STAB damage", s16 damage)
{
    enum Move affinity;
    enum Move attack;
    enum Species species;
    PARAMETRIZE { affinity = MOVE_CELEBRATE; species = SPECIES_MACHAMP; attack = MOVE_KARATE_CHOP; }
    PARAMETRIZE { affinity = MOVE_UNHOLY_SANCTIFY; species = SPECIES_MACHAMP; attack = MOVE_KARATE_CHOP; }
    PARAMETRIZE { affinity = MOVE_UNHOLY_DEMONIFY; species = SPECIES_MACHAMP; attack = MOVE_KARATE_CHOP; }
    PARAMETRIZE { affinity = MOVE_CELEBRATE; species = SPECIES_CHARIZARD; attack = MOVE_FLAMETHROWER; }
    PARAMETRIZE { affinity = MOVE_UNHOLY_SANCTIFY; species = SPECIES_CHARIZARD; attack = MOVE_FLAMETHROWER; }
    PARAMETRIZE { affinity = MOVE_UNHOLY_DEMONIFY; species = SPECIES_CHARIZARD; attack = MOVE_FLAMETHROWER; }
    GIVEN {
        PLAYER(species) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); HP(2000); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, affinity); }
        TURN { MOVE(player, attack); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage);
        EXPECT_LT(results[2].damage, results[0].damage);
        EXPECT_LT(results[4].damage, results[3].damage);
        EXPECT_GT(results[5].damage, results[3].damage);
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: Healer can remove one custom negative status")
{
    PASSES_RANDOMLY(30, 100, RNG_HEALER);
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_CHANSEY) { Ability(ABILITY_HEALER); Speed(50); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_UNHOLY_CONFINE, target: opponentLeft); }
    } SCENE {
        ABILITY_POPUP(opponentRight, ABILITY_HEALER);
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: an exiled battler cannot keep its team alive after its last partner faints")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(50); HP(1); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft, WITH_RNG(RNG_UNHOLY_BANISH, 5));
            MOVE(playerRight, MOVE_TACKLE, target: opponentRight);
        }
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_WON);
        EXPECT(opponentLeft->hp > 0);
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: exile blocks No Guard OHKO attacks and sandstorm damage")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_NO_GUARD); Speed(200); HP(2000); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); HP(2000); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); HP(2000); }
        OPPONENT(SPECIES_WYNAUT) { Speed(50); HP(2000); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft, WITH_RNG(RNG_UNHOLY_BANISH, 5)); MOVE(playerRight, MOVE_SANDSTORM); }
        TURN { MOVE(playerLeft, MOVE_FISSURE, target: opponentLeft); }
    } THEN {
        EXPECT_EQ(opponentLeft->hp, 2000);
        EXPECT(opponentRight->hp < 2000);
    }
}

SINGLE_BATTLE_TEST("Unholy statuses: forced phasing cannot return a banished reserve")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(90); }
    } WHEN {
        TURN { MOVE(player, MOVE_UNHOLY_BANISH, WITH_RNG(RNG_UNHOLY_BANISH, 5)); MOVE(opponent, MOVE_CELEBRATE); SEND_OUT(opponent, 1); }
        TURN { MOVE(player, MOVE_ROAR); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(gBattlerPartyIndexes[B_BATTLER_1], 1);
        EXPECT(UnholyStatus_PartyExiled(B_TRAINER_OPPONENT_A, 0));
        EXPECT(!CanBattlerSwitch(B_BATTLER_1));
        EXPECT(HasNoMonsToSwitch(B_BATTLER_1, PARTY_SIZE, PARTY_SIZE));
    }
}

DOUBLE_BATTLE_TEST("Unholy statuses: the one-turn exile ends at the end of its application turn")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(200); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(150); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WYNAUT) { Speed(50); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_UNHOLY_BANISH, target: opponentLeft, WITH_RNG(RNG_UNHOLY_BANISH, 1)); }
    } THEN {
        EXPECT(!UnholyStatus_Has(B_BATTLER_1, UNHOLY_STATUS_BANISHED));
        EXPECT_EQ(GetMonData(GetBattlerMon(B_BATTLER_1), MON_DATA_HAS_BEEN_EXILED), TRUE);
    }
}
