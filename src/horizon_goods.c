#include "global.h"
#include "horizon_goods.h"
#include "hfc_watch.h"
#include "event_data.h"
#include "random.h"
#include "item.h"
#include "money.h"
#include "unholy_save.h"
#include "constants/items.h"
#include "constants/vars.h"

struct HorizonGoodsEntry
{
    u16 itemId;
    u8 minBadges;
    u8 timeRestriction;
    u8 rarity;
};

static const struct HorizonGoodsEntry sHorizonItemPool[] =
{
    {ITEM_GREAT_BALL, 0, 0, 0},
    {ITEM_SUPER_POTION, 0, 0, 0},
    {ITEM_REPEL, 0, 0, 0},
    {ITEM_ETHER, 0, 0, 0},
    {ITEM_HEAL_BALL, 0, 0, 0},
    {ITEM_NET_BALL, 0, 0, 0},
    {ITEM_NEST_BALL, 0, 0, 0},
    {ITEM_ANTIDOTE, 0, 0, 0},
    {ITEM_FULL_HEAL, 0, 0, 0},
    {ITEM_X_ATTACK, 0, 0, 0},
    {ITEM_X_DEFENSE, 0, 0, 0},
    {ITEM_EXP_CANDY_XS, 0, 0, 0},
    {ITEM_EXP_CANDY_S, 0, 0, 0},
    {ITEM_ULTRA_BALL, 2, 0, 1},
    {ITEM_DUSK_BALL, 2, 0, 1},
    {ITEM_QUICK_BALL, 2, 0, 1},
    {ITEM_LUXURY_BALL, 2, 0, 1},
    {ITEM_TIMER_BALL, 2, 0, 1},
    {ITEM_REPEAT_BALL, 2, 0, 1},
    {ITEM_DIVE_BALL, 2, 0, 1},
    {ITEM_HYPER_POTION, 2, 0, 1},
    {ITEM_REVIVE, 2, 0, 1},
    {ITEM_SUPER_REPEL, 2, 0, 1},
    {ITEM_MAX_REPEL, 2, 0, 1},
    {ITEM_EXP_CANDY_M, 2, 0, 1},
    {ITEM_HEALTH_FEATHER, 2, 0, 1},
    {ITEM_MUSCLE_FEATHER, 2, 0, 1},
    {ITEM_RESIST_FEATHER, 2, 0, 1},
    {ITEM_GENIUS_FEATHER, 2, 0, 1},
    {ITEM_CLEVER_FEATHER, 2, 0, 1},
    {ITEM_SWIFT_FEATHER, 2, 0, 1},
    {ITEM_FIRE_STONE, 2, 0, 1},
    {ITEM_WATER_STONE, 2, 0, 1},
    {ITEM_THUNDER_STONE, 2, 0, 1},
    {ITEM_LEAF_STONE, 2, 0, 1},
    {ITEM_MOON_STONE, 2, 0, 1},
    {ITEM_SUN_STONE, 2, 0, 1},
    {ITEM_MAX_POTION, 4, 0, 2},
    {ITEM_FULL_RESTORE, 4, 0, 2},
    {ITEM_MAX_REVIVE, 4, 0, 2},
    {ITEM_MAX_ETHER, 4, 0, 2},
    {ITEM_ELIXIR, 4, 0, 2},
    {ITEM_PP_UP, 4, 0, 2},
    {ITEM_EXP_CANDY_L, 4, 0, 2},
    {ITEM_ABILITY_CAPSULE, 4, 0, 2},
    {ITEM_DYNAMAX_CANDY, 4, 0, 2},
    {ITEM_WISHING_PIECE, 4, 0, 2},
    {ITEM_ARMORITE_ORE, 4, 0, 2},
    {ITEM_DYNITE_ORE, 4, 0, 2},
    {ITEM_MAX_MUSHROOMS, 4, 0, 2},
    {ITEM_MAX_HONEY, 4, 0, 2},
    {ITEM_BOTTLE_CAP, 4, 0, 2},
    {ITEM_CHOICE_BAND, 4, 0, 2},
    {ITEM_CHOICE_SCARF, 4, 0, 2},
    {ITEM_CHOICE_SPECS, 4, 0, 2},
    {ITEM_FOCUS_SASH, 4, 0, 2},
    {ITEM_LEFTOVERS, 4, 0, 2},
    {ITEM_LIFE_ORB, 4, 0, 2},
    {ITEM_EVIOLITE, 4, 0, 2},
    {ITEM_EXP_CANDY_XL, 5, 0, 3},
    {ITEM_ABILITY_PATCH, 5, 0, 3},
    {ITEM_GOLD_BOTTLE_CAP, 5, 0, 3},
    {ITEM_PP_MAX, 5, 0, 3},
    {ITEM_MAX_ELIXIR, 5, 0, 3},
    {ITEM_ROTOM_CATALOG, 5, 0, 3},
    {ITEM_GRACIDEA, 5, 0, 3},
    {ITEM_REVEAL_GLASS, 5, 0, 3},
    {ITEM_DNA_SPLICERS, 6, 0, 4},
    {ITEM_N_SOLARIZER, 6, 0, 4},
    {ITEM_N_LUNARIZER, 6, 0, 4},
    {ITEM_REINS_OF_UNITY, 6, 0, 4},
    {ITEM_PRISON_BOTTLE, 6, 0, 4},
    {ITEM_ZYGARDE_CUBE, 6, 0, 4},
    {ITEM_RED_ORB, 6, 0, 4},
    {ITEM_BLUE_ORB, 6, 0, 4},
    {ITEM_GRISEOUS_CORE, 6, 0, 4},
    {ITEM_SOUL_DEW, 6, 0, 4},
    {ITEM_MASTER_BALL, 5, 0, 5},
    {ITEM_LONELY_MINT, 2, 0, 1},
    {ITEM_ADAMANT_MINT, 2, 0, 1},
    {ITEM_NAUGHTY_MINT, 2, 0, 1},
    {ITEM_BRAVE_MINT, 2, 0, 1},
    {ITEM_BOLD_MINT, 2, 0, 1},
    {ITEM_IMPISH_MINT, 2, 0, 1},
    {ITEM_LAX_MINT, 2, 0, 1},
    {ITEM_RELAXED_MINT, 2, 0, 1},
    {ITEM_MODEST_MINT, 2, 0, 1},
    {ITEM_MILD_MINT, 2, 0, 1},
    {ITEM_RASH_MINT, 2, 0, 1},
    {ITEM_QUIET_MINT, 2, 0, 1},
    {ITEM_CALM_MINT, 2, 0, 1},
    {ITEM_GENTLE_MINT, 2, 0, 1},
    {ITEM_CAREFUL_MINT, 2, 0, 1},
    {ITEM_SASSY_MINT, 2, 0, 1},
    {ITEM_TIMID_MINT, 2, 0, 1},
    {ITEM_HASTY_MINT, 2, 0, 1},
    {ITEM_JOLLY_MINT, 2, 0, 1},
    {ITEM_NAIVE_MINT, 2, 0, 1},
    {ITEM_SERIOUS_MINT, 2, 0, 1},
    {ITEM_VENUSAURITE, 5, 0, 3},
    {ITEM_CHARIZARDITE_X, 5, 0, 3},
    {ITEM_CHARIZARDITE_Y, 5, 0, 3},
    {ITEM_BLASTOISINITE, 5, 0, 3},
    {ITEM_BEEDRILLITE, 5, 0, 3},
    {ITEM_PIDGEOTITE, 5, 0, 3},
    {ITEM_ALAKAZITE, 5, 0, 3},
    {ITEM_SLOWBRONITE, 5, 0, 3},
    {ITEM_GENGARITE, 5, 0, 3},
    {ITEM_KANGASKHANITE, 5, 0, 3},
    {ITEM_PINSIRITE, 5, 0, 3},
    {ITEM_GYARADOSITE, 5, 0, 3},
    {ITEM_AERODACTYLITE, 5, 0, 3},
    {ITEM_MEWTWONITE_X, 5, 0, 4},
    {ITEM_MEWTWONITE_Y, 5, 0, 4},
    {ITEM_AMPHAROSITE, 5, 0, 3},
    {ITEM_STEELIXITE, 5, 0, 3},
    {ITEM_SCIZORITE, 5, 0, 3},
    {ITEM_HERACRONITE, 5, 0, 3},
    {ITEM_HOUNDOOMINITE, 5, 0, 3},
    {ITEM_TYRANITARITE, 5, 0, 3},
    {ITEM_SCEPTILITE, 5, 0, 3},
    {ITEM_BLAZIKENITE, 5, 0, 3},
    {ITEM_SWAMPERTITE, 5, 0, 3},
    {ITEM_GARDEVOIRITE, 5, 0, 3},
    {ITEM_SABLENITE, 5, 0, 3},
    {ITEM_MAWILITE, 5, 0, 3},
    {ITEM_AGGRONITE, 5, 0, 3},
    {ITEM_MEDICHAMITE, 5, 0, 3},
    {ITEM_MANECTITE, 5, 0, 3},
    {ITEM_SHARPEDONITE, 5, 0, 3},
    {ITEM_CAMERUPTITE, 5, 0, 3},
    {ITEM_ALTARIANITE, 5, 0, 3},
    {ITEM_BANETTITE, 5, 0, 3},
    {ITEM_ABSOLITE, 5, 0, 3},
    {ITEM_GLALITITE, 5, 0, 3},
    {ITEM_SALAMENCITE, 5, 0, 3},
    {ITEM_METAGROSSITE, 5, 0, 3},
    {ITEM_LATIASITE, 5, 0, 4},
    {ITEM_LATIOSITE, 5, 0, 4},
    {ITEM_LOPUNNITE, 5, 0, 3},
    {ITEM_GARCHOMPITE, 5, 0, 3},
    {ITEM_LUCARIONITE, 5, 0, 3},
    {ITEM_ABOMASITE, 5, 0, 3},
    {ITEM_GALLADITE, 5, 0, 3},
    {ITEM_AUDINITE, 5, 0, 3},
    {ITEM_DIANCITE, 5, 0, 4},
    {ITEM_CLEFABLITE, 5, 0, 3},
    {ITEM_VICTREEBELITE, 5, 0, 3},
    {ITEM_STARMINITE, 5, 0, 3},
    {ITEM_DRAGONINITE, 5, 0, 3},
    {ITEM_MEGANIUMITE, 5, 0, 3},
    {ITEM_FERALIGITE, 5, 0, 3},
    {ITEM_SKARMORITE, 5, 0, 3},
    {ITEM_FROSLASSITE, 5, 0, 3},
    {ITEM_EMBOARITE, 5, 0, 3},
    {ITEM_EXCADRITE, 5, 0, 3},
    {ITEM_SCOLIPITE, 5, 0, 3},
    {ITEM_SCRAFTINITE, 5, 0, 3},
    {ITEM_EELEKTROSSITE, 5, 0, 3},
    {ITEM_CHANDELURITE, 5, 0, 3},
    {ITEM_CHESNAUGHTITE, 5, 0, 3},
    {ITEM_DELPHOXITE, 5, 0, 3},
    {ITEM_GRENINJITE, 5, 0, 3},
    {ITEM_PYROARITE, 5, 0, 3},
    {ITEM_FLOETTITE, 5, 0, 3},
    {ITEM_MALAMARITE, 5, 0, 3},
    {ITEM_BARBARACITE, 5, 0, 3},
    {ITEM_DRAGALGITE, 5, 0, 3},
    {ITEM_HAWLUCHANITE, 5, 0, 3},
    {ITEM_ZYGARDITE, 5, 0, 4},
    {ITEM_DRAMPANITE, 5, 0, 3},
    {ITEM_FALINKSITE, 5, 0, 3},
    {ITEM_HEATRANITE, 5, 0, 4},
    {ITEM_DARKRANITE, 5, 0, 4},
    {ITEM_ZERAORITE, 5, 0, 4},
    {ITEM_RAICHUNITE_X, 5, 0, 3},
    {ITEM_RAICHUNITE_Y, 5, 0, 3},
    {ITEM_CHIMECHITE, 5, 0, 3},
    {ITEM_ABSOLITE_Z, 5, 0, 3},
    {ITEM_STARAPTITE, 5, 0, 3},
    {ITEM_GARCHOMPITE_Z, 5, 0, 3},
    {ITEM_LUCARIONITE_Z, 5, 0, 3},
    {ITEM_GOLURKITE, 5, 0, 3},
    {ITEM_MEOWSTICITE, 5, 0, 3},
    {ITEM_CRABOMINITE, 5, 0, 3},
    {ITEM_GOLISOPITE, 5, 0, 3},
    {ITEM_MAGEARNITE, 5, 0, 4},
    {ITEM_SCOVILLAINITE, 5, 0, 3},
    {ITEM_BAXCALIBRITE, 5, 0, 3},
    {ITEM_TATSUGIRINITE, 5, 0, 3},
    {ITEM_GLIMMORANITE, 5, 0, 3},
    {ITEM_NORMALIUM_Z, 5, 0, 2},
    {ITEM_FIRIUM_Z, 5, 0, 2},
    {ITEM_WATERIUM_Z, 5, 0, 2},
    {ITEM_ELECTRIUM_Z, 5, 0, 2},
    {ITEM_GRASSIUM_Z, 5, 0, 2},
    {ITEM_ICIUM_Z, 5, 0, 2},
    {ITEM_FIGHTINIUM_Z, 5, 0, 2},
    {ITEM_POISONIUM_Z, 5, 0, 2},
    {ITEM_GROUNDIUM_Z, 5, 0, 2},
    {ITEM_FLYINIUM_Z, 5, 0, 2},
    {ITEM_PSYCHIUM_Z, 5, 0, 2},
    {ITEM_BUGINIUM_Z, 5, 0, 2},
    {ITEM_ROCKIUM_Z, 5, 0, 2},
    {ITEM_GHOSTIUM_Z, 5, 0, 2},
    {ITEM_DRAGONIUM_Z, 5, 0, 2},
    {ITEM_DARKINIUM_Z, 5, 0, 2},
    {ITEM_STEELIUM_Z, 5, 0, 2},
    {ITEM_FAIRIUM_Z, 5, 0, 2},
    {ITEM_PIKANIUM_Z, 5, 0, 3},
    {ITEM_EEVIUM_Z, 5, 0, 3},
    {ITEM_SNORLIUM_Z, 5, 0, 3},
    {ITEM_MEWNIUM_Z, 5, 0, 4},
    {ITEM_DECIDIUM_Z, 5, 0, 3},
    {ITEM_INCINIUM_Z, 5, 0, 3},
    {ITEM_PRIMARIUM_Z, 5, 0, 3},
    {ITEM_LYCANIUM_Z, 5, 0, 3},
    {ITEM_MIMIKIUM_Z, 5, 0, 3},
    {ITEM_KOMMONIUM_Z, 5, 0, 3},
    {ITEM_TAPUNIUM_Z, 5, 0, 3},
    {ITEM_SOLGANIUM_Z, 5, 0, 4},
    {ITEM_LUNALIUM_Z, 5, 0, 4},
    {ITEM_MARSHADIUM_Z, 5, 0, 4},
    {ITEM_ALORAICHIUM_Z, 5, 0, 3},
    {ITEM_PIKASHUNIUM_Z, 5, 0, 3},
    {ITEM_ULTRANECROZIUM_Z, 5, 0, 4},
    {ITEM_BUG_TERA_SHARD, 5, 0, 2},
    {ITEM_DARK_TERA_SHARD, 5, 0, 2},
    {ITEM_DRAGON_TERA_SHARD, 5, 0, 2},
    {ITEM_ELECTRIC_TERA_SHARD, 5, 0, 2},
    {ITEM_FAIRY_TERA_SHARD, 5, 0, 2},
    {ITEM_FIGHTING_TERA_SHARD, 5, 0, 2},
    {ITEM_FIRE_TERA_SHARD, 5, 0, 2},
    {ITEM_FLYING_TERA_SHARD, 5, 0, 2},
    {ITEM_GHOST_TERA_SHARD, 5, 0, 2},
    {ITEM_GRASS_TERA_SHARD, 5, 0, 2},
    {ITEM_GROUND_TERA_SHARD, 5, 0, 2},
    {ITEM_ICE_TERA_SHARD, 5, 0, 2},
    {ITEM_NORMAL_TERA_SHARD, 5, 0, 2},
    {ITEM_POISON_TERA_SHARD, 5, 0, 2},
    {ITEM_PSYCHIC_TERA_SHARD, 5, 0, 2},
    {ITEM_ROCK_TERA_SHARD, 5, 0, 2},
    {ITEM_STEEL_TERA_SHARD, 5, 0, 2},
    {ITEM_WATER_TERA_SHARD, 5, 0, 2},
    {ITEM_STELLAR_TERA_SHARD, 5, 0, 3},
};
static const struct HorizonGoodsEntry sHorizonDiscPool[] =
{
    {ITEM_TM01, 0, 0, 0}, {ITEM_TM05, 0, 0, 0},
    {ITEM_TM10, 0, 0, 0}, {ITEM_TM11, 0, 0, 0},
    {ITEM_TM17, 0, 0, 0}, {ITEM_TM21, 0, 0, 0},
    {ITEM_TM28, 0, 0, 0}, {ITEM_TM32, 0, 0, 0},
    {ITEM_TM40, 0, 0, 0}, {ITEM_TM45, 0, 0, 0},
    {ITEM_TM04, 1, 0, 0}, {ITEM_TM06, 1, 2, 0},
    {ITEM_TM13, 2, 0, 1}, {ITEM_TM24, 2, 1, 1},
    {ITEM_TM30, 3, 2, 1}, {ITEM_TM35, 3, 0, 1},
    {ITEM_TM36, 4, 0, 1}, {ITEM_TM42, 4, 0, 1},
    {ITEM_TM44, 5, 0, 2}, {ITEM_TM46, 5, 0, 2},
    {ITEM_TM26, 7, 0, 2}, {ITEM_TM38, 7, 0, 2},
    {ITEM_TM50, 8, 0, 2},
};

static const enum Item sHorizonTypeGems[] =
{
#define GEAR_BLOOD_VARIANT(type, name, moveTypeName, bagIcon) ITEM_##type##_GEM,
#include "constants/blood_gears.h"
#undef GEAR_BLOOD_VARIANT
};

// Explicitly reserved after the activity record in Save4; never overwrite an unknown record.
#define HORIZON_GOODS_MAGIC 0x48475632u
#define HORIZON_GOODS_VERSION 1
struct HorizonGoodsSave
{
    u32 magic;
    u16 version;
    u16 day;
    u32 seed;
    u8 period;
    u8 badges;
    u16 sold[2];
    struct HorizonGoodsOffer offers[2][HORIZON_GOODS_ITEM_COUNT];
};
STATIC_ASSERT(UNHOLY_SAVE4_HORIZON_OFFSET + sizeof(struct HorizonGoodsSave) <= UNHOLY_SAVE4_WALLET_OFFSET, HorizonGoodsFitSave4);
static EWRAM_DATA u16 sHorizonLists[2][HORIZON_GOODS_ITEM_COUNT + 1];

u8 HorizonGoodsItemCountForTier(u8 watchTier)
{
    return watchTier >= HFC_WATCH_TIER_HF ? 12 : watchTier >= HFC_WATCH_TIER_SUPER ? 8 : 5;
}

u8 HorizonGoodsDiscCountForTier(u8 watchTier)
{
    return watchTier >= HFC_WATCH_TIER_HF ? 12 : watchTier >= HFC_WATCH_TIER_SUPER ? 6 : 3;
}

static u32 HorizonGoodsNextRandom(u32 *seed)
{
    *seed ^= *seed << 13;
    *seed ^= *seed >> 17;
    *seed ^= *seed << 5;
    return *seed;
}

static u32 HorizonGoodsSavedSeed(void)
{
    return ((u32)VarGet(VAR_HORIZON_GOODS_SEED_A) << 16) | VarGet(VAR_HORIZON_GOODS_SEED_B);
}

void HorizonGoodsRollSeeds(void)
{
    VarSet(VAR_HORIZON_GOODS_SEED_A, (Random32() & 0xFFFF) | 1);
    VarSet(VAR_HORIZON_GOODS_SEED_B, (Random32() & 0xFFFF) | 1);
}

void HorizonGoodsEnsureSeeds(void)
{
    if (!VarGet(VAR_HORIZON_GOODS_SEED_A) || !VarGet(VAR_HORIZON_GOODS_SEED_B))
        HorizonGoodsRollSeeds();
}

u32 HorizonGoodsReferencePrice(enum Item item)
{
    u32 price = GetItemPrice(item);
    if (item == ITEM_MASTER_BALL)
        return 150000;
    if (item >= ITEM_NORMAL_GEM && item <= ITEM_FAIRY_GEM && price == 0)
        price = 2500;
    if (price == 0 && GetItemPocket(item) == POCKET_TM_HM)
        price = 4000;
    switch (item)
    {
    case ITEM_VENUSAURITE: price = 50000; break;
    case ITEM_CHARIZARDITE_X: price = 50000; break;
    case ITEM_CHARIZARDITE_Y: price = 50000; break;
    case ITEM_BLASTOISINITE: price = 50000; break;
    case ITEM_BEEDRILLITE: price = 50000; break;
    case ITEM_PIDGEOTITE: price = 50000; break;
    case ITEM_ALAKAZITE: price = 50000; break;
    case ITEM_SLOWBRONITE: price = 50000; break;
    case ITEM_GENGARITE: price = 50000; break;
    case ITEM_KANGASKHANITE: price = 50000; break;
    case ITEM_PINSIRITE: price = 50000; break;
    case ITEM_GYARADOSITE: price = 50000; break;
    case ITEM_AERODACTYLITE: price = 50000; break;
    case ITEM_MEWTWONITE_X: price = 120000; break;
    case ITEM_MEWTWONITE_Y: price = 120000; break;
    case ITEM_AMPHAROSITE: price = 50000; break;
    case ITEM_STEELIXITE: price = 50000; break;
    case ITEM_SCIZORITE: price = 50000; break;
    case ITEM_HERACRONITE: price = 50000; break;
    case ITEM_HOUNDOOMINITE: price = 50000; break;
    case ITEM_TYRANITARITE: price = 50000; break;
    case ITEM_SCEPTILITE: price = 50000; break;
    case ITEM_BLAZIKENITE: price = 50000; break;
    case ITEM_SWAMPERTITE: price = 50000; break;
    case ITEM_GARDEVOIRITE: price = 50000; break;
    case ITEM_SABLENITE: price = 50000; break;
    case ITEM_MAWILITE: price = 50000; break;
    case ITEM_AGGRONITE: price = 50000; break;
    case ITEM_MEDICHAMITE: price = 50000; break;
    case ITEM_MANECTITE: price = 50000; break;
    case ITEM_SHARPEDONITE: price = 50000; break;
    case ITEM_CAMERUPTITE: price = 50000; break;
    case ITEM_ALTARIANITE: price = 50000; break;
    case ITEM_BANETTITE: price = 50000; break;
    case ITEM_ABSOLITE: price = 50000; break;
    case ITEM_GLALITITE: price = 50000; break;
    case ITEM_SALAMENCITE: price = 50000; break;
    case ITEM_METAGROSSITE: price = 50000; break;
    case ITEM_LATIASITE: price = 120000; break;
    case ITEM_LATIOSITE: price = 120000; break;
    case ITEM_LOPUNNITE: price = 50000; break;
    case ITEM_GARCHOMPITE: price = 50000; break;
    case ITEM_LUCARIONITE: price = 50000; break;
    case ITEM_ABOMASITE: price = 50000; break;
    case ITEM_GALLADITE: price = 50000; break;
    case ITEM_AUDINITE: price = 50000; break;
    case ITEM_DIANCITE: price = 120000; break;
    case ITEM_CLEFABLITE: price = 50000; break;
    case ITEM_VICTREEBELITE: price = 50000; break;
    case ITEM_STARMINITE: price = 50000; break;
    case ITEM_DRAGONINITE: price = 50000; break;
    case ITEM_MEGANIUMITE: price = 50000; break;
    case ITEM_FERALIGITE: price = 50000; break;
    case ITEM_SKARMORITE: price = 50000; break;
    case ITEM_FROSLASSITE: price = 50000; break;
    case ITEM_EMBOARITE: price = 50000; break;
    case ITEM_EXCADRITE: price = 50000; break;
    case ITEM_SCOLIPITE: price = 50000; break;
    case ITEM_SCRAFTINITE: price = 50000; break;
    case ITEM_EELEKTROSSITE: price = 50000; break;
    case ITEM_CHANDELURITE: price = 50000; break;
    case ITEM_CHESNAUGHTITE: price = 50000; break;
    case ITEM_DELPHOXITE: price = 50000; break;
    case ITEM_GRENINJITE: price = 50000; break;
    case ITEM_PYROARITE: price = 50000; break;
    case ITEM_FLOETTITE: price = 50000; break;
    case ITEM_MALAMARITE: price = 50000; break;
    case ITEM_BARBARACITE: price = 50000; break;
    case ITEM_DRAGALGITE: price = 50000; break;
    case ITEM_HAWLUCHANITE: price = 50000; break;
    case ITEM_ZYGARDITE: price = 120000; break;
    case ITEM_DRAMPANITE: price = 50000; break;
    case ITEM_FALINKSITE: price = 50000; break;
    case ITEM_HEATRANITE: price = 120000; break;
    case ITEM_DARKRANITE: price = 120000; break;
    case ITEM_ZERAORITE: price = 120000; break;
    case ITEM_RAICHUNITE_X: price = 50000; break;
    case ITEM_RAICHUNITE_Y: price = 50000; break;
    case ITEM_CHIMECHITE: price = 50000; break;
    case ITEM_ABSOLITE_Z: price = 80000; break;
    case ITEM_STARAPTITE: price = 50000; break;
    case ITEM_GARCHOMPITE_Z: price = 80000; break;
    case ITEM_LUCARIONITE_Z: price = 80000; break;
    case ITEM_GOLURKITE: price = 50000; break;
    case ITEM_MEOWSTICITE: price = 50000; break;
    case ITEM_CRABOMINITE: price = 50000; break;
    case ITEM_GOLISOPITE: price = 50000; break;
    case ITEM_MAGEARNITE: price = 120000; break;
    case ITEM_SCOVILLAINITE: price = 50000; break;
    case ITEM_BAXCALIBRITE: price = 50000; break;
    case ITEM_TATSUGIRINITE: price = 50000; break;
    case ITEM_GLIMMORANITE: price = 50000; break;
    case ITEM_NORMALIUM_Z: price = 25000; break;
    case ITEM_FIRIUM_Z: price = 25000; break;
    case ITEM_WATERIUM_Z: price = 25000; break;
    case ITEM_ELECTRIUM_Z: price = 25000; break;
    case ITEM_GRASSIUM_Z: price = 25000; break;
    case ITEM_ICIUM_Z: price = 25000; break;
    case ITEM_FIGHTINIUM_Z: price = 25000; break;
    case ITEM_POISONIUM_Z: price = 25000; break;
    case ITEM_GROUNDIUM_Z: price = 25000; break;
    case ITEM_FLYINIUM_Z: price = 25000; break;
    case ITEM_PSYCHIUM_Z: price = 25000; break;
    case ITEM_BUGINIUM_Z: price = 25000; break;
    case ITEM_ROCKIUM_Z: price = 25000; break;
    case ITEM_GHOSTIUM_Z: price = 25000; break;
    case ITEM_DRAGONIUM_Z: price = 25000; break;
    case ITEM_DARKINIUM_Z: price = 25000; break;
    case ITEM_STEELIUM_Z: price = 25000; break;
    case ITEM_FAIRIUM_Z: price = 25000; break;
    case ITEM_PIKANIUM_Z: price = 60000; break;
    case ITEM_EEVIUM_Z: price = 60000; break;
    case ITEM_SNORLIUM_Z: price = 60000; break;
    case ITEM_MEWNIUM_Z: price = 100000; break;
    case ITEM_DECIDIUM_Z: price = 60000; break;
    case ITEM_INCINIUM_Z: price = 60000; break;
    case ITEM_PRIMARIUM_Z: price = 60000; break;
    case ITEM_LYCANIUM_Z: price = 60000; break;
    case ITEM_MIMIKIUM_Z: price = 60000; break;
    case ITEM_KOMMONIUM_Z: price = 60000; break;
    case ITEM_TAPUNIUM_Z: price = 60000; break;
    case ITEM_SOLGANIUM_Z: price = 100000; break;
    case ITEM_LUNALIUM_Z: price = 100000; break;
    case ITEM_MARSHADIUM_Z: price = 100000; break;
    case ITEM_ALORAICHIUM_Z: price = 60000; break;
    case ITEM_PIKASHUNIUM_Z: price = 60000; break;
    case ITEM_ULTRANECROZIUM_Z: price = 100000; break;
    case ITEM_LONELY_MINT: price = 15000; break;
    case ITEM_ADAMANT_MINT: price = 15000; break;
    case ITEM_NAUGHTY_MINT: price = 15000; break;
    case ITEM_BRAVE_MINT: price = 15000; break;
    case ITEM_BOLD_MINT: price = 15000; break;
    case ITEM_IMPISH_MINT: price = 15000; break;
    case ITEM_LAX_MINT: price = 15000; break;
    case ITEM_RELAXED_MINT: price = 15000; break;
    case ITEM_MODEST_MINT: price = 15000; break;
    case ITEM_MILD_MINT: price = 15000; break;
    case ITEM_RASH_MINT: price = 15000; break;
    case ITEM_QUIET_MINT: price = 15000; break;
    case ITEM_CALM_MINT: price = 15000; break;
    case ITEM_GENTLE_MINT: price = 15000; break;
    case ITEM_CAREFUL_MINT: price = 15000; break;
    case ITEM_SASSY_MINT: price = 15000; break;
    case ITEM_TIMID_MINT: price = 15000; break;
    case ITEM_HASTY_MINT: price = 15000; break;
    case ITEM_JOLLY_MINT: price = 15000; break;
    case ITEM_NAIVE_MINT: price = 15000; break;
    case ITEM_SERIOUS_MINT: price = 15000; break;
    case ITEM_EXP_CANDY_XS: price = 300; break;
    case ITEM_EXP_CANDY_S: price = 1000; break;
    case ITEM_EXP_CANDY_M: price = 3000; break;
    case ITEM_EXP_CANDY_L: price = 9000; break;
    case ITEM_EXP_CANDY_XL: price = 24000; break;
    case ITEM_ABILITY_CAPSULE: price = 60000; break;
    case ITEM_ABILITY_PATCH: price = 150000; break;
    case ITEM_BOTTLE_CAP: price = 25000; break;
    case ITEM_GOLD_BOTTLE_CAP: price = 100000; break;
    case ITEM_ESCAPE_ROPE: price = 1000; break;
    case ITEM_DYNAMAX_CANDY: price = 4000; break;
    case ITEM_MAX_MUSHROOMS: price = 15000; break;
    case ITEM_MAX_HONEY: price = 12000; break;
    case ITEM_WISHING_PIECE: price = 8000; break;
    case ITEM_ARMORITE_ORE: price = 3000; break;
    case ITEM_DYNITE_ORE: price = 5000; break;
    case ITEM_DNA_SPLICERS: price = 120000; break;
    case ITEM_N_SOLARIZER: price = 120000; break;
    case ITEM_N_LUNARIZER: price = 120000; break;
    case ITEM_REINS_OF_UNITY: price = 100000; break;
    case ITEM_PRISON_BOTTLE: price = 75000; break;
    case ITEM_ZYGARDE_CUBE: price = 75000; break;
    case ITEM_ROTOM_CATALOG: price = 20000; break;
    case ITEM_GRACIDEA: price = 20000; break;
    case ITEM_REVEAL_GLASS: price = 50000; break;
    case ITEM_RED_ORB: price = 120000; break;
    case ITEM_BLUE_ORB: price = 120000; break;
    case ITEM_GRISEOUS_CORE: price = 100000; break;
    case ITEM_SOUL_DEW: price = 80000; break;
    case ITEM_BOOSTER_ENERGY: price = 15000; break;
    case ITEM_BUG_TERA_SHARD: price = 2500; break;
    case ITEM_DARK_TERA_SHARD: price = 2500; break;
    case ITEM_DRAGON_TERA_SHARD: price = 2500; break;
    case ITEM_ELECTRIC_TERA_SHARD: price = 2500; break;
    case ITEM_FAIRY_TERA_SHARD: price = 2500; break;
    case ITEM_FIGHTING_TERA_SHARD: price = 2500; break;
    case ITEM_FIRE_TERA_SHARD: price = 2500; break;
    case ITEM_FLYING_TERA_SHARD: price = 2500; break;
    case ITEM_GHOST_TERA_SHARD: price = 2500; break;
    case ITEM_GRASS_TERA_SHARD: price = 2500; break;
    case ITEM_GROUND_TERA_SHARD: price = 2500; break;
    case ITEM_ICE_TERA_SHARD: price = 2500; break;
    case ITEM_NORMAL_TERA_SHARD: price = 2500; break;
    case ITEM_POISON_TERA_SHARD: price = 2500; break;
    case ITEM_PSYCHIC_TERA_SHARD: price = 2500; break;
    case ITEM_ROCK_TERA_SHARD: price = 2500; break;
    case ITEM_STEEL_TERA_SHARD: price = 2500; break;
    case ITEM_WATER_TERA_SHARD: price = 2500; break;
    case ITEM_STELLAR_TERA_SHARD: price = 8000; break;
    case ITEM_LEVEL_BALL: price = 4000; break;
    case ITEM_LURE_BALL: price = 4000; break;
    case ITEM_MOON_BALL: price = 4000; break;
    case ITEM_FRIEND_BALL: price = 4000; break;
    case ITEM_LOVE_BALL: price = 4000; break;
    case ITEM_FAST_BALL: price = 4000; break;
    case ITEM_HEAVY_BALL: price = 4000; break;
    case ITEM_DREAM_BALL: price = 4000; break;
    case ITEM_BEAST_BALL: price = 4000; break;
    case ITEM_FIRE_STONE: price = 5000; break;
    case ITEM_WATER_STONE: price = 5000; break;
    case ITEM_THUNDER_STONE: price = 5000; break;
    case ITEM_LEAF_STONE: price = 5000; break;
    case ITEM_ICE_STONE: price = 5000; break;
    case ITEM_SUN_STONE: price = 5000; break;
    case ITEM_MOON_STONE: price = 5000; break;
    case ITEM_SHINY_STONE: price = 5000; break;
    case ITEM_DUSK_STONE: price = 5000; break;
    case ITEM_DAWN_STONE: price = 5000; break;
    case ITEM_SWEET_APPLE: price = 12000; break;
    case ITEM_TART_APPLE: price = 12000; break;
    case ITEM_CRACKED_POT: price = 12000; break;
    case ITEM_CHIPPED_POT: price = 12000; break;
    case ITEM_GALARICA_CUFF: price = 12000; break;
    case ITEM_GALARICA_WREATH: price = 12000; break;
    case ITEM_DRAGON_SCALE: price = 12000; break;
    case ITEM_UPGRADE: price = 12000; break;
    case ITEM_PROTECTOR: price = 12000; break;
    case ITEM_ELECTIRIZER: price = 12000; break;
    case ITEM_MAGMARIZER: price = 12000; break;
    case ITEM_DUBIOUS_DISC: price = 12000; break;
    case ITEM_REAPER_CLOTH: price = 12000; break;
    case ITEM_PRISM_SCALE: price = 12000; break;
    case ITEM_WHIPPED_DREAM: price = 12000; break;
    case ITEM_SACHET: price = 12000; break;
    case ITEM_OVAL_STONE: price = 5000; break;
    case ITEM_EVERSTONE: price = 5000; break;
    case ITEM_LINKING_CORD: price = 12000; break;
    case ITEM_METAL_COAT: price = 12000; break;
    case ITEM_KINGS_ROCK: price = 12000; break;
    case ITEM_BLACK_AUGURITE: price = 12000; break;
    case ITEM_HEALTH_FEATHER: price = 500; break;
    case ITEM_MUSCLE_FEATHER: price = 500; break;
    case ITEM_RESIST_FEATHER: price = 500; break;
    case ITEM_GENIUS_FEATHER: price = 500; break;
    case ITEM_CLEVER_FEATHER: price = 500; break;
    case ITEM_SWIFT_FEATHER: price = 500; break;
    case ITEM_HP_UP: price = 5000; break;
    case ITEM_PROTEIN: price = 5000; break;
    case ITEM_IRON: price = 5000; break;
    case ITEM_CALCIUM: price = 5000; break;
    case ITEM_ZINC: price = 5000; break;
    case ITEM_CARBOS: price = 5000; break;
    default: break;
    }
    // Unpriced legacy items cannot become free purchases. Never undercut resale.
    return max(max(1, price), GetItemSellPrice(item) + 1);
}

static bool32 HorizonGoodsOwned(enum Item item)
{
    return GetItemImportance(item) && (CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1));
}

// Rates in basis points; common goods have far more promotions than mythical goods.
static const u16 sPromoOdds[] = {6500, 5000, 3500, 2000, 1000, 500};
static const u16 sChromaticOdds[] = {300, 150, 80, 30, 10, 5};
static const u16 sRarityCutoffs[] = {4500, 7300, 9000, 9800, 9980, 10000};

static u8 RollRarity(u32 *seed)
{
    u16 roll = HorizonGoodsNextRandom(seed) % 10000;
    for (u8 rank = 0; rank < 6; rank++)
        if (roll < sRarityCutoffs[rank])
            return rank;
    return 0;
}

static u8 RollQuantity(u32 *seed, u8 maxQty)
{
    // Geometrically decreasing chances; every quantity up to the cap is possible.
    u8 qty = 2;
    while (qty < maxQty && HorizonGoodsNextRandom(seed) % 100 < 48)
        qty++;
    return qty;
}

static void MakeOffer(struct HorizonGoodsOffer *offer, enum Item item, u8 rank, u8 tier, u32 *seed)
{
    u16 roll = HorizonGoodsNextRandom(seed) % 10000;
    u8 promoScale = tier >= HFC_WATCH_TIER_HF ? 100 : tier >= HFC_WATCH_TIER_SUPER ? 60 : 30;
    u8 maxQty = tier >= HFC_WATCH_TIER_HF ? 10 : tier >= HFC_WATCH_TIER_SUPER ? 6 : 3;
    u16 chromatic = tier >= HFC_WATCH_TIER_SUPER ? sChromaticOdds[rank] * promoScale / 100 : 0;
    u16 promo = sPromoOdds[rank] * promoScale / 100;
    u8 payPercent = 40 + HorizonGoodsNextRandom(seed) % 31;
    bool32 unique = GetItemImportance(item);
    bool32 stone = GetItemHoldEffect(item) == HOLD_EFFECT_MEGA_STONE
                 || (item >= ITEM_NORMALIUM_Z && item <= ITEM_ULTRANECROZIUM_Z);
    offer->itemId = item;
    offer->quantity = 1;
    offer->grade = HORIZON_DEAL_NORMAL;
    if (roll < chromatic)
    {
        u8 cap = rank <= 1 ? 99 : rank <= 3 ? 30 : 10;
        if (stone)
            cap = 5;
        offer->grade = HORIZON_DEAL_CHROMATIC;
        offer->quantity = unique ? 1 : cap <= 10 ? cap : 10 + HorizonGoodsNextRandom(seed) % (cap - 9);
        payPercent = 1 + HorizonGoodsNextRandom(seed) % 15;
    }
    else if (roll < chromatic + promo)
    {
        if (stone)
            maxQty = min(maxQty, 3);
        offer->grade = HORIZON_DEAL_PROMO;
        offer->quantity = unique ? 1 : RollQuantity(seed, maxQty);
        if (unique)
            payPercent = 10 + HorizonGoodsNextRandom(seed) % 30;
    }
    offer->price = max(1u, (HorizonGoodsReferencePrice(item) * payPercent) / 100);
}

static bool32 Eligible(const struct HorizonGoodsEntry *e, const struct HorizonGoodsOffer *offers,
                       u8 count, u8 badges, u8 period, u8 tier)
{
    if (e->minBadges > badges || (e->timeRestriction && e->timeRestriction != period + 1)
     || (e->rarity >= 4 && tier < HFC_WATCH_TIER_HF)
     || (e->rarity == 3 && tier < HFC_WATCH_TIER_SUPER)
     || HorizonGoodsOwned(e->itemId))
        return FALSE;
    for (u8 i = 0; i < count; i++)
        if (offers[i].itemId == e->itemId)
            return FALSE;
    return TRUE;
}

static void BuildOffers(struct HorizonGoodsOffer *offers, u8 count, const struct HorizonGoodsEntry *pool,
                        u16 poolSize, u8 badges, u8 period, u8 tier, u32 *seed)
{
    while (count < HORIZON_GOODS_ITEM_COUNT)
    {
        u8 rank = RollRarity(seed);
        const struct HorizonGoodsEntry *chosen = NULL;
        // If the rolled rarity is empty, step down; finally use any eligible entry.
        for (s8 attempt = rank; attempt >= -1 && chosen == NULL; attempt--)
        {
            u16 eligible = 0;
            for (u16 i = 0; i < poolSize; i++)
                if ((attempt < 0 || pool[i].rarity == attempt)
                 && Eligible(&pool[i], offers, count, badges, period, tier))
                    if (HorizonGoodsNextRandom(seed) % ++eligible == 0)
                        chosen = &pool[i];
        }
        if (chosen == NULL)
            break;
        MakeOffer(&offers[count++], chosen->itemId, chosen->rarity, tier, seed);
    }
}

static struct HorizonGoodsSave *GetStock(void)
{
    struct SaveBlock4 *block = UnholySave4_Get();
    if (block == NULL)
        return NULL;
    struct HorizonGoodsSave *save = (struct HorizonGoodsSave *)(block->payload + UNHOLY_SAVE4_HORIZON_OFFSET);
    if (save->magic != HORIZON_GOODS_MAGIC)
    {
        for (u32 i = 0; i < sizeof(*save); i++)
            if (((u8 *)save)[i] != 0)
                return NULL;
    }
    else if (save->version != HORIZON_GOODS_VERSION)
        return NULL;

    HorizonGoodsEnsureSeeds();
    u32 seed = HorizonGoodsSavedSeed();
    u16 day = VarGet(VAR_HORIZON_MARKET_DAY);
    u8 period = VarGet(VAR_HORIZON_MARKET_PERIOD);
    if (save->magic == HORIZON_GOODS_MAGIC && save->seed == seed
     && save->day == day && save->period == period)
        return save;

    memset(save, 0, sizeof(*save));
    save->magic = HORIZON_GOODS_MAGIC;
    save->version = HORIZON_GOODS_VERSION;
    save->seed = seed;
    save->day = day;
    save->period = period;
    save->badges = VarGet(VAR_HORIZON_MARKET_BADGES);
    u8 tier = HorizonMarketGetRotationTier();
    u32 itemSeed = seed ^ 0xB15A3A2Fu;
    u32 discSeed = seed ^ 0x7338C71Du;
    if (!itemSeed) itemSeed = 1;
    if (!discSeed) discSeed = 1;
    MakeOffer(&save->offers[0][0], ITEM_POKE_BALL, 0, tier, &itemSeed);
    MakeOffer(&save->offers[0][1], ITEM_POTION, 0, tier, &itemSeed);
    MakeOffer(&save->offers[0][2], ITEM_ESCAPE_ROPE, 0, tier, &itemSeed);
    MakeOffer(&save->offers[0][3], ITEM_LINKING_CORD, 1, tier, &itemSeed);
    MakeOffer(&save->offers[0][4], sHorizonTypeGems[HorizonGoodsNextRandom(&itemSeed) % ARRAY_COUNT(sHorizonTypeGems)], 1, tier, &itemSeed);
    BuildOffers(save->offers[0], 5, sHorizonItemPool, ARRAY_COUNT(sHorizonItemPool), save->badges, period, tier, &itemSeed);
    BuildOffers(save->offers[1], 0, sHorizonDiscPool, ARRAY_COUNT(sHorizonDiscPool), save->badges, period, tier, &discSeed);
    return UnholySave4_Commit() ? save : NULL;
}

static u8 VisibleCount(bool8 discs)
{
    return discs ? HorizonGoodsDiscCountForTier(HfcWatch_GetTier()) : HorizonGoodsItemCountForTier(HfcWatch_GetTier());
}

static const u16 *GetList(bool8 discs)
{
    struct HorizonGoodsSave *save = GetStock();
    u16 *list = sHorizonLists[discs != FALSE];
    memset(list, 0, sizeof(sHorizonLists[0]));
    if (save != NULL)
        for (u8 i = 0; i < VisibleCount(discs); i++)
        {
            list[i] = save->offers[discs != FALSE][i].itemId;
            if (list[i] == ITEM_NONE)
                break;
        }
    return list;
}

const u16 *HorizonGoodsGetItems(void) { return GetList(FALSE); }
const u16 *HorizonGoodsGetDiscs(void) { return GetList(TRUE); }

const struct HorizonGoodsOffer *HorizonGoodsGetOffer(bool8 discs, u8 slot)
{
    struct HorizonGoodsSave *save = GetStock();
    if (save == NULL || slot >= VisibleCount(discs) || !save->offers[discs != FALSE][slot].itemId)
        return NULL;
    return &save->offers[discs != FALSE][slot];
}

bool32 HorizonGoodsOfferSold(bool8 discs, u8 slot)
{
    struct HorizonGoodsSave *save = GetStock();
    if (save == NULL || slot >= VisibleCount(discs))
        return TRUE;
    const struct HorizonGoodsOffer *offer = &save->offers[discs != FALSE][slot];
    return !offer->itemId || (save->sold[discs != FALSE] & (1 << slot))
        || HorizonGoodsOwned(offer->itemId);
}

enum HorizonMarketPurchaseResult HorizonGoodsPurchase(bool8 discs, u8 slot, enum Item expected, u32 price)
{
    struct HorizonGoodsSave *save = GetStock();
    const struct HorizonGoodsOffer *offer = HorizonGoodsGetOffer(discs, slot);
    if (save == NULL || offer == NULL || HorizonGoodsOfferSold(discs, slot)
     || offer->itemId != expected || offer->price != price || offer->quantity == 0 || offer->quantity > 99)
        return HORIZON_MARKET_PURCHASE_SOLD_OUT;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, offer->price))
        return HORIZON_MARKET_PURCHASE_NO_MONEY;
    if (!CheckBagHasSpace(offer->itemId, offer->quantity) || !AddBagItem(offer->itemId, offer->quantity))
        return HORIZON_MARKET_PURCHASE_NO_SPACE;
    RemoveMoney(&gSaveBlock1Ptr->money, offer->price);
    save->sold[discs != FALSE] |= 1 << slot;
    UnholySave4_Commit();
    return HORIZON_MARKET_PURCHASE_OK;
}
