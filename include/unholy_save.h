#ifndef GUARD_UNHOLY_SAVE_H
#define GUARD_UNHOLY_SAVE_H

// Unholy's FOURTH *logical* SaveBlock is packed into the unused tail of
// vanilla SaveBlock3's 116-byte-per-sector sidecar (14 sectors).
// It is NOT extra flash hardware and does NOT change the 128 KiB .sav format.
// Size: 1620 bytes + the existing 4-byte SaveBlock3 = 1624 bytes exactly.
// Changes to SaveBlock3's other optional config members must first reserve
// a smaller payload (the compile-time bound in src/save.c will catch overlap).
#define UNHOLY_SAVE4_MAGIC         0x554E4834 // ASCII 'UNH4'
#define UNHOLY_SAVE4_VERSION       1
#define UNHOLY_SAVE4_PAYLOAD_BYTES 1608 // 4 bytes reserved for CRC32

// A fifth logical block fits in PokemonStorage's unused ninth-sector tail:
// 9 * 3968 - 34144 = 1568 bytes. Separate from the BoxPokemon data, after
// all existing fields; therefore box offsets and encryption are untouched.
#define UNHOLY_SAVE5_MAGIC         0x554E4835 // ASCII 'UNH5'
#define UNHOLY_SAVE5_VERSION       1
#define UNHOLY_SAVE5_PAYLOAD_BYTES 1556 // 4 bytes reserved for CRC32

struct SaveBlock4
{
    u32 magic;
    u16 version;
    u16 flags; // Reserved for format migrations, NOT game progression flags.
    u32 crc32; // Covers version, flags and payload (not magic or crc32 itself).
    u8 payload[UNHOLY_SAVE4_PAYLOAD_BYTES];
};

struct SaveBlock5
{
    u32 magic;
    u16 version;
    u16 flags;
    u32 crc32;
    u8 payload[UNHOLY_SAVE5_PAYLOAD_BYTES];
};

// Initializes a new/legacy save's extension on first use. Returns NULL when
// the existing extension has a *recognized magic but unsupported version*:
// don't wipe data written by a newer ROM.
struct SaveBlock4 *UnholySave4_Get(void);

// Explicit reset for a deliberate, versioned save migration or a new game.
// Normal callers should use UnholySave4_Get instead.
void UnholySave4_Reset(void);

// Call Commit after editing payload or flags; without it integrity validation
// will deliberately reject the half-written record on the next load.
bool32 UnholySave4_Commit(void);
bool32 UnholySave4_Validate(const struct SaveBlock4 *block);

struct SaveBlock5 *UnholySave5_Get(void);
void UnholySave5_Reset(void);
bool32 UnholySave5_Commit(void);
bool32 UnholySave5_Validate(const struct SaveBlock5 *block);

#endif // GUARD_UNHOLY_SAVE_H
