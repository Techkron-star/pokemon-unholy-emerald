#include "global.h"
#include "pokemon.h"
#include "config/special_badges.h"
#include "test/battle.h"

// REQUEST_ALL_BATTLE copies a BattlePokemon across the controller boundary.
// Regression: uninitialized / omitted specialBadges must never penalize an
// ordinary wild Pokemon or erase an explicitly configured mutation.
WILD_BATTLE_TEST("Unholy capture: ordinary wild Pokemon has no mutation penalty")
{
    u32 odds;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&odds);
    } THEN {
        EXPECT_EQ(opponent->specialBadges, 0);
        EXPECT_EQ(odds, 50);
    }
}

WILD_BATTLE_TEST("Unholy capture: Alpha badge survives the battle controller")
{
    u32 odds;
    u16 badges = POKEMON_BADGE_ALPHA;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA) {
            SetMonData(gBattleTestRunnerState->data.currentMon, MON_DATA_SPECIAL_BADGES, &badges);
        }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&odds);
    } THEN {
        EXPECT_EQ(opponent->specialBadges, POKEMON_BADGE_ALPHA);
        EXPECT_EQ(odds, 50 * SPECIAL_BADGE_ALPHA_CAPTURE_PERCENT / 100);
    }
}
