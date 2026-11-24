#include "global.h"
#include "pc_search.h"

bool32 PcSearch_Matches(const struct PcSearchMon *mon, const struct PcSearchQuery *query)
{
    if (mon == NULL || query == NULL || mon->species == 0 || mon->invalid)
        return FALSE;
    if (query->mutationMode > PC_SEARCH_MUTATED)
        return FALSE;
    if (mon->egg && (query->species || query->type || query->mutations
        || query->mutationMode != PC_SEARCH_ANY || (query->flags & ~PC_SEARCH_FAVORITE)))
        return FALSE;
    if (query->species && mon->species != query->species)
        return FALSE;
    if (query->type && mon->types[0] != query->type && mon->types[1] != query->type)
        return FALSE;
    if ((mon->flags & query->flags) != query->flags)
        return FALSE;
    if (query->mutationMode == PC_SEARCH_NORMAL && mon->mutations != 0)
        return FALSE;
    if (query->mutationMode == PC_SEARCH_MUTATED && mon->mutations == 0)
        return FALSE;
    return (mon->mutations & query->mutations) == query->mutations;
}

u16 PcSearch_FindNext(const u8 *mask, u16 slots, u16 after)
{
    if (mask == NULL || slots == 0)
        return PC_SEARCH_NONE;
    if (after >= slots)
        after = slots - 1;
    for (u32 step = 1; step <= slots; step++)
    {
        u16 pos = (after + step) % slots;
        if (mask[pos / 8] & (1 << (pos % 8)))
            return pos;
    }
    return PC_SEARCH_NONE;
}
