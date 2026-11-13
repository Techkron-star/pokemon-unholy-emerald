#include "global.h"
#include "mutation_icons.h"
#include "pokemon.h"
#include "sprite.h"
#include "battle.h"

#define MUTATION_TAG_BASE 0xD7E0 // Nine immutable sheets/palettes, shared by matching icons.
#define MUTATION_CYCLE_FRAMES 90

static const u32 salphaGfx[] = INCGFX_U32("graphics/mutations/badges/alpha.png", ".4bpp");
static const u16 salphaPal[] = INCGFX_U16("graphics/mutations/badges/palettes/alpha.pal", ".gbapal");
static const u32 shyperGfx[] = INCGFX_U32("graphics/mutations/badges/hyper.png", ".4bpp");
static const u16 shyperPal[] = INCGFX_U16("graphics/mutations/badges/palettes/hyper.pal", ".gbapal");
static const u32 ssuperGfx[] = INCGFX_U32("graphics/mutations/badges/super.png", ".4bpp");
static const u16 ssuperPal[] = INCGFX_U16("graphics/mutations/badges/palettes/super.pal", ".gbapal");
static const u32 soverloadedGfx[] = INCGFX_U32("graphics/mutations/badges/overloaded.png", ".4bpp");
static const u16 soverloadedPal[] = INCGFX_U16("graphics/mutations/badges/palettes/overloaded.pal", ".gbapal");
static const u32 senragedGfx[] = INCGFX_U32("graphics/mutations/badges/enraged.png", ".4bpp");
static const u16 senragedPal[] = INCGFX_U16("graphics/mutations/badges/palettes/enraged.pal", ".gbapal");
static const u32 sancestralGfx[] = INCGFX_U32("graphics/mutations/badges/ancestral.png", ".4bpp");
static const u16 sancestralPal[] = INCGFX_U16("graphics/mutations/badges/palettes/ancestral.pal", ".gbapal");
static const u32 stitanGfx[] = INCGFX_U32("graphics/mutations/badges/titan.png", ".4bpp");
static const u16 stitanPal[] = INCGFX_U16("graphics/mutations/badges/palettes/titan.pal", ".gbapal");
static const u32 sascendedGfx[] = INCGFX_U32("graphics/mutations/badges/ascended.png", ".4bpp");
static const u16 sascendedPal[] = INCGFX_U16("graphics/mutations/badges/palettes/ascended.pal", ".gbapal");
static const u32 somegaGfx[] = INCGFX_U32("graphics/mutations/badges/omega.png", ".4bpp");
static const u16 somegaPal[] = INCGFX_U16("graphics/mutations/badges/palettes/omega.pal", ".gbapal");

static const u16 sBadges[] =
{
    POKEMON_BADGE_ALPHA, POKEMON_BADGE_HYPER, POKEMON_BADGE_SUPER,
    POKEMON_BADGE_OVERLOADED, POKEMON_BADGE_ENRAGED, POKEMON_BADGE_ANCESTRAL,
    POKEMON_BADGE_TITAN, POKEMON_BADGE_ASCENDED, POKEMON_BADGE_OMEGA,
};

struct MutationArt { const u32 *gfx; const u16 *palette; };
static const struct MutationArt sArt[] =
{
    {salphaGfx, salphaPal}, {shyperGfx, shyperPal}, {ssuperGfx, ssuperPal},
    {soverloadedGfx, soverloadedPal}, {senragedGfx, senragedPal},
    {sancestralGfx, sancestralPal}, {stitanGfx, stitanPal},
    {sascendedGfx, sascendedPal}, {somegaGfx, somegaPal},
};
static const struct OamData sOam =
{
    .shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16), .priority = 1,
};
#define BADGE_TEMPLATE(i) { .tileTag = MUTATION_TAG_BASE + i, .paletteTag = MUTATION_TAG_BASE + i, .oam = &sOam, .callback = SpriteCallbackDummy }
static const struct SpriteTemplate sTemplates[] =
{
    BADGE_TEMPLATE(0), BADGE_TEMPLATE(1), BADGE_TEMPLATE(2),
    BADGE_TEMPLATE(3), BADGE_TEMPLATE(4), BADGE_TEMPLATE(5),
    BADGE_TEMPLATE(6), BADGE_TEMPLATE(7), BADGE_TEMPLATE(8),
};

static EWRAM_DATA u8 sBattleIcons[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA u16 sBattleMasks[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA u32 sBattleFrames[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA bool8 sBattleShown[MAX_BATTLERS_COUNT] = {0};

static const u8 *const sLabels[] =
{
    COMPOUND_STRING(" A"), COMPOUND_STRING(" H"), COMPOUND_STRING(" S"),
    COMPOUND_STRING(" OL"), COMPOUND_STRING(" E"), COMPOUND_STRING(" AN"),
    COMPOUND_STRING(" T"), COMPOUND_STRING(" AS"), COMPOUND_STRING(" O"),
};

const u8 *MutationIcon_BattleLabel(u16 badges)
{
    u16 badge = MutationIcon_BadgeAt(badges, 0);
    for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
        if (sBadges[i] == badge)
            return sLabels[i];
    return COMPOUND_STRING("");
}

bool32 MutationIcon_IsPaletteTag(u16 tag)
{
    return tag >= MUTATION_TAG_BASE && tag < MUTATION_TAG_BASE + ARRAY_COUNT(sBadges);
}

u16 MutationIcon_BadgeAt(u16 badges, u32 ordinal)
{
    u8 count = 0;
    for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
        if (badges & sBadges[i])
            count++;
    if (count == 0)
        return 0;
    ordinal %= count;
    for (u8 i = 0; i < ARRAY_COUNT(sBadges); i++)
        if ((badges & sBadges[i]) && ordinal-- == 0)
            return sBadges[i];
    return 0;
}

static s32 ArtIndex(const struct Sprite *sprite)
{
    for (u8 i = 0; i < ARRAY_COUNT(sTemplates); i++)
        if (sprite->template == &sTemplates[i])
            return i;
    return -1;
}

u8 MutationIcon_Create(u16 badge, s16 x, s16 y, u8 priority)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sBadges); i++)
        if (sBadges[i] == badge)
            break;
    if (i == ARRAY_COUNT(sBadges))
        return MAX_SPRITES;
    u16 tag = MUTATION_TAG_BASE + i;
    bool32 newTiles = GetSpriteTileStartByTag(tag) == TAG_NONE;
    bool32 newPalette = IndexOfSpritePaletteTag(tag) == 0xFF;
    const struct SpriteSheet sheet = {sArt[i].gfx, 4 * TILE_SIZE_4BPP, tag};
    const struct SpritePalette palette = {sArt[i].palette, tag};
    if (newTiles && (!CanAllocSpriteTiles(4) || LoadSpriteSheet(&sheet) == TAG_NONE))
        return MAX_SPRITES;
    if (newPalette && LoadSpritePalette(&palette) == 0xFF)
    {
        if (newTiles)
            FreeSpriteTilesByTag(tag);
        return MAX_SPRITES;
    }
    u8 id = CreateSprite(&sTemplates[i], x, y, 0);
    if (id != MAX_SPRITES)
        gSprites[id].oam.priority = priority;
    else
    {
        if (newTiles)
            FreeSpriteTilesByTag(tag);
        if (newPalette)
            FreeSpritePaletteByTag(tag);
    }
    return id;
}

void MutationIcon_Destroy(u8 id)
{
    if (id >= MAX_SPRITES || !gSprites[id].inUse)
        return;
    s32 index = ArtIndex(&gSprites[id]);
    if (index < 0)
        return;
    DestroySprite(&gSprites[id]);
    for (u8 i = 0; i < MAX_SPRITES; i++)
        if (gSprites[i].inUse && gSprites[i].template == &sTemplates[index])
            return;
    FreeSpriteTilesByTag(MUTATION_TAG_BASE + index);
    FreeSpritePaletteByTag(MUTATION_TAG_BASE + index);
}

void MutationIcon_ResetBattle(void)
{
    // Called after the engine resets sprites and their resource tables.
    for (u8 i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        sBattleIcons[i] = MAX_SPRITES;
        sBattleMasks[i] = 0;
        sBattleFrames[i] = 0;
        sBattleShown[i] = FALSE;
    }
}

void MutationIcon_ClearBattle(void)
{
    for (u8 i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        MutationIcon_Destroy(sBattleIcons[i]);
        sBattleIcons[i] = MAX_SPRITES;
    }
}

bool32 MutationIcon_BattleVisible(u8 battler)
{
    return battler < MAX_BATTLERS_COUNT && sBattleIcons[battler] < MAX_SPRITES
        && gSprites[sBattleIcons[battler]].inUse && ArtIndex(&gSprites[sBattleIcons[battler]]) >= 0;
}

// Essential move/type/ability palettes take precedence over this optional HUD.
bool32 MutationIcon_ReleaseBattlePalette(void)
{
    for (u8 i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        if (!MutationIcon_BattleVisible(i))
            continue;
        s32 art = ArtIndex(&gSprites[sBattleIcons[i]]);
        for (u8 j = 0; j < MAX_BATTLERS_COUNT; j++)
            if (MutationIcon_BattleVisible(j) && ArtIndex(&gSprites[sBattleIcons[j]]) == art)
            {
                MutationIcon_Destroy(sBattleIcons[j]);
                sBattleIcons[j] = MAX_SPRITES;
            }
        if (IndexOfSpritePaletteTag(MUTATION_TAG_BASE + art) == 0xFF)
            return TRUE;
    }
    return FALSE;
}

static bool32 BattleVisualChanged(u8 battler)
{
    bool8 visible = MutationIcon_BattleVisible(battler);
    bool32 changed = sBattleShown[battler] != visible;
    sBattleShown[battler] = visible;
    return changed;
}

bool32 MutationIcon_UpdateBattle(u8 battler, const struct Sprite *box, u16 badges, s16 x, s16 y)
{
    if (battler >= MAX_BATTLERS_COUNT)
        return FALSE;
    if (sBattleMasks[battler] != badges)
    {
        sBattleMasks[battler] = badges;
        sBattleFrames[battler] = 0;
    }
    // Cycle through the complete combination; never pick only the first flag.
    u16 badge = MutationIcon_BadgeAt(badges, sBattleFrames[battler] / MUTATION_CYCLE_FRAMES);
    sBattleFrames[battler]++;
    u8 id = sBattleIcons[battler];
    if (id < MAX_SPRITES && (badge == 0 || !gSprites[id].inUse || ArtIndex(&gSprites[id]) < 0
        || sBadges[ArtIndex(&gSprites[id])] != badge))
    {
        MutationIcon_Destroy(id);
        id = sBattleIcons[battler] = MAX_SPRITES;
    }
    if (badge == 0)
        return BattleVisualChanged(battler);
    if (id == MAX_SPRITES)
        id = sBattleIcons[battler] = MutationIcon_Create(badge, x, y, box->oam.priority);
    if (id == MAX_SPRITES)
        return BattleVisualChanged(battler);
    gSprites[id].x = x;
    gSprites[id].y = y;
    gSprites[id].x2 = box->x2;
    gSprites[id].y2 = box->y2;
    gSprites[id].oam.priority = box->oam.priority;
    gSprites[id].invisible = box->invisible;
    return BattleVisualChanged(battler);
}
