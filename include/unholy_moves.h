#ifndef GUARD_UNHOLY_MOVES_H
#define GUARD_UNHOLY_MOVES_H

#include "pokemon.h"

enum UnholyMoveSource
{
    UNHOLY_MOVE_LEVEL = 1 << 0,
    UNHOLY_MOVE_PRE_EVO = 1 << 1,
    UNHOLY_MOVE_EGG = 1 << 2,
    UNHOLY_MOVE_MACHINE = 1 << 3,
    UNHOLY_MOVE_TUTOR = 1 << 4,
    UNHOLY_MOVE_ALL = (1 << 5) - 1,
};

enum UnholyMoveStatus
{
    UNHOLY_MOVE_READY,
    UNHOLY_MOVE_KNOWN,
    UNHOLY_MOVE_NEEDS_LEVEL,
    UNHOLY_MOVE_NEEDS_DISC,
    UNHOLY_MOVE_INVALID,
    UNHOLY_MOVE_CHANGED,
    UNHOLY_MOVE_CANNOT_FORGET,
};

// Indexed by move ID: bounded, deduplicated, and independent of the old 60-move list.
struct UnholyMoveCatalog
{
    u8 sources[MOVES_COUNT];
    u8 level[MOVES_COUNT];
};

struct UnholyMovePlan
{
    struct Pokemon before;
    u16 move, item;
    u8 partySlot, moveSlot;
    bool8 consumed;
};

void UnholyMoves_Build(struct Pokemon *mon, struct UnholyMoveCatalog *catalog);
u16 UnholyMoves_Count(const struct UnholyMoveCatalog *catalog, u8 source);
enum Move UnholyMoves_At(const struct UnholyMoveCatalog *catalog, u8 source, u16 ordinal);
u8 UnholyMoves_Status(struct Pokemon *mon, const struct UnholyMoveCatalog *catalog, enum Move move, u16 *item);
const u8 *UnholyMoves_SourceName(u8 source);
u8 UnholyMoves_Prepare(u8 partySlot, enum Move move, u8 moveSlot, struct UnholyMovePlan *plan);
u8 UnholyMoves_Apply(struct UnholyMovePlan *plan);
bool32 UnholyMoves_Swap(u8 partySlot, u8 first, u8 second);
bool32 UnholyMoves_Forget(u8 partySlot, u8 moveSlot, enum Move expected);

#endif
