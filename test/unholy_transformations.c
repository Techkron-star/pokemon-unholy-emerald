#include "global.h"
#include "item.h"
#include "item_use.h"
#include "pokemon.h"
#include "battle_terastal.h"
#include "test/test.h"

TEST("Transformation devices are unique key items and soup uses the party menu")
{
    const enum Item devices[] = {ITEM_MEGA_RING, ITEM_Z_POWER_RING, ITEM_DYNAMAX_BAND, ITEM_TERA_ORB};
    for (u8 i = 0; i < ARRAY_COUNT(devices); i++)
    {
        EXPECT(GetItemImportance(devices[i]));
        EXPECT_EQ(GetItemPocket(devices[i]), POCKET_KEY_ITEMS);
    }
    EXPECT(!GetItemImportance(ITEM_MAX_SOUP));
    EXPECT_EQ(GetItemType(ITEM_MAX_SOUP), ITEM_USE_PARTY_MENU);
    EXPECT(GetItemFieldFunc(ITEM_MAX_SOUP) == ItemUseOutOfBattle_MaxSoup);
}

TEST("All nineteen tera shards have the new party-menu use")
{
    for (enum Item item = ITEM_BUG_TERA_SHARD; item <= ITEM_WATER_TERA_SHARD; item++)
    {
        EXPECT_EQ(GetItemType(item), ITEM_USE_PARTY_MENU);
        EXPECT(GetItemFieldFunc(item) == ItemUseOutOfBattle_TeraShard);
    }
    EXPECT_EQ(GetItemType(ITEM_STELLAR_TERA_SHARD), ITEM_USE_PARTY_MENU);
    EXPECT(GetItemFieldFunc(ITEM_STELLAR_TERA_SHARD) == ItemUseOutOfBattle_TeraShard);
}

TEST("Unholy Tera Orb is always charged under the current configuration")
{
    EXPECT(IsTeraOrbCharged());
}
