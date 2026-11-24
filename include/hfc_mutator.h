#ifndef GUARD_HFC_MUTATOR_H
#define GUARD_HFC_MUTATOR_H

#include "pokemon.h"

enum HfcMutatorPlan
{
    HFC_MUTATOR_BASIC,
    HFC_MUTATOR_ENHANCED,
    HFC_MUTATOR_INTENSIVE,
    HFC_MUTATOR_PLAN_COUNT,
};

enum HfcMutatorResult
{
    HFC_MUTATOR_READY,
    HFC_MUTATOR_INVALID_MON,
    HFC_MUTATOR_EGG,
    HFC_MUTATOR_OMEGA,
    HFC_MUTATOR_NEEDS_SUPER_LEAGUE,
    HFC_MUTATOR_NO_ALTERNATIVE,
    HFC_MUTATOR_INVALID_PLAN,
    HFC_MUTATOR_INSUFFICIENT_FUNDS,
    HFC_MUTATOR_ROLL_FAILED,
    HFC_MUTATOR_APPLIED,
};

enum HfcMutatorResult HfcMutator_Attempt(struct Pokemon *mon, enum HfcMutatorPlan plan,
                                        bool32 canReroll, u32 *money, enum PokemonBadge *mutation);
void HfcMutator_Open(void (*returnToWatch)(void));

#endif // GUARD_HFC_MUTATOR_H
