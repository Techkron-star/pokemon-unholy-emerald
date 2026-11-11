#ifndef GUARD_UNHOLY_MON_EXT_H
#define GUARD_UNHOLY_MON_EXT_H

// Versioned schema for *future extended-save* Pokémon metadata. Not wired to
// MON_DATA_* or BoxPokemon yet: this file introduces NO save-format changes.
// Physical storage and stable UID migration require a tested extended backend.
#define UNHOLY_MON_EXT_MAGIC   0x554D4F4Eu // 'UMON'
#define UNHOLY_MON_EXT_VERSION 1

enum UnholyMonExtDomain
{
    UNHOLY_EXT_STAT = 1,          // Derived-stat modifiers and future stats
    UNHOLY_EXT_MUTATION = 2,      // Mutation IDs (one record per mutation)
    UNHOLY_EXT_PERMANENT = 3,     // Persistent attribute modifiers
    UNHOLY_EXT_ACHIEVEMENT = 4,   // Pokémon-specific history/progression
};

// Individual stat IDs are data, not fixed C struct members. New stats can be
// added without changing the size or alignment of BoxPokemon or SaveBlock1.
enum UnholyStatId
{
    UNHOLY_STAT_NONE,
    UNHOLY_STAT_STAMINA_MAX_BONUS,
    UNHOLY_STAT_STAMINA_REGEN_BONUS,
    UNHOLY_STAT_CRITICAL_RATE_BONUS,
    UNHOLY_STAT_CRITICAL_DAMAGE_BONUS,
    UNHOLY_STAT_CUSTOM_FIRST = 0x100,
};

struct UnholyMonExtHeader
{
    u32 magic;
    u16 version;
    u16 totalBytes; // Full record size including header and entries
    u32 uidLow;
    u32 uidHigh;    // 64-bit stable identity, minted during future migration
    u16 gearId;     // 0 = none; no 5-bit Gear limitation in this schema
    u8 hiddenAbilitySlot;
    u8 reserved;
    u16 entryCount;
    u16 flags;
};

// Repeated (domain,key) entries are rejected for domains with one value per
// key. Mutations can use *arbitrary numeric IDs* instead of a limited bitmask.
// Values intentionally use s32 so new stats aren't restricted to 8-bit slots.
struct UnholyMonExtEntry
{
    u16 domain;
    u16 key;
    s32 value;
};

// Validate a serialized header + immediately following fixed-size entries
// before anyone interprets its contents. Does not modify the save or allocate.
bool32 UnholyMonExt_Validate(const void *record, u32 availableBytes);

#endif // GUARD_UNHOLY_MON_EXT_H
