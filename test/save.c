#include "global.h"
#include "pokemon_storage_system.h"
#include "unholy_save.h"
#include "test/test.h"

// If you would like to ensure save compatibility, update the values below with those for your hack. You can find these through the debug menu.
// Please note that this simple check is not 100% foolproof, but should be able to catch most unintended shifts.
#define T_SAVEBLOCK1_SIZE 15784 // Intentional 216-byte expanded bag layout
#define T_SAVEBLOCK2_SIZE 3884
#define T_SAVEBLOCK3_SIZE 1624 // 4 bytes original + 1620 bytes logical SaveBlock4
#define T_POKEMONSTORAGE_SIZE 35712 // 34144 original + 1568 bytes logical SaveBlock5

TEST("SaveBlock1 fits the existing four flash sectors")
{
    EXPECT_EQ(sizeof(struct SaveBlock1), T_SAVEBLOCK1_SIZE);
}

TEST("SaveBlock2 is backwards compatible")
{
    EXPECT_EQ(sizeof(struct SaveBlock2), T_SAVEBLOCK2_SIZE);
}

TEST("SaveBlock3 and SaveBlock4 fit the existing fourteen sidecars")
{
    EXPECT_EQ(sizeof(struct SaveBlock3), T_SAVEBLOCK3_SIZE);
}

TEST("PokemonStorage and SaveBlock5 fit the existing nine sectors")
{
    EXPECT_EQ(sizeof(struct PokemonStorage), T_POKEMONSTORAGE_SIZE);
}

#undef T_SAVEBLOCK1_SIZE
#undef T_SAVEBLOCK2_SIZE
#undef T_SAVEBLOCK3_SIZE
#undef T_POKEMONSTORAGE_SIZE

TEST("Logical SaveBlock4 and SaveBlock5 have exact versioned envelopes")
{
    EXPECT_EQ(sizeof(struct SaveBlock4), 1620);
    EXPECT_EQ(sizeof(struct SaveBlock5), 1568);
    EXPECT_EQ(offsetof(struct SaveBlock3, unholySave4), 4);
    EXPECT_EQ(offsetof(struct PokemonStorage, unholySave5), 34144);
}
