#include "global.h"
#include "event_data.h"
#include "unholy_qol.h"
#include "test/test.h"
#include "constants/vars.h"

TEST("Quick QoL settings retain Autorun defaults and persist independent switches")
{
    u16 savedFlags = VarGet(VAR_UNHOLY_QOL_FLAGS);
    u16 savedRepel = VarGet(VAR_REPEL_AUTO_RENEW);
    u16 savedScriptArg = gSpecialVar_0x8000;

    VarSet(VAR_UNHOLY_QOL_FLAGS, 0);
    VarSet(VAR_REPEL_AUTO_RENEW, 0);
    EXPECT(UnholyQolIsEnabled(UQOL_AUTO_RUN));
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_CUT));
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_DIALOGUE));
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_REPEL));

    UnholyQolToggleOption(UQOL_AUTO_CUT);
    UnholyQolToggleOption(UQOL_AUTO_DIALOGUE);
    EXPECT(UnholyQolIsEnabled(UQOL_AUTO_RUN));
    EXPECT(UnholyQolIsEnabled(UQOL_AUTO_CUT));
    EXPECT(UnholyQolIsEnabled(UQOL_AUTO_DIALOGUE));
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_STRENGTH));

    gSpecialVar_0x8000 = UQOL_AUTO_CUT;
    EXPECT(UnholyQolGetScriptSetting());
    gSpecialVar_0x8000 = UQOL_AUTO_STRENGTH;
    EXPECT(!UnholyQolGetScriptSetting());

    UnholyQolToggleOption(UQOL_AUTO_RUN);
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_RUN));
    UnholyQolToggleOption(UQOL_AUTO_REPEL);
    EXPECT(UnholyQolIsEnabled(UQOL_AUTO_REPEL));
    EXPECT_EQ(VarGet(VAR_REPEL_AUTO_RENEW), 2);
    UnholyQolToggleOption(UQOL_AUTO_REPEL);
    EXPECT(!UnholyQolIsEnabled(UQOL_AUTO_REPEL));
    EXPECT_EQ(VarGet(VAR_REPEL_AUTO_RENEW), 1);

    VarSet(VAR_UNHOLY_QOL_FLAGS, savedFlags);
    VarSet(VAR_REPEL_AUTO_RENEW, savedRepel);
    gSpecialVar_0x8000 = savedScriptArg;
}
