#ifndef GUARD_CONFIG_GEARS_H
#define GUARD_CONFIG_GEARS_H

// Sistema de Gears (ver docs/GEARS.md). El Gear equipado se guarda en el Pokémon;
// el estado del ciclo sobrecarga/recarga solo existe durante el combate.

#define GEARS_ENABLED                   TRUE

// Posición del icono del Gear respecto a la caja de vida (píxeles). Queda en la
// esquina superior derecha de la caja. Si en el emulador no encaja, ajusta estos números.
#define GEAR_ICON_X_OFFSET_PLAYER       64
#define GEAR_ICON_X_OFFSET_OPPONENT     56
#define GEAR_ICON_Y_OFFSET              -20

#endif // GUARD_CONFIG_GEARS_H
