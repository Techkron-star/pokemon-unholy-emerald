#ifndef GUARD_CONSTANTS_GEARS_H
#define GUARD_CONSTANTS_GEARS_H

// ID del Gear equipado en cada Pokémon. Ahora se guarda en 8 bits (0-255)
// repartidos entre los 5 bits originales y 3 bits antes sin usar.
// Los antiguos Gear IDs (0-31) mantienen su representación binaria.
//
// Para añadir un Gear nuevo: pon su constante justo ANTES de GEAR_COUNT y NO
// reordenes ni borres las anteriores (los IDs ya están en las partidas guardadas).
// Guía completa: docs/GEARS.md
#define GEAR_MAX_COUNT 256

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
