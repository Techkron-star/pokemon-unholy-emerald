#ifndef GUARD_BATTLE_CHAIN_H
#define GUARD_BATTLE_CHAIN_H

// Cada Pokémon resuelve su primera acción antes de los movimientos encadenados.
#define CHAIN_ENABLED   TRUE
#define CHAIN_FOLLOW_UP_DAMAGE_PERCENT 70

// ¿Puede este combatiente encadenar ahora mismo?
bool32 Chain_IsAvailable(enum BattlerId battler);

// ¿Este movimiento se puede encadenar? (los de protección no)
bool32 Chain_IsMoveChainable(enum Move move);
bool32 Chain_IsPairAllowed(enum BattlerId battler, enum Move first, enum Move second);
bool32 Chain_CanAfford(enum BattlerId battler, enum Move first, enum Move second);
bool32 Chain_AllowsResolvedCategory(enum BattlerId battler, enum Move move);
bool32 Chain_IsFollowingUp(enum BattlerId battler);
void Chain_RecordOpeningMove(enum BattlerId battler, enum Move move);
void Chain_InvalidateFollowUps(void);
void Chain_OrderFollowUps(void);

// Permite que el controlador rival prepare un segundo movimiento para este turno.
void Chain_SelectOpponentMove(enum BattlerId battler, u8 firstMovePos);

// Reconstruye el orden de turno cuando hay cadena (se llama tras calcular el orden normal).
void Chain_ExpandTurnOrder(void);

// Se llama al empezar cada acción de movimiento. Devuelve TRUE si la acción debe saltarse.
bool32 Chain_PrepareAction(enum BattlerId battler);

// Número de acciones del turno actual (normalmente gBattlersCount).
u32 Chain_GetNumTurnActions(void);

// Limpia el estado al final del turno.
void Chain_Reset(void);

// Cancela las acciones pendientes del turno encadenado si hay un KO.
void Chain_MarkKnockout(void);
bool32 Chain_WasInterrupted(void);

#endif // GUARD_BATTLE_CHAIN_H
