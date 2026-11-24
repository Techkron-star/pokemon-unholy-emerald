#include "global.h"
#include "pokemon.h"
#include "constants/gears.h"
#include "test/test.h"

TEST("Unholy Gear IDs support 0 through 255 without altering moves or IVs")
{
    struct Pokemon mon;
    u32 originalMove4;
    u32 originalIv;
    u8 gear;
    u32 values[] = {0, 1, 7, 31, 32, 60, 127, 128, 254, 255};

    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 40, 0xAABBCCDD,
                     OTID_STRUCT_PRESET(0x10203040), MAX_PER_STAT_IVS);

    originalMove4 = GetMonData(&mon, MON_DATA_MOVE4);
    originalIv = GetMonData(&mon, MON_DATA_HP_IV);

    EXPECT_EQ(GEAR_MAX_COUNT, 256);

    for (u32 i = 0; i < ARRAY_COUNT(values); i++)
    {
        gear = values[i];
        SetMonData(&mon, MON_DATA_GEAR, &gear);

        EXPECT_EQ(GetMonData(&mon, MON_DATA_GEAR), values[i]);
        EXPECT_EQ(GetBoxMonData(&mon.box, MON_DATA_GEAR), values[i]);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE4), originalMove4);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV), originalIv);
    }

    // Old saves used only the five lower bits: values <= 31 still map exactly
    // to their old numeric IDs.
    gear = GEAR_PERMAFROST_FRAGMENT;
    SetMonData(&mon, MON_DATA_GEAR, &gear);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_GEAR), GEAR_PERMAFROST_FRAGMENT);
}
