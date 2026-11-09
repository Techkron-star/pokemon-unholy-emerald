#ifndef GUARD_BATTLE_CHAIN_H
#define GUARD_BATTLE_CHAIN_H

// Encadenado de movimientos: un combatiente puede usar 2 movimientos distintos en un turno
// a cambio de que el rival actúe una vez extra. Solo en combates individuales y sin enlace.
#define CHAIN_ENABLED   TRUE

// ¿Puede este combatiente encadenar ahora mismo?
bool32 Chain_IsAvailable(enum BattlerId battler);

// ¿Este movimiento se puede encadenar? (los de protección no)
bool32 Chain_IsMoveChainable(enum Move move);

// Permite que el controlador rival prepare un segundo movimiento para este turno.
void Chain_SelectOpponentMove(enum BattlerId battler, u8 firstMovePos);

// Reconstruye el orden de turno cuando hay cadena (se llama tras calcular el orden normal).
void Chain_ExpandTurnOrder(void);

// Se llama al empezar cada acción de movimiento. Devuelve TRUE si la acción debe saltarse.
bool32 Chain_PrepareAction(enum BattlerId battler, u32 turnActionIndex);

// Número de acciones del turno actual (normalmente gBattlersCount).
u32 Chain_GetNumTurnActions(void);

// Limpia el estado al final del turno.
void Chain_Reset(void);

// Cancela las acciones pendientes del turno encadenado si hay un KO.
void Chain_MarkKnockout(void);
bool32 Chain_WasInterrupted(void);

#endif // GUARD_BATTLE_CHAIN_H
