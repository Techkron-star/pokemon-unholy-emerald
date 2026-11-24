#include "global.h"
#include "test/battle.h"

WILD_BATTLE_TEST("Badge caps clamp a large battle reward and preserve an existing overlevel Pokemon")
{
    u8 level = 0;
    PARAMETRIZE { level = 14; }
    PARAMETRIZE { level = 15; }
    PARAMETRIZE { level = 20; }
    GIVEN {
        for (u16 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++) FLAG_CLEAR(flag);
        FLAG_CLEAR(FLAG_IS_CHAMPION);
        PLAYER(SPECIES_WOBBUFFET) { Level(level); }
        OPPONENT(SPECIES_BLISSEY) { Level(100); HP(1); Moves(MOVE_SPLASH); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_SPLASH); }
    } THEN {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
        u8 expected = level < 15 ? 15 : level;
        EXPECT_EQ(GetMonData(mon, MON_DATA_LEVEL), expected);
        EXPECT_EQ(GetMonData(mon, MON_DATA_EXP), gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][expected]);
    }
}
