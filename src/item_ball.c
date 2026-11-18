#include "global.h"
#include "item_ball.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"

static u32 GetItemBallAmountFromTemplate(const struct ObjectEventTemplate *template)
{
    u32 amount = template->movementRangeX;

    if (amount > MAX_BAG_ITEM_CAPACITY)
        return MAX_BAG_ITEM_CAPACITY;

    return (amount == 0) ? 1 : amount;
}

static u32 GetItemBallIdFromTemplate(const struct ObjectEventTemplate *template)
{
    enum Item itemId = template->trainerRange_berryTreeId;

    return (itemId == ITEM_NONE || itemId >= ITEMS_COUNT) ? ITEM_POKE_BALL : itemId;
}

void GetItemBallIdAndAmountFromTemplate(void)
{
    // Event local IDs need not match template array indices, especially when
    // object templates are changed at runtime. Read the current map's template.
    const struct ObjectEventTemplate *template = GetObjectEventTemplateByLocalIdAndMap(
        gSpecialVar_LastTalked,
        gSaveBlock1Ptr->location.mapNum,
        gSaveBlock1Ptr->location.mapGroup);

    // A malformed item ball must still yield a usable item, never an invalid
    // item ID or an out-of-bounds template access.
    if (template == NULL)
    {
        gSpecialVar_Result = ITEM_POKE_BALL;
        gSpecialVar_0x8009 = 1;
        return;
    }

    gSpecialVar_Result = GetItemBallIdFromTemplate(template);
    gSpecialVar_0x8009 = GetItemBallAmountFromTemplate(template);
}
