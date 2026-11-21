#include "global.h"
#include "battle.h"
#include "battle_unholy_abilities.h"
#include "battle_stamina.h"
#include "battle_util.h"
#include "gear.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Unholy 37: Knight raises both defenses on entry")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_KNIGHT); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Reactor raises Sp. Atk at the end of every turn")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_REACTOR); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Wizard and Monje block their own Sp. Atk drops")
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_WIZARD; }
    PARAMETRIZE { ability = ABILITY_MONJE; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_OVERHEAT); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Cuerpo Magma evaporates Water without healing")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_MAGMA_ARMOR); HP(100); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_WATER_GUN); }
    } THEN {
        EXPECT_EQ(player->hp, 100);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: World Tree seeds a hit once and respects Grass immunity")
{
    enum Species species;
    bool32 seeded;
    PARAMETRIZE { species = SPECIES_WOBBUFFET; seeded = TRUE; }
    PARAMETRIZE { species = SPECIES_BULBASAUR; seeded = FALSE; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_WORLD_TREE); }
        OPPONENT(species) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_VINE_WHIP); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(opponent->volatiles.leechSeed != 0, seeded);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Colerico boosts once for a multi-hit punch")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_COLERICO); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_IRON_BASH); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Gula doubles Sitrus healing without consuming twice")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GLUTTONY); Item(ITEM_SITRUS_BERRY); HP(50); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->hp, 150);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Greedy cannot activate its held Berry")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GREEDY); Item(ITEM_SITRUS_BERRY); HP(50); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->hp, 50);
        EXPECT_EQ(player->item, ITEM_SITRUS_BERRY);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Envy consumes a virtual Berry and preserves both actual items")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ENVY); Item(ITEM_ORAN_BERRY); HP(50); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_SITRUS_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->hp, 100);
        EXPECT_EQ(player->item, ITEM_ORAN_BERRY);
        EXPECT_EQ(opponent->item, ITEM_SITRUS_BERRY);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Antithesis turns an enemy Sitrus heal into damage")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ANTITHESIS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_SITRUS_BERRY); HP(80); MaxHP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(opponent->hp, 30);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Antithesis inverts enemy boosts")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ANTITHESIS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SWORDS_DANCE); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_ATK], DEFAULT_STAT_STAGE - 2);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Antithesis keeps poison but turns its damage into healing")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ANTITHESIS); HP(100); MaxHP(200); Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->hp, 125);
        EXPECT(player->status1 & STATUS1_POISON);
    }
}

DOUBLE_BATTLE_TEST("Unholy 37: Space Realm blocks area damage to both allies")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_SPACE_REALM); HP(200); }
        PLAYER(SPECIES_WOBBUFFET) { HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, MOVE_EARTHQUAKE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(playerLeft->hp, 200);
        EXPECT_EQ(playerRight->hp, 200);
    }
}

DOUBLE_BATTLE_TEST("Unholy 37: Hyperbaric deflects projectiles aimed at its ally")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_HYPERBARIC); HP(200); }
        PLAYER(SPECIES_WOBBUFFET) { HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, MOVE_ENERGY_BALL, target: playerRight); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(playerLeft->hp, 200);
        EXPECT_EQ(playerRight->hp, 200);
    }
}

DOUBLE_BATTLE_TEST("Unholy 37: Black Hole attracts and absorbs a projectile aimed at its ally")
{
    GIVEN {
        PLAYER(SPECIES_GUZZLORD) { Ability(ABILITY_BLACK_HOLE); HP(100); MaxHP(200); }
        PLAYER(SPECIES_WOBBUFFET) { HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, MOVE_ENERGY_BALL, target: playerRight); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(playerLeft->hp, 125);
        EXPECT_EQ(playerRight->hp, 200);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: God Realm prevents status but does not bypass Protect")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GOD_REALM); HP(200); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TOXIC); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_PROTECT); }
    } THEN {
        EXPECT_EQ(player->status1, 0);
        EXPECT_EQ(opponent->hp, 200);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: category mixing does not double equal-stat damage", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_ONI; }
    PARAMETRIZE { ability = ABILITY_GODHAND; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); Attack(100); SpAttack(100); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Defense(100); SpDefense(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_CRUNCH, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
        EXPECT_EQ(results[0].damage, results[2].damage);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: original Dragon attacks can paralyze; converted Normal cannot")
{
    bool32 original;
    PARAMETRIZE { original = TRUE; }
    PARAMETRIZE { original = FALSE; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_DRAGONBORN); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        if (original) TURN { MOVE(player, MOVE_DRAGON_PULSE, WITH_RNG(RNG_UNHOLY_DRAGONBORN, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
        else TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ((opponent->status1 & STATUS1_PARALYSIS) != 0, original);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: original Water attacks can freeze; converted Normal cannot")
{
    bool32 original;
    PARAMETRIZE { original = TRUE; }
    PARAMETRIZE { original = FALSE; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ABISAL); Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Speed(100); }
    } WHEN {
        if (original) TURN { MOVE(player, MOVE_WATER_GUN, WITH_RNG(RNG_UNHOLY_ABISAL, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
        else TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ((opponent->status1 & STATUS1_FREEZE) != 0, original);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Gelido inflicts Frostbite instead of Freeze")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GELIDO); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, WITH_RNG(RNG_UNHOLY_GELIDO, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_FROSTBITE);
        EXPECT_EQ(opponent->status1 & STATUS1_FREEZE, 0);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Manthra meditates on its second turn and heals independently of status")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_MANTHRA); HP(100); MaxHP(300); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->hp, 200);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Reaper heals after an enemy knockout")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_REAPER); HP(100); MaxHP(300); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } THEN {
        EXPECT_EQ(player->hp, 200);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Drake ignores twenty percent of the relevant defense", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_DRAKE; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); SpAttack(200); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); SpDefense(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_PULSE, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.25), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Anti Realm neutralizes weaknesses but keeps resistances", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_ANTI_REALM; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_BITE, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: GodHand adapts to become immune before receiving Normal damage")
{
    GIVEN {
        PLAYER(SPECIES_ARCEUS) { Ability(ABILITY_GODHAND); HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT_EQ(player->hp, 200);
        EXPECT_EQ(player->types[0], TYPE_GHOST);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Great Eclipse preserves Electric to Ground immunity")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GREAT_ECLIPSE); }
        OPPONENT(SPECIES_DIGLETT) { HP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(opponent->hp, 200);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Great Eclipse prevents new sunny weather")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GREAT_ECLIPSE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SUNNY_DAY); }
    } THEN {
        EXPECT_EQ(gBattleWeather & B_WEATHER_SUN, 0);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Steel Scales reflects Steel-scaled contact damage")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_STEEL_SCALES); HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(200); MaxHP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SCRATCH); }
    } THEN {
        EXPECT_EQ(opponent->hp, 175);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Marathoner discounts only positive Speed stages")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_MARATONISTA); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 normal = Stamina_GetMoveCost(B_BATTLER_0, MOVE_TACKLE);
        player->statStages[STAT_SPEED] = MIN_STAT_STAGE;
        EXPECT_EQ(Stamina_GetMoveCost(B_BATTLER_0, MOVE_TACKLE), normal);
        player->statStages[STAT_SPEED] = MAX_STAT_STAGE;
        EXPECT_LT(Stamina_GetMoveCost(B_BATTLER_0, MOVE_TACKLE), normal / 2);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Antithesis inverts instant Stamina recovery but keeps passive regeneration")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_ANTITHESIS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        u32 battler = B_BATTLER_1;
        gBattleStruct->stamina[battler] = 50;
        gBattleStruct->maxStamina[battler] = 100;
        Stamina_RestorePercent(battler, 20);
        EXPECT_EQ(Stamina_GetCurrent(battler), 30);
        Stamina_EndOfTurn(battler);
        EXPECT_GT(Stamina_GetCurrent(battler), 30);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Super Battery skips an Electric charge turn")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_SUPER_BATTERY); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_ELECTRO_SHOT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ELECTRO_SHOT, player);
        HP_BAR(opponent);
    }
}

DOUBLE_BATTLE_TEST("Unholy 37: Dual Wield hits only enemies, preserving the main target", s16 mainDamage, s16 otherDamage)
{
    PARAMETRIZE { }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_DUAL_WIELD); }
        PLAYER(SPECIES_WOBBUFFET) { HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Defense(100); SpDefense(100); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Defense(100); SpDefense(100); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_ENERGY_BALL, target: opponentLeft, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0));
            MOVE(playerRight, MOVE_CELEBRATE); MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &results[i].mainDamage);
        HP_BAR(opponentRight, captureDamage: &results[i].otherDamage);
    } THEN {
        EXPECT_EQ(playerRight->hp, 1000);
        EXPECT_MUL_EQ(results[i].otherDamage, Q_4_12(2.0), results[i].mainDamage);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Fallen Angel splits Fairy power rather than multiplying two type charts", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_FALLEN_ANGEL; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_MOONBLAST, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Oni computes both unequal attacking stats", s16 damage)
{
    enum Ability ability;
    u32 attack;
    PARAMETRIZE { ability = ABILITY_NONE; attack = 50; }
    PARAMETRIZE { ability = ABILITY_NONE; attack = 200; }
    PARAMETRIZE { ability = ABILITY_ONI; attack = 200; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); Attack(attack); SpAttack(50); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); Defense(100); SpDefense(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_CRUNCH, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[2].damage, (results[0].damage + results[1].damage) / 2);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Samurai can parry a faster contact before its selected cut")
{
    PASSES_RANDOMLY(50, 100, RNG_UNHOLY_SAMURAI);
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_SAMURAI); Speed(50); HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT_EQ(player->hp, 1000);
        EXPECT_LT(opponent->hp, 1000);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Belico creates one layer of steel entry hazards")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_BELICO); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_METAL_CLAW, WITH_RNG(RNG_UNHOLY_BELICO, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_METAL_CLAW, WITH_RNG(RNG_UNHOLY_BELICO, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(IsHazardOnSide(B_SIDE_OPPONENT, HAZARDS_STEELSURGE));
        EXPECT_EQ(gBattleStruct->numHazards[B_SIDE_OPPONENT], 1);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Dawn Light grants bonuses only when it cures a status")
{
    PASSES_RANDOMLY(30, 100, RNG_UNHOLY_DAWN);
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_DAWN_LIGHT); HP(100); MaxHP(300); Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->status1, 0);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
        EXPECT_GT(player->hp, 100);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Time Realm accelerates harmful clocks without unsigned underflow")
{
    GIVEN {
        PLAYER(SPECIES_DIALGA) { Ability(ABILITY_TIME_REALM); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        player->volatiles.perishSong = TRUE;
        player->volatiles.perishSongTimer = 1;
        player->volatiles.disableTimer = 4;
        opponent->volatiles.disableTimer = 4;
        Unholy_AdvanceTimers();
        EXPECT_EQ((u32)player->volatiles.perishSongTimer, 0);
        EXPECT_GE((u32)player->volatiles.disableTimer, 1);
        EXPECT_LE((u32)player->volatiles.disableTimer, 3);
        EXPECT_EQ((u32)opponent->volatiles.disableTimer, 4);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Slayer adds a critical stage only to the shared slicing catalogue")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_SLAYER); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        struct DamageContext ctx = {.battlerAtk = B_BATTLER_0, .battlerDef = B_BATTLER_1,
            .move = MOVE_SLASH};
        s32 ordinary = CalcCritChanceStage(&ctx);
        ctx.abilities[B_BATTLER_0] = ABILITY_SLAYER;
        EXPECT_EQ(CalcCritChanceStage(&ctx), ordinary + 1);
        ctx.move = MOVE_DOUBLE_EDGE;
        EXPECT_EQ(CalcCritChanceStage(&ctx), 0);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Malvado rolls priority once before turn ordering")
{
    PASSES_RANDOMLY(20, 100, RNG_UNHOLY_MALVADO);
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_MALVADO); Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_NIGHT_SLASH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Abra used Night Slash!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Greedy steals ordinary items through field control")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GREEDY); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_LEFTOVERS); }
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE, WITH_RNG(RNG_UNHOLY_GREEDY, TRUE)); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->item, ITEM_LEFTOVERS);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Space Realm reduces single-target contact damage by thirty percent", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_SPACE_REALM; }
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ability); HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TACKLE, criticalHit: FALSE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.7), results[1].damage);
    }
}

DOUBLE_BATTLE_TEST("Unholy 37: Black Hole leaves super-effective projectiles aimed at its ally alone")
{
    GIVEN {
        PLAYER(SPECIES_GUZZLORD) { Ability(ABILITY_BLACK_HOLE); HP(100); MaxHP(200); }
        PLAYER(SPECIES_WOBBUFFET) { HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, MOVE_FOCUS_BLAST, target: playerRight); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(playerLeft->hp, 100);
        EXPECT_LT(playerRight->hp, 1000);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: GodHand avoids a type-absorbing ability when choosing its attack")
{
    GIVEN {
        PLAYER(SPECIES_ARCEUS) { Ability(ABILITY_GODHAND); }
        OPPONENT(SPECIES_CAMERUPT) { Ability(ABILITY_WATER_ABSORB); HP(1000); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_LT(opponent->hp, 1000);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Gula doubles a stat Berry without treating cures as numerical")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GLUTTONY); Item(ITEM_LIECHI_BERRY); HP(20); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("Unholy 37: Gula doubles and remembers a Micle Berry accuracy bonus")
{
    GIVEN {
        PLAYER(SPECIES_ABRA) { Ability(ABILITY_GLUTTONY); Item(ITEM_MICLE_BERRY); HP(100); MaxHP(200); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(50); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_NIGHT_SHADE); }
    } THEN {
        EXPECT(gBattleStruct->unholyDoubledMicleMask & (1u << B_BATTLER_0));
        struct BattleCalcValues cv = {.battlerAtk = B_BATTLER_0, .battlerDef = B_BATTLER_1, .move = MOVE_WILL_O_WISP};
        cv.abilities[B_BATTLER_0] = ABILITY_NONE;
        // The doubled item effect survives a later ability change.
        player->ability = ABILITY_NONE;
        gBattleStruct->battlerState[B_BATTLER_0].usedMicleBerry = FALSE;
        u32 normal = GetTotalAccuracy(&cv, B_WEATHER_NONE);
        gBattleStruct->battlerState[B_BATTLER_0].usedMicleBerry = TRUE;
        EXPECT_EQ(GetTotalAccuracy(&cv, B_WEATHER_NONE), normal * 140 / 100);
    }
}
