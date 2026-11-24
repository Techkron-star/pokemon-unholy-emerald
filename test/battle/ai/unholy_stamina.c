#include "global.h"
#include "battle_stamina.h"
#include "test/battle.h"

AI_SINGLE_BATTLE_TEST("Unholy trainer AI breaks primary move ties by avoiding exhaustion")
{
    bool32 preferHigherDamage;

    PARAMETRIZE { preferHigherDamage = FALSE; }
    PARAMETRIZE { preferHigherDamage = TRUE; }

    GIVEN {
        ASSUME(STAMINA_ENABLED);
        AI_FLAGS(preferHigherDamage ? AI_FLAG_BASIC_TRAINER : AI_FLAG_CHECK_BAD_MOVE);
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_SINGLES, SCORE_TIE_LO, 0);
        PLAYER(SPECIES_WOBBUFFET) { HP(1000); Defense(500); SpDefense(200); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(100); HPIV(20); SpAttack(50); Moves(MOVE_BOOMBURST, MOVE_ENDURE); }
    } WHEN {
        // With 100 max stamina, Boomburst costs 28 and 18 is restored per turn.
        // The ninth turn starts at 20: Endure still fits, but Boomburst exhausts.
        for (u32 turn = 0; turn < 8; turn++)
        {
            TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_BOOMBURST); }
        }
        TURN {
            MOVE(player, MOVE_SPLASH);
            if (preferHigherDamage)
            {
                SCORE_GT(opponent, MOVE_BOOMBURST, MOVE_ENDURE);
                EXPECT_MOVE(opponent, MOVE_BOOMBURST);
            }
            else
            {
                SCORE_EQ(opponent, MOVE_BOOMBURST, MOVE_ENDURE);
                EXPECT_MOVE(opponent, MOVE_ENDURE);
            }
        }
    }
}
