#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "unholy_exp_share.h"
#include "constants/flags.h"
#include "constants/vars.h"

// The low byte carries unused EXP. Bit 8 requests a hatch on the next safe field step.
#define OVERLOADED_EGG_READY (1u << 8)

enum Item UnholyExpShare_GetActiveItem(void)
{
    if (FlagGet(FLAG_UNHOLY_OVERLOADED_EXP_SHARE_ENABLED) && CheckBagHasItem(ITEM_OVERLOADED_EXP_SHARE, 1))
        return ITEM_OVERLOADED_EXP_SHARE;
    if (FlagGet(FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED) && CheckBagHasItem(ITEM_SUPER_EXP_SHARE, 1))
        return ITEM_SUPER_EXP_SHARE;
    return IsGen6ExpShareEnabled() ? ITEM_EXP_SHARE : ITEM_NONE;
}

static bool32 IsPartyEgg(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES)
        && GetMonData(mon, MON_DATA_IS_EGG)
        && !GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG);
}

void UnholyExpShare_AdvanceEggs(u32 experience)
{
    u16 state = VarGet(VAR_UNHOLY_EGG_EXP);
    u32 progress = experience + (state & (OVERLOADED_EXP_PER_EGG_CYCLE - 1));
    u32 cycles = progress / OVERLOADED_EXP_PER_EGG_CYCLE;
    bool32 needsProgress = FALSE;
    bool32 ready = FALSE;

    for (u8 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (!IsPartyEgg(mon))
            continue;
        u8 remaining = GetMonData(mon, MON_DATA_FRIENDSHIP);
        if (remaining != 0 && cycles != 0)
        {
            remaining -= min(remaining, cycles);
            SetMonData(mon, MON_DATA_FRIENDSHIP, &remaining);
            if (remaining == 0)
                ready = TRUE;
        }
        else if (remaining == 0 && (state & OVERLOADED_EGG_READY))
            ready = TRUE;
        needsProgress |= remaining != 0;
    }

    state = needsProgress ? progress % OVERLOADED_EXP_PER_EGG_CYCLE : 0;
    if (ready)
        state |= OVERLOADED_EGG_READY;
    VarSet(VAR_UNHOLY_EGG_EXP, state);
}

bool32 UnholyExpShare_FindReadyEgg(void)
{
    u16 state = VarGet(VAR_UNHOLY_EGG_EXP);
    if (!(state & OVERLOADED_EGG_READY))
        return FALSE;
    for (u8 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (IsPartyEgg(mon) && GetMonData(mon, MON_DATA_FRIENDSHIP) == 0)
        {
            gSpecialVar_0x8004 = i;
            return TRUE;
        }
    }
    VarSet(VAR_UNHOLY_EGG_EXP, state & ~OVERLOADED_EGG_READY);
    return FALSE;
}

void UnholyExpShare_TrainIvs(struct Pokemon *mon, u8 previousLevel)
{
    if (GetMonData(mon, MON_DATA_SPECIAL_BADGES) & (POKEMON_BADGE_ASCENDED | POKEMON_BADGE_OMEGA))
        return;

    u8 level = GetLevelFromMonExp(mon);
    while (previousLevel < level)
    {
        u8 lowest = MAX_PER_STAT_IVS;
        enum Stat selected = NUM_STATS;
        for (enum Stat stat = STAT_HP; stat < NUM_STATS; stat++)
        {
            u8 value = GetMonData(mon, MON_DATA_HP_IV + stat);
            if (value < lowest && !GetMonData(mon, MON_DATA_HYPER_TRAINED_HP + stat))
            {
                lowest = value;
                selected = stat;
            }
        }
        if (selected == NUM_STATS)
            break;
        lowest++;
        SetMonData(mon, MON_DATA_HP_IV + selected, &lowest);
        previousLevel++;
    }
}
