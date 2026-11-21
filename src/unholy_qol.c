#include "global.h"
#include "event_data.h"
#include "menu.h"
#include "script_menu.h"
#include "unholy_qol.h"
#include "constants/vars.h"

#define QOL_FLAGS_INITIALIZED (1 << 15)
#define QOL_DEFAULT_FLAGS (1 << UQOL_AUTO_RUN)

// Names are kept short enough for the GBA overworld menu.
static const u8 sText_QolRepelOn[] = _("Repel auto: ON");
static const u8 sText_QolRepelOff[] = _("Repel auto: OFF");
static const u8 sText_QolRunOn[] = _("Autorun: ON");
static const u8 sText_QolRunOff[] = _("Autorun: OFF");
static const u8 sText_QolCutOn[] = _("Auto Cut: ON");
static const u8 sText_QolCutOff[] = _("Auto Cut: OFF");
static const u8 sText_QolStrengthOn[] = _("Auto Strength: ON");
static const u8 sText_QolStrengthOff[] = _("Auto Strength: OFF");
static const u8 sText_QolSmashOn[] = _("Auto Smash: ON");
static const u8 sText_QolSmashOff[] = _("Auto Smash: OFF");
static const u8 sText_QolSurfOn[] = _("Auto Surf: ON");
static const u8 sText_QolSurfOff[] = _("Auto Surf: OFF");
static const u8 sText_QolTextOn[] = _("Auto Text: ON");
static const u8 sText_QolTextOff[] = _("Auto Text: OFF");
static const u8 sText_QolWorldUpdate[] = _("Actualizar mundo");

static const u8 *const sQolLabels[UQOL_OPTION_COUNT][2] =
{
    [UQOL_AUTO_REPEL]      = {sText_QolRepelOff, sText_QolRepelOn},
    [UQOL_AUTO_RUN]        = {sText_QolRunOff, sText_QolRunOn},
    [UQOL_AUTO_CUT]        = {sText_QolCutOff, sText_QolCutOn},
    [UQOL_AUTO_STRENGTH]   = {sText_QolStrengthOff, sText_QolStrengthOn},
    [UQOL_AUTO_ROCK_SMASH] = {sText_QolSmashOff, sText_QolSmashOn},
    [UQOL_AUTO_SURF]       = {sText_QolSurfOff, sText_QolSurfOn},
    [UQOL_AUTO_DIALOGUE]   = {sText_QolTextOff, sText_QolTextOn},
};

static EWRAM_DATA u8 sQolMenuCursor = 0;

static u16 GetQolFlags(void)
{
    u16 flags = VarGet(VAR_UNHOLY_QOL_FLAGS);

    // Preserve the previous fixed-on Autorun behavior for existing saves.
    return (flags & QOL_FLAGS_INITIALIZED) ? flags : QOL_DEFAULT_FLAGS;
}

bool32 UnholyQolIsEnabled(enum UnholyQolOption option)
{
    if (option >= UQOL_OPTION_COUNT)
        return FALSE;
    if (option == UQOL_AUTO_REPEL)
        return VarGet(VAR_REPEL_AUTO_RENEW) == 2;

    return (GetQolFlags() & (1 << option)) != 0;
}

void UnholyQolToggleOption(enum UnholyQolOption option)
{
    if (option >= UQOL_OPTION_COUNT)
        return;

    if (option == UQOL_AUTO_REPEL)
    {
        // 1 means disabled/previously answered; 2 enables automatic renewal.
        VarSet(VAR_REPEL_AUTO_RENEW, UnholyQolIsEnabled(option) ? 1 : 2);
    }
    else
    {
        VarSet(VAR_UNHOLY_QOL_FLAGS, (GetQolFlags() ^ (1 << option)) | QOL_FLAGS_INITIALIZED);
    }
}

void UnholyQolDrawMenu(struct ScriptContext *ctx)
{
    struct MenuAction menuItems[UQOL_OPTION_COUNT + 1] = {0};
    u32 i;

    for (i = 0; i < UQOL_OPTION_COUNT; i++)
        menuItems[i].text = sQolLabels[i][UnholyQolIsEnabled(i)];
    menuItems[UQOL_OPTION_COUNT].text = sText_QolWorldUpdate;

    DrawMultichoiceMenuInternal(1, 1, 0, FALSE, sQolMenuCursor, menuItems, UQOL_OPTION_COUNT + 1);
}

void UnholyQolToggleSelection(struct ScriptContext *ctx)
{
    u16 choice = gSpecialVar_Result;

    if (choice >= UQOL_OPTION_COUNT)
        return;
    sQolMenuCursor = choice;
    UnholyQolToggleOption(choice);
}

u32 UnholyQolGetScriptSetting(void)
{
    return UnholyQolIsEnabled(gSpecialVar_0x8000);
}
