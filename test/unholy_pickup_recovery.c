#include "global.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"

TEST("Unholy native item balls recover script item and flag from current ROM map")
{
    const struct ObjectEventTemplate romBall =
    {
        .localId = 1,
        .graphicsId = OBJ_EVENT_GFX_ITEM_BALL,
        .trainerRange_berryTreeId = ITEM_BURN_HEAL,
        .script = Common_EventScript_FindItem,
        .flagId = FLAG_ITEM_JAGGED_PASS_BURN_HEAL,
    };
    const struct MapEvents mapEvents =
    {
        .objectEventCount = 1,
        .objectEvents = &romBall,
    };
    const struct MapEvents *originalEvents = gMapHeader.events;
    struct ObjectEventTemplate originalTemplate = gSaveBlock1Ptr->objectEventTemplates[0];
    struct ObjectEventTemplate *savedBall = &gSaveBlock1Ptr->objectEventTemplates[0];
    const struct ObjectEventTemplate *found;

    gMapHeader.events = &mapEvents;
    *savedBall = romBall;
    savedBall->script = NULL;
    savedBall->trainerRange_berryTreeId = ITEM_NONE;
    savedBall->flagId = 0;

    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT_EQ(found, &romBall);
    EXPECT_EQ(found->script, Common_EventScript_FindItem);
    EXPECT_EQ(found->trainerRange_berryTreeId, ITEM_BURN_HEAL);
    EXPECT_EQ(found->flagId, FLAG_ITEM_JAGGED_PASS_BURN_HEAL);

    // A non-null pointer saved with the previous ROM is equally dangerous.
    *savedBall = romBall;
    savedBall->script = Common_EventScript_FindItem + 1;
    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT_EQ(found, &romBall);

    // A complete saved template remains authoritative: random/dynamic item
    // locations must not be reset just because a ROM template exists.
    *savedBall = romBall;
    savedBall->trainerRange_berryTreeId = ITEM_POTION;
    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT_EQ(found, savedBall);
    EXPECT_EQ(found->trainerRange_berryTreeId, ITEM_POTION);

    *savedBall = originalTemplate;
    gMapHeader.events = originalEvents;
}
