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
    GEAR_BLOOD_NORMAL,
    GEAR_BLOOD_FIRE,
    GEAR_BLOOD_WATER,
    GEAR_BLOOD_ELECTRIC,
    GEAR_BLOOD_GRASS,
    GEAR_BLOOD_ICE,
    GEAR_BLOOD_FIGHTING,
    GEAR_BLOOD_POISON,
    GEAR_BLOOD_GROUND,
    GEAR_BLOOD_FLYING,
    GEAR_BLOOD_PSYCHIC,
    GEAR_BLOOD_BUG,
    GEAR_BLOOD_ROCK,
    GEAR_BLOOD_GHOST,
    GEAR_BLOOD_DRAGON,
    GEAR_BLOOD_DARK,
    GEAR_BLOOD_STEEL,
    GEAR_BLOOD_FAIRY,
    GEAR_STORM_STAFF,
    GEAR_SHACKLE,
    GEAR_ESCUDO_AEGIS,
    GEAR_ARIETE_NAUTILO,
    GEAR_GUANTES_COHETE,
    GEAR_TUNICA_RUNICA,
    GEAR_CARTEL_BUSQUEDA,
    GEAR_NIHIL_ENGINE,
    GEAR_ANILLO_FEERICO,
    GEAR_PIEDRA_AFILAR,
    GEAR_BATERIA_RESERVA,
    GEAR_BOTAS_UMBRAL,
    GEAR_VELO_CRIPTICO,
    GEAR_AMULETO_INQUEBRANTABLE,
    GEAR_CRISOL_ESPIRITUAL,
    GEAR_LENTE_CERTEZA,
    GEAR_MODULO_VECTORIAL,
    GEAR_AMPLIFICADOR,
    GEAR_HEADPHONES,
    GEAR_BOTON_PANICO,
    GEAR_NUCLEO_HYDRA,
    GEAR_CASCO_HOPLITA,
    GEAR_COUNT
};

#endif // GUARD_CONSTANTS_GEARS_H
