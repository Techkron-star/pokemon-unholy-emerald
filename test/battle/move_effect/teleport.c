#include "global.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(GetMoveEffect(MOVE_TELEPORT) == EFFECT_BATON_PASS);
}

SINGLE_BATTLE_TEST("Unholy Teleport fails to switch out when there is no other Pokémon")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } SCENE {
        MESSAGE("But it failed!");
    }
}

SINGLE_BATTLE_TEST("Unholy Teleport fails when the remaining Pokémon are fainted")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT) { HP(0); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } SCENE {
        MESSAGE("But it failed!");
    }
}

SINGLE_BATTLE_TEST("Unholy Teleport switches in trainer battles regardless of generation setting")
{
    GIVEN {
        WITH_CONFIG(B_TELEPORT_BEHAVIOR, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Teleport!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TELEPORT, opponent);
        MESSAGE("2 sent out Wynaut!");
    }
}

SINGLE_BATTLE_TEST("Unholy Teleport switches out the user in trainer battles")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); SEND_OUT(opponent, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TELEPORT, opponent);
        MESSAGE("2 sent out Wynaut!");
    }
}

SINGLE_BATTLE_TEST("Unholy Teleport does not fail if the user is trapped")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_SPIN); MOVE(opponent, MOVE_TELEPORT); SEND_OUT(opponent, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FIRE_SPIN, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TELEPORT, opponent);
        MESSAGE("2 sent out Wynaut!");
    }
}

SINGLE_BATTLE_TEST("Unholy Teleport passes both positive and negative stat changes")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SWORDS_DANCE, MOVE_TELEPORT); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SCARY_FACE, MOVE_SPLASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SWORDS_DANCE); MOVE(opponent, MOVE_SCARY_FACE); }
        TURN { MOVE(player, MOVE_TELEPORT); MOVE(opponent, MOVE_SPLASH); SEND_OUT(player, 1); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE - 2);
    }
}
