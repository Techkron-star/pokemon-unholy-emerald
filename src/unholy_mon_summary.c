#include "global.h"
#include "pokemon.h"
#include "battle_stamina.h"
#include "unholy_mon_summary.h"

bool32 UnholyMonSummary_Build(struct Pokemon *mon, struct UnholyMonSummary *out)
{
    struct UnholyMonSummary summary = {0};

    if (mon == NULL || out == NULL)
        return FALSE;

    summary.species = GetMonData(mon, MON_DATA_SPECIES);
    if (summary.species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;

    summary.level = GetMonData(mon, MON_DATA_LEVEL);
    summary.mutationBadges = GetMonData(mon, MON_DATA_SPECIAL_BADGES);
    summary.gearId = GetMonData(mon, MON_DATA_GEAR);
    summary.staminaMax = Stamina_CalcMaxForMon(mon);

    for (u32 i = 0; i < NUM_STATS; i++)
    {
        summary.stats[i] = GetMonData(mon, MON_DATA_MAX_HP + i);
        summary.ivs[i] = GetMonData(mon, MON_DATA_HP_IV + i);
        summary.evs[i] = GetMonData(mon, MON_DATA_HP_EV + i);
        summary.mutationBonusPercent[i] = Pokemon_GetMutationStatBonusPercent(mon, i);
    }

    *out = summary;
    return TRUE;
}
