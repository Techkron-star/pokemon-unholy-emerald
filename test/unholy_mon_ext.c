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
