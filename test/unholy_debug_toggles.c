#include "global.h"
#include "event_data.h"
#include "test/test.h"

#if !IS_FRLG
TEST("Unholy debug cheats use distinct persistent flags and a saved bag variable")
{
    const u16 flags[] =
    {
        OW_FLAG_NO_COLLISION,
        WE_FLAG_NO_ENCOUNTER,
        OW_FLAG_NO_TRAINER_SEE,
        WE_FLAG_NO_CATCHING,
    };

    for (u8 i = 0; i < ARRAY_COUNT(flags); i++)
    {
        EXPECT(flags[i] != 0);
        for (u8 j = 0; j < i; j++)
            EXPECT(flags[i] != flags[j]);
    }
    EXPECT(B_VAR_NO_BAG_USE >= VARS_START);
    EXPECT(B_VAR_NO_BAG_USE <= VARS_END);

    // An actual toggle should change the option without touching story flags.
    {
        bool8 wasEnabled = FlagGet(OW_FLAG_NO_COLLISION);
        FlagToggle(OW_FLAG_NO_COLLISION);
        EXPECT(FlagGet(OW_FLAG_NO_COLLISION) != wasEnabled);
        FlagToggle(OW_FLAG_NO_COLLISION);
        EXPECT_EQ(FlagGet(OW_FLAG_NO_COLLISION), wasEnabled);
    }
}
#endif
