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
    EXPECT(found == &romBall);
    EXPECT(found->script == Common_EventScript_FindItem);
    EXPECT_EQ(found->trainerRange_berryTreeId, ITEM_BURN_HEAL);
    EXPECT_EQ(found->flagId, FLAG_ITEM_JAGGED_PASS_BURN_HEAL);

    // A non-null pointer saved with the previous ROM is equally dangerous.
    *savedBall = romBall;
    savedBall->script = Common_EventScript_FindItem + 1;
    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT(found == &romBall);

    // A complete saved template remains authoritative: random/dynamic item
    // locations must not be reset just because a ROM template exists.
    *savedBall = romBall;
    savedBall->trainerRange_berryTreeId = ITEM_POTION;
    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT(found == savedBall);
    EXPECT_EQ(found->trainerRange_berryTreeId, ITEM_POTION);

    *savedBall = originalTemplate;
    gMapHeader.events = originalEvents;
}

#if !IS_FRLG
TEST("New Mauville generator balls ignore stale saved pickup and Voltorb templates")
{
    const struct ObjectEventTemplate romBalls[] =
    {
        {
            .localId = 1,
            .graphicsId = OBJ_EVENT_GFX_ITEM_BALL,
            .trainerRange_berryTreeId = ITEM_THUNDER_STONE,
            .script = Common_EventScript_FindItem,
            .flagId = FLAG_ITEM_NEW_MAUVILLE_THUNDER_STONE,
        },
        {
            .localId = 2,
            .graphicsId = OBJ_EVENT_GFX_ITEM_BALL,
            .script = Common_EventScript_FindItem + 1, // Scripted wild encounter, not a pickup.
            .flagId = FLAG_HIDE_NEW_MAUVILLE_VOLTORB_1,
        },
    };
    const struct MapEvents mapEvents =
    {
        .objectEventCount = ARRAY_COUNT(romBalls),
        .objectEvents = romBalls,
    };
    const struct MapEvents *originalEvents = gMapHeader.events;
    struct ObjectEventTemplate originalTemplates[2];
    u8 originalMapNum = gSaveBlock1Ptr->location.mapNum;
    u8 originalMapGroup = gSaveBlock1Ptr->location.mapGroup;
    const struct ObjectEventTemplate *found;

    originalTemplates[0] = gSaveBlock1Ptr->objectEventTemplates[0];
    originalTemplates[1] = gSaveBlock1Ptr->objectEventTemplates[1];
    gMapHeader.events = &mapEvents;
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NEW_MAUVILLE_INSIDE);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NEW_MAUVILLE_INSIDE);
    gSaveBlock1Ptr->objectEventTemplates[0] = romBalls[0];
    gSaveBlock1Ptr->objectEventTemplates[0].trainerRange_berryTreeId = ITEM_POTION;
    gSaveBlock1Ptr->objectEventTemplates[0].flagId = FLAG_ITEM_ROUTE_102_POTION;
    gSaveBlock1Ptr->objectEventTemplates[1] = romBalls[1];
    gSaveBlock1Ptr->objectEventTemplates[1].script = Common_EventScript_FindItem;

    found = GetObjectEventTemplateByLocalIdAndMap(1,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT(found == &romBalls[0]);
    EXPECT_EQ(found->trainerRange_berryTreeId, ITEM_THUNDER_STONE);
    EXPECT_EQ(found->flagId, FLAG_ITEM_NEW_MAUVILLE_THUNDER_STONE);

    found = GetObjectEventTemplateByLocalIdAndMap(2,
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    EXPECT(found == &romBalls[1]);
    EXPECT(found->script != Common_EventScript_FindItem);
    EXPECT_EQ(found->flagId, FLAG_HIDE_NEW_MAUVILLE_VOLTORB_1);

    gSaveBlock1Ptr->objectEventTemplates[0] = originalTemplates[0];
    gSaveBlock1Ptr->objectEventTemplates[1] = originalTemplates[1];
    gSaveBlock1Ptr->location.mapNum = originalMapNum;
    gSaveBlock1Ptr->location.mapGroup = originalMapGroup;
    gMapHeader.events = originalEvents;
}
#endif
