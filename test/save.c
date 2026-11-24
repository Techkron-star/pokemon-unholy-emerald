#include "global.h"
#include "agb_flash.h"
#include "gba/flash_internal.h"
#include "load_save.h"
#include "pokemon_storage_system.h"
#include "unholy_save.h"
#include "constants/gears.h"
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

static void CreateSaveFixtureMon(struct Pokemon *mon, enum Species species, u8 gear, u32 personality)
{
    CreateMonWithIVs(mon, species, 10, personality, OTID_STRUCT_PRESET(0xAABBCCDD), MAX_PER_STAT_IVS);
    SetMonData(mon, MON_DATA_GEAR, &gear);
    Pokemon_SetBadge(mon, POKEMON_BADGE_OMEGA);
}

TEST("Unholy save loads the newest coherent slot or recovers Pokémon and Gears from its predecessor")
{
    // The headless test runner provides temporary Flash storage.
    ASSUME(gTestRunnerHeadless && gFlashMemoryPresent);

    bool32 corrupt = FALSE;
    bool32 wrap = FALSE;
    PARAMETRIZE { }
    PARAMETRIZE { corrupt = TRUE; }
    PARAMETRIZE { corrupt = TRUE; wrap = TRUE; }
    PARAMETRIZE { wrap = TRUE; }

    void (*testTimer)(void) = gIntrTable[7];
    Save_ResetSaveCounters();
    InitFlashTimer();
    if (wrap)
        gSaveCounter = 0xFFFFFFFEu;

    u16 damageResult = 0;
    u8 olderGear = 255;
    u8 newerGear = GEAR_VISOR_COSMICO;
    u8 expectedGear = corrupt ? olderGear : newerGear;
    struct Pokemon stored;
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    CreateSaveFixtureMon(&party[0], SPECIES_TREECKO, olderGear, 7);
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    CreateSaveFixtureMon(&stored, SPECIES_TORCHIC, olderGear, 8);
    SetBoxMonAt(0, 0, &stored.box);
    CreateSaveFixtureMon(&stored, SPECIES_MUDKIP, olderGear, 9);
    gSaveBlock1Ptr->daycare.mons[0].mon = stored.box;
    CreateSaveFixtureMon(&stored, SPECIES_ZEKROM, olderGear, 10);
    gPokemonStoragePtr->fusions[0] = stored;
    gSaveBlock2Ptr->playerTrainerId[0] = 10;
    UnholySave4_Reset();
    UnholySave5_Reset();
    gSaveBlock3Ptr->unholySave4.payload[0] = 10;
    UnholySave4_Commit();
    gPokemonStoragePtr->unholySave5.payload[0] = 10;
    UnholySave5_Commit();

    u8 firstSave = TrySavingData(SAVE_NORMAL);
    u32 olderCounter = gSaveCounter;
    SetMonData(&party[0], MON_DATA_GEAR, &newerGear);
    SetBoxMonDataAt(0, 0, MON_DATA_GEAR, &newerGear);
    SetBoxMonData(&gSaveBlock1Ptr->daycare.mons[0].mon, MON_DATA_GEAR, &newerGear);
    SetMonData(&gPokemonStoragePtr->fusions[0], MON_DATA_GEAR, &newerGear);
    gSaveBlock2Ptr->playerTrainerId[0] = 20;
    gSaveBlock3Ptr->unholySave4.payload[0] = 20;
    UnholySave4_Commit();
    gPokemonStoragePtr->unholySave5.payload[0] = 20;
    UnholySave5_Commit();

    u8 secondSave = TrySavingData(SAVE_NORMAL);
    u32 expectedCounter = corrupt ? olderCounter : gSaveCounter;
    if (corrupt && secondSave == SAVE_STATUS_OK)
    {
        u8 damagedSector = gLastWrittenSector + NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
        ReadFlash(damagedSector, 0, (u8 *)&gSaveDataBuffer, SECTOR_SIZE);
        if (wrap)
            gSaveDataBuffer.id = 0xFFFF;
        else
            gSaveDataBuffer.counter -= NUM_SAVE_SLOTS;
        damageResult = ProgramFlashSectorAndVerify(damagedSector, gSaveDataBuffer.data);
    }

    Save_ResetSaveCounters();
    ClearSav1();
    ClearSav2();
    ClearSav3();
    ZeroPlayerPartyMons();
    ZeroBoxMonAt(0, 0);
    ZeroMonData(&gPokemonStoragePtr->fusions[0]);
    memset(&gPokemonStoragePtr->unholySave5, 0, sizeof(gPokemonStoragePtr->unholySave5));

    u16 trainerOffset = GetSaveBlocksPointersBaseOffset();
    u8 loaded = LoadGameSave(SAVE_NORMAL);
    u32 loadedCounter = gSaveCounter;
    u16 erase = EraseFlashChip();
    gIntrTable[7] = testTimer;
    Save_ResetSaveCounters();

    EXPECT_EQ(firstSave, SAVE_STATUS_OK);
    EXPECT_EQ(secondSave, SAVE_STATUS_OK);
    EXPECT_EQ(damageResult, 0);
    EXPECT_EQ(erase, 0);
    EXPECT_EQ(loaded, corrupt ? SAVE_STATUS_ERROR : SAVE_STATUS_OK);
    EXPECT_EQ(loadedCounter, expectedCounter);
    EXPECT_EQ(trainerOffset, corrupt ? 10 : 20);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 1);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_PERSONALITY), 7);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_GEAR), expectedGear);
    EXPECT(Pokemon_HasBadge(&party[0], POKEMON_BADGE_OMEGA));
    EXPECT_EQ(GetBoxMonDataAt(0, 0, MON_DATA_GEAR), expectedGear);
    EXPECT(BoxPokemon_HasBadge(GetBoxedMonPtr(0, 0), POKEMON_BADGE_OMEGA));
    EXPECT_EQ(GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[0].mon, MON_DATA_GEAR), expectedGear);
    EXPECT(BoxPokemon_HasBadge(&gSaveBlock1Ptr->daycare.mons[0].mon, POKEMON_BADGE_OMEGA));
    EXPECT_EQ(GetMonData(&gPokemonStoragePtr->fusions[0], MON_DATA_GEAR), expectedGear);
    EXPECT(Pokemon_HasBadge(&gPokemonStoragePtr->fusions[0], POKEMON_BADGE_OMEGA));
    EXPECT(UnholySave4_Validate(&gSaveBlock3Ptr->unholySave4));
    EXPECT(UnholySave5_Validate(&gPokemonStoragePtr->unholySave5));
    EXPECT_EQ(gSaveBlock3Ptr->unholySave4.payload[0], corrupt ? 10 : 20);
    EXPECT_EQ(gPokemonStoragePtr->unholySave5.payload[0], corrupt ? 10 : 20);
    EXPECT_EQ(GetSaveBlocksPointersBaseOffset(), 0);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_EMPTY);

    ZeroPlayerPartyMons();
    ZeroBoxMonAt(0, 0);
    ZeroMonData(&gPokemonStoragePtr->fusions[0]);
    memset(&gPokemonStoragePtr->unholySave5, 0, sizeof(gPokemonStoragePtr->unholySave5));
}
