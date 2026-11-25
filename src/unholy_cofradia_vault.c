#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "unholy_save.h"
#include "unholy_cofradia.h"
#include "battle.h"
#include "item.h"
#include "mail.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/item.h"

#define VAULT_MAGIC 0x56415531u
#define VAULT_VERSION 1
enum VaultOrigin { VAULT_EMPTY, VAULT_PARTY, VAULT_PC };

struct VaultMon
{
    struct Pokemon mon;
    u32 value;
    u8 origin;
};
struct VaultItem
{
    u32 value;
    u16 item;
    u16 count;
};
struct VaultRecord
{
    u32 magic;
    u16 version;
    u16 reserved;
    struct VaultMon mons[COFRADIA_VAULT_MON_CAPACITY];
    struct VaultItem items[COFRADIA_VAULT_ITEM_CAPACITY];
};
STATIC_ASSERT(UNHOLY_SAVE5_COFRADIA_VAULT_OFFSET + sizeof(struct VaultRecord) <= UNHOLY_SAVE5_PAYLOAD_BYTES, CofradiaVaultFitsSave5);

static struct VaultRecord *GetVault(void)
{
    struct SaveBlock5 *block = UnholySave5_Get();
    if (block == NULL)
        return NULL;
    struct VaultRecord *v = (struct VaultRecord *)(block->payload + UNHOLY_SAVE5_COFRADIA_VAULT_OFFSET);
    if (v->magic == 0)
    {
        for (u32 i = 0; i < sizeof(*v); i++)
            if (((u8 *)v)[i] != 0)
                return NULL;
        v->magic = VAULT_MAGIC;
        v->version = VAULT_VERSION;
        if (!UnholySave5_Commit())
            return NULL;
    }
    if (v->magic != VAULT_MAGIC || v->version != VAULT_VERSION)
        return NULL;
    for (u8 i = 0; i < COFRADIA_VAULT_MON_CAPACITY; i++)
        if (v->mons[i].origin > VAULT_PC || (!!v->mons[i].origin != !!v->mons[i].value))
            return NULL;
    for (u8 i = 0; i < COFRADIA_VAULT_ITEM_CAPACITY; i++)
        if ((!!v->items[i].count != !!v->items[i].value)
         || (v->items[i].count && (v->items[i].item == ITEM_NONE || v->items[i].item >= ITEMS_COUNT)))
            return NULL;
    return v;
}

static struct CofradiaVaultView Summarize(const struct VaultRecord *v)
{
    struct CofradiaVaultView view = {0};
    u64 total = 0;
    for (u8 i = 0; i < COFRADIA_VAULT_MON_CAPACITY; i++)
        if (v->mons[i].origin)
        {
            view.pokemon++;
            total += v->mons[i].value;
        }
    for (u8 i = 0; i < COFRADIA_VAULT_ITEM_CAPACITY; i++)
        if (v->items[i].count)
        {
            view.itemStacks++;
            total += v->items[i].value;
        }
    view.totalValue = min(total, 0xFFFFFFFFULL);
    return view;
}

bool32 CofradiaVaultGetView(struct CofradiaVaultView *view)
{
    struct VaultRecord *v = GetVault();
    if (v == NULL || view == NULL)
        return FALSE;
    *view = Summarize(v);
    return TRUE;
}

static bool32 CanHold(const struct VaultRecord *v)
{
    struct CofradiaView ledger;
    return v != NULL && CofradiaGetView(&ledger) && ledger.state == COFRADIA_IDLE
        && ledger.debt > Summarize(v).totalValue && !CofradiaBattleActive();
}

bool32 CofradiaVaultReturnAvailable(void)
{
    struct CofradiaView ledger;
    return CofradiaGetView(&ledger) && ledger.state == COFRADIA_IDLE
        && ledger.debt == 0 && !CofradiaBattleActive();
}

// Explicit safe optional keys only. Fusion devices remain protected until
// ownership of their externally stored partner is audited.
static bool32 OptionalKey(enum Item item)
{
    switch (item)
    {
    case ITEM_ROTOM_CATALOG:
    case ITEM_GRACIDEA:
    case ITEM_REVEAL_GLASS:
    case ITEM_PRISON_BOTTLE:
        return TRUE;
    default:
        return FALSE;
    }
}

u32 CofradiaAppraiseItem(enum Item item)
{
    if (item == ITEM_NONE || item >= ITEMS_COUNT || ItemIsMail(item))
        return 0;
    if (GetItemDisplayPocket(item) == POCKET_GEARS)
        return 15000; // provisional artifact floor, not a shop sale price
    if (OptionalKey(item))
        return 20000;
    if (GetItemImportance(item) || GetItemPocket(item) == POCKET_KEY_ITEMS
     || GetItemPocket(item) == POCKET_TM_HM)
        return 0;
    return GetItemSellPrice(item);
}

static bool32 SafeMon(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || species >= NUM_SPECIES
        || GetMonData(mon, MON_DATA_IS_EGG)
        || GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG)
        || mon->box.isFavorite || MonHasMail(mon))
        return FALSE;
    // Protect forms connected to the external fusion storage.
#if P_FUSION_FORMS
    if (gFusionTablePointers[species] != NULL
     || gFusionTablePointers[GET_BASE_SPECIES_ID(species)] != NULL)
        return FALSE;
#endif
    // Do not seize a protected key or mail through a held item.
    enum Item held = GetMonData(mon, MON_DATA_HELD_ITEM);
    if (held != ITEM_NONE && CofradiaAppraiseItem(held) == 0)
        return FALSE;
    return TRUE;
}

static u8 LivingPartyCount(void)
{
    u8 count = 0;
    for (u8 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
            && !GetMonData(mon, MON_DATA_IS_EGG)
            && !GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG)
            && GetMonData(mon, MON_DATA_HP))
            count++;
    }
    return count;
}

static void CompactPartyAfterHold(u8 slot)
{
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    for (u8 i = slot; i + 1 < PARTY_SIZE; i++)
        party[i] = party[i + 1];
    ZeroMonData(&party[PARTY_SIZE - 1]);
    CalculatePlayerPartyCount();
}

bool32 CofradiaVaultHoldMon(u8 box, u8 slot)
{
    struct VaultRecord *v = GetVault();
    if (!CanHold(v))
        return FALSE;
    u8 index = 0;
    while (index < COFRADIA_VAULT_MON_CAPACITY && v->mons[index].origin)
        index++;
    if (index == COFRADIA_VAULT_MON_CAPACITY)
        return FALSE;
    struct VaultMon entry = {0};
    struct BoxPokemon *source = NULL;
    if (box == COFRADIA_VAULT_PARTY)
    {
        if (slot >= PARTY_SIZE || LivingPartyCount() <= 2)
            return FALSE;
        entry.mon = gParties[B_TRAINER_PLAYER][slot];
        entry.origin = VAULT_PARTY;
    }
    else
    {
        source = GetBoxedMonPtr(box, slot);
        if (source == NULL || !source->hasSpecies || source->isEgg || source->isBadEgg)
            return FALSE;
        // Convert only a private copy for appraisal, then preserve the exact
        // original boxed data (including encrypted identity and gear).
        struct BoxPokemon copy = *source;
        BoxMonToMon(&copy, &entry.mon);
        entry.mon.box = *source;
        entry.origin = VAULT_PC;
    }
    if (!SafeMon(&entry.mon))
        return FALSE;
    entry.value = CofradiaAppraisePokemon(&entry.mon);
    if (!entry.value)
        return FALSE;
    struct VaultMon old = v->mons[index];
    v->mons[index] = entry;
    if (!UnholySave5_Commit())
    {
        v->mons[index] = old;
        return FALSE;
    }
    if (entry.origin == VAULT_PARTY)
        CompactPartyAfterHold(slot);
    else
        ZeroBoxMonAt(box, slot);
    return TRUE;
}

bool32 CofradiaVaultHoldItem(enum Item item, u16 count)
{
    struct VaultRecord *v = GetVault();
    u32 unit = CofradiaAppraiseItem(item);
    if (!CanHold(v) || !unit || !count || !CheckBagHasItem(item, count))
        return FALSE;
    if (GetItemPocket(item) == POCKET_KEY_ITEMS && count != 1)
        return FALSE;
    u8 index = 0;
    while (index < COFRADIA_VAULT_ITEM_CAPACITY && v->items[index].count)
        index++;
    if (index == COFRADIA_VAULT_ITEM_CAPACITY)
        return FALSE;
    u64 value = (u64)unit * count;
    if (value > 0xFFFFFFFFULL)
        return FALSE;
    struct VaultItem old = v->items[index];
    v->items[index] = (struct VaultItem){value, item, count};
    if (!UnholySave5_Commit())
    {
        v->items[index] = old;
        return FALSE;
    }
    if (!RemoveBagItem(item, count))
    {
        v->items[index] = old;
        UnholySave5_Commit();
        return FALSE;
    }
    return TRUE;
}

bool32 CofradiaVaultReturnMon(u8 index)
{
    struct VaultRecord *v = GetVault();
    if (v == NULL || !CofradiaVaultReturnAvailable()
        || index >= COFRADIA_VAULT_MON_CAPACITY || !v->mons[index].origin)
        return FALSE;
    struct VaultMon entry = v->mons[index];
    // Return to the original storage class. A full destination leaves the
    // entry intact and recoverable; no overwrite or automatic release.
    struct Pokemon *partyDest = NULL;
    struct BoxPokemon *boxDest = NULL;
    if (entry.origin == VAULT_PARTY)
    {
        for (u8 i = 0; i < PARTY_SIZE; i++)
            if (!gParties[B_TRAINER_PLAYER][i].box.hasSpecies && !gParties[B_TRAINER_PLAYER][i].box.isBadEgg)
            {
                partyDest = &gParties[B_TRAINER_PLAYER][i];
                break;
            }
        if (partyDest == NULL)
            return FALSE;
    }
    else
    {
        for (u8 box = 0; box < TOTAL_BOXES_COUNT && boxDest == NULL; box++)
            for (u8 slot = 0; slot < IN_BOX_COUNT; slot++)
            {
                struct BoxPokemon *candidate = GetBoxedMonPtr(box, slot);
                if (candidate != NULL && !candidate->hasSpecies && !candidate->isBadEgg)
                {
                    boxDest = candidate;
                    break;
                }
            }
        if (boxDest == NULL)
            return FALSE;
    }
    memset(&v->mons[index], 0, sizeof(v->mons[index]));
    if (!UnholySave5_Commit())
    {
        v->mons[index] = entry;
        return FALSE;
    }
    if (partyDest != NULL)
    {
        *partyDest = entry.mon;
        CalculatePlayerPartyCount();
    }
    else
        *boxDest = entry.mon.box;
    return TRUE;
}

bool32 CofradiaVaultReturnItem(u8 index)
{
    struct VaultRecord *v = GetVault();
    if (v == NULL || !CofradiaVaultReturnAvailable()
        || index >= COFRADIA_VAULT_ITEM_CAPACITY || !v->items[index].count)
        return FALSE;
    struct VaultItem entry = v->items[index];
    if (GetItemPocket(entry.item) == POCKET_KEY_ITEMS
        && (CheckBagHasItem(entry.item, 1) || CheckPCHasItem(entry.item, 1)))
        return FALSE; // preserve custody until duplicate unique item is resolved
    if (!CheckBagHasSpace(entry.item, entry.count))
        return FALSE;
    memset(&v->items[index], 0, sizeof(v->items[index]));
    if (!UnholySave5_Commit())
    {
        v->items[index] = entry;
        return FALSE;
    }
    if (!AddBagItem(entry.item, entry.count))
    {
        v->items[index] = entry;
        UnholySave5_Commit();
        return FALSE;
    }
    return TRUE;
}

// Select highest-value eligible Pokemon first. The engine is intentionally
// callable by lobby default processing or a confirmed account transaction,
// never while the battle party is active.
bool32 CofradiaVaultCollect(void)
{
    struct CofradiaView ledger;
    struct CofradiaVaultView view;
    if (!CofradiaGetView(&ledger) || ledger.state != COFRADIA_IDLE
        || !ledger.debt || !CofradiaVaultGetView(&view))
        return FALSE;
    while (view.totalValue < ledger.debt && view.pokemon < COFRADIA_VAULT_MON_CAPACITY)
    {
        u32 best = 0;
        u8 bestBox = COFRADIA_VAULT_PARTY, bestSlot = 0;
        if (LivingPartyCount() > 2)
            for (u8 i = 0; i < PARTY_SIZE; i++)
            {
                struct Pokemon copy = gParties[B_TRAINER_PLAYER][i];
                if (SafeMon(&copy))
                {
                    u32 value = CofradiaAppraisePokemon(&copy);
                    if (value > best)
                    {
                        best = value;
                        bestSlot = i;
                    }
                }
            }
        for (u8 box = 0; box < TOTAL_BOXES_COUNT; box++)
            for (u8 slot = 0; slot < IN_BOX_COUNT; slot++)
            {
                struct BoxPokemon *source = GetBoxedMonPtr(box, slot);
                if (source == NULL || !source->hasSpecies || source->isEgg
                    || source->isBadEgg || source->isFavorite)
                    continue;
                struct BoxPokemon boxed = *source;
                struct Pokemon copy = {0};
                BoxMonToMon(&boxed, &copy);
                if (!SafeMon(&copy))
                    continue;
                u32 value = CofradiaAppraisePokemon(&copy);
                if (value > best)
                {
                    best = value;
                    bestBox = box;
                    bestSlot = slot;
                }
            }
        if (!best || !CofradiaVaultHoldMon(bestBox, bestSlot))
            break;
        if (!CofradiaVaultGetView(&view))
            return FALSE;
    }
    // Fill the remainder with the highest-value individual item units, using
    // physical pockets (filtered display pockets would duplicate entries).
    while (view.totalValue < ledger.debt && view.itemStacks < COFRADIA_VAULT_ITEM_CAPACITY)
    {
        enum Item bestItem = ITEM_NONE;
        u32 best = 0;
        u16 available = 0;
        for (u8 pocket = 0; pocket < POCKET_GEARS; pocket++)
            for (u32 slot = 0; slot < gBagPockets[pocket].capacity; slot++)
            {
                struct ItemSlot item = GetBagItemIdAndQuantity(pocket, slot);
                if (!item.quantity)
                    continue;
                u32 value = CofradiaAppraiseItem(item.itemId);
                if (value > best)
                {
                    best = value;
                    bestItem = item.itemId;
                    available = item.quantity;
                }
            }
        if (!best || !available)
            break;
        u32 remaining = ledger.debt - view.totalValue;
        u32 needed = remaining / best + (remaining % best != 0);
        u16 count = min(available, needed);
        if (GetItemPocket(bestItem) == POCKET_KEY_ITEMS)
            count = 1;
        if (!CofradiaVaultHoldItem(bestItem, count))
            break;
        if (!CofradiaVaultGetView(&view))
            return FALSE;
    }
    return view.totalValue >= ledger.debt;
}

bool32 CofradiaVaultDescribeMon(u8 index, u8 *name, u32 *value)
{
    struct VaultRecord *v = GetVault();
    if (v == NULL || index >= COFRADIA_VAULT_MON_CAPACITY
        || !v->mons[index].origin || name == NULL || value == NULL)
        return FALSE;
    struct Pokemon copy = v->mons[index].mon;
    GetMonData(&copy, MON_DATA_NICKNAME, name);
    *value = v->mons[index].value;
    return TRUE;
}

bool32 CofradiaVaultDescribeItem(u8 index, enum Item *item, u16 *count, u32 *value)
{
    struct VaultRecord *v = GetVault();
    if (v == NULL || index >= COFRADIA_VAULT_ITEM_CAPACITY
        || !v->items[index].count || item == NULL || count == NULL || value == NULL)
        return FALSE;
    *item = v->items[index].item;
    *count = v->items[index].count;
    *value = v->items[index].value;
    return TRUE;
}
