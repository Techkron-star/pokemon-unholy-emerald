#include "global.h"
#include "pokemon.h"
#include "string_util.h"
#include "test/test.h"

TEST("All ability names and descriptions are available in both UI languages")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;
    u32 ability;
    u32 language;

    for (language = 0; language < 2; language++)
    {
        gSaveBlock2Ptr->optionsLanguageSpanish = language;
        for (ability = 0; ability < ABILITIES_COUNT; ability++)
        {
            const u8 *name = GetLocalizedAbilityName(ability);
            const u8 *description = GetLocalizedAbilityDescription(ability);

            EXPECT_NE(name, NULL);
            EXPECT_NE(description, NULL);
            EXPECT_LE(StringLength(name), ABILITY_NAME_LENGTH);
            EXPECT_NE(StringLength(description), 0);
        }
    }

    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}
