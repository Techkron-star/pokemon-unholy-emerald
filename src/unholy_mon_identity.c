#include "global.h"
#include "pokemon.h"
#include "unholy_mon_identity.h"

bool32 UnholyMonIdentity_ReadBox(const struct BoxPokemon *mon,
                                 struct UnholyMonLegacyKey *out)
{
    if (mon == NULL || out == NULL || !mon->hasSpecies || mon->isBadEgg)
        return FALSE;

    // The species getter can mark an invalid checksum as a Bad Egg.
    struct BoxPokemon copy = *mon;
    u32 species = GetBoxMonData(&copy, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;

    *out = (struct UnholyMonLegacyKey){ .personality = mon->personality, .otId = mon->otId };
    return TRUE;
}

bool32 UnholyMonIdentity_ReadMon(const struct Pokemon *mon,
                                 struct UnholyMonLegacyKey *out)
{
    if (mon == NULL)
        return FALSE;
    return UnholyMonIdentity_ReadBox(&mon->box, out);
}

bool32 UnholyMonIdentity_KeyEquals(const struct UnholyMonLegacyKey *a,
                                   const struct UnholyMonLegacyKey *b)
{
    return a != NULL && b != NULL
        && a->personality == b->personality
        && a->otId == b->otId;
}

void UnholyMonIdentity_ScanInit(struct UnholyMonIdentityScan *scan,
                                const struct UnholyMonLegacyKey *key)
{
    if (scan == NULL)
        return;

    memset(scan, 0, sizeof(*scan));
    if (key != NULL)
    {
        scan->key = *key;
        scan->valid = TRUE;
    }
}

void UnholyMonIdentity_ScanObserve(struct UnholyMonIdentityScan *scan,
                                   const struct UnholyMonLegacyKey *candidate)
{
    if (scan == NULL || candidate == NULL || !scan->valid)
        return;

    if (UnholyMonIdentity_KeyEquals(&scan->key, candidate)
     && scan->matches < 2)
        scan->matches++; // Saturating count; unique vs. ambiguous is enough.
}

enum UnholyMonIdentityStatus UnholyMonIdentity_ScanStatus(
    const struct UnholyMonIdentityScan *scan)
{
    if (scan == NULL || !scan->valid)
        return UNHOLY_MON_ID_INVALID;
    if (scan->matches == 0)
        return UNHOLY_MON_ID_MISSING;
    if (scan->matches == 1)
        return UNHOLY_MON_ID_UNIQUE;
    return UNHOLY_MON_ID_AMBIGUOUS;
}
