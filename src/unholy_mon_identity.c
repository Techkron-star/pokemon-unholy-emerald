#include "global.h"
#include "pokemon.h"
#include "unholy_mon_identity.h"

bool32 UnholyMonIdentity_ReadBox(struct BoxPokemon *mon,
                                 struct UnholyMonLegacyKey *out)
{
    struct UnholyMonLegacyKey result;

    if (mon == NULL || out == NULL
     || GetBoxMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;

    result.personality = GetBoxMonData(mon, MON_DATA_PERSONALITY);
    result.otId = GetBoxMonData(mon, MON_DATA_OT_ID);
    *out = result;
    return TRUE;
}

bool32 UnholyMonIdentity_ReadMon(struct Pokemon *mon,
                                 struct UnholyMonLegacyKey *out)
{
    if (mon == NULL || out == NULL)
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
        scan->key = *key;
}

void UnholyMonIdentity_ScanObserve(struct UnholyMonIdentityScan *scan,
                                   const struct UnholyMonLegacyKey *candidate)
{
    if (scan == NULL || candidate == NULL)
        return;

    if (UnholyMonIdentity_KeyEquals(&scan->key, candidate)
     && scan->matches < 2)
        scan->matches++; // Saturating count; unique vs. ambiguous is enough.
}

enum UnholyMonIdentityStatus UnholyMonIdentity_ScanStatus(
    const struct UnholyMonIdentityScan *scan)
{
    if (scan == NULL)
        return UNHOLY_MON_ID_INVALID;
    if (scan->matches == 0)
        return UNHOLY_MON_ID_MISSING;
    if (scan->matches == 1)
        return UNHOLY_MON_ID_UNIQUE;
    return UNHOLY_MON_ID_AMBIGUOUS;
}
