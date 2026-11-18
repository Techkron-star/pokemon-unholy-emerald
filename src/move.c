#include "global.h"
#include "battle.h"
#include "main.h"
#include "move.h"
#include "unholy_localization.h"

#include "data/moves_info.h"

struct MoveTranslation
{
    const u8 *name;
    const u8 *description;
};

// All regular, Z, Dynamax and Gigantamax moves share the same localized catalog.
static const struct MoveTranslation sSpanishMoveTranslations[MOVES_COUNT_ALL] =
{
#include "data/move_localization_es_gen1.h"
#include "data/move_localization_es_gen2.h"
#include "data/move_localization_es_gen3.h"
#include "data/move_localization_es_gen4.h"
#include "data/move_localization_es_gen5.h"
#include "data/move_localization_es_gen6.h"
#include "data/move_localization_es_gen7.h"
#include "data/move_localization_es_gen8.h"
#include "data/move_localization_es_gen9.h"
#include "data/move_localization_es_special.h"
};

const u8 *GetLocalizedMoveName(enum Move moveId)
{
    if (UnholyIsSpanish() && moveId < ARRAY_COUNT(sSpanishMoveTranslations)
     && sSpanishMoveTranslations[moveId].name != NULL)
        return sSpanishMoveTranslations[moveId].name;
    return gMovesInfo[moveId].name;
}

const u8 *GetLocalizedMoveDescription(enum Move moveId)
{
    if (UnholyIsSpanish() && moveId < ARRAY_COUNT(sSpanishMoveTranslations)
     && sSpanishMoveTranslations[moveId].description != NULL)
        return sSpanishMoveTranslations[moveId].description;
    return gMovesInfo[moveId].description;
}
