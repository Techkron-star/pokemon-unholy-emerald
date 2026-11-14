#include "global.h"
#include "unholy_moves.h"
#include "battle.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "move.h"
#include "party_menu.h"

extern const u16 gTutorMoves[];

static bool32 Valid(struct Pokemon *mon)
{
    return mon != NULL && SanitizeSpeciesId(GetMonData(mon, MON_DATA_SPECIES)) != SPECIES_NONE
        && !GetMonData(mon, MON_DATA_IS_EGG) && !GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG);
}

static struct Pokemon *PartyMon(u8 slot)
{
    if (gMain.inBattle || slot >= PARTY_SIZE || slot >= gPartiesCount[B_TRAINER_PLAYER]) return NULL;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    return Valid(mon) ? mon : NULL;
}

static bool32 ValidMove(enum Move move)
{
    return move > MOVE_NONE && move < MOVES_COUNT;
}

static bool32 TutorMove(enum Move move)
{
    for (u16 i = 0; gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
        if (gTutorMoves[i] == move) return TRUE;
    return FALSE;
}

void UnholyMoves_Build(struct Pokemon *mon, struct UnholyMoveCatalog *catalog)
{
    memset(catalog, 0, sizeof(*catalog));
    if (!Valid(mon)) return;
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    enum Species ancestor = species;
    enum Species visited[8];
    u8 depth = 0;
    while (ancestor != SPECIES_NONE && depth < ARRAY_COUNT(visited))
    {
        bool32 repeat = FALSE;
        for (u8 i = 0; i < depth; i++) repeat |= visited[i] == ancestor;
        if (repeat) break;
        visited[depth++] = ancestor;
        const struct LevelUpMove *levels = GetSpeciesLevelUpLearnset(ancestor);
        for (u16 i = 0; levels[i].move != LEVEL_UP_MOVE_END; i++)
        {
            enum Move move = levels[i].move;
            if (!ValidMove(move)) continue;
            if (!(catalog->sources[move] & (UNHOLY_MOVE_LEVEL | UNHOLY_MOVE_PRE_EVO))
             || levels[i].level < catalog->level[move]) catalog->level[move] = levels[i].level;
            catalog->sources[move] |= ancestor == species ? UNHOLY_MOVE_LEVEL : UNHOLY_MOVE_PRE_EVO;
        }
        const u16 *eggs = GetSpeciesEggMoves(ancestor);
        for (u16 i = 0; eggs[i] != MOVE_UNAVAILABLE; i++)
            if (ValidMove(eggs[i])) catalog->sources[eggs[i]] |= UNHOLY_MOVE_EGG;
        ancestor = GetSpeciesPreEvolution(ancestor);
    }
    const u16 *teachable = GetSpeciesTeachableLearnset(species);
    for (u16 i = 0; teachable[i] != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = teachable[i];
        if (!ValidMove(move)) continue;
        if (GetTMHMItemIdFromMoveId(move) != ITEM_NONE) catalog->sources[move] |= UNHOLY_MOVE_MACHINE;
        // A move can have both a machine and tutor route; either valid route is enough.
        if (TutorMove(move) || GetTMHMItemIdFromMoveId(move) == ITEM_NONE)
            catalog->sources[move] |= UNHOLY_MOVE_TUTOR;
    }
}

u16 UnholyMoves_Count(const struct UnholyMoveCatalog *catalog, u8 source)
{
    u16 count = 0;
    for (u16 move = 1; move < MOVES_COUNT; move++) count += !!(catalog->sources[move] & source);
    return count;
}

enum Move UnholyMoves_At(const struct UnholyMoveCatalog *catalog, u8 source, u16 ordinal)
{
    for (u16 move = 1; move < MOVES_COUNT; move++)
        if ((catalog->sources[move] & source) && ordinal-- == 0) return move;
    return MOVE_NONE;
}

u8 UnholyMoves_Status(struct Pokemon *mon, const struct UnholyMoveCatalog *catalog, enum Move move, u16 *item)
{
    *item = ITEM_NONE;
    if (!Valid(mon) || !ValidMove(move) || catalog->sources[move] == 0) return UNHOLY_MOVE_INVALID;
    if (MonKnowsMove(mon, move)) return UNHOLY_MOVE_KNOWN;
    u8 sources = catalog->sources[move];
    if (sources & (UNHOLY_MOVE_EGG | UNHOLY_MOVE_TUTOR)) return UNHOLY_MOVE_READY;
    if ((sources & (UNHOLY_MOVE_LEVEL | UNHOLY_MOVE_PRE_EVO))
     && catalog->level[move] <= GetMonData(mon, MON_DATA_LEVEL)) return UNHOLY_MOVE_READY;
    if (sources & UNHOLY_MOVE_MACHINE)
    {
        *item = GetTMHMItemIdFromMoveId(move);
        return CheckBagHasItem(*item, 1) ? UNHOLY_MOVE_READY : UNHOLY_MOVE_NEEDS_DISC;
    }
    return UNHOLY_MOVE_NEEDS_LEVEL;
}

const u8 *UnholyMoves_SourceName(u8 source)
{
    switch (source)
    {
    case UNHOLY_MOVE_LEVEL: return COMPOUND_STRING("LEVEL");
    case UNHOLY_MOVE_PRE_EVO: return COMPOUND_STRING("PRE-EVO");
    case UNHOLY_MOVE_EGG: return COMPOUND_STRING("EGG");
    case UNHOLY_MOVE_MACHINE: return COMPOUND_STRING("TM / HM");
    case UNHOLY_MOVE_TUTOR: return COMPOUND_STRING("TUTOR / OTHER");
    default: return COMPOUND_STRING("ALL MOVES");
    }
}

u8 UnholyMoves_Prepare(u8 partySlot, enum Move move, u8 moveSlot, struct UnholyMovePlan *plan)
{
    memset(plan, 0, sizeof(*plan));
    struct Pokemon *mon = PartyMon(partySlot);
    if (mon == NULL || moveSlot >= MAX_MON_MOVES) return UNHOLY_MOVE_INVALID;
    struct UnholyMoveCatalog *catalog = Alloc(sizeof(*catalog));
    if (catalog == NULL) return UNHOLY_MOVE_INVALID;
    UnholyMoves_Build(mon, catalog);
    u8 status = UnholyMoves_Status(mon, catalog, move, &plan->item);
    Free(catalog);
    if (status != UNHOLY_MOVE_READY) return status;
    enum Move oldMove = GetMonData(mon, MON_DATA_MOVE1 + moveSlot);
    if (oldMove != MOVE_NONE && CannotForgetMove(oldMove)) return UNHOLY_MOVE_CANNOT_FORGET;
    plan->before = *mon;
    plan->partySlot = partySlot;
    plan->moveSlot = moveSlot;
    plan->move = move;
    return status;
}

u8 UnholyMoves_Apply(struct UnholyMovePlan *plan)
{
    struct Pokemon *mon = PartyMon(plan->partySlot);
    if (plan->consumed || mon == NULL || memcmp(mon, &plan->before, sizeof(*mon))) return UNHOLY_MOVE_CHANGED;
    struct UnholyMovePlan current;
    u8 status = UnholyMoves_Prepare(plan->partySlot, plan->move, plan->moveSlot, &current);
    if (status != UNHOLY_MOVE_READY) return status;
    if (current.item != plan->item) return UNHOLY_MOVE_CHANGED;
    if (current.item != ITEM_NONE && !I_REUSABLE_TMS && !GetItemImportance(current.item))
        if (!RemoveBagItem(current.item, 1)) return UNHOLY_MOVE_NEEDS_DISC;
    u8 bonuses = GetMonData(mon, MON_DATA_PP_BONUSES) & ~(3 << (plan->moveSlot * 2));
    u8 pp = GetMovePP(plan->move);
    SetMonData(mon, MON_DATA_MOVE1 + plan->moveSlot, &plan->move);
    SetMonData(mon, MON_DATA_PP1 + plan->moveSlot, &pp);
    SetMonData(mon, MON_DATA_PP_BONUSES, &bonuses);
    plan->consumed = TRUE;
    return UNHOLY_MOVE_READY;
}

bool32 UnholyMoves_Swap(u8 partySlot, u8 first, u8 second)
{
    struct Pokemon *mon = PartyMon(partySlot);
    if (mon == NULL || first >= MAX_MON_MOVES || second >= MAX_MON_MOVES) return FALSE;
    u16 moves[] = {GetMonData(mon, MON_DATA_MOVE1 + first), GetMonData(mon, MON_DATA_MOVE1 + second)};
    u8 pp[] = {GetMonData(mon, MON_DATA_PP1 + first), GetMonData(mon, MON_DATA_PP1 + second)};
    u8 bonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
    u8 a = (bonuses >> (first * 2)) & 3, b = (bonuses >> (second * 2)) & 3;
    bonuses = (bonuses & ~(3 << (first * 2)) & ~(3 << (second * 2))) | (a << (second * 2)) | (b << (first * 2));
    SetMonData(mon, MON_DATA_MOVE1 + first, &moves[1]);
    SetMonData(mon, MON_DATA_MOVE1 + second, &moves[0]);
    SetMonData(mon, MON_DATA_PP1 + first, &pp[1]);
    SetMonData(mon, MON_DATA_PP1 + second, &pp[0]);
    SetMonData(mon, MON_DATA_PP_BONUSES, &bonuses);
    return TRUE;
}

bool32 UnholyMoves_Forget(u8 partySlot, u8 moveSlot, enum Move expected)
{
    struct Pokemon *mon = PartyMon(partySlot);
    if (mon == NULL || moveSlot >= MAX_MON_MOVES || expected == MOVE_NONE
     || GetMonData(mon, MON_DATA_MOVE1 + moveSlot) != expected) return FALSE;
    u8 count = 0;
    for (u8 i = 0; i < MAX_MON_MOVES; i++) count += GetMonData(mon, MON_DATA_MOVE1 + i) != MOVE_NONE;
    if (count <= 1) return FALSE;
    // Move Deleter rules permit field moves; only learning/replacement uses CannotForgetMove.
    for (u8 i = moveSlot; i + 1 < MAX_MON_MOVES; i++) UnholyMoves_Swap(partySlot, i, i + 1);
    u16 none = MOVE_NONE;
    u8 zero = 0, bonuses = GetMonData(mon, MON_DATA_PP_BONUSES) & ~(3 << ((MAX_MON_MOVES - 1) * 2));
    SetMonData(mon, MON_DATA_MOVE4, &none);
    SetMonData(mon, MON_DATA_PP4, &zero);
    SetMonData(mon, MON_DATA_PP_BONUSES, &bonuses);
    return TRUE;
}
