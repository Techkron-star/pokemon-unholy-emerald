#include "global.h"
#include "battle_chain.h"
#include "test/test.h"

TEST("Unholy chains refuse protection and turn-nullification moves")
{
    EXPECT(!Chain_IsMoveChainable(MOVE_NONE));
    EXPECT(!Chain_IsMoveChainable(MOVE_PROTECT));
    EXPECT(!Chain_IsMoveChainable(MOVE_DETECT));
    EXPECT(!Chain_IsMoveChainable(MOVE_ENDURE));
    EXPECT(!Chain_IsMoveChainable(MOVE_KINGS_SHIELD));
    EXPECT(!Chain_IsMoveChainable(MOVE_SPIKY_SHIELD));
    EXPECT(!Chain_IsMoveChainable(MOVE_BANEFUL_BUNKER));
    EXPECT(!Chain_IsMoveChainable(MOVE_MAT_BLOCK));
    EXPECT(!Chain_IsMoveChainable(MOVE_MAGIC_COAT));
    EXPECT(Chain_IsMoveChainable(MOVE_TACKLE));
}
