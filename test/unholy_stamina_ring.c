#include "global.h"
#include "stamina_ring.h"
#include "config/stamina.h"
#include "test/test.h"

// In-game smoke coverage of the exact 32x32 4bpp renderer used by OBJ VRAM.
TEST("Unholy circular stamina meter handles empty, partial and full values")
{
    u8 pixels[STAMINA_RING_TILE_BYTES];
    u8 full[STAMINA_RING_TILE_BYTES];
    u32 nonZero = 0;

    EXPECT_EQ(StaminaRing_FilledSegments(0, 100), 0);
    EXPECT_EQ(StaminaRing_FilledSegments(20, 100), 13);
    EXPECT_EQ(StaminaRing_FilledSegments(50, 100), 32);
    EXPECT_EQ(StaminaRing_FilledSegments(100, 100), 64);
    EXPECT_EQ(StaminaRing_FilledSegments(200, 100), 64);
    EXPECT_EQ(StaminaRing_FilledSegments(0, 0), 0);

    StaminaRing_Render(pixels, 0, 100, FALSE);
    for (u32 i = 0; i < sizeof(pixels); i++)
    {
        if (pixels[i] != 0)
            nonZero++;
    }
    EXPECT_GT(nonZero, 0);
    StaminaRing_Render(full, 100, 100, FALSE);
    EXPECT_EQ(memcmp(full, pixels, sizeof(full)) == 0, FALSE);

    // The centre stays transparent; sprite pixels use tile-major ordering.
    EXPECT_EQ(pixels[(2 * 4 + 2) * 32], 0);

    // HUD sprites use 16x16/4 tiles on both sides.  Same percentage,
    // same empty/full semantics, and a transparent centre.
    {
        u8 small[STAMINA_RING_COMPACT_TILE_BYTES];
        u8 smallFull[STAMINA_RING_COMPACT_TILE_BYTES];
        StaminaRing_RenderCompact(small, 0, 100, FALSE);
        StaminaRing_RenderCompact(smallFull, 100, 100, FALSE);
        EXPECT_EQ(memcmp(small, smallFull, sizeof(small)) == 0, FALSE);
        EXPECT_EQ(small[(1 * 2 + 1) * 32], 0);
    }
}

TEST("Unholy Stamina rings fit inside the left healthbox in singles and doubles")
{
    // Both standard and compact healthboxes anchor their 16x16 circle to
    // the centre of the same 64x32 left sprite. The opponent circle must
    // start below the nickname row (at least 14px from the top).
    const s32 ringSize = 16;
    const s32 leftBoxHalfWidth = 32;
    const s32 leftBoxHalfHeight = 16;
    const s32 halfRing = ringSize / 2;
    s32 playerLeft = leftBoxHalfWidth + STAMINA_RING_PLAYER_X_OFFSET - halfRing;
    s32 opponentLeft = leftBoxHalfWidth + STAMINA_RING_OPP_X_OFFSET - halfRing;
    s32 playerTop = leftBoxHalfHeight + STAMINA_RING_PLAYER_Y_OFFSET - halfRing;
    s32 opponentTop = leftBoxHalfHeight + STAMINA_RING_OPP_Y_OFFSET - halfRing;

    EXPECT_EQ(playerLeft >= 0, TRUE);
    EXPECT_EQ(playerLeft + ringSize <= 64, TRUE);
    EXPECT_EQ(opponentLeft >= 0, TRUE);
    EXPECT_EQ(opponentLeft + ringSize <= 64, TRUE);
    EXPECT_EQ(playerTop >= 0, TRUE);
    EXPECT_EQ(playerTop + ringSize <= 32, TRUE);
    EXPECT_EQ(opponentTop >= 14, TRUE);
    EXPECT_EQ(opponentTop + ringSize <= 32, TRUE);

    // The player has room above EXP, while the rival clears the name.
    EXPECT_EQ(playerTop, 12);
    EXPECT_EQ(opponentTop, 14);
}
