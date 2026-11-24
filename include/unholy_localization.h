#ifndef GUARD_UNHOLY_LOCALIZATION_H
#define GUARD_UNHOLY_LOCALIZATION_H

#include "global.h"

// Display language is deliberately independent of GAME_LANGUAGE, which labels
// Pokémon origin and is relevant to trades, Pokédex data and link records.
static inline bool8 UnholyIsSpanish(void)
{
    return gSaveBlock2Ptr != NULL && gSaveBlock2Ptr->optionsLanguageSpanish;
}

static inline const u8 *UnholyLocalize(const u8 *english, const u8 *spanish)
{
    return UnholyIsSpanish() ? spanish : english;
}

#endif // GUARD_UNHOLY_LOCALIZATION_H
