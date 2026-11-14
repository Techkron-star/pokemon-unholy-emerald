#include "global.h"
#include "horizon_goods.h"
#include "event_data.h"
#include "random.h"
#include "constants/items.h"
#include "constants/vars.h"

// Horizon Goods uses the same rotation schedule as the six Pokémon offers.
// Only two 16-bit seed vars are saved. A local PRNG reconstructs independent
// item and TM lists without spending global RNG on shop reopening.
struct HorizonGoodsEntry
{
    u16 itemId;
    u8 minBadges;
    u8 timeRestriction; // 0 either, 1 daytime, 2 nighttime
};

#define HORIZON_GOODS_TIME_DAY   0
#define HORIZON_GOODS_TIME_NIGHT 1

static const struct HorizonGoodsEntry sHorizonItemPool[] =
{
    {ITEM_GREAT_BALL,       1, 0},
    {ITEM_ULTRA_BALL,       3, 0},
    {ITEM_DUSK_BALL,        0, 2},
    {ITEM_QUICK_BALL,       0, 1},
    {ITEM_LUXURY_BALL,      2, 0},
    {ITEM_SUPER_POTION,     1, 0},
    {ITEM_HYPER_POTION,     3, 0},
    {ITEM_REVIVE,           2, 0},
    {ITEM_FULL_HEAL,        2, 0},
    {ITEM_ETHER,            1, 0},
    {ITEM_PP_UP,            6, 0},
    {ITEM_ABILITY_CAPSULE,  0, 0},
    {ITEM_ABILITY_PATCH,    0, 0},
    {ITEM_REPEL,            0, 0},
    {ITEM_SUPER_REPEL,      1, 0},
    {ITEM_MAX_REPEL,        3, 0},
    {ITEM_X_ATTACK,         0, 0},
    {ITEM_X_DEFENSE,        0, 0},
    {ITEM_RARE_CANDY,       5, 0},
};

static const struct HorizonGoodsEntry sHorizonDiscPool[] =
{
    {ITEM_TM01, 0, 0}, {ITEM_TM05, 0, 0},
    {ITEM_TM10, 0, 0}, {ITEM_TM11, 0, 0},
    {ITEM_TM17, 0, 0}, {ITEM_TM21, 0, 0},
    {ITEM_TM28, 0, 0}, {ITEM_TM32, 0, 0},
    {ITEM_TM40, 0, 0}, {ITEM_TM45, 0, 0},
    {ITEM_TM04, 1, 0}, {ITEM_TM06, 1, 2},
    {ITEM_TM13, 2, 0}, {ITEM_TM24, 2, 1},
    {ITEM_TM30, 3, 2}, {ITEM_TM35, 3, 0},
    {ITEM_TM36, 4, 0}, {ITEM_TM42, 4, 0},
    {ITEM_TM44, 5, 0}, {ITEM_TM46, 5, 0},
    {ITEM_TM26, 7, 0}, {ITEM_TM38, 7, 0},
    {ITEM_TM50, 8, 0},
};

// Three everyday consumables and Linking Cord always appear, plus four rotating options.
// Those items are intentionally NOT limited to one purchase per rotation.
static EWRAM_DATA u16 sHorizonItems[HORIZON_GOODS_ITEM_COUNT + 1];
static EWRAM_DATA u16 sHorizonDiscs[HORIZON_GOODS_DISC_COUNT + 1];

static u32 HorizonGoodsNextRandom(u32 *seed)
{
    u32 value = *seed;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    *seed = value;
    return value;
}

static u32 HorizonGoodsSavedSeed(u32 mix)
{
    u32 seed = ((u32)VarGet(VAR_HORIZON_GOODS_SEED_A) << 16)
             | VarGet(VAR_HORIZON_GOODS_SEED_B);

    seed ^= mix;
    return seed != 0 ? seed : 0x1A9D0B4Fu;
}

void HorizonGoodsRollSeeds(void)
{
    // Force both halves to nonzero values, so old saves with uninitialized
    // stock can be reliably migrated on their first Horizon opening.
    VarSet(VAR_HORIZON_GOODS_SEED_A, (Random32() & 0xFFFF) | 1);
    VarSet(VAR_HORIZON_GOODS_SEED_B, (Random32() & 0xFFFF) | 1);
}

void HorizonGoodsEnsureSeeds(void)
{
    if (VarGet(VAR_HORIZON_GOODS_SEED_A) == 0
     || VarGet(VAR_HORIZON_GOODS_SEED_B) == 0)
        HorizonGoodsRollSeeds();
}

static bool8 HorizonGoodsEligible(const struct HorizonGoodsEntry *entry, u8 badges, u8 period)
{
    return entry->minBadges <= badges
        && (entry->timeRestriction == 0 || entry->timeRestriction == period + 1);
}

static void HorizonGoodsBuildList(const struct HorizonGoodsEntry *pool, u16 poolSize,
                                  u16 *dest, u8 targetCount, u32 seed)
{
    u8 badges = VarGet(VAR_HORIZON_MARKET_BADGES);
    u8 period = VarGet(VAR_HORIZON_MARKET_PERIOD);
    u8 count = 0;

    // The low-level shop expects distinct ITEM_NONE-terminated entries.
    // The pool always has more eligible entries than the requested slots.
    for (u8 i = 0; i < targetCount; i++)
        dest[i] = ITEM_NONE;

    while (count < targetCount)
    {
        u16 eligibleCount = 0, chosenIndex = 0;
        u16 selected = ITEM_NONE;

        for (u16 i = 0; i < poolSize; i++)
        {
            bool8 alreadyUsed = FALSE;

            if (!HorizonGoodsEligible(&pool[i], badges, period))
                continue;

            for (u8 slot = 0; slot < count; slot++)
            {
                if (dest[slot] == pool[i].itemId)
                {
                    alreadyUsed = TRUE;
                    break;
                }
            }
            if (!alreadyUsed)
                eligibleCount++;
        }

        // This also terminates safely if a future designer restricts
        // too many offers. No repeats or infinite loops.
        if (eligibleCount == 0)
            break;

        chosenIndex = HorizonGoodsNextRandom(&seed) % eligibleCount;
        for (u16 i = 0; i < poolSize; i++)
        {
            bool8 alreadyUsed = FALSE;

            if (!HorizonGoodsEligible(&pool[i], badges, period))
                continue;

            for (u8 slot = 0; slot < count; slot++)
            {
                if (dest[slot] == pool[i].itemId)
                {
                    alreadyUsed = TRUE;
                    break;
                }
            }
            if (!alreadyUsed && chosenIndex-- == 0)
            {
                selected = pool[i].itemId;
                break;
            }
        }

        if (selected == ITEM_NONE)
            break;
        dest[count++] = selected;
    }
    dest[count] = ITEM_NONE;
}

const u16 *HorizonGoodsGetItems(void)
{
    u32 seed;

    HorizonGoodsEnsureSeeds();
    seed = HorizonGoodsSavedSeed(0xB15A3A2Fu);
    sHorizonItems[0] = ITEM_POKE_BALL;
    sHorizonItems[1] = ITEM_POTION;
    sHorizonItems[2] = ITEM_ESCAPE_ROPE;
    sHorizonItems[3] = ITEM_LINKING_CORD;

    HorizonGoodsBuildList(sHorizonItemPool, ARRAY_COUNT(sHorizonItemPool),
                          &sHorizonItems[4], HORIZON_GOODS_ITEM_COUNT - 4, seed);
    return sHorizonItems;
}

const u16 *HorizonGoodsGetDiscs(void)
{
    HorizonGoodsEnsureSeeds();
    HorizonGoodsBuildList(sHorizonDiscPool, ARRAY_COUNT(sHorizonDiscPool),
                          sHorizonDiscs, HORIZON_GOODS_DISC_COUNT,
                          HorizonGoodsSavedSeed(0x7338C71Du));
    return sHorizonDiscs;
}
