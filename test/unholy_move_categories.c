#include "global.h"
#include "move.h"
#include "test/test.h"

TEST("Unholy Piedra Afilar accepts blades, claws, aerial cuts and special cuts")
{
    EXPECT(IsSlicingMove(MOVE_CUT));
    EXPECT(IsSlicingMove(MOVE_SLASH));
    EXPECT(IsSlicingMove(MOVE_METAL_CLAW));
    EXPECT(IsSlicingMove(MOVE_CRUSH_CLAW));
    EXPECT(IsSlicingMove(MOVE_DRAGON_CLAW));
    EXPECT(IsSlicingMove(MOVE_SHADOW_CLAW));
    EXPECT(IsSlicingMove(MOVE_DIRE_CLAW));
    EXPECT(IsSlicingMove(MOVE_RAZOR_LEAF));
    EXPECT(IsSlicingMove(MOVE_AIR_SLASH));
    EXPECT(IsSlicingMove(MOVE_PSYCHO_CUT));
    // Population Bomb is canonically a slicing move despite its name.
    EXPECT(IsSlicingMove(MOVE_POPULATION_BOMB));
    EXPECT(!IsSlicingMove(MOVE_TACKLE));
    EXPECT(!IsSlicingMove(MOVE_HYPER_BEAM));
    EXPECT(!IsSlicingMove(MOVE_SWORDS_DANCE));
}

TEST("Unholy punch and sound artefacts recognize their full move families")
{
    EXPECT(IsPunchingMove(MOVE_MACH_PUNCH));
    EXPECT(IsPunchingMove(MOVE_ICE_PUNCH));
    EXPECT(IsPunchingMove(MOVE_DOUBLE_SHOCK));
    EXPECT(!IsPunchingMove(MOVE_BRICK_BREAK));
    EXPECT(!IsPunchingMove(MOVE_SLASH));

    EXPECT(IsSoundMove(MOVE_BOOMBURST));
    EXPECT(IsSoundMove(MOVE_HOWL));
    EXPECT(IsSoundMove(MOVE_DRAGON_CHEER));
    EXPECT(!IsSoundMove(MOVE_HYPER_BEAM));
}

TEST("Unholy ballistic moves exclude beams, pulses and cutting waves")
{
    EXPECT(IsBallisticMove(MOVE_BULLET_SEED));
    EXPECT(IsBallisticMove(MOVE_SHADOW_BALL));
    EXPECT(IsBallisticMove(MOVE_ROCK_BLAST));
    EXPECT(!IsBallisticMove(MOVE_HYPER_BEAM));
    EXPECT(!IsBallisticMove(MOVE_WATER_PULSE));
    EXPECT(!IsBallisticMove(MOVE_AIR_SLASH));
}
