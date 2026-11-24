#include "global.h"
#include "gear.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Piedra de Afilar obeys Contrary and Simple when boosting Attack")
{
    enum Ability ability;
    u8 expected;

    PARAMETRIZE { ability = ABILITY_NONE; expected = DEFAULT_STAT_STAGE + 1; }
    PARAMETRIZE { ability = ABILITY_CONTRARY; expected = DEFAULT_STAT_STAGE - 1; }
    PARAMETRIZE { ability = ABILITY_SIMPLE; expected = DEFAULT_STAT_STAGE + 2; }

    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); Speed(100); Moves(MOVE_SLASH); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_PIEDRA_AFILAR);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], expected);
    }
}

SINGLE_BATTLE_TEST("Ariete Nautilo obeys defensive abilities on its target")
{
    enum Ability ability;
    u8 defenderExpected, attackerExpected;

    PARAMETRIZE { ability = ABILITY_NONE;         defenderExpected = DEFAULT_STAT_STAGE - 1; attackerExpected = DEFAULT_STAT_STAGE; }
    PARAMETRIZE { ability = ABILITY_CONTRARY;     defenderExpected = DEFAULT_STAT_STAGE + 1; attackerExpected = DEFAULT_STAT_STAGE; }
    PARAMETRIZE { ability = ABILITY_SIMPLE;       defenderExpected = DEFAULT_STAT_STAGE - 2; attackerExpected = DEFAULT_STAT_STAGE; }
    PARAMETRIZE { ability = ABILITY_CLEAR_BODY;   defenderExpected = DEFAULT_STAT_STAGE;     attackerExpected = DEFAULT_STAT_STAGE; }
    PARAMETRIZE { ability = ABILITY_MIRROR_ARMOR; defenderExpected = DEFAULT_STAT_STAGE;     attackerExpected = DEFAULT_STAT_STAGE - 1; }

    GIVEN {
        PLAYER(SPECIES_ABRA) { Speed(100); Moves(MOVE_SCRATCH); }
        Gear_SetEquipped(&PLAYER_PARTY[0], GEAR_ARIETE_NAUTILO);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); HP(1000); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_DEF], defenderExpected);
        EXPECT_EQ(player->statStages[STAT_DEF], attackerExpected);
    }
}
