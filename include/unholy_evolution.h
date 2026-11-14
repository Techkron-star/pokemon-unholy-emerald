#ifndef GUARD_UNHOLY_EVOLUTION_H
#define GUARD_UNHOLY_EVOLUTION_H

#include "pokemon.h"

#define UNHOLY_EVO_MAX_COSTS 4

enum UnholyEvolutionStatus
{
    UNHOLY_EVO_READY,
    UNHOLY_EVO_LEVEL,
    UNHOLY_EVO_ITEM,
    UNHOLY_EVO_CONDITION,
    UNHOLY_EVO_EVERSTONE,
    UNHOLY_EVO_SPECIAL,
    UNHOLY_EVO_PREVIEW,
    UNHOLY_EVO_INVALID,
    UNHOLY_EVO_CHANGED,
};

struct UnholyEvolutionCost
{
    u16 item;
    u16 count;
    bool8 held;
};

struct UnholyEvolutionOption
{
    u16 source, target, entry;
    u16 minimumLevel, missingCondition;
    u8 status, costCount;
    struct UnholyEvolutionCost costs[UNHOLY_EVO_MAX_COSTS];
};

struct UnholyEvolutionPlan
{
    struct Pokemon before;
    struct UnholyEvolutionOption option;
    u8 partySlot;
    bool8 consumed;
};

// Read-only queries. Each target appears once, using its best available recipe.
u16 UnholyEvolution_Count(enum Species species);
bool32 UnholyEvolution_GetOption(struct Pokemon *mon, enum Species viewSpecies, u16 ordinal,
                                u8 partySlot, struct UnholyEvolutionOption *out);
u16 UnholyEvolution_RequirementCount(const struct UnholyEvolutionOption *option);
void UnholyEvolution_RequirementText(const struct UnholyEvolutionOption *option, u16 line, u8 *text);
void UnholyEvolution_StatusText(const struct UnholyEvolutionOption *option, u8 *text);
u8 UnholyEvolution_Prepare(u8 partySlot, u16 entry, struct UnholyEvolutionPlan *plan);
// Only consumes the confirmed recipe. The caller starts the native evolution scene.
u8 UnholyEvolution_Consume(struct UnholyEvolutionPlan *plan);

#endif
