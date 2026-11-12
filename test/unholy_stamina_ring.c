#include "global.h"
#include "stamina_ring.h"
#include "config/stamina.h"
#include "test/test.h"

// Exercise both 4bpp renderers and their tile-major layout.
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

    // The 9x9 HUD ring uses a 16x16 OBJ with a transparent centre.
    {
        u8 small[STAMINA_RING_COMPACT_TILE_BYTES];
        u8 smallFull[STAMINA_RING_COMPACT_TILE_BYTES];
        StaminaRing_RenderCompact(small, 0, 100, FALSE);
        StaminaRing_RenderCompact(smallFull, 100, 100, FALSE);
        EXPECT_EQ(memcmp(small, smallFull, sizeof(small)) == 0, FALSE);
        EXPECT_EQ(small[(1 * 2 + 1) * 32], 0);
    }
}

TEST("Unholy Stamina ring pixels clear healthbox text and status rows")
{
    u8 pixels[STAMINA_RING_COMPACT_TILE_BYTES];
    s32 radius = STAMINA_RING_COMPACT_RADIUS;
    u32 painted = 0;

    StaminaRing_RenderCompact(pixels, 100, 100, FALSE);
    for (s32 y = 0; y < 16; y++)
    {
        for (s32 x = 0; x < 16; x++)
        {
            u32 index = ((y >> 3) * 2 + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1);
            if (((pixels[index] >> ((x & 1) * 4)) & 0xF) == 0)
                continue;

            s32 dx = x - 8;
            s32 dy = y - 8;
            s32 singleX = 32 + STAMINA_RING_PLAYER_SINGLE_X_OFFSET + dx;
            s32 singleY = 16 + STAMINA_RING_PLAYER_SINGLE_Y_OFFSET + dy;
            s32 nicknameY = 16 + STAMINA_RING_NICKNAME_Y_OFFSET + dy;
            s32 doublePlayerX = 32 + STAMINA_RING_PLAYER_DOUBLE_X_OFFSET + dx;
            s32 opponentX = 32 + STAMINA_RING_OPP_X_OFFSET + dx;
            painted++;

            EXPECT_EQ(dx >= -radius && dx <= radius && dy >= -radius && dy <= radius, TRUE);
            EXPECT_EQ(singleX >= 14 && singleX < 24, TRUE);
            EXPECT_EQ(singleY >= 15 && singleY < 24, TRUE);
            EXPECT_EQ(nicknameY >= 5 && nicknameY < 16, TRUE);
            EXPECT_EQ(doublePlayerX < 16 + STAMINA_RING_NICKNAME_PADDING - 1, TRUE);
            EXPECT_EQ(opponentX < 8 + STAMINA_RING_NICKNAME_PADDING - 1, TRUE);
            if (nicknameY >= 12)
                EXPECT_EQ(opponentX >= 9, TRUE); // clears the caught icon
        }
    }
    EXPECT_GT(painted, 0);
}
