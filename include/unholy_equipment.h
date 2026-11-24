#ifndef GUARD_UNHOLY_EQUIPMENT_H
#define GUARD_UNHOLY_EQUIPMENT_H

#include "pokemon.h"

enum UnholyEquipmentResult
{
    UNHOLY_EQUIP_OK, UNHOLY_EQUIP_ALREADY, UNHOLY_EQUIP_MISSING,
    UNHOLY_EQUIP_BAG_FULL, UNHOLY_EQUIP_MAIL, UNHOLY_EQUIP_INVALID, UNHOLY_EQUIP_CHANGED,
};

struct UnholyEquipmentPlan
{
    struct Pokemon before;
    u16 item;
    u8 partySlot;
    bool8 gear, applied;
};

u16 UnholyEquipment_List(bool8 gear, u16 *items, u16 capacity);
u8 UnholyEquipment_Prepare(u8 partySlot, bool8 gear, enum Item item, struct UnholyEquipmentPlan *plan);
u8 UnholyEquipment_Apply(struct UnholyEquipmentPlan *plan);

#endif
