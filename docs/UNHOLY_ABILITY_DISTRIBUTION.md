# Unholy Emerald — reparto de las 37 habilidades

**Propuesta implementada, pendiente de compilación y pruebas de juego (21-11-2026).**

Se distribuyen las **37 habilidades** entre **403 especies y formas** que encajan por anatomía, tipo, personalidad, estilo de combate o evolución. No se impone una habilidad a Pokémon sin relación temática solo por cubrir toda la Pokédex. El catálogo exacto y editable, con **cada especie**, está en `src/data/pokemon/unholy_secondary_hidden_abilities.h`.

## Cómo funciona

- Cada Pokémon conserva sus **dos opciones normales y su habilidad oculta original**. La propuesta añade una **segunda habilidad oculta** (slot 3); no le quita Multiescamas a Dragonite, Liviano a Sceptile ni las habilidades iniciales a los starters.
- El campo de guardado `abilityNum` ya tiene **2 bits**, suficientes para los slots 0, 1, 2 y 3. No se cambia el formato de la partida guardada. Un Pokémon existente no obtiene automáticamente la nueva habilidad: conserva la que ya tenía.
- El **Parche de Habilidad** avanza de una normal a la primera oculta disponible, de la primera oculta a la segunda (si existe) y de la última oculta de vuelta a la normal. La **Cápsula** sigue alternando las dos normales. No hay selección aleatoria ni consumo del Parche si el cambio no existe.
- En **DexNav**, al salir la oportunidad de habilidad oculta para una especie ya capturada, puede aparecer **cualquiera de las dos**. El panel resumido de DexNav todavía muestra una sola habilidad oculta; esta limitación de visualización debe revisarse.
- En la **crianza**, ambas habilidades ocultas usan la misma probabilidad de herencia definida por la generación (60 % a partir de Gen 6), sin convertir la segunda en herencia garantizada.
- El editor de Pokémon de depuración y la asignación de equipos de entrenadores reconocen las cuatro opciones. Los cambios no activan **dos habilidades simultáneas**: cada Pokémon sigue usando una sola.

## Reparto temático

| Habilidad | Ejemplos de especies | Criterio |
| --- | --- | --- |
| Dragonborn | Dragonite, Garchomp, Dragapult, Charizard | Dragones y linajes dracónicos. |
| Reaper | **Sceptile, Lurantis, Inteleon**, Absol | Rematadores, cazadores y precisión letal. |
| Dual Wield | Blastoise, Clawitzer, Decidueye, Genesect | Artillería y proyectiles. |
| Maratonista | Rapidash, Cinderace, Cyclizar, Quaquaval | Carrera, velocidad sostenida y agilidad. |
| Reactor | Magnezone, Porygon-Z, Typhlosion, Regieleki | Acumulación o emisión de energía. |
| Cuerpo Magma | Torkoal, Camerupt, Heatran, Coalossal | Cuerpos de calor extremo o volcánicos. |
| Árbol Mundial | Venusaur, Meganium, Torterra, Trevenant, Serperior | Plantas, bosques y raíces. |
| Abisal | Tentacruel, Jellicent, Wailord, Palafin | Mar profundo y criaturas acuáticas. |
| Super Batería | Raichu, Ampharos, Electivire, Toxtricity | Acumuladores y descargas eléctricas. |
| Bélico | Aggron, Skarmory, Kingambit, Archaludon | Armaduras, acero y control del campo. |
| Slayer | Scyther, Weavile, Kartana, Ceruledge | Cuchillas y cortes críticos. |
| Knight | Chesnaught, Corviknight, Empoleon, Zamazenta | Defensores y armaduras. |
| Wizard | Alakazam, Gothitelle, Delphox, Hatterene | Conjuradores psíquicos. |
| Malvado | Zoroark, Greninja, Meowscarada, Darkrai | Emboscadas y trucos siniestros. |
| Greedy | Meowth, Sableye, Gholdengo, Greedent | Ladrones y acaparadores de tesoros. |
| Gula | Slowbro, Cramorant, Gourgeist, Clodsire | Apetito y consumo de bayas; también aprovecha el ID existente. |
| Envy | Ditto, Kecleon, Smeargle, Mimikyu | Imitación y suplantación. |
| Colérico | Primeape, Infernape, Emboar, Hitmonchan | Ira y técnicas de puño. |
| Manthra | Medicham, Bronzong, Oranguru, Cresselia | Meditación y espera. |
| Fallen Angel | Togekiss, Sylveon, Primarina, Enamorus | Hadas celestiales con aspecto oscuro. |
| Samurai | Samurott, Gallade, Shiftry, Kabutops | Disciplina con armas de filo. |
| Gélido | Glaceon, Froslass, Walrein, Baxcalibur | Hielo, escarcha y Frostbite. |
| Banshee | Gengar, Mismagius, Chandelure, Skeledirge | Espectros, lamentos y estados secundarios. |
| Oni | Tyranitar, Houndoom, Incineroar, Krookodile | Depredadores físicos y fuerza siniestra. |
| Drake | Rayquaza, Dragalge, Kommo-o, Regidrago | Ataques Dragón que atraviesan defensas. |
| Monje | Lucario, Mienshao, Blaziken, Marshadow | Combate y disciplina física/espiritual. |
| Escama Acero | Ferrothorn, Steelix, Forretress, Orthworm | Espinas y placas metálicas. |
| Luz de Alba | Ho-Oh, Xerneas, Lilligant, Solgaleo | Luz restauradora y amanecer. |
| Gran Eclipse | Umbreon, Lunala, Yveltal, Necrozma | Eclipses y oscuridad cósmica. |
| Hiperbárico | Pidgeot, Swellow, Noivern, Tornadus | Corrientes aéreas y viento. |
| Antithesis | Hypno, Hoopa, Giratina alterada, Runerigus | Inversión y distorsión de efectos. |

## Exclusivas: nunca en otra especie

| Habilidad | Solo la recibe |
| --- | --- |
| **GodHand** | Arceus (todas sus formas de tipo). |
| **Time Realm** | Dialga y Dialga Origen. |
| **Space Realm** | Palkia y Palkia Origen. |
| **Anti Realm** | Giratina **Origen**; Giratina alterada recibe Antithesis. |
| **God Realm** | Necrozma **Ultra**; Necrozma normal recibe Gran Eclipse. |
| **Black Hole** | Guzzlord. |

## Comprobaciones y límites

1. Tests propuestos en `test/unholy_ability_distribution.c`: existencia de las 37 habilidades, preservación de las habilidades originales de Sceptile, persistencia del slot 3 y continuidad de formas relevantes.
2. No se han rebalanceado **Torrente, Mar Llamas ni Espesura** en esta fase. Se puede revisitar la distribución de los iniciales al hacerlo.
3. Las formas Mega/Gigamax siguen las reglas propias de transformación y sus habilidades especiales, que pueden sustituir temporalmente la opción de base.
4. **Validación pendiente:** compilación con toolchain GBA y pruebas en partida de Parche, DexNav, evoluciones, formas y guardados antiguos. No se debe presentar como una versión ya jugable hasta superar esos controles.
