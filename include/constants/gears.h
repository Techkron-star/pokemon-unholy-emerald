#ifndef GUARD_CONSTANTS_GEARS_H
#define GUARD_CONSTANTS_GEARS_H

// ID del Gear equipado en cada Pokémon. Se guarda en 5 bits (0-31), así que
// el sistema admite como máximo GEAR_MAX_COUNT Gears (GEAR_NONE incluido).
//
// Para añadir un Gear nuevo: pon su constante justo ANTES de GEAR_COUNT y NO
// reordenes ni borres las anteriores (los IDs ya están en las partidas guardadas).
// Guía completa: docs/GEARS.md
#define GEAR_MAX_COUNT 32

enum Gear
{
    GEAR_NONE,
    // Valores persistidos: no reordenar tras publicar partidas guardadas.
    GEAR_VISOR_COSMICO,
    GEAR_CORAZON_TITAN,
    GEAR_SKI_MASK,
    GEAR_CETRO_SINIESTRO,
    GEAR_IMPULSO_COSMICO,
    GEAR_PERMAFROST_FRAGMENT,
    GEAR_COUNT
};

#endif // GUARD_CONSTANTS_GEARS_H
