#include "global.h"
#include "unholy_mon_ext.h"
#include "test/test.h"

TEST("Unholy per-mon extension accepts sorted stats and mutations")
{
    struct
    {
        struct UnholyMonExtHeader header;
        struct UnholyMonExtEntry fields[2];
    } record;

    memset(&record, 0, sizeof(record));
    record.header.magic = UNHOLY_MON_EXT_MAGIC;
    record.header.version = UNHOLY_MON_EXT_VERSION;
    record.header.totalBytes = sizeof(record);
    record.header.uidLow = 0x01234567;
    record.header.uidHigh = 0x89ABCDEF;
    record.header.gearId = 60; // Beyond the legacy five-bit field.
    record.header.hiddenAbilitySlot = 2;
    record.header.entryCount = 2;

    record.fields[0].domain = UNHOLY_EXT_STAT;
    record.fields[0].key = UNHOLY_STAT_STAMINA_MAX_BONUS;
    record.fields[0].value = 250;
    record.fields[1].domain = UNHOLY_EXT_MUTATION;
    record.fields[1].key = 55; // An extension key, NOT a hardcoded bitmask.
    record.fields[1].value = 1;
    EXPECT_EQ(UnholyMonExt_Validate(&record, sizeof(record)), TRUE);
    {
        s32 actual = 0;

        EXPECT_EQ(UnholyMonExt_TryGetValue(&record, sizeof(record),
                  UNHOLY_EXT_STAT, UNHOLY_STAT_STAMINA_MAX_BONUS, &actual), TRUE);
        EXPECT_EQ(actual, 250);
        EXPECT_EQ(UnholyMonExt_TryGetValue(&record, sizeof(record),
                  UNHOLY_EXT_MUTATION, 55, &actual), TRUE);
        EXPECT_EQ(actual, 1);
        EXPECT_EQ(UnholyMonExt_TryGetValue(&record, sizeof(record),
                  UNHOLY_EXT_STAT, UNHOLY_STAT_CRITICAL_DAMAGE_BONUS, &actual), FALSE);
    }

    // A validator must not assume 4-byte alignment of serialized data.
    {
        u8 bytes[sizeof(record) + 1];

        memcpy(&bytes[1], &record, sizeof(record));
        EXPECT_EQ(UnholyMonExt_Validate(&bytes[1], sizeof(record)), TRUE);
    }

    // Duplicate keys cannot silently override another value.
    record.fields[1].domain = UNHOLY_EXT_STAT;
    record.fields[1].key = UNHOLY_STAT_STAMINA_MAX_BONUS;
    EXPECT_EQ(UnholyMonExt_Validate(&record, sizeof(record)), FALSE);
    record.fields[1].domain = UNHOLY_EXT_MUTATION;
    record.fields[1].key = 55;

    EXPECT_EQ(UnholyMonExt_Validate(&record, sizeof(record) - 1), FALSE);
    record.header.version++;
    EXPECT_EQ(UnholyMonExt_Validate(&record, sizeof(record)), FALSE);
    record.header.version = UNHOLY_MON_EXT_VERSION;

    record.header.uidLow = 0;
    record.header.uidHigh = 0;
    EXPECT_EQ(UnholyMonExt_Validate(&record, sizeof(record)), FALSE);
}


TEST("Unholy per-mon extension supports bounded stat and mutation edits")
{
    u8 bytes[1 + sizeof(struct UnholyMonExtHeader) + 2 * sizeof(struct UnholyMonExtEntry)];
    u8 snapshot[sizeof(bytes)];
    u8 *record = &bytes[1]; // A byte-unaligned record, as in a packed save page.
    const u32 capacity = sizeof(bytes) - 1;
    s32 value = 0;

    memset(bytes, 0xCC, sizeof(bytes));
    EXPECT_EQ(UnholyMonExt_Init(record, capacity, 0xDEADBEEF, 0xCAFE1234, 60, 2), TRUE);
    EXPECT_EQ(UnholyMonExt_Validate(record, capacity), TRUE);

    // Insert out of order; writer must keep keys sorted for binary lookup.
    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_MUTATION, 55, 1), TRUE);
    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                     UNHOLY_STAT_STAMINA_MAX_BONUS, 250), TRUE);
    EXPECT_EQ(UnholyMonExt_Validate(record, capacity), TRUE);
    EXPECT_EQ(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_MUTATION, 55, &value), TRUE);
    EXPECT_EQ(value, 1);

    // Updating an existing key consumes no additional capacity.
    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                     UNHOLY_STAT_STAMINA_MAX_BONUS, 999), TRUE);
    EXPECT_EQ(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_STAT,
                                      UNHOLY_STAT_STAMINA_MAX_BONUS, &value), TRUE);
    EXPECT_EQ(value, 999);

    // Capacity exhaustion must leave the complete buffer byte-for-byte intact.
    memcpy(snapshot, bytes, sizeof(bytes));
    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                     UNHOLY_STAT_CRITICAL_RATE_BONUS, 5), FALSE);
    EXPECT_EQ(memcmp(snapshot, bytes, sizeof(bytes)), 0);

    // Removing one entry frees exactly one slot for a new stat.
    EXPECT_EQ(UnholyMonExt_TryRemoveValue(record, capacity, UNHOLY_EXT_MUTATION, 55), TRUE);
    EXPECT_EQ(UnholyMonExt_TryGetValue(record, capacity, UNHOLY_EXT_MUTATION, 55, &value), FALSE);
    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, UNHOLY_EXT_STAT,
                                     UNHOLY_STAT_CRITICAL_RATE_BONUS, -12), TRUE);
    EXPECT_EQ(UnholyMonExt_Validate(record, capacity), TRUE);

    EXPECT_EQ(UnholyMonExt_TrySetValue(record, capacity, 0,
                                     UNHOLY_STAT_STAMINA_MAX_BONUS, 1), FALSE);
    EXPECT_EQ(UnholyMonExt_TryRemoveValue(record, capacity, UNHOLY_EXT_MUTATION, 55), FALSE);
}
