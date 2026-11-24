#ifndef GUARD_PC_SEARCH_H
#define GUARD_PC_SEARCH_H

#define PC_SEARCH_FAVORITE (1 << 0)
#define PC_SEARCH_SHINY    (1 << 1)
#define PC_SEARCH_GEAR     (1 << 2)
#define PC_SEARCH_NONE     0xFFFF
#define PC_SEARCH_MASK_BYTES(slots) (((slots) + 7) / 8)

enum PcSearchMutationMode
{
    PC_SEARCH_ANY,
    PC_SEARCH_NORMAL,
    PC_SEARCH_MUTATED,
};

// Read-only snapshots: no dependence on box positions, RNG, save or hardware.
struct PcSearchMon
{
    u16 species;
    u16 mutations;
    u8 types[2];
    u8 flags;
    bool8 egg;
    bool8 invalid;
};

struct PcSearchQuery
{
    u16 species; // 0 means any; never reveals an Egg's hidden species.
    u16 mutations;
    u8 type; // TYPE_NONE means any.
    u8 flags; // Required PC_SEARCH_* attributes, combined with AND.
    u8 mutationMode;
};

bool32 PcSearch_Matches(const struct PcSearchMon *mon, const struct PcSearchQuery *query);
u16 PcSearch_FindNext(const u8 *mask, u16 slots, u16 after);

// Native PC modal. The caller suspends its input until onClose; no boxes move.
bool32 PcSearch_Open(struct PcSearchQuery *query, u16 species, u16 after, void (*onClose)(u16 result));

#endif
