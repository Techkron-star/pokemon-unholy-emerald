#ifndef GUARD_UNHOLY_QOL_H
#define GUARD_UNHOLY_QOL_H

#include "global.h"
#include "constants/field_move.h"

struct ScriptContext;

enum UnholyQolOption
{
    UQOL_AUTO_REPEL,
    UQOL_AUTO_RUN,
    UQOL_AUTO_CUT,
    UQOL_AUTO_STRENGTH,
    UQOL_AUTO_ROCK_SMASH,
    UQOL_AUTO_SURF,
    UQOL_AUTO_DIALOGUE,
    UQOL_GENERAL_OPTION_COUNT,
    UQOL_INFINITE_REPEL = UQOL_GENERAL_OPTION_COUNT,
    UQOL_AUTO_FLASH,
    UQOL_AUTO_HEAL,
    UQOL_OPTION_COUNT,
};

bool32 UnholyQolIsEnabled(enum UnholyQolOption option);
bool32 UnholyQolAutoFieldMoveAvailable(enum FieldMove fieldMove);
void UnholyQolToggleOption(enum UnholyQolOption option);
void UnholyQolDrawMenu(struct ScriptContext *ctx);
void UnholyQolToggleSelection(struct ScriptContext *ctx);
u32 UnholyQolGetScriptSetting(void);
void UnholyQolAutoHealAfterBattle(void);

#endif // GUARD_UNHOLY_QOL_H
