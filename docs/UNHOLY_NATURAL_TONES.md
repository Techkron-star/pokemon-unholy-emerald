# Identidad cromática individual — Algoritmo V1 (inmutable)

Cada Pokémon se colorea de forma determinista según su **identidad
individual**, que combina los dos campos ya persistidos en el `.sav`:

- `personality` (PID de 32 bits): identifica al ejemplar, no la especie.
- `otId` (OT ID de 32 bits): entrenador original; no cambia al intercambiarlo.

El algoritmo aplica dos permutaciones de 32 bits y forma un código de 64 bits
sin perder información de ninguno de los campos. Así, **dos pares PID+OT
distintos tienen códigos distintos**. Se derivan de él, sin usar el RNG del
juego, los parámetros de recoloración. Clones que comparten ambos campos
son indistinguibles sin introducir un serial adicional en el guardado.
Que dos códigos sean distintos **no garantiza** que sus paletas RGB555
resultantes sean visualmente distintas: el hardware tiene colores finitos.

## Distribución inicial

| Variante | Normales | Shinies | Operación |
| --- | ---: | ---: | --- |
| Matiz cercano | 90 % | 88 % | Giro continuo de aproximadamente −75° a +75° |
| Complementaria | 9 % | 9,5 % | Giro cromático de aproximadamente 160° a 200° |
| Inversión de luminosidad | 1 % | 2,5 % | Oscuros neutros → grises claros/blancos, manteniendo contornos |

Adicionalmente, el código individual determina una variación de saturación
de ±6 % y luminosidad de ±1 paso RGB555. Para evitar que la reducción a
15 colores colapse cientos de identidades en una misma paleta, cada índice
de color intermedio recibe una **microvariación fija de ±1 paso RGB555**.
Los colores acromáticos ajustan su brillo y los saturados desplazan
suavemente rojo/azul, sin tocar transparencia, contornos o altas luces.
No es ruido aleatorio por frame: usa el mismo código PID + OT ID e índice
de paleta en todas las pantallas. Ya no hay once categorías fijas.
No se altera la bandera ni la probabilidad de shiny. La base de un shiny
es siempre su paleta shiny propia de la especie.

**Ejemplo:** un Charizard shiny con cuerpo oscuro y la variante de
inversión tonal puede mostrar el cuerpo blanco-grisáceo, sin convertir
los negros profundos de sus contornos en blanco. La transformación
es genérica sobre colores de paleta, no una textura o sprite especial
para Charizard; depende de los índices de color que use cada especie.

## Permanencia y compatibilidad

El color nunca se sortea de nuevo al entrar en batalla, cambiar de caja,
guardar, cargar o intercambiar: se recalcula a partir del mismo PID y OT ID,
incluido el contexto de combate. Al evolucionar se conserva el patrón
individual, aplicado a la paleta propia de la evolución.

**Contrato de compatibilidad:**
`NATURAL_TONE_ALGORITHM_VERSION = 1`. Congelar la función de identidad,
pesos, hue y operaciones de color. Los vectores de prueba fijos impiden
cambiar accidentalmente el aspecto de Pokémon existentes en una próxima
build. Modificaciones futuras necesitarían migración o compatibilidad con V1.

- No se cambia la estructura `BoxPokemon`, ni el formato `.sav`, ni se usa
  RAM extendida. Se usa la OT ID ya guardada en equipo/PC/resumen.
- Se recolorean sprites de combate, resumen, retrato ampliado del PC clásico,
  evolución (forma anterior y nueva, incluida la evolución por intercambio),
  animación de intercambio y eclosión, además de sprites temporales de combate.
  En intercambios, las dos partes usan paletas separadas incluso si ambos
  Pokémon son de la misma especie, para preservar sus tonos individuales.
  Pantallas que solo presentan una especie, sin un Pokémon concreto (por
  ejemplo, la Pokédex), conservan la paleta canónica.
- Los iconos de 16×16 del PC y los overworlds comparten paletas; todavía
  conservan los colores de especie para evitar cambiar otros Pokémon.
- Se preservan entrada transparente 0, contornos muy oscuros, blancos
  casi puros y bits de paleta reservados.
- No se alteran IV, EV, stats, habilidades, mutaciones ni captura.

**Experimental:** comprobar visualmente en Pizza Boy Pro cambios de forma,
Transform, Illusion, combates dobles, shinies, evolución, PC y fades.
En una simulación de 1.000 Heatran normales y 1.000 Charizard shiny
con PID distintos del mismo entrenador, la versión con microvariaciones
produjo 0 paletas exactamente repetidas en cada lote (frente a centenares
sin ese refinamiento). Esta es una muestra reproducible, no una garantía
universal. RGB555 todavía puede producir tonos visualmente parecidos.

## V2 — regional palette engine (experimental)

V1 is unchanged. The previously zeroed bit in encrypted substructure 0 selects V2 without adding save bytes. New individuals use V2, old zero-bit individuals retain V1. Eggs retain their version at hatching. SELECT in the new editable summary INFO page toggles the version persistently and reversibly.
The V2 palette engine uses PID + OT ID, three automatic hue roles, analogous/complementary/triadic/split harmonies, and six cosmetic finishes with design weights: natural 70%, pastel 12%, copper 7%, obsidian 5%, porcelain 4%, melanism 2%. Transparent entries and extreme outlines/highlights are protected; the shiny flag is unchanged. Packed role overrides are supported by the engine but not yet authored per species; shared entries cannot be recolored independently. Pixel patterns and small icons are not included.
Existing palette hooks use the stored version for battle, summary, PC portraits, hatching, trade and evolution. Synthetic previews without an actual individual retain V1. Compile, run tests and validate real save migration and visual consistency in emulator before merging.

### V2 partial regions and exotic finishes (draft, not yet released)

Scope uses an independent identity hash: 40% one role, 30% two roles, 15% three/all available roles, 10% global over eligible entries, 5% accent-first one/two roles. With fewer than three eligible roles the selection falls back to available roles. Selected roles use the same deterministic profile everywhere; unselected palette entries are copied unchanged. On three-role palettes all-role and global may look identical. Existing V1 is untouched; V2 is still a draft and this revision changes its appearance compared with the earlier prototype.

Palette roles are inferred automatically or matched against ten exact RGB555 palette signatures from Zigzagoon, Alakazam and Mega Rayquaza (normal/shiny and alternate GBA assets where present). These are first-pass semantic index assignments, not pixel-level masks; visual review is required for shared indices, front/back sprites and alternate assets. Curated zero-role entries are kept unchanged. Neutral accents can be tinted, and protected white/black endpoints remain unchanged. No sprite pixels or save bytes changed.

Finish weights (basis points, out of 10,000): natural 5129, pastel 1200, copper 700, obsidian 500, porcelain 400, melanism 200; solar 100, magnetar 20, quasar 10, cosmic 50, ghostly 150, infernal 50, godly 1, aqua 300, jungle 300, nightly 200, albinism 30, spectral 100, golden 100, iridescent 40, crimson 120, glacial 100, toxic 100, lunar 100. Godly is 1 in 10,000 in the hash distribution. Finish and selected region count are separate. All weights are provisional; a finish may be subtle or invisible on a given palette. Themed target colors are blended into existing shade ramps, not animated effects or gameplay modifications.

### Universal build-time spatial roles

The host tool `tools/misc/unholy_chromatic_regions.py` recursively scans normal and GBA battle palettes and compatible indexed front/back/animation PNGs, including nested forms. It measures each color index's area, background-edge exposure and neighboring indices, then clusters shade ramps using hue, chroma and spatial contact. The largest cluster is body, remaining clusters secondary/accent. Normal and shiny share the same inferred index roles. Unknown/unsupported assets or conflicting signatures fall back to the existing curated/automatic hue logic. No manual species list is needed for generated entries. The generated header is rebuilt by Make when dependencies change; a pair of 32-bit signatures and a packed 32-bit mask cost 12 bytes per entry, with binary search at runtime. The generator uses Python stdlib and a synthetic test before processing assets. This is heuristic palette-index segmentation, not semantic object recognition or pixel recoloring: shared indices cannot be separated, different palettes can collide, and visual QA is required. No save size or V1 changes. The V2 draft changes appearance as the algorithm evolves; freeze the generated mapping and generation rules before releasing.
