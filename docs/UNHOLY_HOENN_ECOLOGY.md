# Unholy Emerald — Ecosistemas y encuentros ampliados de Hoenn

## Objetivo

Ampliar **todas las rutas de Hoenn** por tandas, sin eliminar los Pokémon originales.
Cada especie nueva debe tener sentido para el hábitat, el método de encuentro,
el tramo de la aventura y su potencial evolutivo. Los Pokémon que normalmente
aparecen como regalos o encuentros especiales pueden tener apariciones raras,
pero no deben invadir los primeros caminos. Kecleon ya aparecía de forma
natural en varias rutas avanzadas de las tablas originales y se ha conservado.

El proyecto usa actualmente **12 casillas de hierba, 5 de Surf y 10 de pesca**
por mapa. Las tandas reutilizan casillas ocupadas por especies repetidas:
conservan las probabilidades de cada casilla, niveles y categorías de caña.
Así, aumenta la variedad sin modificar el motor ni perder los encuentros nativos.
Si otras zonas requieren más especies únicas que casillas disponibles,
se diseñará una ampliación del motor, no una sustitución de especies originales.

## Tanda 1 — rutas iniciales

| Zona | Hábitat | Nuevos encuentros | Variedad terrestre final |
| --- | --- | --- | ---: |
| Ruta 101 | praderas de inicio | Sentret, Bidoof, Starly, Rookidee, Scatterbug | 8 |
| Ruta 102 | praderas, flores y río | Lillipup, Bounsweet, Sewaddle, Nymble; Surf: Wooper, Dewpider y Surskit; pesca: Poliwag, Barboach, Chewtle y Tympole | 10 |
| Ruta 103 | praderas junto al mar | Shinx, Nickit, Lillipup, Pidove y Ducklett; Surf: Buizel y Frillish; pesca: Shellder, Remoraid y Clauncher | 8 |
| Ruta 104 | costa y entrada al bosque | Fletchling, Bounsweet, Sewaddle y Combee; Surf: Buizel, Ducklett y Frillish; pesca: Krabby, Tentacool, Shellder, Horsea, Remoraid, Clauncher y Corsola | 9 |
| Bosque Petalia | sotobosque y árboles | Sewaddle, Scatterbug, Pineco y Phantump | 11 |
| Ruta 116 | pastizal rocoso y obras del túnel | Diglett, Roggenrola, Timburr y Dwebble | 10 |

- **Phantump** en Bosque Petalia aparece en la casilla rara del 1 %, como
  encuentro extraordinario coherente con un bosque.
- Las especies de evolución muy poderosa se introducen en etapas tempranas
  solo cuando su primera fase es razonable; no hay legendarios, míticos ni
  formas finales añadidos a las primeras rutas.
- Surf y cañas siguen requiriendo los desbloqueos normales. Los niveles ya
  configurados para estos métodos no cambian.

El manifiesto es `tools/unholy/hoenn_ecology_wave1.py`: ejecutarlo con
`--write` aplica esta selección determinista y sin cambiar niveles ni pesos.
Sin argumentos, comprueba los 48 cambios y garantiza la supervivencia de las
especies originales en las 12 tablas afectadas.

## Tanda 2 — todas las rutas restantes de Hoenn

Se han ampliado las **29 rutas restantes, de la 105 a la 134** (la 116 ya
estaba incluida en la primera tanda), en **66 tablas y 204 casillas**.
Estas son las líneas ecológicas y algunos encuentros representativos:

| Rutas | Bioma y progresión | Incorporaciones destacadas |
| --- | --- | --- |
| 105–109 | Islas, playas, calas y barco abandonado | Finneon y Mantyke en Surf; Krabby, Shellder, Horsea y Remoraid por pesca; Dhelmise al 1 % con Supercaña en la 108 |
| 110–111 | Costa electrificada y desierto | Shinx, Yamper, Sandile, Helioptile, Hippopotas y Sigilyph raro; fauna fluvial por Surf y pesca |
| 112–115 | Volcán, ceniza, montaña y acantilados | Rolycoly, Houndour, Salandit, Larvesta raro, Rockruff, Roggenrola y Dwebble |
| 117–118 | Jardines, río y litoral | Cottonee, Petilil, Cutiefly, Combee, Buizel y Shinx |
| 119–121 | Selva lluviosa, bosque y proximidad al Monte Pírico | Sewaddle, Foongus, Dewpider, Applin, Morelull y Phantump; Kecleon original conservado |
| 122–123 | Aguas del Monte Pírico y huertos de bayas | Frillish, Skrelp y Dhelmise raro; Smoliv, Bounsweet y Combee |
| 124–131 | Mar abierto, arrecifes y proximidad a Arrecípolis | Chinchou, Shellder, Clauncher, Mareanie y Relicanth raro, variando por zona |
| 132–134 | Corrientes y aguas de las ruinas | Mantyke, Finneon, Skrelp, Chinchou y Relicanth raro |

**Salvaguardas de la segunda tanda:**

- **Todas las especies originales** siguen apareciendo por su mismo método.
  En pesca se comprueba por separado la Caña Vieja, Caña Buena y Supercaña:
  ninguna especie nativa desaparece de una categoría de caña.
- **Ruta 130**: las 12 casillas de Wynaut en Isla Espejismo permanecen intactas;
  solo se ha diversificado el agua que la rodea. También se conserva el
  Wailord original de Surf en la Ruta 129.
- No se han modificado las probabilidades por casilla, las tasas de encuentro,
  los niveles, los requisitos de Surf ni la lógica de pesca. Las nuevas especies
  raras ocupan las casillas de baja probabilidad ya existentes.
- No hay legendarios ni míticos añadidos. La potencia evolutiva y el hábitat se
  equilibran por tramo: especies iniciales en las primeras praderas y Pokémon
  más exóticos o difíciles de encontrar en rutas posteriores.

El manifiesto reproducible es `tools/unholy/hoenn_ecology_remaining.py`. En
modo normal valida el contenido aplicado, los 29 mapas, sus casillas y los
encuentros nativos por caña. Con `--write` aplica las incorporaciones sin
reescribir otros métodos. En total, ambas tandas abarcan **las 34 rutas
numeradas de Hoenn y el Bosque Petalia**, con 252 casillas curadas en 78 tablas.

## Tanda 3 — primeras cuevas y entornos rocosos

**12 mapas, 21 tablas y 90 casillas** en Túnel Fervergal, Cueva Granito
(cuatro zonas), Senda Ígnea, Desfiladero y Cascada Meteoro (cinco zonas).

| Entorno | Nuevos encuentros representativos | Encuentros especiales |
| --- | --- | --- |
| Túnel Fervergal | Roggenrola, Diglett, Geodude, Dwebble | Rockruff (4 %), Noibat (1 %) |
| Cueva Granito | Drilbur, Dwebble, Rockruff | Carbink (1 % en varias plantas, incluida Golpe Roca) |
| Senda Ígnea | Salandit, Rolycoly, Houndour | Charcadet (4 %), Larvesta (1 %) |
| Desfiladero | Rockruff, Rolycoly, Houndour, Dwebble | Glimmet y Salandit (1 % cada uno) |
| Cascada Meteoro | Rockruff, Roggenrola, Noibat, Carbink, Glimmet, Lunatone y Woobat | Minior (1 %); Gible (1 %) y Dratini por Supercaña (1 %) solo en la cámara más profunda |

Los Pokémon fuertes o con gran potencial están restringidos a pisos avanzados
o casillas de muy baja frecuencia. La ubicación original de **Bagon sigue
exclusivamente en la cámara profunda de Cascada Meteoro**. Los originales de
Golpe Roca, Surf y cada categoría de caña se conservan.

`tools/unholy/hoenn_ecology_caves.py` contiene el manifiesto determinista y
valida cada casilla frente a su especie original, los métodos de captura y la
exclusividad de Bagon. Al cerrar esta tercera tanda se alcanzaron **342
casillas curadas en 99 tablas**.

## Tanda 4 — mazmorras avanzadas, leyendas y descubrimientos

**30 mapas adicionales, 42 tablas y 213 casillas**, adaptados a cada planta:

| Zona | Características del hábitat | Encuentros especiales |
| --- | --- | --- |
| Monte Pírico (8 mapas) | Shuppet y Duskull nativos; Gastly, Litwick, Yamask, Sinistea, Golett, Phantump, Murkrow y Misdreavus | Mimikyu al 1 % en pisos altos y Spiritomb al 1 % en el sexto piso |
| Cueva Cardumen (5 mapas) | Spheal, Snorunt, Zubat y Golbat conservados; Seel, Bergmite, Snom, Cryogonal, Shellder y Woobat | Amaura al 1 % en la sala de hielo; **Lapras al 1 % con Supercaña** en la cámara interior |
| Calle Victoria (3 mapas) | Drilbur, Rockruff, Riolu, Durant, Noibat, Axew; Surf y pesca subterránea diversificados | Gible y Deino al 1 % en los sótanos; Dratini al 1 % por Surf o Supercaña en B2F |
| Caverna Abisal (9 mapas) | Woobat, Binacle, Dwebble, Wimpod, Carbink, Ferroseed y Sableye; pesca de Chinchou, Skrelp y Clauncher | Dhelmise, Minior y Relicanth en casillas raras específicas |
| Cueva Ancestral (2 mapas jugables) | Woobat, Carbink, Glimmet, Noibat y Roggenrola | Minior y Gible en las casillas más escasas; pisos obsoletos de Rubí/Zafiro intactos |
| Pilar Celeste (3 mapas) | Noibat, Lunatone, Minior, Carbink y dragones por profundidad | Deino y Gible al 1 %; se conserva el Altaria original del quinto piso |

Estas especies permiten opciones potentes sin regalarlas al principio de la
aventura. **No se añade ningún legendario ni mítico a encuentros aleatorios**:
Groudon, Kyogre y Rayquaza mantienen sus eventos especiales de historia.
Las formas originales de los encuentros y la separación por cañas permanecen
intactas; tampoco se han alterado los niveles, las tasas ni los pesos.

`tools/unholy/hoenn_ecology_endgame.py` comprueba los 30 mapas, sus tablas y
los Pokémon nativos por método y categoría de caña. **Acumulado de las cuatro
tandas: 555 casillas curadas en 141 tablas**, que abarcan las 34 rutas
numeradas, Bosque Petalia y 42 mapas de cuevas y mazmorras.

## Tanda 5 — zonas secundarias y generador de Nueva Malvalona

**19 mapas, 19 tablas y 107 casillas adicionales**, incluidos los dos mapas
submarinos con encuentros originales de Hoenn.

| Zona | Incorporaciones ecológicas | Hallazgo especial |
| --- | --- | --- |
| Nueva Malvalona: entrada | Elekid, Joltik, Tynamo, Klink, Toxel y Charjabug | Las dos especies eléctricas originales permanecen |
| **Nueva Malvalona: generador** | Joltik, Klink, Elekid, Tynamo y Toxel | **Rotom al 1 %** dentro de la central |
| Torre Espejismo (4 plantas) | Sandile, Baltoy, Dwebble, Yamask, Helioptile y Golett | Sigilyph y Unown al 1 %, Tyrunt al 1 % solo arriba |
| Paso Desértico | Drilbur, Roggenrola, Diglett, Dwebble y Noibat | Zorua al 1 % entre los Ditto originales |
| Cueva Taller (2 plantas) | Smeargle original sigue siendo dominante; Shroodle, Grafaiai, Unown y Klefki | Sudowoodo al 1 % y Ditto/Zorua raros según la planta |
| Guarida Magma (8 mapas) | Rolycoly, Slugma, Numel, Houndour, Salandit, Koffing y Charcadet | Larvesta al 1 % en las salas profundas |
| Bajo las Rutas 124 y 126 | Fauna marina: Skrelp, Mareanie, Clauncher y Tynamo | Relicanth original conservado por Surf |

En el generador de Nueva Malvalona se desplaza Magneton desde su casilla
original del 1 % a una casilla del 4 % ocupada por Magnemite. **Así siguen
disponibles Voltorb, Magnemite, Electrode y Magneton**, mientras Rotom entra
en la casilla excepcional del 1 %. Los interruptores, el generador apagable,
la misión de Wattson y los tres Voltorb de combate fijo **no se modifican**:
Rotom es un encuentro salvaje dentro del mapa, no un nuevo evento sobre el
objeto del generador.

`tools/unholy/hoenn_ecology_side_areas.py` verifica las 19 tablas originales,
la presencia de todas las especies nativas, Rotom en el espacio de baja
probabilidad y las etiquetas de los eventos del generador.

**Total de las cinco tandas: 662 casillas curadas en 160 tablas de 96 mapas**,
incluidas las 34 rutas numeradas y los biomas principales y secundarios.

## Zonas sin tabla de encuentros o por revisar

Quedan por estudiar la Zona Safari y otros interiores especiales. La zona
`MAP_SCORCHED_SLAB`, por ejemplo, no tiene una tabla silvestre en este archivo:
crear encuentros allí exige revisar primero su mapa, sus eventos y las
condiciones de acceso; no se ha añadido una tabla artificialmente.

## Validación de juego pendiente

Comprobar en mGBA/Pizza Boy que los niveles, probabilidades y desbloqueos de
Surf/pesca son correctos; que siguen apareciendo todas las especies nativas;
que las formas y sprites añadidos se muestran bien; y que el Pokélocator y los
encuentros con mutaciones funcionan para especies nuevas.
