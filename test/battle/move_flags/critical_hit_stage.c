#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Slicing moves have 25 percent base critical chance in Gen 3+ rules")
{
    u32 j, genConfig = 0, passes = 0, trials = 0;

    PARAMETRIZE { genConfig = GEN_1; passes = 1; trials = 2; }     // 50% with Wobbuffet's base speed
    PARAMETRIZE { genConfig = GEN_2; passes = 32; trials = 256; }
    for (j = GEN_3; j <= GEN_9; j++) {
        PARAMETRIZE { genConfig = j; passes = 1; trials = 4; }
    }
    PASSES_RANDOMLY(passes, trials, RNG_CRITICAL_HIT);
    GIVEN {
        WITH_CONFIG(B_CRIT_CHANCE, genConfig);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(GetSpeciesBaseSpeed(SPECIES_WOBBUFFET) == 33);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

DOUBLE_BATTLE_TEST("Critical hits mention each affected target")
{
    GIVEN {
        WITH_CONFIG(B_CRIT_CHANCE, GEN_9);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(GetSpeciesBaseSpeed(SPECIES_WOBBUFFET) == 33);
        PLAYER(SPECIES_DRAGONITE) { Item(ITEM_SCOPE_LENS); SpAttack(1); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_DRAGON_CHEER); MOVE(playerLeft, MOVE_SURF); }
        TURN { MOVE(playerLeft, MOVE_SURF); }
        TURN { MOVE(playerLeft, MOVE_DAZZLING_GLEAM); }
    } SCENE {
        // Surf against all targets
        MESSAGE("A critical hit on the opposing Wynaut!");
        MESSAGE("A critical hit on Wobbuffet!");
        MESSAGE("A critical hit on the opposing Wobbuffet!");
        // Surf against one opponent and one partner
        MESSAGE("A critical hit on Wobbuffet!");
        MESSAGE("A critical hit on the opposing Wobbuffet!");
        // Dazzling Gleam against one target
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("Unholy Dragon Claw has 25 percent base critical chance")
{
    PASSES_RANDOMLY(1, 4, RNG_CRITICAL_HIT);
    GIVEN {
        WITH_CONFIG(B_CRIT_CHANCE, GEN_9);
        ASSUME(IsSlicingMove(MOVE_DRAGON_CLAW));
        PLAYER(SPECIES_GABITE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_CLAW); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}
