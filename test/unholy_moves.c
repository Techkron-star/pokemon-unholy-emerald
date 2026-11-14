#include "global.h"
#include "unholy_moves.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "random.h"
#include "test/test.h"
#include "constants/items.h"

struct MoveBackup
{
    struct SaveBlock1 save;
    struct Pokemon party[PARTY_SIZE];
    u8 count;
    bool8 battle;
};

static struct MoveBackup *Begin(enum Species species, u8 level)
{
    struct MoveBackup *old = Alloc(sizeof(*old));
    old->save = *gSaveBlock1Ptr;
    memcpy(old->party, gParties[B_TRAINER_PLAYER], sizeof(old->party));
    old->count = gPartiesCount[B_TRAINER_PLAYER];
    old->battle = gMain.inBattle;
    gMain.inBattle = FALSE;
    ClearBag();
    ZeroPartyMons(gParties[B_TRAINER_PLAYER]);
    CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][0], species, level, 123, OTID_STRUCT_PRESET(456), 12);
    for (u8 i = 0; i < MAX_MON_MOVES; i++) SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_NONE, i);
    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_POUND, 0);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    return old;
}

static void End(struct MoveBackup *old)
{
    *gSaveBlock1Ptr = old->save;
    memcpy(gParties[B_TRAINER_PLAYER], old->party, sizeof(old->party));
    gPartiesCount[B_TRAINER_PLAYER] = old->count;
    gMain.inBattle = old->battle;
    Free(old);
}

TEST("Move QoL catalogs egg, pre-evolution and compatible teachable routes without RNG")
{
    struct MoveBackup *old = Begin(SPECIES_SCEPTILE, 36);
    struct UnholyMoveCatalog *catalog = Alloc(sizeof(*catalog));
    struct Pokemon before = gParties[B_TRAINER_PLAYER][0];
    rng_value_t rng = gRngValue;
    UnholyMoves_Build(&gParties[B_TRAINER_PLAYER][0], catalog);
    EXPECT(catalog->sources[MOVE_LEECH_SEED] & UNHOLY_MOVE_EGG);
    EXPECT(catalog->sources[MOVE_POUND] & UNHOLY_MOVE_PRE_EVO);
    EXPECT(catalog->sources[MOVE_SOLAR_BEAM] & UNHOLY_MOVE_MACHINE);
    EXPECT(catalog->sources[MOVE_BODY_SLAM] & UNHOLY_MOVE_TUTOR);
    EXPECT_EQ(catalog->sources[MOVE_SPACIAL_REND], 0);
    u16 last = MOVE_NONE, count = UnholyMoves_Count(catalog, UNHOLY_MOVE_ALL);
    for (u16 i = 0; i < count; i++)
    {
        enum Move move = UnholyMoves_At(catalog, UNHOLY_MOVE_ALL, i);
        EXPECT_GT(move, last); // Every move appears exactly once in ALL.
        last = move;
    }
    EXPECT_EQ(UnholyMoves_At(catalog, UNHOLY_MOVE_ALL, count), MOVE_NONE);
    EXPECT_EQ(memcmp(&before, &gParties[B_TRAINER_PLAYER][0], sizeof(before)), 0);
    EXPECT_EQ(memcmp(&rng, &gRngValue, sizeof(rng)), 0);
    Free(catalog);
    End(old);
}

TEST("Move QoL teaches a compatible egg move without breeding and preserves Pokemon identity")
{
    struct MoveBackup *old = Begin(SPECIES_TREECKO, 5);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u16 badges = POKEMON_BADGE_ALPHA | POKEMON_BADGE_OMEGA;
    SetMonData(mon, MON_DATA_SPECIAL_BADGES, &badges);
    Gear_SetEquipped(mon, GEAR_PERMAFROST_FRAGMENT);
    struct UnholyMovePlan plan;
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_LEECH_SEED, 1, &plan), UNHOLY_MOVE_READY);
    EXPECT_EQ(plan.item, ITEM_NONE);
    struct Pokemon before = *mon;
    EXPECT_EQ(UnholyMoves_Apply(&plan), UNHOLY_MOVE_READY);
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE2), MOVE_LEECH_SEED);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PERSONALITY), GetMonData(&before, MON_DATA_PERSONALITY));
    EXPECT_EQ(GetMonData(mon, MON_DATA_EXP), GetMonData(&before, MON_DATA_EXP));
    EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIAL_BADGES), badges);
    EXPECT_EQ(Gear_GetEquipped(mon), GEAR_PERMAFROST_FRAGMENT);
    EXPECT_EQ(UnholyMoves_Apply(&plan), UNHOLY_MOVE_CHANGED);
    End(old);
}

TEST("Move QoL requires the correct disc and consumes a non-reusable TM only after confirmation")
{
    struct MoveBackup *old = Begin(SPECIES_TREECKO, 5);
    struct UnholyMovePlan plan;
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_SOLAR_BEAM, 1, &plan), UNHOLY_MOVE_NEEDS_DISC);
    EXPECT(AddBagItem(ITEM_TM22, 2));
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_SOLAR_BEAM, 1, &plan), UNHOLY_MOVE_READY);
    EXPECT_EQ(plan.item, ITEM_TM22);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM22), 2);
    EXPECT_EQ(UnholyMoves_Apply(&plan), UNHOLY_MOVE_READY);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM22), I_REUSABLE_TMS ? 2 : 1);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE2), MOVE_SOLAR_BEAM);
    End(old);
}

TEST("Move QoL rejects stale Pokemon, missing discs, incompatible moves and egg edits")
{
    struct MoveBackup *old = Begin(SPECIES_TREECKO, 5);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    struct UnholyMovePlan plan;
    EXPECT(AddBagItem(ITEM_TM22, 1));
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_SOLAR_BEAM, 1, &plan), UNHOLY_MOVE_READY);
    u8 pp = 1;
    SetMonData(mon, MON_DATA_PP1, &pp);
    EXPECT_EQ(UnholyMoves_Apply(&plan), UNHOLY_MOVE_CHANGED);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM22), 1);
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_SOLAR_BEAM, 1, &plan), UNHOLY_MOVE_READY);
    EXPECT(RemoveBagItem(ITEM_TM22, 1));
    EXPECT_EQ(UnholyMoves_Apply(&plan), UNHOLY_MOVE_NEEDS_DISC);
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE2), MOVE_NONE);
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_SPACIAL_REND, 1, &plan), UNHOLY_MOVE_INVALID);
    EXPECT_EQ(UnholyMoves_Prepare(PARTY_SIZE, MOVE_LEECH_SEED, 1, &plan), UNHOLY_MOVE_INVALID);
    u8 egg = TRUE;
    SetMonData(mon, MON_DATA_IS_EGG, &egg);
    EXPECT_EQ(UnholyMoves_Prepare(0, MOVE_LEECH_SEED, 1, &plan), UNHOLY_MOVE_INVALID);
    struct UnholyMoveCatalog *catalog = Alloc(sizeof(*catalog));
    UnholyMoves_Build(mon, catalog);
    EXPECT_EQ(UnholyMoves_Count(catalog, UNHOLY_MOVE_ALL), 0);
    Free(catalog);
    End(old);
}

TEST("Move QoL swaps and deletes slots together with their PP and PP Up bonuses")
{
    struct MoveBackup *old = Begin(SPECIES_TREECKO, 20);
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    SetMonMoveSlot(mon, MOVE_LEER, 1);
    SetMonMoveSlot(mon, MOVE_QUICK_ATTACK, 2);
    u8 first = 7, second = 13, bonuses = 1 | (3 << 2) | (2 << 4);
    SetMonData(mon, MON_DATA_PP1, &first);
    SetMonData(mon, MON_DATA_PP2, &second);
    SetMonData(mon, MON_DATA_PP_BONUSES, &bonuses);
    EXPECT(UnholyMoves_Swap(0, 0, 1));
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE1), MOVE_LEER);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PP1), second);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PP2), first);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PP_BONUSES), 3 | (1 << 2) | (2 << 4));
    EXPECT(!UnholyMoves_Forget(0, 1, MOVE_LEER)); // The selected slot changed.
    EXPECT(UnholyMoves_Forget(0, 1, MOVE_POUND));
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE2), MOVE_QUICK_ATTACK);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PP_BONUSES), 3 | (2 << 2));
    EXPECT(UnholyMoves_Forget(0, 1, MOVE_QUICK_ATTACK));
    EXPECT(!UnholyMoves_Forget(0, 0, MOVE_LEER));
    End(old);
}

TEST("Move QoL keeps future level moves locked when there is no other compatible route")
{
    struct MoveBackup *old = Begin(SPECIES_TREECKO, 5);
    struct UnholyMoveCatalog *catalog = Alloc(sizeof(*catalog));
    UnholyMoves_Build(&gParties[B_TRAINER_PLAYER][0], catalog);
    u16 item;
    bool8 found = FALSE;
    for (u16 move = 1; move < MOVES_COUNT; move++)
        if (catalog->sources[move] == UNHOLY_MOVE_LEVEL && catalog->level[move] > 5)
        {
            EXPECT_EQ(UnholyMoves_Status(&gParties[B_TRAINER_PLAYER][0], catalog, move, &item), UNHOLY_MOVE_NEEDS_LEVEL);
            found = TRUE;
        }
    EXPECT(found);
    Free(catalog);
    End(old);
}

TEST("Move QoL safely supports catalogs larger than the legacy 60-move buffer")
{
    struct MoveBackup *old = Begin(SPECIES_MEW, 100);
    struct { struct UnholyMoveCatalog catalog; u32 guard; } *guarded = Alloc(sizeof(*guarded));
    guarded->guard = 0x1234ABCD;
    UnholyMoves_Build(&gParties[B_TRAINER_PLAYER][0], &guarded->catalog);
    u16 count = UnholyMoves_Count(&guarded->catalog, UNHOLY_MOVE_ALL);
    EXPECT_GT(count, 60);
    EXPECT_EQ(guarded->guard, 0x1234ABCD);
    EXPECT_EQ(UnholyMoves_At(&guarded->catalog, UNHOLY_MOVE_ALL, count), MOVE_NONE);
    Free(guarded);
    End(old);
}
