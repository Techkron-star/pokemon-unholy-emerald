#include "global.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fldeff.h"
#include "follower_npc.h"
#include "party_menu.h"
#include "overworld.h"
#include "task.h"
#include "constants/field_effects.h"

static void StartTeleportFieldEffect(void);

bool32 SetUpFieldMove_Teleport(void)
{
    if (!CheckFollowerNPCFlag(FOLLOWER_NPC_FLAG_CAN_LEAVE_ROUTE))
        return FALSE;

    // The party menu now routes Teleport through the Fly destination picker.
    // Do not install the legacy field callback: it would warp to the previous
    // healing spot after a destination was selected or the map was cancelled.
    return Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType);
}

bool8 FldEff_UseTeleport(void)
{
    u8 taskId = CreateFieldMoveTask();
    gTasks[taskId].data[8] = (u32)StartTeleportFieldEffect >> 16;
    gTasks[taskId].data[9] = (u32)StartTeleportFieldEffect;
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
    return FALSE;
}

static void StartTeleportFieldEffect(void)
{
    FieldEffectActiveListRemove(FLDEFF_USE_TELEPORT);
    FldEff_TeleportWarpOut();
}
