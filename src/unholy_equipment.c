#include "global.h"
#include "unholy_equipment.h"
#include "battle.h"
#include "gear.h"
#include "item.h"
#include "mail.h"
#include "main.h"
#include "menu_helpers.h"

static bool32 Allowed(enum Item item, bool8 gear)
{
    if (item <= ITEM_NONE || item >= ITEMS_COUNT) return FALSE;
    if (gear) return Gear_FromItem(item) != GEAR_NONE;
    return Gear_FromItem(item) == GEAR_NONE && !ItemIsMail(item)
        && GetItemPocket(item) != POCKET_KEY_ITEMS && !GetItemImportance(item) && IsHoldingItemAllowed(item);
}

static void Owned(bool8 gear, u8 *bits)
{
    memset(bits, 0, (ITEMS_COUNT + 7) / 8);
    for (u8 pocket = 0; pocket < POCKETS_COUNT; pocket++)
        for (u16 slot = 0; slot < gBagPockets[pocket].capacity; slot++)
        {
            struct ItemSlot item = BagPocket_GetSlotData(&gBagPockets[pocket], slot);
            if (item.itemId > ITEM_NONE && item.itemId < ITEMS_COUNT && item.quantity != 0
             && Allowed(item.itemId, gear) && CheckBagHasItem(item.itemId, 1))
                bits[item.itemId / 8] |= 1 << (item.itemId % 8);
        }
}

u16 UnholyEquipment_List(bool8 gear, u16 *items, u16 capacity)
{
    u8 bits[(ITEMS_COUNT + 7) / 8];
    Owned(gear, bits);
    u16 count = 1;
    if (capacity) items[0] = ITEM_NONE;
    for (u16 item = 1; item < ITEMS_COUNT; item++)
        if (bits[item / 8] & (1 << (item % 8)))
        {
            if (count < capacity) items[count] = item;
            count++;
        }
    return count;
}

u8 UnholyEquipment_Prepare(u8 partySlot, bool8 gear, enum Item item, struct UnholyEquipmentPlan *plan)
{
    memset(plan, 0, sizeof(*plan));
    if (gMain.inBattle || partySlot >= PARTY_SIZE || partySlot >= gPartiesCount[B_TRAINER_PLAYER]) return UNHOLY_EQUIP_INVALID;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][partySlot];
    if (GetMonData(mon, MON_DATA_IS_EGG) || GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG)
     || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE) return UNHOLY_EQUIP_INVALID;
    if (!gear && ItemIsMail(GetMonData(mon, MON_DATA_HELD_ITEM))) return UNHOLY_EQUIP_MAIL;
    u16 current = gear ? Gear_ToItem(Gear_GetEquipped(mon)) : GetMonData(mon, MON_DATA_HELD_ITEM);
    if (item == current) return UNHOLY_EQUIP_ALREADY;
    if (item != ITEM_NONE && !Allowed(item, gear)) return UNHOLY_EQUIP_INVALID;
    if (item != ITEM_NONE && !CheckBagHasItem(item, 1)) return UNHOLY_EQUIP_MISSING;
    plan->before = *mon;
    plan->item = item;
    plan->partySlot = partySlot;
    plan->gear = gear;
    return UNHOLY_EQUIP_OK;
}

u8 UnholyEquipment_Apply(struct UnholyEquipmentPlan *plan)
{
    if (plan->applied || plan->partySlot >= PARTY_SIZE) return UNHOLY_EQUIP_CHANGED;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][plan->partySlot];
    if (memcmp(mon, &plan->before, sizeof(*mon))) return UNHOLY_EQUIP_CHANGED;
    struct UnholyEquipmentPlan current;
    u8 status = UnholyEquipment_Prepare(plan->partySlot, plan->gear, plan->item, &current);
    if (status != UNHOLY_EQUIP_OK) return status;
    if (plan->gear && plan->item != ITEM_NONE)
    {
        enum GearEquipResult result = Gear_TryEquipFromBag(mon, plan->item);
        if (result != GEAR_EQUIP_OK) return result == GEAR_EQUIP_BAG_FULL ? UNHOLY_EQUIP_BAG_FULL : UNHOLY_EQUIP_MISSING;
    }
    else
    {
        u16 old = plan->gear ? Gear_ToItem(Gear_GetEquipped(mon)) : GetMonData(mon, MON_DATA_HELD_ITEM);
        if (plan->item != ITEM_NONE && !RemoveBagItem(plan->item, 1)) return UNHOLY_EQUIP_MISSING;
        if (old != ITEM_NONE && !AddBagItem(old, 1))
        {
            if (plan->item != ITEM_NONE) AddBagItem(plan->item, 1);
            return UNHOLY_EQUIP_BAG_FULL;
        }
        if (plan->gear) Gear_SetEquipped(mon, GEAR_NONE);
        else SetMonData(mon, MON_DATA_HELD_ITEM, &plan->item);
    }
    plan->applied = TRUE;
    return UNHOLY_EQUIP_OK;
}
