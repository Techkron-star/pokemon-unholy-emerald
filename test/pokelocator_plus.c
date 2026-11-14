#include "global.h"
#include "pokelocator.h"
#include "caps.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/vars.h"

TEST("Pokelocator badge caps advance at all nine milestones and stop EXP at the cap")
{
    const u16 flags[] = {FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
        FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET, FLAG_IS_CHAMPION};
    const u8 caps[] = {15, 19, 24, 29, 31, 33, 42, 46, 58, 100};
    bool8 old[ARRAY_COUNT(flags)];
    for (u8 i = 0; i < ARRAY_COUNT(flags); i++) { old[i] = FlagGet(flags[i]); FlagClear(flags[i]); }
    for (u8 i = 0; i <= ARRAY_COUNT(flags); i++)
    {
        EXPECT_EQ(GetCurrentLevelCap(), caps[i]);
        EXPECT_EQ(GetSoftLevelCapExpValue(caps[i], 1000), 0);
        EXPECT_EQ(GetSoftLevelCapExpValue(caps[i] - 1, 1000), 1000);
        if (i < ARRAY_COUNT(flags)) FlagSet(flags[i]);
    }
    for (u8 i = 0; i < ARRAY_COUNT(flags); i++) if (!old[i]) FlagClear(flags[i]);
}

TEST("Pokelocator level candies respect the cap without lowering an existing overlevel Pokemon")
{
    struct Pokemon mon;
    u8 level = min(MAX_LEVEL - 1, GetCurrentLevelCap());
    CreateMonWithIVs(&mon, SPECIES_TREECKO, level, 123, OTID_STRUCT_PRESET(456), 0);
    u32 exp = GetMonData(&mon, MON_DATA_EXP);
    EXPECT(PokemonUseItemEffects(&mon, ITEM_RARE_CANDY, 0, 0, FALSE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_EXP), exp);
    level++;
    CreateMonWithIVs(&mon, SPECIES_TREECKO, level, 123, OTID_STRUCT_PRESET(456), 0);
    exp = GetMonData(&mon, MON_DATA_EXP);
    EXPECT(PokemonUseItemEffects(&mon, ITEM_EXP_CANDY_XL, 0, 0, FALSE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_EXP), exp);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_LEVEL), level);
}

TEST("Pokelocator R target survives event vars and rejects legacy or invalid packed data")
{
    u16 old = VarGet(VAR_POKELOCATOR_TARGET), species;
    u8 method;
    VarSet(VAR_POKELOCATOR_TARGET, 0);
    EXPECT(!Pokelocator_ReadTarget(&species, &method));
    EXPECT(Pokelocator_RegisterTarget(SPECIES_RALTS, POKELOCATOR_METHOD_LAND));
    EXPECT(Pokelocator_ReadTarget(&species, &method));
    EXPECT_EQ(species, SPECIES_RALTS);
    EXPECT_EQ(method, POKELOCATOR_METHOD_LAND);
    EXPECT(!Pokelocator_RegisterTarget(SPECIES_NONE, POKELOCATOR_METHOD_LAND));
    EXPECT(!Pokelocator_RegisterTarget(SPECIES_RALTS, POKELOCATOR_METHOD_COUNT));
    VarSet(VAR_POKELOCATOR_TARGET, 0xB800 | SPECIES_RALTS); // Invalid method 7.
    EXPECT(!Pokelocator_ReadTarget(&species, &method));
    VarSet(VAR_POKELOCATOR_TARGET, old);
}

TEST("Pokelocator hidden and rock groups remain separate and honey has no invented encounters")
{
    EXPECT(Pokelocator_MethodInGroup(POKELOCATOR_METHOD_ROCKS, POKELOCATOR_GROUP_ROCKS));
    EXPECT(!Pokelocator_MethodInGroup(POKELOCATOR_METHOD_HIDDEN, POKELOCATOR_GROUP_ROCKS));
    EXPECT(Pokelocator_MethodInGroup(POKELOCATOR_METHOD_HIDDEN, POKELOCATOR_GROUP_HIDDEN));
    for (u8 i = 0; i < POKELOCATOR_METHOD_COUNT; i++)
        EXPECT(!Pokelocator_MethodInGroup(i, POKELOCATOR_GROUP_HONEY));
    EXPECT(!Pokelocator_IsMethodUnlocked(HEADER_NONE, POKELOCATOR_METHOD_HIDDEN));
}

TEST("Pokelocator gifts confirm the category and cap, deliver to party then PC and preflight full batches")
{
    struct SaveBlock1 *save1 = Alloc(sizeof(*save1));
    struct SaveBlock2 *save2 = Alloc(sizeof(*save2));
    struct PokemonStorage *storage = Alloc(sizeof(*storage));
    struct Pokemon party[PARTY_SIZE];
    EXPECT(save1 != NULL && save2 != NULL && storage != NULL);
    memcpy(save1, gSaveBlock1Ptr, sizeof(*save1));
    memcpy(save2, gSaveBlock2Ptr, sizeof(*save2));
    memcpy(storage, gPokemonStoragePtr, sizeof(*storage));
    memcpy(party, gParties[B_TRAINER_PLAYER], sizeof(party));
    bool8 battle = gMain.inBattle;
    u8 oldCount = gPartiesCount[B_TRAINER_PLAYER];
    gMain.inBattle = FALSE;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE102);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE102);
    FlagClear(FLAG_IS_CHAMPION);
    ResetPokemonStorageSystem();
    memset(gParties[B_TRAINER_PLAYER], 0, sizeof(party));
    gPartiesCount[B_TRAINER_PLAYER] = 0;
    struct PokelocatorObtainPlan plan;
    u16 header = GetCurrentMapWildMonHeaderId();
    rng_value_t rng = gRngValue;
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ONE, SPECIES_POOCHYENA), POKELOCATOR_OBTAIN_READY);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0); // Review / cancel has no RNG or gift side effects.
    EXPECT_EQ(plan.level, GetCurrentLevelCap());
    EXPECT_EQ(plan.count, 1);
    struct PokelocatorObtainPlan changed = plan;
    changed.level++;
    EXPECT_EQ(Pokelocator_ExecuteObtain(&changed).status, POKELOCATOR_OBTAIN_CHANGED);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    struct PokelocatorObtainResult result = Pokelocator_ExecuteObtain(&plan);
    EXPECT_EQ(result.status, POKELOCATOR_OBTAIN_READY);
    EXPECT_EQ(result.party, 1);
    EXPECT_EQ(result.pc, 0);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), plan.level);
    EXPECT_EQ(Pokelocator_GetDexState(SPECIES_POOCHYENA), POKELOCATOR_DEX_CAUGHT);
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ALL, SPECIES_NONE), POKELOCATOR_OBTAIN_READY);
    u8 all = plan.count;
    EXPECT_GT(all, 1);
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_NEW, SPECIES_NONE), POKELOCATOR_OBTAIN_READY);
    EXPECT_LT(plan.count, all);
    for (u8 i = 0; i < plan.count; i++) EXPECT_NE(plan.species[i], SPECIES_POOCHYENA);
    for (u8 i = 1; i < PARTY_SIZE; i++) gParties[B_TRAINER_PLAYER][i] = gParties[B_TRAINER_PLAYER][0];
    gPartiesCount[B_TRAINER_PLAYER] = PARTY_SIZE;
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ONE, SPECIES_POOCHYENA), POKELOCATOR_OBTAIN_READY);
    result = Pokelocator_ExecuteObtain(&plan);
    EXPECT_EQ(result.party, 0);
    EXPECT_EQ(result.pc, 1);
    struct Pokemon received;
    BoxMonToMon(GetBoxedMonPtr(0, 0), &received);
    EXPECT_EQ(GetMonData(&received, MON_DATA_LEVEL), plan.level);
    for (u8 box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (u8 slot = 0; slot < IN_BOX_COUNT; slot++)
            *GetBoxedMonPtr(box, slot) = gParties[B_TRAINER_PLAYER][0].box;
    // One free slot cannot satisfy ALL; reject before delivering the first mon.
    ZeroBoxMonData(GetBoxedMonPtr(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1));
    rng = gRngValue;
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ALL, SPECIES_NONE), POKELOCATOR_OBTAIN_NO_SPACE);
    EXPECT_EQ(Pokelocator_ExecuteObtain(&plan).status, POKELOCATOR_OBTAIN_NO_SPACE);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(Pokelocator_CountFreeSlots(), 1);
    *GetBoxedMonPtr(TOTAL_BOXES_COUNT - 1, IN_BOX_COUNT - 1) = gParties[B_TRAINER_PLAYER][0].box;
    rng = gRngValue;
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ALL, SPECIES_NONE), POKELOCATOR_OBTAIN_NO_SPACE);
    EXPECT_EQ(Pokelocator_ExecuteObtain(&plan).status, POKELOCATOR_OBTAIN_NO_SPACE);
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, header, POKELOCATOR_GROUP_ALL, POKELOCATOR_OBTAIN_ALL, SPECIES_NONE), POKELOCATOR_OBTAIN_EMPTY);
    EXPECT_EQ(Pokelocator_PrepareObtain(&plan, HEADER_NONE, POKELOCATOR_GROUP_LAND, POKELOCATOR_OBTAIN_ALL, SPECIES_NONE), POKELOCATOR_OBTAIN_WRONG_AREA);
    memcpy(gSaveBlock1Ptr, save1, sizeof(*save1));
    memcpy(gSaveBlock2Ptr, save2, sizeof(*save2));
    memcpy(gPokemonStoragePtr, storage, sizeof(*storage));
    memcpy(gParties[B_TRAINER_PLAYER], party, sizeof(party));
    gPartiesCount[B_TRAINER_PLAYER] = oldCount;
    gMain.inBattle = battle;
    Free(save1); Free(save2); Free(storage);
}
