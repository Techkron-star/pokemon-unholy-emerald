#include "global.h"
#include "battle.h"
#include "move.h"
#include "pokemon.h"
#include "unholy_qol.h"
#include "event_data.h"
#include "item.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/vars.h"

TEST("Unholy HMs may be replaced without visiting the Move Deleter")
{
    EXPECT(!CannotForgetMove(MOVE_CUT));
    EXPECT(!CannotForgetMove(MOVE_FLASH));
    EXPECT(!CannotForgetMove(MOVE_SURF));
    EXPECT(!CannotForgetMove(MOVE_STRENGTH));
}

TEST("Unholy cutting moves gain critical ratio without misclassifying Double-Edge")
{
    EXPECT_EQ(GetMoveCriticalHitStage(MOVE_CUT), 1);
    EXPECT_EQ(GetMoveCriticalHitStage(MOVE_SACRED_SWORD), 1);
    EXPECT_EQ(GetMoveCriticalHitStage(MOVE_AERIAL_ACE), 1);
    EXPECT_EQ(GetMoveCriticalHitStage(MOVE_PSYCHO_CUT), 1);
    EXPECT_EQ(GetMoveCriticalHitStage(MOVE_DOUBLE_EDGE), 0);
    EXPECT(!IsSlicingMove(MOVE_DOUBLE_EDGE));
}

TEST("Unholy field moves have useful battle effects without breaking HM identity")
{
    EXPECT_EQ(GetMovePower(MOVE_CUT), 60);
    EXPECT_EQ(GetMoveType(MOVE_CUT), TYPE_STEEL);
    EXPECT_EQ(GetMovePower(MOVE_FLASH), 60);
    EXPECT_EQ(GetMoveType(MOVE_FLASH), TYPE_FAIRY);
    EXPECT(!IsBattleMoveStatus(MOVE_FLASH));
    EXPECT_EQ(GetMovePower(MOVE_ROCK_SMASH), 65);
    EXPECT_EQ(GetMovePower(MOVE_STRENGTH), 90);
    EXPECT_EQ(GetMoveType(MOVE_STRENGTH), TYPE_FIGHTING);
    EXPECT_EQ(GetMovePower(MOVE_WHIRLPOOL), 50);
    EXPECT_EQ(GetMovePower(MOVE_SECRET_POWER), 80);
    EXPECT_EQ(GetMoveType(MOVE_ROCK_CLIMB), TYPE_ROCK);
}

TEST("Unholy Auto Cut requires its HM badge and setting but no party move")
{
    const u16 badgeFlag = IS_FRLG ? FLAG_BADGE02_GET : FLAG_BADGE01_GET;
    u16 savedOptions = VarGet(VAR_UNHOLY_QOL_FLAGS);
    bool32 hadBadge = FlagGet(badgeFlag);
    bool32 hadHM = CheckBagHasItem(ITEM_HM_CUT, 1);

    // Grant the trainer's normal progression requirements, not a Pokemon move.
    if (!hadBadge)
        FlagSet(badgeFlag);
    if (!hadHM)
        EXPECT(AddBagItem(ITEM_HM_CUT, 1));

    VarSet(VAR_UNHOLY_QOL_FLAGS, (1 << 15) | (1 << UQOL_AUTO_CUT));
    EXPECT(UnholyQolAutoFieldMoveAvailable(FIELD_MOVE_CUT));
    EXPECT(!UnholyQolAutoFieldMoveAvailable(FIELD_MOVE_SURF));

    FlagClear(badgeFlag);
    EXPECT(!UnholyQolAutoFieldMoveAvailable(FIELD_MOVE_CUT));
    FlagSet(badgeFlag);
    if (!hadHM)
    {
        EXPECT(RemoveBagItem(ITEM_HM_CUT, 1));
        EXPECT(!UnholyQolAutoFieldMoveAvailable(FIELD_MOVE_CUT));
        EXPECT(AddBagItem(ITEM_HM_CUT, 1));
    }
    VarSet(VAR_UNHOLY_QOL_FLAGS, 1 << 15);
    EXPECT(!UnholyQolAutoFieldMoveAvailable(FIELD_MOVE_CUT));

    VarSet(VAR_UNHOLY_QOL_FLAGS, savedOptions);
    if (!hadHM)
        EXPECT(RemoveBagItem(ITEM_HM_CUT, 1));
    if (!hadBadge)
        FlagClear(badgeFlag);
}
