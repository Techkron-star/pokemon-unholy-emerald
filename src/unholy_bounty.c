#include "global.h"
#include "unholy_bounty.h"
#include "unholy_activities.h"
#include "unholy_localization.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "follower_npc.h"
#include "hfc_watch.h"
#include "item.h"
#include "malloc.h"
#include "money.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "constants/trainer_types.h"
#include "random.h"
#include "region_map.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/trainers.h"

#define BOUNTY_TEXT(en, es) UnholyLocalize(COMPOUND_STRING(en), COMPOUND_STRING(es))

struct BountyRank
{
    const u8 *name;
    const u8 *nameEs;
    u32 fee;
    u32 minimumReward;
    u32 rewardRange;
    u16 trainer;
    u8 badges;
    struct HfcRaidLoot loot[2];
};

static const struct BountyRank sRanks[] =
{
    {COMPOUND_STRING("BRONZE"), COMPOUND_STRING("BRONCE"), 500, 1200, 601, TRAINER_UNHOLY_BOUNTY_BRONZE, 0,
        {{ITEM_POTION, 2}, {ITEM_RED_SHARD, 1}}},
    {COMPOUND_STRING("SILVER"), COMPOUND_STRING("PLATA"), 2000, 4000, 2001, TRAINER_UNHOLY_BOUNTY_SILVER, 2,
        {{ITEM_SUPER_POTION, 2}, {ITEM_BLUE_SHARD, 2}}},
    {COMPOUND_STRING("GOLD"), COMPOUND_STRING("ORO"), 6000, 12000, 6001, TRAINER_UNHOLY_BOUNTY_GOLD, 4,
        {{ITEM_HYPER_POTION, 2}, {ITEM_YELLOW_SHARD, 3}}},
    {COMPOUND_STRING("PLATINUM"), COMPOUND_STRING("PLATINO"), 15000, 30000, 15001, TRAINER_UNHOLY_BOUNTY_PLATINUM, 6,
        {{ITEM_FULL_RESTORE, 2}, {ITEM_GREEN_SHARD, 4}}},
    {COMPOUND_STRING("LEGENDARY"), COMPOUND_STRING("LEGENDARIO"), 40000, 80000, 40001, TRAINER_UNHOLY_BOUNTY_LEGENDARY, 8,
        {{ITEM_RARE_CANDY, 3}, {ITEM_RED_SHARD, 5}}},
};
STATIC_ASSERT(ARRAY_COUNT(sRanks) == UNHOLY_BOUNTY_RANK_COUNT, BountyRankCount);

struct BountyRoute
{
    u8 mapGroup;
    u8 mapNum;
    u16 section;
    u16 visitedFlag;
    s16 x;
    s16 y;
};

// All positions are on land with several open approaches. Route 102 is the
// guaranteed early-game fallback; later routes require their neighboring town.
static const struct BountyRoute sRoutes[] =
{
    {MAP_GROUP(MAP_ROUTE102), MAP_NUM(MAP_ROUTE102), MAPSEC_ROUTE_102, FLAG_VISITED_OLDALE_TOWN, 43, 8},
    {MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104), MAPSEC_ROUTE_104, FLAG_VISITED_PETALBURG_CITY, 28, 68},
    {MAP_GROUP(MAP_ROUTE110), MAP_NUM(MAP_ROUTE110), MAPSEC_ROUTE_110, FLAG_VISITED_MAUVILLE_CITY, 20, 17},
    {MAP_GROUP(MAP_ROUTE117), MAP_NUM(MAP_ROUTE117), MAPSEC_ROUTE_117, FLAG_VISITED_VERDANTURF_TOWN, 50, 6},
    {MAP_GROUP(MAP_ROUTE113), MAP_NUM(MAP_ROUTE113), MAPSEC_ROUTE_113, FLAG_VISITED_FALLARBOR_TOWN, 87, 10},
};

extern const u8 EventScript_UnholyBountyTarget[];
static EWRAM_DATA struct ObjectEventTemplate sTargetTemplate;
static EWRAM_DATA u32 sPurchaseSeed;
static EWRAM_DATA u8 sPurchaseRank;

static void RemoveTarget(void)
{
    u8 objectId = GetObjectEventIdByLocalId(LOCALID_BOUNTY_TARGET);
    // Dynamic targets do not own a map hide flag or a saved map template.
    if (objectId < OBJECT_EVENTS_COUNT)
        RemoveObjectEvent(&gObjectEvents[objectId]);
}

u32 UnholyBounty_GetFee(u8 rank)
{
    return rank < ARRAY_COUNT(sRanks) ? sRanks[rank].fee : 0;
}

u8 UnholyBounty_GetRequiredBadges(u8 rank)
{
    return rank < ARRAY_COUNT(sRanks) ? sRanks[rank].badges : 255;
}

u32 UnholyBounty_GetReward(u8 rank, u32 seed)
{
    return rank < ARRAY_COUNT(sRanks) ? sRanks[rank].minimumReward + seed % sRanks[rank].rewardRange : 0;
}

static u8 SelectRoute(u32 seed)
{
    u8 eligible[ARRAY_COUNT(sRoutes)];
    u8 count = 1;
    eligible[0] = 0;
    for (u8 i = 1; i < ARRAY_COUNT(sRoutes); i++)
        if (FlagGet(sRoutes[i].visitedFlag))
            eligible[count++] = i;
    return eligible[(seed >> 16) % count];
}

static bool32 ValidContract(const struct UnholyActivitySave *save)
{
    return save != NULL && (save->bountyState == BOUNTY_ACTIVE || save->bountyState == BOUNTY_REWARD_READY)
        && save->bountyRank < ARRAY_COUNT(sRanks) && save->bountyRoute < ARRAY_COUNT(sRoutes);
}

u16 UnholyBounty_GetMapSection(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    return ValidContract(save) && save->bountyState == BOUNTY_ACTIVE ? sRoutes[save->bountyRoute].section : MAPSEC_NONE;
}

const struct ObjectEventTemplate *UnholyBounty_GetTemplate(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    const struct BountyRoute *route;
    if (!ValidContract(save))
        return NULL;
    route = &sRoutes[save->bountyRoute];
    if (route->mapGroup != gSaveBlock1Ptr->location.mapGroup || route->mapNum != gSaveBlock1Ptr->location.mapNum)
        return NULL;
    sTargetTemplate = (struct ObjectEventTemplate){
        .localId = LOCALID_BOUNTY_TARGET,
        .graphicsId = OBJ_EVENT_GFX_MAN_3,
        .kind = OBJ_KIND_NORMAL,
        .x = route->x,
        .y = route->y,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_FACE_DOWN,
        .trainerType = TRAINER_TYPE_NONE,
        .script = EventScript_UnholyBountyTarget,
    };
    return &sTargetTemplate;
}

void UnholyBounty_TrySpawn(s16 cameraX, s16 cameraY)
{
    const struct ObjectEventTemplate *target;
    bool32 candidateMap = FALSE;
    for (u8 i = 0; i < ARRAY_COUNT(sRoutes); i++)
        if (sRoutes[i].mapGroup == gSaveBlock1Ptr->location.mapGroup && sRoutes[i].mapNum == gSaveBlock1Ptr->location.mapNum)
            candidateMap = TRUE;
    if (!candidateMap || GetObjectEventIdByLocalId(LOCALID_BOUNTY_TARGET) < OBJECT_EVENTS_COUNT)
        return;
    target = UnholyBounty_GetTemplate();
    if (target == NULL || abs(target->x - gSaveBlock1Ptr->pos.x) > 12 || abs(target->y - gSaveBlock1Ptr->pos.y) > 10)
        return;
    // Keep ordinary NPCs and the player safe if a map edit occupies the target tile.
    if (GetObjectEventIdByXY(target->x + MAP_OFFSET, target->y + MAP_OFFSET) < OBJECT_EVENTS_COUNT
     || MapGridGetCollisionAt(target->x + MAP_OFFSET, target->y + MAP_OFFSET) != 0)
        return;
    TrySpawnObjectEventTemplate(target, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, cameraX, cameraY);
}

enum UnholyBountyResult UnholyBounty_Purchase(u8 rank, u32 seed)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    if (save == NULL)
        return BOUNTY_UNAVAILABLE;
    if (save->bountyState != BOUNTY_NONE)
        return BOUNTY_BUSY;
    if (rank >= ARRAY_COUNT(sRanks) || HfcWatch_CountBadges() < UnholyBounty_GetRequiredBadges(rank))
        return BOUNTY_LOCKED;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, sRanks[rank].fee))
        return BOUNTY_NO_MONEY;
    RemoveMoney(&gSaveBlock1Ptr->money, sRanks[rank].fee);
    save->bountyState = BOUNTY_ACTIVE;
    save->bountyRank = rank;
    save->bountyRoute = SelectRoute(seed);
    save->bountySeed = seed;
    save->bountyMoney = UnholyBounty_GetReward(rank, seed);
    UnholyActivities_Commit();
    return BOUNTY_OK;
}

void UnholyBounty_OnBattleEnd(u8 outcome)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    if (ValidContract(save) && save->bountyState == BOUNTY_ACTIVE && outcome == B_OUTCOME_WON)
    {
        save->bountyState = BOUNTY_REWARD_READY;
        UnholyActivities_Commit();
    }
}

enum UnholyBountyResult UnholyBounty_Claim(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    const struct BountyRank *rank;
    if (!ValidContract(save) || save->bountyState != BOUNTY_REWARD_READY)
        return BOUNTY_UNAVAILABLE;
    rank = &sRanks[save->bountyRank];
    for (u8 i = 0; i < ARRAY_COUNT(rank->loot); i++)
    {
        if (!AddBagItem(rank->loot[i].item, rank->loot[i].quantity))
        {
            while (i > 0)
            {
                i--;
                RemoveBagItem(rank->loot[i].item, rank->loot[i].quantity);
            }
            return BOUNTY_BAG_FULL;
        }
    }
    AddMoney(&gSaveBlock1Ptr->money, save->bountyMoney);
    save->bountyState = BOUNTY_NONE;
    UnholyActivities_Commit();
    return BOUNTY_OK;
}

static void ResultText(enum UnholyBountyResult result)
{
    const u8 *text;
    switch (result)
    {
    case BOUNTY_OK: text = BOUNTY_TEXT("Contract accepted! Check the map marker.\nDefeat the trainer for money and loot.", "¡Contrato aceptado! Mira la marca del mapa.\nVence al entrenador por dinero y botín."); break;
    case BOUNTY_LOCKED: text = BOUNTY_TEXT("You need more badges for this contract.", "Necesitas más medallas para este contrato."); break;
    case BOUNTY_NO_MONEY: text = BOUNTY_TEXT("You don't have enough money.", "No tienes suficiente dinero."); break;
    case BOUNTY_BUSY: text = BOUNTY_TEXT("Finish or abandon your current contract.", "Completa o abandona tu contrato actual."); break;
    case BOUNTY_BAG_FULL: text = BOUNTY_TEXT("Make room in your Bag. Your reward is safe.\nReturn to the target or any contract seller.", "Haz espacio en la mochila. Tu premio sigue\naquí. Habla con el objetivo o un vendedor."); break;
    default: text = BOUNTY_TEXT("No contract is available right now.", "No hay un contrato disponible ahora."); break;
    }
    StringCopy(gStringVar4, text);
}

void UnholyBounty_OpenMenu(void)
{
    struct ListMenuItem *items = AllocZeroed(sizeof(*items) * ARRAY_COUNT(sRanks));
    gSpecialVar_Result = FALSE;
    if (items == NULL)
        return;
    for (u8 i = 0; i < ARRAY_COUNT(sRanks); i++)
    {
        u8 *text = Alloc(48);
        u8 *cursor;
        if (text == NULL)
        {
            while (i > 0)
                Free((void *)items[--i].name);
            Free(items);
            return;
        }
        cursor = StringCopy(text, UnholyLocalize(sRanks[i].name, sRanks[i].nameEs));
        cursor = StringCopy(cursor, COMPOUND_STRING("  ¥"));
        cursor = ConvertUIntToDecimalStringN(cursor, sRanks[i].fee, STR_CONV_MODE_LEFT_ALIGN, 5);
        if (HfcWatch_CountBadges() < sRanks[i].badges)
        {
            cursor = StringCopy(cursor, BOUNTY_TEXT(" / badges ", " / medallas "));
            ConvertUIntToDecimalStringN(cursor, sRanks[i].badges, STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        items[i] = (struct ListMenuItem){text, i};
    }
    gSpecialVar_Result = ScriptMenu_MultichoiceDynamic(0, 0, ARRAY_COUNT(sRanks), items, FALSE, 5, 0, DYN_MULTICHOICE_CB_NONE);
}

void UnholyBounty_PreparePurchase(void)
{
    u8 rank = gSpecialVar_0x8004;
    static const u8 levels[] = {12, 24, 36, 48, 60};
    u8 route;
    u8 *cursor;
    sPurchaseRank = rank;
    sPurchaseSeed = Random32();
    if (rank >= ARRAY_COUNT(sRanks) || HfcWatch_CountBadges() < UnholyBounty_GetRequiredBadges(rank))
    {
        gSpecialVar_Result = FALSE;
        ResultText(BOUNTY_LOCKED);
        return;
    }
    route = SelectRoute(sPurchaseSeed);
    GetMapName(gStringVar1, sRoutes[route].section, 0);
    cursor = StringCopy(gStringVar4, BOUNTY_TEXT("Target: ", "Objetivo: "));
    cursor = StringCopy(cursor, gStringVar1);
    cursor = StringCopy(cursor, BOUNTY_TEXT("  Lv. ", "  Nv. "));
    cursor = ConvertUIntToDecimalStringN(cursor, levels[rank], STR_CONV_MODE_LEFT_ALIGN, 2);
    cursor = StringCopy(cursor, BOUNTY_TEXT("\nFee: ¥", "\nPrecio: ¥"));
    cursor = ConvertUIntToDecimalStringN(cursor, sRanks[rank].fee, STR_CONV_MODE_LEFT_ALIGN, 5);
    cursor = StringCopy(cursor, BOUNTY_TEXT("  Reward: ¥", "  Premio: ¥"));
    cursor = ConvertUIntToDecimalStringN(cursor, UnholyBounty_GetReward(rank, sPurchaseSeed), STR_CONV_MODE_LEFT_ALIGN, 6);
    StringCopy(cursor, BOUNTY_TEXT("\lPlus loot. Buy this contract?", "\lMás botín. ¿Comprar este contrato?"));
    gSpecialVar_Result = TRUE;
}

void UnholyBounty_Buy(void)
{
    ResultText(UnholyBounty_Purchase(sPurchaseRank, sPurchaseSeed));
}

void UnholyBounty_Status(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    gSpecialVar_Result = save == NULL ? 3 : save->bountyState;
    if (ValidContract(save))
    {
        u8 *cursor = StringCopy(gStringVar4, BOUNTY_TEXT("Active contract: ", "Contrato activo: "));
        GetMapName(gStringVar1, sRoutes[save->bountyRoute].section, 0);
        cursor = StringCopy(cursor, gStringVar1);
        StringCopy(cursor, BOUNTY_TEXT("\nLose? You can retry without paying again.\lAbandon it? The fee is not refunded.",
            "\nSi pierdes, reintentas sin pagar de nuevo.\l¿Abandonar? No se devuelve el pago."));
    }
}

void UnholyBounty_Abandon(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    if (ValidContract(save) && save->bountyState == BOUNTY_ACTIVE)
    {
        save->bountyState = BOUNTY_NONE;
        UnholyActivities_Commit();
        RemoveTarget();
    }
}

void UnholyBounty_Interact(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    gSpecialVar_Result = FALSE;
    if (!ValidContract(save) || UnholyBounty_GetTemplate() == NULL)
        return;
    if (save->bountyState == BOUNTY_REWARD_READY)
        return;
    if (CountPartyAliveNonEggMonsExcept(PARTY_SIZE) == 0 || IsNPCFollowerWildBattle())
    {
        StringCopy(gStringVar4, BOUNTY_TEXT("Prepare your party before this duel.", "Prepara tu equipo antes de este duelo."));
        gSpecialVar_Result = 2;
        return;
    }
    gSpecialVar_Result = TRUE;
    BattleSetup_StartBountyBattle(sRanks[save->bountyRank].trainer);
}

void UnholyBounty_ClaimScript(void)
{
    struct UnholyActivitySave *save = UnholyActivities_Get();
    u32 reward = ValidContract(save) ? save->bountyMoney : 0;
    enum UnholyBountyResult result = UnholyBounty_Claim();
    ResultText(result);
    if (result == BOUNTY_OK)
    {
        u8 *cursor = StringCopy(gStringVar4, BOUNTY_TEXT("Contract complete!\nYou received ¥", "¡Contrato completado!\nRecibiste ¥"));
        cursor = ConvertUIntToDecimalStringN(cursor, reward, STR_CONV_MODE_LEFT_ALIGN, 6);
        StringCopy(cursor, BOUNTY_TEXT(" and loot.", " y botín."));
        RemoveTarget();
    }
}
