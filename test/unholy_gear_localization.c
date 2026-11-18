#include "global.h"
#include "gear.h"
#include "item.h"
#include "string_util.h"
#include "test/test.h"

TEST("All 47 equipped artifacts have Spanish and English names and descriptions")
{
    bool8 previousLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;
    u32 language;
    u32 gear;

    EXPECT_EQ(GEAR_COUNT - 1, 47);
    for (language = 0; language < 2; language++)
    {
        gSaveBlock2Ptr->optionsLanguageSpanish = language;
        for (gear = GEAR_VISOR_COSMICO; gear < GEAR_COUNT; gear++)
        {
            enum Item item = Gear_ToItem(gear);
            const u8 *name = Gear_GetLocalizedName(gear);
            const u8 *description = Gear_GetLocalizedDescription(gear);
            u32 lineBreaks = 0;

            EXPECT_NE(item, ITEM_NONE);
            EXPECT_EQ(Gear_FromItem(item), gear);
            EXPECT_NE(name, NULL);
            EXPECT_NE(description, NULL);
            EXPECT_LE(StringLength(name), ITEM_NAME_LENGTH);
            EXPECT_NE(StringLength(description), 0);
            EXPECT_EQ(GetItemName(item), name);
            EXPECT_EQ(GetItemDescription(item), description);

            for (u32 c = 0; description[c] != EOS; c++)
                if (description[c] == CHAR_NEWLINE)
                    lineBreaks++;
            EXPECT_LE(lineBreaks, 1);
        }
    }

    gSaveBlock2Ptr->optionsLanguageSpanish = previousLanguage;
}

TEST("Artifact translation switches languages without changing ordinary items")
{
    bool8 previousLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;
    const u8 *normalName;
    const u8 *normalDescription;
    const u8 *englishGearName;
    const u8 *englishGearDescription;

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    normalName = GetItemName(ITEM_POTION);
    normalDescription = GetItemDescription(ITEM_POTION);
    englishGearName = GetItemName(ITEM_GEAR_NIHIL_ENGINE);
    englishGearDescription = GetItemDescription(ITEM_GEAR_NIHIL_ENGINE);

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    EXPECT_EQ(GetItemName(ITEM_POTION), normalName);
    EXPECT_EQ(GetItemDescription(ITEM_POTION), normalDescription);
    EXPECT_NE(GetItemName(ITEM_GEAR_NIHIL_ENGINE), englishGearName);
    EXPECT_NE(GetItemDescription(ITEM_GEAR_NIHIL_ENGINE), englishGearDescription);
    EXPECT_EQ(GetItemName(ITEM_GEAR_NIHIL_ENGINE), Gear_GetLocalizedName(GEAR_NIHIL_ENGINE));

    gSaveBlock2Ptr->optionsLanguageSpanish = previousLanguage;
}
