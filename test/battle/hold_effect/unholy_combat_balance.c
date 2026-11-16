#include "global.h"
#include "gear.h"
#include "test/battle.h"

DOUBLE_BATTLE_TEST("Unholy Focus Sash protects from separate attacks until the turn ends")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(120); SpAttack(180); }
        PLAYER(SPECIES_ABRA) { Speed(100); SpAttack(180); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FOCUS_SASH); Level(5); Speed(50); }
        OPPONENT(SPECIES_WYNAUT) { Speed(40); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_POWER_GEM, target: opponentLeft);
            MOVE(playerRight, MOVE_POWER_GEM, target: opponentLeft);
            MOVE(opponentLeft, MOVE_CELEBRATE);
            MOVE(opponentRight, MOVE_CELEBRATE);
        }
        TURN {
            MOVE(playerLeft, MOVE_POWER_GEM, target: opponentLeft);
            MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, playerRight);
        MESSAGE("The opposing Wobbuffet endured the hit!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, playerLeft);
        MESSAGE("The opposing Wobbuffet fainted!");
    }
}

SINGLE_BATTLE_TEST("Unholy Focus Sash can be broken by the Storm Staff's additional impact")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(120); SpAttack(180); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_STORM_STAFF);
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FOCUS_SASH); Level(5); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_POWER_GEM); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, player);
        HP_BAR(opponent);
        HP_BAR(opponent);
        MESSAGE("The Pokémon was hit 2 times!");
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Unholy Shackle checks once across a multihit move")
{
    PASSES_RANDOMLY(1, 5, RNG_GEAR_SHACKLE);

    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(120); Attack(50); Ability(ABILITY_SKILL_LINK); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_SHACKLE);
        OPPONENT(SPECIES_SNORLAX) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_FURY_ATTACK, hit: TRUE); }
    } SCENE {
        MESSAGE("The opposing Snorlax can't chain moves!");
    }
}

SINGLE_BATTLE_TEST("Unholy Shackle prevents chaining for the following complete turn")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(120); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_SHACKLE);
        OPPONENT(SPECIES_SNORLAX) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_POWER_GEM, WITH_RNG(RNG_GEAR_SHACKLE, TRUE)); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Snorlax can't chain moves!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    } THEN {
        EXPECT_EQ(gBattleStruct->shackleTurns[B_BATTLER_1], 0);
    }
}

SINGLE_BATTLE_TEST("Unholy Shackle does not trigger when its holder replaces the Future Sight user")
{
    PASSES_RANDOMLY(1, 1, RNG_GEAR_SHACKLE);

    GIVEN {
        PLAYER(SPECIES_ABRA);
        PLAYER(SPECIES_WYNAUT);
        Gear_SetEquipped(&PLAYER_PARTY[1], GEAR_SHACKLE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_FUTURE_SIGHT); }
        TURN { SWITCH(player, 1); }
        TURN {}
    } SCENE {
        MESSAGE("The opposing Wobbuffet took the Future Sight attack!");
        HP_BAR(opponent);
        NOT MESSAGE("The opposing Wobbuffet can't chain moves!");
    }
}
