#include "global.h"
#include "mutation_icons.h"
#include "pokemon.h"
#include "sprite.h"
#include "test/test.h"

TEST("Mutation icons enumerate every combined flag without revealing unknown bits")
{
    const u16 expected[] = {POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER,
        POKEMON_BADGE_OVERLOADED, POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
        POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA};
    for (u32 i = 0; i < ARRAY_COUNT(expected); i++)
    {
        EXPECT_EQ(MutationIcon_BadgeAt(POKEMON_BADGES_ALL, i), expected[i]);
        EXPECT_EQ(MutationIcon_BadgeAt(expected[i], 1000), expected[i]);
    }
    EXPECT_EQ(MutationIcon_BadgeAt(POKEMON_BADGES_ALL, 9), POKEMON_BADGE_ALPHA);
    EXPECT_EQ(MutationIcon_BadgeAt(POKEMON_BADGE_ALPHA | POKEMON_BADGE_OMEGA, 1), POKEMON_BADGE_OMEGA);
    EXPECT_EQ(MutationIcon_BadgeAt(0, 0), 0);
    EXPECT_EQ(MutationIcon_BadgeAt(0x8000, 0), 0);
    EXPECT_EQ(MutationIcon_Create(0, 0, 0, 0), MAX_SPRITES);
    EXPECT_EQ(MutationIcon_Create(POKEMON_BADGES_ALL, 0, 0, 0), MAX_SPRITES);
}

TEST("Mutation icon instances share resources until their last owner releases them")
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    MutationIcon_ResetBattle();
    u8 first = MutationIcon_Create(POKEMON_BADGE_ALPHA, 16, 16, 0);
    u8 second = MutationIcon_Create(POKEMON_BADGE_ALPHA, 32, 16, 0);
    EXPECT_NE(first, MAX_SPRITES);
    EXPECT_NE(second, MAX_SPRITES);
    u16 tag = gSprites[first].template->paletteTag;
    EXPECT_EQ((u32)gSprites[first].oam.paletteNum, (u32)gSprites[second].oam.paletteNum);
    MutationIcon_Destroy(first);
    EXPECT(gSprites[second].inUse);
    EXPECT_NE(IndexOfSpritePaletteTag(tag), 0xFF);
    MutationIcon_Destroy(second);
    EXPECT_EQ(IndexOfSpritePaletteTag(tag), 0xFF);
    EXPECT_EQ(GetSpriteTileStartByTag(tag), TAG_NONE);

    struct Sprite box = {0};
    MutationIcon_UpdateBattle(0, &box, POKEMON_BADGE_ALPHA, 16, 16);
    MutationIcon_UpdateBattle(1, &box, POKEMON_BADGE_ALPHA, 32, 16);
    EXPECT(MutationIcon_BattleVisible(0));
    EXPECT(MutationIcon_BattleVisible(1));
    EXPECT(MutationIcon_ReleaseBattlePalette());
    EXPECT(!MutationIcon_BattleVisible(0));
    EXPECT(!MutationIcon_BattleVisible(1));
    EXPECT_EQ(IndexOfSpritePaletteTag(tag), 0xFF);
    EXPECT(!MutationIcon_ReleaseBattlePalette());
    MutationIcon_ClearBattle();
}
