#ifndef GUARD_CONFIG_STAMINA_H
#define GUARD_CONFIG_STAMINA_H

// Sistema de stamina (solo existe durante el combate; no se guarda en la partida).
// Todos los valores están aquí para poder ajustar el balance sin tocar la lógica.

#define STAMINA_ENABLED                 TRUE

// Stamina máxima = BASE + nivel / LEVEL_DIVISOR + EV de HP / EV_DIVISOR + IV de HP / IV_DIVISOR
#define STAMINA_MAX_BASE                40
#define STAMINA_MAX_LEVEL_DIVISOR       2
#define STAMINA_MAX_EV_DIVISOR          8   // 0-252 EV  -> 0-31
#define STAMINA_MAX_IV_DIVISOR          2   // 0-31 IV   -> 0-15

// El costo escala con la stamina máxima del Pokémon, de forma que subir
// de nivel jamás vuelve gratuito un movimiento. Fórmula:
//   coste = ceil(maxStamina * (BASE_PERCENT
//                       + floor(potencia * precision / 100 / POWER_DIVISOR)) / 100)
// Los movimientos de estado y daño variable usan solo BASE_PERCENT.
// Ejemplos: potencia 40/100 = 18%; 100/100 = 24%; 150/90 = 27%.
// Los movimientos infalibles cuentan como 100% de precisión.
#define STAMINA_COST_BASE_PERCENT      14
#define STAMINA_COST_POWER_DIVISOR     10

// Recarga al final de cada turno, en % de la stamina máxima.
#define STAMINA_REGEN_PERCENT           20

// Si la stamina llega a 0, el Pokémon queda agotado (pierde el turno, sin Struggle)
// hasta que su stamina vuelva a este % del máximo.
#define STAMINA_RECOVER_PERCENT         25

// Habilidades de "snowball" (Moxie, Beast Boost, Chilling/Grim Neigh...): al noquear a un
// Pokémon restauran este % de la stamina máxima por cada KO, además de su subida de stat.
#define STAMINA_KO_RESTORE_PERCENT      30

// Offsets from the centre of the left healthbox OBJ. The tall player box
// has a gap above status and EXP. The other boxes use the nickname row.
#define STAMINA_RING_PLAYER_SINGLE_X_OFFSET (-14)
#define STAMINA_RING_PLAYER_SINGLE_Y_OFFSET  3
#define STAMINA_RING_PLAYER_DOUBLE_X_OFFSET (-12)
#define STAMINA_RING_OPP_X_OFFSET           (-20)
#define STAMINA_RING_NICKNAME_Y_OFFSET      (-7)
#define STAMINA_RING_NICKNAME_PADDING       10
#define STAMINA_RING_SEGMENTS           64

#endif // GUARD_CONFIG_STAMINA_H
