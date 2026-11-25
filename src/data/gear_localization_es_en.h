// Textos visibles de los 47 artefactos implementados. Se incluye solo desde
// gear.c; los identificadores guardados y las mecánicas no cambian.
// Cada descripción ocupa como máximo dos líneas en el resumen compacto.

struct GearLocalizedText
{
    const u8 *spanishName;
    const u8 *englishName;
    const u8 *spanishDescription;
    const u8 *englishDescription;
};

#define GEAR_LOCALE(id, esName, enName, esDescription, enDescription) \
    [GEAR_##id] = { \
        .spanishName = COMPOUND_STRING_SIZE_LIMIT(esName, ITEM_NAME_LENGTH), \
        .englishName = COMPOUND_STRING_SIZE_LIMIT(enName, ITEM_NAME_LENGTH), \
        .spanishDescription = COMPOUND_STRING(esDescription), \
        .englishDescription = COMPOUND_STRING(enDescription), \
    },

#define GEAR_BLOOD_LOCALE(id, esName, enName, esType, enType) \
    GEAR_LOCALE(BLOOD_##id, esName, enName, \
                "1.er golpe: +40%;\nrecarga: +12% x3.", \
                "First hit: +40%;\nrecharge: +12% x3.")

static const struct GearLocalizedText sGearLocalizedText[GEAR_COUNT] =
{
    GEAR_LOCALE(VISOR_COSMICO, "Visor Cósmico", "Cosmic Visor",
        "Fantasma x2 ante Normal,\nPsíquico y Hada.",
        "Ghost x2 vs. Normal,\nPsychic and Fairy.")
    GEAR_LOCALE(CORAZON_TITAN, "Corazón Titán", "Titan Heart",
        "Daño físico: +20%;\n+10% por 3 turnos.",
        "Physical damage: +20%;\n+10% for 3 turns.")
    GEAR_LOCALE(SKI_MASK, "Máscara de Esquí", "Ski Mask",
        "Siniestro: +30%;\n+21% por 3 turnos.",
        "Dark moves: +30%;\n+21% for 3 turns.")
    GEAR_LOCALE(CETRO_SINIESTRO, "Cetro Siniestro", "Dark Scepter",
        "Siniestro x2 ante Normal,\nLucha, Psíquico y Hada.",
        "Dark x2 vs. Normal,\nFighting, Psychic, Fairy.")
    GEAR_LOCALE(IMPULSO_COSMICO, "Impulso Cósmico", "Cosmic Drive",
        "Daño especial: +20%;\n+10% por 3 turnos.",
        "Special damage: +20%;\n+10% for 3 turns.")
    GEAR_LOCALE(PERMAFROST_FRAGMENT, "Fragmento Glacial", "Permafrost Shard",
        "Ataques Hielo: x2 cong.\nTipo Hielo: +1/16 PS.",
        "Ice attacks: x2 freeze.\nIce type: heals 1/16 HP.")

    // 18 tipos: el porcentaje es por ataque compatible; la recarga dura 3 turnos.
    GEAR_BLOOD_LOCALE(NORMAL, "Inyector Normal", "Normal Injector", "Normal", "Normal")
    GEAR_BLOOD_LOCALE(FIRE, "Inyector Fuego", "Fire Injector", "Fuego", "Fire")
    GEAR_BLOOD_LOCALE(WATER, "Inyector Agua", "Water Injector", "Agua", "Water")
    GEAR_BLOOD_LOCALE(ELECTRIC, "Inyector Eléctrico", "Electric Injector", "Eléctrico", "Electric")
    GEAR_BLOOD_LOCALE(GRASS, "Inyector Planta", "Grass Injector", "Planta", "Grass")
    GEAR_BLOOD_LOCALE(ICE, "Inyector Hielo", "Ice Injector", "Hielo", "Ice")
    GEAR_BLOOD_LOCALE(FIGHTING, "Inyector Lucha", "Fighting Injector", "Lucha", "Fighting")
    GEAR_BLOOD_LOCALE(POISON, "Inyector Veneno", "Poison Injector", "Veneno", "Poison")
    GEAR_BLOOD_LOCALE(GROUND, "Inyector Tierra", "Ground Injector", "Tierra", "Ground")
    GEAR_BLOOD_LOCALE(FLYING, "Inyector Volador", "Flying Injector", "Volador", "Flying")
    GEAR_BLOOD_LOCALE(PSYCHIC, "Inyector Psíquico", "Psychic Injector", "Psíquico", "Psychic")
    GEAR_BLOOD_LOCALE(BUG, "Inyector Bicho", "Bug Injector", "Bicho", "Bug")
    GEAR_BLOOD_LOCALE(ROCK, "Inyector Roca", "Rock Injector", "Roca", "Rock")
    GEAR_BLOOD_LOCALE(GHOST, "Inyector Fantasma", "Ghost Injector", "Fantasma", "Ghost")
    GEAR_BLOOD_LOCALE(DRAGON, "Inyector Dragón", "Dragon Injector", "Dragón", "Dragon")
    GEAR_BLOOD_LOCALE(DARK, "Inyector Siniestro", "Dark Injector", "Siniestro", "Dark")
    GEAR_BLOOD_LOCALE(STEEL, "Inyector Acero", "Steel Injector", "Acero", "Steel")
    GEAR_BLOOD_LOCALE(FAIRY, "Inyector Hada", "Fairy Injector", "Hada", "Fairy")

    GEAR_LOCALE(STORM_STAFF, "Báculo Tormentoso", "Storm Staff",
        "Repite ataque especial\nal 25%, sin efectos.",
        "Repeats special attacks\nat 25%, no added effects.")
    GEAR_LOCALE(SHACKLE, "Grillete", "Shackle",
        "Golpes dañinos: 20%\nde impedir otra cadena.",
        "Damaging hits: 20%\nto block an enemy chain.")
    GEAR_LOCALE(ESCUDO_AEGIS, "Escudo de Aegis", "Aegis Shield",
        "Cada mejora de ambas\nDefensas cuenta doble.",
        "Each positive Defense\nstage counts twice.")
    GEAR_LOCALE(ARIETE_NAUTILO, "Ariete Nautilo", "Nautilus Ram",
        "Contacto con daño:\nDefensa rival -1.",
        "Damaging contact:\ntarget Defense -1.")
    GEAR_LOCALE(GUANTES_COHETE, "Guantes Cohete", "Rocket Gloves",
        "Puñetazos: prioridad +1.",
        "Punch moves: +1 priority.")
    GEAR_LOCALE(TUNICA_RUNICA, "Túnica Rúnica", "Runic Robe",
        "Evita daño por clima\ny trampas de entrada.",
        "Blocks weather damage\nand entry hazards.")
    GEAR_LOCALE(CARTEL_BUSQUEDA, "Cartel de Búsqueda", "Wanted Poster",
        "Daño +5% por mejora\nrival; máximo +60%.",
        "+5% damage per foe's\nstat boost; max +60%.")
    GEAR_LOCALE(NIHIL_ENGINE, "Motor Nihil", "Nihil Engine",
        "Quita una trampa rival\ncada 3 turnos activos.",
        "Clears one enemy hazard\nevery 3 active turns.")
    GEAR_LOCALE(ANILLO_FEERICO, "Anillo Feérico", "Fairy Ring",
        "Hada: reduce 25% del\ndaño directo de Veneno.",
        "Fairy: takes 25% less\ndirect Poison damage.")
    GEAR_LOCALE(PIEDRA_AFILAR, "Piedra de Afilar", "Whetstone",
        "Crítico con Ataque < 0:\nrestaura Ataque a +1.",
        "Crit with Attack < 0:\nsets Attack to +1.")
    GEAR_LOCALE(BATERIA_RESERVA, "Batería de Reserva", "Reserve Battery",
        "Stamina <50%: +20%;\n+4% durante recarga.",
        "Stamina below 50%: +20%;\n+4% while recharging.")
    GEAR_LOCALE(BOTAS_UMBRAL, "Botas del Umbral", "Threshold Boots",
        "Evita trampas al entrar;\nrecarga en 3 turnos.",
        "Avoids entry hazards;\nrecharges in 3 turns.")
    GEAR_LOCALE(VELO_CRIPTICO, "Velo Críptico", "Cryptic Veil",
        "Anula un efecto extra;\nrecarga en 3 turnos.",
        "Blocks one bonus effect;\nrecharges in 3 turns.")
    GEAR_LOCALE(AMULETO_INQUEBRANTABLE, "Amuleto Irrompible", "Unbreakable Charm",
        "Evita bajar atributos;\nrecarga en 3 turnos.",
        "Blocks one stat drop;\nrecharges in 3 turns.")
    GEAR_LOCALE(CRISOL_ESPIRITUAL, "Crisol Espiritual", "Soul Crucible",
        "Cura un estado alterado;\nrecarga en 3 turnos.",
        "Cures one status effect;\nrecharges in 3 turns.")
    GEAR_LOCALE(LENTE_CERTEZA, "Lente de Certeza", "Lens of Certainty",
        "Un ataque ignora evasión;\nrecarga en 3 turnos.",
        "One move ignores evasion;\nrecharges in 3 turns.")
    GEAR_LOCALE(MODULO_VECTORIAL, "Módulo Vectorial", "Vector Module",
        "Mov. de pivote: -25%\nStamina; -8% recarga.",
        "Pivot moves cost 25%\nless Stamina; 8% on cd.")
    GEAR_LOCALE(AMPLIFICADOR, "Amplificador", "Amplifier",
        "Sonido: +25% daño;\n+10% gasto Stamina.",
        "Sound: +25% damage;\n+10% Stamina cost.")
    GEAR_LOCALE(HEADPHONES, "Audífonos", "Headphones",
        "Recibe -40% daño sonoro;\nsin efectos extra.",
        "Takes 40% less sound\ndamage; no added effects.")
    GEAR_LOCALE(BOTON_PANICO, "Botón de Pánico", "Panic Button",
        "Golpe <30% PS: Vel.+1,\nSta.+18%; corta cadena.",
        "Hit below 30%: Spd+1,\nSta+18%; stops chain.")
    GEAR_LOCALE(NUCLEO_HYDRA, "Núcleo Hydra", "Hydra Core",
        "Tras daño bajo 50% PS:\ncura 8% cada turno.",
        "After damage below 50% HP:\nheals 8% each turn.")
    GEAR_LOCALE(CASCO_HOPLITA, "Casco Hoplita", "Hoplite Helm",
        "Dobles: aliado -12% daño.\nProtección: +12% Sta.",
        "Doubles: ally -12% dmg.\nProtect: +12% Stamina.")
    GEAR_LOCALE(ARMADURA_METEORICA, "Armadura Meteórica", "Meteor Armor",
        "Golpe letal único: 30% PS.\nDef. -1 a -3; quema rival.",
        "Single lethal hit: 30% HP.\nDef. -1 to -3; burns foe.")
};

#undef GEAR_BLOOD_LOCALE
#undef GEAR_LOCALE
