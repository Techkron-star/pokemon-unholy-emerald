#include "global.h"
#include "gear.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Unholy Blood Gear averages matching held-item bonuses", s16 damage)
{
    enum Gear gear;
    enum Item item;

    PARAMETRIZE { gear = GEAR_NONE;       item = ITEM_NONE; }
    PARAMETRIZE { gear = GEAR_BLOOD_FIRE; item = ITEM_NONE; }
    PARAMETRIZE { gear = GEAR_BLOOD_FIRE; item = ITEM_FLAME_PLATE; }
    PARAMETRIZE { gear = GEAR_BLOOD_FIRE; item = ITEM_FIRE_GEM; }
    PARAMETRIZE { gear = GEAR_BLOOD_FIRE; item = ITEM_SPLASH_PLATE; }

    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_FLAME_PLATE) == HOLD_EFFECT_PLATE);
        ASSUME(GetItemHoldEffectParam(ITEM_FLAME_PLATE) == 20);
        ASSUME(GetItemHoldEffect(ITEM_FIRE_GEM) == HOLD_EFFECT_GEMS);
        PLAYER(SPECIES_ALAKAZAM) { Item(item); }
        Gear_SetEquipped(&PLAYER_PARTY[0], gear);
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_INCINERATE); MOVE(opponent, MOVE_SPLASH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_INCINERATE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.4), results[1].damage);
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[2].damage);
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(I_GEM_BOOST_POWER >= GEN_6 ? 1.35 : 1.45), results[3].damage);
        EXPECT_EQ(results[4].damage, results[1].damage);
    }
}
