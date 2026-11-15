#include "global.h"
#include "gear.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Unholy Storm Staff strikes again with one quarter of the original power")
{
    s16 firstHit, extraHit;

    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(100); SpAttack(120); Moves(MOVE_POWER_GEM); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_STORM_STAFF);
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); SpDefense(70); }
    } WHEN {
        TURN { MOVE(player, MOVE_POWER_GEM); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, player);
        HP_BAR(opponent, captureDamage: &firstHit);
        HP_BAR(opponent, captureDamage: &extraHit);
        MESSAGE("The Pokémon was hit 2 times!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    } THEN {
        EXPECT_GT(firstHit, extraHit * 3);
        EXPECT_LT(firstHit, extraHit * 5);
        EXPECT_EQ(gBattleMovePower, GetMovePower(MOVE_POWER_GEM) / 4);
        EXPECT_EQ(player->pp[0], GetMovePP(MOVE_POWER_GEM) - 1);
    }
}

SINGLE_BATTLE_TEST("Unholy Storm Staff suppresses guaranteed secondary effects and keeps primary drawbacks")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(100); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_STORM_STAFF);
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_ACID_SPRAY); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_OVERHEAT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent);
        HP_BAR(opponent);
        MESSAGE("The Pokémon was hit 2 times!");
        HP_BAR(opponent);
        HP_BAR(opponent);
        MESSAGE("The Pokémon was hit 2 times!");
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 2);
    }
}

SINGLE_BATTLE_TEST("Unholy Storm Staff respects Protect and leaves a physical move at one hit")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_STORM_STAFF);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POWER_GEM); MOVE(opponent, MOVE_PROTECT); }
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PROTECT, opponent);
        NOT HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent);
        NONE_OF {
            HP_BAR(opponent);
            MESSAGE("The Pokémon was hit 2 times!");
        }
    }
}

SINGLE_BATTLE_TEST("Unholy Storm Staff on an opponent stops when the first hit faints the target")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Speed(40); }
        OPPONENT(SPECIES_ABRA) { Speed(100); }
        Gear_SetEquipped(&OPPONENT_PARTY[0], GEAR_STORM_STAFF);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_POWER_GEM); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POWER_GEM, opponent);
        HP_BAR(player);
        NONE_OF {
            HP_BAR(player);
            MESSAGE("The Pokémon was hit 2 times!");
        }
    }
}
