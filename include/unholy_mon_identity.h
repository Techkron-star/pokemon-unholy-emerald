#ifndef GUARD_UNHOLY_MON_IDENTITY_H
#define GUARD_UNHOLY_MON_IDENTITY_H

struct BoxPokemon;
struct Pokemon;

// A stable *legacy fingerprint* for migration, NOT a guaranteed unique UID.
// OTID and personality normally follow a Pokémon through party/PC moves and
// evolutions; clones may share both values. Never attach persistent extension
// records to a fingerprint before scanning the relevant collection for
// duplicates. The extended save will eventually assign a separate 64-bit UID.
struct UnholyMonLegacyKey
{
    u32 personality;
    u32 otId;
};

struct UnholyMonIdentityScan
{
    struct UnholyMonLegacyKey key;
    u32 matches;
    bool32 valid;
};

enum UnholyMonIdentityStatus
{
    UNHOLY_MON_ID_INVALID,
    UNHOLY_MON_ID_MISSING,
    UNHOLY_MON_ID_UNIQUE,
    UNHOLY_MON_ID_AMBIGUOUS,
};

// Read without altering the encrypted Pokémon or its internal fields.
bool32 UnholyMonIdentity_ReadBox(struct BoxPokemon *mon,
                                 struct UnholyMonLegacyKey *out);
bool32 UnholyMonIdentity_ReadMon(struct Pokemon *mon,
                                 struct UnholyMonLegacyKey *out);
bool32 UnholyMonIdentity_KeyEquals(const struct UnholyMonLegacyKey *a,
                                   const struct UnholyMonLegacyKey *b);

// No dynamic 420-entry list on the GBA: call Observe for every source slot
// (party, PC, daycare, fusions, trades, etc.). A duplicate fails closed.
void UnholyMonIdentity_ScanInit(struct UnholyMonIdentityScan *scan,
                                const struct UnholyMonLegacyKey *key);
void UnholyMonIdentity_ScanObserve(struct UnholyMonIdentityScan *scan,
                                   const struct UnholyMonLegacyKey *candidate);
enum UnholyMonIdentityStatus UnholyMonIdentity_ScanStatus(
    const struct UnholyMonIdentityScan *scan);

#endif // GUARD_UNHOLY_MON_IDENTITY_H
