#include <assert.h>
#include <stdio.h>
#include "global.h"
#include "pokemon_storage_system.h"
#include "unholy_mon_ext.h"
#include "stamina_ring.h"

static struct SaveBlock3 sSave3;
static struct PokemonStorage sStorage;
struct SaveBlock3 *gSaveBlock3Ptr = &sSave3;
struct PokemonStorage *gPokemonStoragePtr = &sStorage;

static void TestSaveEnvelopes(void)
{
    struct SaveBlock4 *four = UnholySave4_Get();
    struct SaveBlock5 *five = UnholySave5_Get();

    assert(four != NULL && five != NULL);
    assert(sizeof(*four) == 1620);
    assert(sizeof(*five) == 1568);
    assert(UnholySave4_Validate(four));
    assert(UnholySave5_Validate(five));

    four->payload[2] = 77;
    assert(!UnholySave4_Validate(four));
    assert(UnholySave4_Get() == NULL);
    assert(UnholySave4_Commit());
    assert(UnholySave4_Validate(four));

    five->payload[UNHOLY_SAVE5_PAYLOAD_BYTES - 1] = 46;
    assert(!UnholySave5_Validate(five));
    assert(UnholySave5_Commit());
    assert(UnholySave5_Validate(five));

    four->version++;
    assert(UnholySave4_Get() == NULL);
    assert(!UnholySave4_Commit());
    four->version--;
    assert(UnholySave4_Validate(four));
}

static void TestMonRecordCodec(void)
{
    u8 bytes[1 + sizeof(struct UnholyMonExtHeader)
               + 2 * sizeof(struct UnholyMonExtEntry)];
    u8 copy[sizeof(bytes)];
    void *record = bytes + 1; // Intentionally unaligned.
    const u32 capacity = sizeof(bytes) - 1;
    s32 value = 0;

    memset(bytes, 0xCD, sizeof(bytes));
    assert(!UnholyMonExt_Init(record, capacity, 0, 0, 0, 0));
    assert(UnholyMonExt_Init(record, capacity, 123, 456, 60, 2));
    assert(UnholyMonExt_Validate(record, capacity));

    assert(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_MUTATION, 55, 1));
    assert(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                    UNHOLY_STAT_STAMINA_MAX_BONUS, 250));
    assert(UnholyMonExt_Validate(record, capacity));
    assert(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_MUTATION, 55, &value));
    assert(value == 1);

    assert(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                    UNHOLY_STAT_STAMINA_MAX_BONUS, 999));
    assert(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_STAT,
                                    UNHOLY_STAT_STAMINA_MAX_BONUS, &value));
    assert(value == 999);

    memcpy(copy, bytes, sizeof(bytes));
    assert(!UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                     UNHOLY_STAT_CRITICAL_DAMAGE_BONUS, 5));
    assert(memcmp(copy, bytes, sizeof(bytes)) == 0);

    assert(UnholyMonExt_TryRemoveValue(record, capacity, UNHOLY_EXT_MUTATION, 55));
    assert(!UnholyMonExt_TryRemoveValue(record, capacity, UNHOLY_EXT_MUTATION, 55));
    assert(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                    UNHOLY_STAT_CRITICAL_DAMAGE_BONUS, -12));
    assert(UnholyMonExt_Validate(record, capacity));
    assert(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_STAT,
                                    UNHOLY_STAT_CRITICAL_DAMAGE_BONUS, &value));
    assert(value == -12);
}

static void TestStaminaRing(void)
{
    u8 empty[STAMINA_RING_TILE_BYTES], half[STAMINA_RING_TILE_BYTES];
    u8 full[STAMINA_RING_TILE_BYTES], exhausted[STAMINA_RING_TILE_BYTES];

    assert(StaminaRing_FilledSegments(0, 100) == 0);
    assert(StaminaRing_FilledSegments(20, 100) == 13);
    assert(StaminaRing_FilledSegments(50, 100) == 32);
    assert(StaminaRing_FilledSegments(100, 100) == 64);
    assert(StaminaRing_FilledSegments(200, 100) == 64);
    assert(StaminaRing_FilledSegments(0, 0) == 0);

    StaminaRing_Render(empty, 0, 100, 0);
    StaminaRing_Render(half, 50, 100, 0);
    StaminaRing_Render(full, 100, 100, 0);
    StaminaRing_Render(exhausted, 0, 100, 1);
    assert(memcmp(empty, half, sizeof(empty)) != 0);
    assert(memcmp(half, full, sizeof(half)) != 0);
    assert(memcmp(empty, exhausted, sizeof(empty)) == 0);
    // Centre of ring remains transparent, the outer track always exists.
    assert(empty[(2 * 4 + 2) * 32 + 0] == 0);
    assert(memcmp(empty, (u8[STAMINA_RING_TILE_BYTES]){0}, sizeof(empty)) != 0);
}

int main(void)
{
    TestStaminaRing();
    TestSaveEnvelopes();
    TestMonRecordCodec();
    puts("Unholy native save schema smoke tests: PASS");
    return 0;
}
