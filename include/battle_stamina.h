#ifndef GUARD_BATTLE_STAMINA_H
#define GUARD_BATTLE_STAMINA_H

#include "config/stamina.h"

struct Pokemon;

// Pure maximum-stamina calculation shared by battle and summary screens.
// Does not read gBattleStruct or initialize a battler's battle state.
u32 Stamina_CalcMaxForMon(struct Pokemon *mon);

// Stamina actual y máxima del combatiente (se inicializa sola al entrar al campo).
u32 Stamina_GetCurrent(enum BattlerId battler);
u32 Stamina_GetMax(enum BattlerId battler);
bool32 Stamina_IsExhausted(enum BattlerId battler);

// Costo de usar un movimiento.
u32 Stamina_GetMoveCost(enum BattlerId battler, enum Move move);

// Se llama cuando un movimiento se ejecuta (junto con la deducción de PP).
void Stamina_SpendForMove(enum BattlerId battler, enum Move move);

// Habilidades de snowball: restaura stamina al noquear (numKos = Pokémon noqueados).
void Stamina_RestoreOnKnockout(enum BattlerId battler, u32 numKos);

// Se llama al final de cada turno: recarga y gestiona el agotamiento.
void Stamina_EndOfTurn(enum BattlerId battler);

#endif // GUARD_BATTLE_STAMINA_H
