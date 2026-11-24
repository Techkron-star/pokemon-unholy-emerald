#include "global.h"
#include "move.h"
#include "string_util.h"
#include "test/test.h"

TEST("All normal, Z, Dynamax and Gigantamax moves have bounded Spanish text")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;
    u32 move;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (move = MOVE_POUND; move < MOVES_COUNT_ALL; move++)
    {
        const u8 *name = GetMoveName(move);
        const u8 *description = GetMoveDescription(move);

        EXPECT_NE(name, NULL);
        EXPECT_NE(description, NULL);
        EXPECT_LE(StringLength(name), MOVE_NAME_LENGTH);
        EXPECT_NE(StringLength(description), 0);
    }

    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("Move localization respects English and includes Z and Dynamax moves")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    EXPECT_EQ(GetMoveName(MOVE_POUND), gMovesInfo[MOVE_POUND].name);
    EXPECT_EQ(GetMoveDescription(MOVE_POUND), gMovesInfo[MOVE_POUND].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    EXPECT_NE(GetMoveName(MOVE_POUND), gMovesInfo[MOVE_POUND].name);
    EXPECT_NE(GetMoveDescription(MOVE_POUND), gMovesInfo[MOVE_POUND].description);
    EXPECT_NE(GetMoveName(MOVE_SKETCH), gMovesInfo[MOVE_SKETCH].name);
    EXPECT_NE(GetMoveName(MOVE_FAKE_OUT), gMovesInfo[MOVE_FAKE_OUT].name);
    EXPECT_NE(GetMoveName(MOVE_ROOST), gMovesInfo[MOVE_ROOST].name);
    EXPECT_NE(GetMoveName(MOVE_HONE_CLAWS), gMovesInfo[MOVE_HONE_CLAWS].name);
    EXPECT_NE(GetMoveDescription(MOVE_HONE_CLAWS), gMovesInfo[MOVE_HONE_CLAWS].description);
    EXPECT_NE(GetMoveName(MOVE_FUSION_BOLT), gMovesInfo[MOVE_FUSION_BOLT].name);
    EXPECT_NE(GetMoveDescription(MOVE_FUSION_BOLT), gMovesInfo[MOVE_FUSION_BOLT].description);
    EXPECT_NE(GetMoveName(MOVE_FLYING_PRESS), gMovesInfo[MOVE_FLYING_PRESS].name);
    EXPECT_NE(GetMoveDescription(MOVE_FLYING_PRESS), gMovesInfo[MOVE_FLYING_PRESS].description);
    EXPECT_NE(GetMoveName(MOVE_HYPERSPACE_FURY), gMovesInfo[MOVE_HYPERSPACE_FURY].name);
    EXPECT_NE(GetMoveDescription(MOVE_HYPERSPACE_FURY), gMovesInfo[MOVE_HYPERSPACE_FURY].description);
    EXPECT_NE(GetMoveName(MOVE_SHORE_UP), gMovesInfo[MOVE_SHORE_UP].name);
    EXPECT_NE(GetMoveDescription(MOVE_SHORE_UP), gMovesInfo[MOVE_SHORE_UP].description);
    EXPECT_NE(GetMoveName(MOVE_DOUBLE_IRON_BASH), gMovesInfo[MOVE_DOUBLE_IRON_BASH].name);
    EXPECT_NE(GetMoveDescription(MOVE_DOUBLE_IRON_BASH), gMovesInfo[MOVE_DOUBLE_IRON_BASH].description);
    EXPECT_NE(GetMoveName(MOVE_DYNAMAX_CANNON), gMovesInfo[MOVE_DYNAMAX_CANNON].name);
    EXPECT_NE(GetMoveDescription(MOVE_DYNAMAX_CANNON), gMovesInfo[MOVE_DYNAMAX_CANNON].description);
    EXPECT_NE(GetMoveName(MOVE_TAKE_HEART), gMovesInfo[MOVE_TAKE_HEART].name);
    EXPECT_NE(GetMoveDescription(MOVE_TAKE_HEART), gMovesInfo[MOVE_TAKE_HEART].description);
    EXPECT_NE(GetMoveName(MOVE_TERA_BLAST), gMovesInfo[MOVE_TERA_BLAST].name);
    EXPECT_NE(GetMoveDescription(MOVE_TERA_BLAST), gMovesInfo[MOVE_TERA_BLAST].description);
    EXPECT_NE(GetMoveName(MOVE_MALIGNANT_CHAIN), gMovesInfo[MOVE_MALIGNANT_CHAIN].name);
    EXPECT_NE(GetMoveDescription(MOVE_MALIGNANT_CHAIN), gMovesInfo[MOVE_MALIGNANT_CHAIN].description);
    EXPECT_NE(GetMoveName(MOVE_BREAKNECK_BLITZ), gMovesInfo[MOVE_BREAKNECK_BLITZ].name);
    EXPECT_NE(GetMoveDescription(MOVE_BREAKNECK_BLITZ), gMovesInfo[MOVE_BREAKNECK_BLITZ].description);
    EXPECT_NE(GetMoveName(MOVE_SOUL_STEALING_7_STAR_STRIKE), gMovesInfo[MOVE_SOUL_STEALING_7_STAR_STRIKE].name);
    EXPECT_NE(GetMoveName(MOVE_MAX_GUARD), gMovesInfo[MOVE_MAX_GUARD].name);
    EXPECT_NE(GetMoveDescription(MOVE_G_MAX_RAPID_FLOW), gMovesInfo[MOVE_G_MAX_RAPID_FLOW].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}
