#include "global.h"
#include "event_data.h"
#include "unholy_world_update.h"
#include "constants/flags.h"
#include "constants/vars.h"

#define UNHOLY_WORLD_VERSION_LATEST 1

#if GAME_VERSION == VERSION_EMERALD
static void ReconcileWorldEvent(u16 hideFlag, bool8 shouldAppear)
{
    if (shouldAppear)
        FlagClear(hideFlag);
    else
        FlagSet(hideFlag);
}

static void ReconcileClaimableWorldEvent(u16 hideFlag, u16 claimedFlag, bool8 unlocked)
{
    // Do not clear a picked-up item's hide flag on a later update or visit.
    ReconcileWorldEvent(hideFlag, unlocked && !FlagGet(claimedFlag));
}
#endif

void UnholyWorldUpdate_Reconcile(void)
{
#if GAME_VERSION == VERSION_EMERALD
    u16 version = VarGet(VAR_UNHOLY_WORLD_VERSION);

    // Future migrations go here in ascending version order. Upgrading from a
    // legacy save does not clear or reassign any existing reward flags.
    if (version < UNHOLY_WORLD_VERSION_LATEST)
        VarSet(VAR_UNHOLY_WORLD_VERSION, UNHOLY_WORLD_VERSION_LATEST);

    // Map-event hide flags are synchronized before object templates spawn.
    // The claimed flags are separate, persistent and never cleared here.
    ReconcileClaimableWorldEvent(FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE,
                                FLAG_UNHOLY_WORLD_ROUTE102_BALL_CLAIMED,
                                FlagGet(FLAG_BADGE01_GET));
    ReconcileWorldEvent(FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE, FlagGet(FLAG_BADGE02_GET));
    ReconcileClaimableWorldEvent(FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE,
                                FLAG_UNHOLY_WORLD_ROUTE104_BALL_CLAIMED,
                                FlagGet(FLAG_BADGE03_GET));
    ReconcileWorldEvent(FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE, FlagGet(FLAG_SYS_GAME_CLEAR));
#endif
}
