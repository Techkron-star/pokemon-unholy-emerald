# Unholy Emerald · Sistema de Gears

Un **Gear** es equipamiento de un slot propio, distinto del objeto equipado ordinario. El motor implementado aquí documenta Gears **cíclicos**: una **sobrecarga** potente y, mientras se **recarga**, una **bonificación residual** modesta. Los diseños futuros también pueden tener efectos **pasivos**, **reactivos** o **de una sola activación por combate**, sin forzarlos a funcionar como una sobrecarga. Los Gears equipados no se consumen.

**Nuevas familias en diseño:** [MASKARA (11 máscaras/cascos) y otros 19 Gears](GEARS_MASKARA_ARSENAL.md). Se trata de especificaciones conceptuales, no de funcionalidades implementadas.

**Estado:** los seis Gears originales, los 18 Inyectores, el Báculo Tormentoso y Grillete tienen equipamiento desde la mochila, efectos de combate e iconos de HUD de 16×32. Los Inyectores se consiguen mediante la investigadora de Ruta 102, en lugar de ocupar 18 recetas de Crafting.

## Cómo funciona

| Pieza | Dónde |
| --- | --- |
| IDs de Gear (`enum Gear`) | `include/constants/gears.h` |
| Ajustes (posición del icono, on/off) | `include/config/gears.h` |
| Estructura `GearInfo` y API | `include/gear.h` |
| Lógica del ciclo | `src/gear.c` |
| **Tabla de Gears (aquí se crean)** | `src/data/gears.h` |
| Icono en la caja de vida | `src/battle_interface.c` (`CreateGearIconSprite`) |

**Guardado.** El Gear equipado ocupa 8 bits de la subestructura de movimientos: los 5 bits originales (`PokemonSubstruct1.gear`) y 3 bits de relleno reutilizados (`PokemonSubstruct1.gearHigh`), accesibles con `MON_DATA_GEAR`. No cambia el tamaño de `BoxPokemon` ni desplaza los movimientos, IV o EV. Los IDs anteriores (0-31) conservan sus bits bajos. Capacidad técnica: **255 Gears** más el valor vacío (`GEAR_MAX_COUNT` = 256); esto no significa que ya existan 255 Gears jugables.

**Ciclo en combate.** El estado vive en `gBattleStruct` y se reinicia al comenzar cada combate. La recarga se conserva al retirar y volver a enviar al Pokémon, incluso en el otro puesto de un combate doble. El ciclo se identifica por entrenador y ranura del equipo. Los cambios de forma requieren comprobación específica.

1. **Armado / sobrecarga.** Según `trigger`:
   - `GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE`: queda armado y se gasta en el **primer movimiento de daño compatible**.
   - `GEAR_TRIGGER_TIMED`: se activa solo y dura `overloadTurns` turnos.
2. **Recarga.** Durante `cooldownTurns` finales de turno aplica la bonificación residual. Con `FIRST_COMPATIBLE_MOVE`, el final del turno en que se gasta ya cuenta como el primero.
3. Al terminar la recarga vuelve al paso 1.

**Reglas fijas**
- Dos movimientos encadenados = **un solo turno**: el ciclo avanza **una vez**, en `Gear_EndOfTurn`.
- Si se encadenan dos movimientos compatibles, la sobrecarga la lleva el **primero** y el segundo recibe el residual. Es la consecuencia directa de la regla de arriba; el diseño sigue pendiente de confirmarla.
- Solo afecta a movimientos **de daño** (nunca de estado) que pasen los filtros `typeMask` y `categoryMask`.
- La sobrecarga se gasta al empezar a ejecutarse el movimiento, **aunque falle**.
- La bonificación del movimiento en ejecución se fija al empezar, así que los golpes múltiples y los objetivos múltiples reciben el mismo valor.
- La IA ve la bonificación al estimar daño sin gastar nada (`Gear_GetPowerModifier` no tiene efectos secundarios).

### Familias de movimientos para los Gears

Un tipo elemental (Acero, Dragón, etc.) **no** determina si un ataque es de
corte, puño o sonido. Tampoco basta con que sea físico o haga contacto. Cada
movimiento declara sus familias en `src/data/moves_info.h`; los Gears consultan
estas mismas etiquetas a través de `include/move.h`.

| Familia | Consumidor actual | Incluidos | No incluidos |
| --- | --- | --- | --- |
| **Corte** (`slicingMove`) | Piedra de Afilar: +1 Ataque al usar un corte de daño | Corte, Cuchillada, Garra Metal, Garra Dragón, Garra Umbría, Hoja Afilada, Tajo Aéreo, Psicocorte, Garra Nociva, Bomba Población | Placaje, Hiperrayo, Danza Espada |
| **Puño** (`punchingMove`) | Guantes Cohete | Ultrapuño, Puño Hielo, Electropalmas (Double Shock) | Demolición, Cuchillada |
| **Sonido** (`soundMove`) | Amplificador (daño) y Auriculares (protección) | Estruendo, Aullido, Bramido Dragón | Hiperrayo |
| **Proyectil** (`ballisticMove`) | Inmunidad nativa Antibalas; familia reservada para futuros artefactos | Recurrente, Bola Sombra, Pedrada (Rock Blast) | Hiperrayo, Hidropulso, Tajo Aéreo |

**Criterio:** la etiqueta describe la **mecánica del movimiento**, no su
animación superficial. Bomba Población pertenece oficialmente a la familia de
corte pese a su nombre; no es un proyectil. Los movimientos de corte especiales
también cumplen Piedra de Afilar, aunque el Gear eleve Ataque físico. Se han
activado etiquetas de Garra Metal, Garra Umbría, Garra Dragón, Garra Nociva,
Garra Brutal, Double Shock y Bramido Dragón que el motor antiguo reservaba
para `GEN_CHAMPIONS` y que, de otro modo, no se activaban en la configuración
`GEN_9` de este proyecto.

Como ajuste general de Unholy, **cada ataque de corte ofensivo tiene al menos
+1 en probabilidad de crítico**, incluso Sacro Espada, aunque de base no lo
tuviera. Los movimientos con mayor nivel de crítico lo conservan. Doble Filo
(`MOVE_DOUBLE_EDGE`) no es un corte: es una embestida con daño de retroceso.

### Cambios de estadísticas y habilidades

Los Gears no ignoran las habilidades que modifican cambios de etapas:

- **Contrario** invierte las subidas y bajadas. Por ejemplo, el +1 Ataque de Piedra de Afilar pasa a −1, y el −3 Defensa/Def. Esp. de Armadura Meteórica pasa a +3.
- **Simple** duplica la magnitud. Una subida de +1 pasa a +2 y una bajada de −3 pasa a −6, siempre respetando los límites de etapas.
- **Efectos provocados por el propio Gear** (Piedra de Afilar, Botón de Pánico, Túnica Rúnica y Armadura Meteórica) usan `Gear_ApplySelfStatChange`: Rompemoldes del rival no anula Contrario, y Cuerpo Puro/Armadura Reflejo no bloquean ni reflejan las penalizaciones propias.
- **Bajadas infligidas a otro Pokémon** (Ariete Náutilo) pasan por `TryStatChange`: Contrario, Simple, Cuerpo Puro, Neblina y Armadura Reflejo conservan sus respuestas nativas. La reflexión puede afectar al portador del Gear.

Para nuevos Gears: usar `Gear_ApplySelfStatChange` para efectos propios y el sistema nativo de cambios de estadísticas para efectos contra adversarios; nunca modificar `statStages[]` directamente.

## Cómo crear un Gear

### Iconos de la tanda de noviembre de 2026

La integración de artefactos mantiene las tres resoluciones separadas:

- `graphics/gears/<nombre>/icon_32x32_source.png`: arte original conservado.
- `graphics/gears/<nombre>/icon.png`: dos fotogramas apilados de 16×16
  (activo y atenuado), compatibles con el HUD de combate existente.
- `graphics/items/icons/<nombre>.png`: icono de 24×24 para mochila y
  fabricación; el motor de iconos de GBA todavía no acepta 32×32 aquí.

Se han conectado 26 parejas de sprites a los artefactos jugables, incluidos
los doce de la tanda Escudo de paladín/Estandarte/Torre/Faro/Rompeolas/Red de
silos/Tablero/CRT/Deshumidificador/Bomba de ariete/Glicerina/Abono. Los
otros conceptos aprobados de 32×32 se conservan en
`graphics/unholy_concepts/approved_32/` sin asignarles mecánicas que aún
no estén programadas. Los iconos propios de la tanda de doce se conservan
en `graphics/unholy_concepts/generated_32/`.

Para regenerar los recursos GBA a partir de los originales se usa
`python3 tools/unholy/refresh_artifact_sprites.py` (ImageMagick y Pillow).
La conversión mantiene un máximo de 16 colores y transparencia en índice 0.
Los archivos generados están versionados; compilar la ROM no requiere
ImageMagick ni Pillow.

### 1. Constante
En `include/constants/gears.h`, añade el ID **justo antes de `GEAR_COUNT`**. Nunca reordenes ni borres los anteriores: los IDs están en las partidas guardadas.

```c
enum Gear
{
    GEAR_NONE,
    GEAR_SKI_MASK,
    GEAR_COUNT
};
```

### 2. Arte del icono
Cada Gear necesita **un PNG de 16×32 píxeles** con dos fotogramas de 16×16 apilados:

| Mitad | Se muestra cuando |
| --- | --- |
| Superior (filas 0-15) | Armado o en sobrecarga («listo») |
| Inferior (filas 16-31) | Recargando (apagado/atenuado; es el aviso de que el residual está activo) |

Requisitos:
- PNG **indexado de 4 bpp, 16 colores**; índice 0 = transparente (reservado para el fondo); contorno oscuro en el índice 1.
- Los dos fotogramas **comparten la misma paleta**.
- Ruta: `graphics/gears/<nombre>/icon.png` y `graphics/gears/<nombre>/icon.pal` (JASC-PAL de 16 entradas). El `Makefile` genera `icon.4bpp` e `icon.gbapal` solo.
- Es un asset distinto del icono de mochila/arte conceptual (24×24, `graphics/unholy_concepts/icons/`): ese no sirve aquí.

Si falta el arte (`iconGfx`/`iconPal` a `NULL`) el Gear funciona pero no dibuja icono.

### 3. Entrada en la tabla
En `src/data/gears.h`, añade la entrada con designador `[GEAR_...]`:

```c
static const u32 sGearIconGfx_SkiMask[] = INCGFX_U32("graphics/gears/ski_mask/icon.png", ".4bpp");
static const u16 sGearIconPal_SkiMask[] = INCGFX_U16("graphics/gears/ski_mask/icon.pal", ".gbapal");

// Dentro de gGearsInfo:
[GEAR_SKI_MASK] =
{
    .name = COMPOUND_STRING("Ski Mask"),
    .description = COMPOUND_STRING("Potencia los movimientos\nde tipo Siniestro."),
    .iconGfx = sGearIconGfx_SkiMask,
    .iconPal = sGearIconPal_SkiMask,
    .trigger = GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE,
    .cooldownTurns = X,                        // decisión de diseño
    .typeMask = GEAR_TYPE(TYPE_DARK),
    .overloadPowerPercent = X,                 // p. ej. 150 = ×1.5
    .residualPowerPercent = X,
},
```

| Campo | Significado |
| --- | --- |
| `name`, `description` | Texto para menús (la interfaz de equipar usará estos campos). |
| `iconGfx`, `iconPal` | Arte del paso 2. |
| `trigger` | `GEAR_TRIGGER_FIRST_COMPATIBLE_MOVE` o `GEAR_TRIGGER_TIMED`. |
| `overloadTurns` | Solo `TIMED`: turnos de sobrecarga (mínimo 1). |
| `cooldownTurns` | Finales de turno de recarga (mínimo 1). Ciclo completo de un Gear de primer movimiento = `cooldownTurns` turnos. |
| `typeMask` | Tipos compatibles; combina con `\|`. `0` = cualquiera. |
| `categoryMask` | `GEAR_CATEGORY_PHYSICAL`, `GEAR_CATEGORY_SPECIAL` o ambos. `0` = física y especial. |
| `overloadPowerPercent` | Multiplicador de potencia en sobrecarga (`150` = ×1.5). `0` = sin efecto. |
| `residualPowerPercent` | Multiplicador de potencia mientras recarga. `0` = sin efecto. |

Los multiplicadores se aplican a la **potencia base** del movimiento, dentro de `CalcMoveBasePowerAfterModifiers`.

### 4. Efectos que no son de potencia
El sistema actual cubre bonificaciones de potencia con filtros. Para un efecto distinto (por ejemplo, el +10 % de stats de **Colmillo de Rayquaza**), añade el campo nuevo a `GearInfo` y un punto de enganche en `src/gear.c` siguiendo `Gear_GetPowerModifier`: el cálculo debe ser **puro** (la IA lo llama muchas veces) y solo `Gear_OnMoveUsed` y `Gear_EndOfTurn` pueden cambiar el estado.

### 5. Equiparlo
`Gear_SetEquipped(mon, GEAR_...)` lo equipa y `GEAR_NONE` lo quita. `Gear_GetEquipped(mon)` lo lee. La opción EQUIP de la mochila llama a `Gear_TryEquipFromBag`, que intercambia el Gear con el inventario y conserva el objeto equipado ordinario. El menú Crafting también permite equipar y retirar un Gear.

## Indicadores del equipo y del combate

- Las tarjetas del equipo y el resumen muestran una **G** cuando el Pokémon tiene un Gear. En Info, pulsa L o R desde la página de habilidad para ver el nombre y el efecto del Gear.
- En combate, el icono queda junto a la caja de vida. **!** indica sobrecarga, **+** indica que el Gear está listo y el número cuenta los turnos que faltan para que termine la recarga. El icono destella al activar la sobrecarga con un movimiento compatible y al terminar la recarga.
- Los Gears temporizados empiezan con sobrecarga. El contador solo avanza cuando termina un turno real.

## Límites de la GBA

- Cada tarjeta visible de un Pokémon con Gear y arte usa un sprite de 32×16, ocho tiles y un hueco de paleta. El HUD los libera al ocultar la tarjeta o retirar al portador. Si faltan recursos, omite el icono hasta que haya espacio y el Gear sigue funcionando.
- Hasta 255 Gears por el ancho del campo persistido (8 bits). La cantidad jugable depende del catálogo implementado.

## Lista de comprobación para cada Gear nuevo

- [ ] La constante está antes de `GEAR_COUNT` y no se tocó ninguna anterior.
- [ ] El PNG es 16×32, indexado, 16 colores, índice 0 transparente, y la paleta tiene 16 entradas.
- [ ] La ROM compila y el icono aparece en la esquina en el emulador, en el lado del jugador y del rival, en singles y dobles.
- [ ] La sobrecarga se aplica una sola vez y el residual aparece al recargar; el icono cambia de fotograma.
- [ ] Dos movimientos encadenados avanzan el ciclo **una** vez.
- [ ] Cambiar de Pokémon y regresar al mismo puesto **conserva** la fase de recarga (no recupera sobrecarga gratis); probar también U-turn y Volt Switch.
- [ ] El ciclo se reinicia al iniciar una batalla nueva, sin persistir temporizadores al guardar.
- [ ] Cambios de posición en dobles o múltiples entrenadores no mezclan estados de Pokémon distintos.
- [ ] Guardar, cargar y mover el Pokémon por el PC conserva el Gear.

## Pendiente (fuera de este sistema)

- Acceso a equipar y retirar desde el resumen o el menú de equipo.
- Vías de obtención para Gears adicionales.
- Extender los Gears de entrenador a más equipos y ajustar su dificultad.
- Casos abiertos del diseño: sobrecarga con dos movimientos encadenados compatibles, y si cambiar de Pokémon debe reiniciar el ciclo.


## Efectos y estado de implementación

### Inyectores

Uno por cada tipo clásico. En la tanda de balance inicial, el Inyector da **+40 % de potencia** al primer movimiento dañino de su tipo. Después da **+12 %** durante tres finales de turno, contando el de activación. Si dos ataques compatibles se encadenan, el primero recibe el +40 % y el segundo el +12 %. La recarga se conserva al cambiar de Pokémon. Cuando además se activa una gema, tabla u otro objeto equipado que refuerza ese mismo tipo, se promedian los bonos y la gema conserva su consumo normal.

La investigadora de **Ruta 102**, visible a partir de la segunda medalla, intercambia cualquiera de los 18 Inyectores por **1 Polvo Estelar, 1 gema del tipo correspondiente y 3.000 pokédólares**. Muestra existencias, coste, saldo y pide confirmación antes de entregar cada uno. No se cobran materiales ni dinero si la mochila no admite el objeto. Horizon vende una gema por rotación en Objetos. Los cuatro Inyectores con arte propio en la mochila conservan sus iconos y los demás usan el vial con una paleta por tipo. El catálogo está en [`include/constants/blood_gears.h`](../include/constants/blood_gears.h).

### Báculo Tormentoso

El Báculo es pasivo. Un ataque especial que acierta a un solo objetivo puede dar un segundo golpe con **25 % de su potencia**, sin gastar PP ni Stamina adicionales. Si el primer golpe derrota al objetivo, termina ahí. El golpe adicional usa las reglas de multigolpe, incluidas las interacciones con Sustituto y los objetos que se activan entre golpes.

El Báculo suprime los efectos secundarios del ataque especial, incluso los de probabilidad del 100 %. El ataque conserva sus efectos principales, como la bajada de Ataque Especial al usar Sofoco, una vez por movimiento. Un portador puede combinar un ataque especial con uno físico en una cadena válida. Jugador e IA quedan sujetos al límite al elegir y al resolver movimientos de categoría variable.

En esta primera versión, los ataques con varios objetivos activos, los multigolpe nativos, los movimientos de dos turnos, los golpes diferidos, de daño fijo o de KO directo y los movimientos Z y Max mantienen su cantidad de impactos original.

**Receta inicial**. 1 Piedra Trueno, 1 Imán y 1 Trozo Estrella. En equipos de entrenador se usa `Gear: Storm Staff`.

### Grillete

Probabilidad inicial del **20 %** al dañar al primer rival elegible de una ejecución. Encadenado bloquea segundas acciones durante el resto de ese turno y el siguiente turno completo. El Gear puede tirar una vez por ejecución y el efecto no se acumula. La receta inicial usa un Revestimiento Metálico y una Piedra Dura.

La tarjeta del Pokémon afectado muestra un candado en su anillo de Stamina mientras dure Encadenado.

| Gear | Efecto confirmado | Qué falta |
|---|---|---|
| **Teseracto** | Excepción a los ciclos: se **consume una sola vez, como una gema equipada**, y hace persistentes clima, campos, salas y trampas hasta que se eliminen **explícitamente**. | Evento exacto de activación, persistencia del estado global hasta fin de combate y reglas de consumo. |
| **Ancla de Realidad** | Al estar en el campo elimina el clima, campos, salas y trampas, incluso los hechos permanentes mediante Teseracto. Los efectos eliminados **no reaparecen** al marcharse el portador. | Si bloquea nuevas activaciones durante su presencia, reglas de prioridad de habilidades y mensajes. |
| **Visor Cósmico** | Ataques Fantasma del portador son supereficaces (×2 por tipo) contra Normal, Psíquico y Hada en todas las fases. | Implementado. |
| **Corazón de Titán** | +20 % físico durante 1 turno sobrecargado; +10 % residual 3 turnos. | **Implementado en rama**, requiere UI y QA. |
| **Impulso Cósmico** | +20 % especial durante un turno de sobrecarga y +10 % durante tres turnos de recarga. | Implementado. |
| **Cetro Siniestro** | Siniestro es supereficaz contra Psíquico, Hada, Normal y Lucha, por tipo objetivo, **en todas las fases**; Lucha/Hada es ×4. | **Implementado en rama**; no altera la tabla global. |
| **Ski Mask** | +30 % en ataques Siniestros durante turno sobrecargado; +21 % continuo durante 3 turnos de recarga; sin cambios de efectividad. | **Implementado en rama**, requiere QA. |
| **Permafrost Fragment** | Doble probabilidad de congelación/Frostbite al dañar con Hielo; si el portador es Hielo cura **1/16 de PS máximos por turno**, sin atacar. | **Implementado en rama**, requiere QA del evento por golpe y curación con Restos. |

### Promedio de Gear + objeto ordinario

Cuando un Gear cíclico y un objeto equipado ordinario potencian el **mismo aspecto**, se usa la media aritmética de sus **bonificaciones en la fase actual**, nunca se suman ni se multiplican ambos factores. Ejemplo: Gear +20 % y Gafas Especiales +10 % → **+15 % total**, multiplicador ×1.15. En la fase residual se recalcula la media con el bono residual correspondiente. Objetos distintos que afecten otros aspectos del combate conservan sus efectos normales; no promediar con cero si no existe una bonificación coincidente. La implementación deberá modificar conjuntamente la contribución Gear y el modificador del objeto normal; aplicar ×1.20 y luego ×1.10 **sería un bug**.

### Riesgos y pruebas antes de registrar Gears

- Guardado: preservar el identificador de Gear (8 bits) al evolucionar, guardar, cargar o mover por PC; permitir IDs de hasta 255 sin alterar el tamaño de `BoxPokemon`. La compatibilidad con saves anteriores debe verificarse sobre copias.
- **Cambios:** fase y temporizador sobreviven al retirar y reintroducir al Pokémon durante el mismo combate. El test del motor verifica cambios cruzados en dobles y separación entre entrenadores. Falta QA visual en batallas completas.
- Teseracto vs Ancla: el Ancla elimina incluso efectos inmortalizados. No regenerarlos desde instantáneas del Teseracto cuando el Ancla salga.
- Permafrost: los movimientos multigolpe o en área deben tirar los estados por cada instancia **solo si el motor normalmente lo permite**. La curación es exactamente **una vez por turno al final**, al estilo Restos, sin depender de golpes. Si se equipa Restos, ambos +1/16 coincidentes se promedian (no se suman).
- Cetro y Visor aplican la misma regla en la IA, la interfaz de supereficacia y el cálculo de daño. La excepción se calcula para el portador en cada ataque.


## Fabricacion y equipamiento de Gears (rama de integracion de 10/11/2026)

El panel **Crafting** conserva Eclipse/Daylight y ahora incorpora ocho recetas (dos mejoras de Repartir EXP y seis Gears). Navegacion: L/R cambia receta; A verifica materiales y vuelve a pulsarse para confirmar; SELECT abre **EQUIP**; en EQUIP, L/R selecciona el Gear, ARRIBA/ABAJO el Pokemon y A cambia o retira el Gear. B cancela una confirmacion o sale.

Cada receta consume materiales de la mochila y entrega un objeto en el bolsillo **Gears**. EQUIP retira una copia del Gear de la mochila, guarda su ID en el slot persistente separado del objeto equipado y devuelve el Gear anterior a la mochila. Para quitarlo se selecciona REMOVE GEAR. Si la mochila no tiene espacio para devolver el anterior, se cancela el intercambio. Varias unidades permiten equipar a varios Pokémon.

| Receta | Materiales consumidos | Requisito no consumible |
| --- | --- | --- |
| Visor Cósmico | 2 Hechizos, 2 Piedras Lunares, 1 Trozo Estrella | Ninguno |
| Corazon de Titan | 1 Banda Musculo, 2 Revestimientos Metalicos, 2 Piedras Duras | Ninguno |
| Ski Mask | 1 Gafas de Sol, 3 Gemas Siniestras, 1 Piedra Noche | Ninguno |
| Cetro Siniestro | 1 Gafas de Sol, 2 Hechizos | Medalla Mente (7.a) |
| Impulso Cósmico | 1 Gafas Especiales, 2 Trozos Estrella, 1 Piedra Lunar | Ninguno |
| Fragmento de Permafrost | **2 Piedras Hielo, 1 Hielo Eterno y 1 Agua Mistica** | Ninguno |

La Super Exp. Share conserva su receta original y su comprobacion por script: 1 Exp. Share + Medalla Piedra. Los IDs de item se agregan al final de la enumeracion para respetar partidas anteriores. Los Gears sin efectos implementados (Teseracto, Ancla y Colmillo) **no** se ofrecen como fabricables.

## Gears en equipos de entrenador

En los archivos `.party`, `Gear: Impulso Cosmico` equipa el Gear del Pokémon. Para los Inyectores, usa el tipo en inglés, por ejemplo `Gear: Blood Fire`. El objeto del encabezado `Especie @ Objeto` ocupa su ranura habitual. Si se omite `Gear:`, el Pokémon se genera sin Gear.

| Combate | Pokémon | Gear | Objeto |
|---|---|---|---|
| Tate y Liza | Lunatone | Impulso Cósmico | Baya Zidra |
| Tate y Liza, cuarto equipo de revancha | Metagross | Corazón de Titán | Metagrossita |
| Phoebe | Dusclops | Visor Cósmico | Baya Zidra |
| Glacia | Walrein | Fragmento de Permafrost | Baya Zidra |
| Wallace | Milotic | Impulso Cósmico | Baya Zidra |
## Escudo de paladín (Artifact)

- Se equipa en la ranura independiente de Gears/Artifacts; no ocupa el objeto equipado.
- Cada movimiento enemigo dirigido al portador tiene su propia tirada, incluso el segundo movimiento encadenado; los múltiples golpes de un mismo movimiento cuentan como **una sola acción**.
- La activación anula la acción completa y sus efectos secundarios, incluidos golpes de KO directo (Guillotina). Solo puede activarse **una vez por Pokémon y turno completo**, también en dobles.
- La probabilidad empieza en **12,5 %** y aumenta **2,5 puntos porcentuales por turno activo sin éxito**, hasta un máximo del **25 %**. Tras **8 turnos activos sin bloqueo**, la próxima instancia hostil se bloquea de forma garantizada (pity). Cada éxito reinicia el contador.
- Se comprueban individualmente los ticks residuales de veneno, tóxico, quemadura, congelación por frostbite, clima dañino, Drenadoras, Pesadilla, Maldición, atrapamiento, sal y otros daños ambientales. Bloquear un tick no cura el estado ni previene ticks futuros.
- Si bloquea el movimiento inicial de una cadena, el movimiento siguiente del mismo atacante hace un **25 % menos de daño** al portador. El modificador no afecta a otros objetivos ni otras acciones.
- El contador persiste al retirar y volver a enviar al Pokémon durante el combate y se reinicia en el siguiente combate.
- El sprite y la receta aún no están definidos. Temporalmente utiliza el icono de Escudo Aegis en el HUD y un icono genérico en la mochila. No aparece en la lista de recetas.

## Lote Artifacts: soporte, control y ofensiva elemental (17/11/2026)

El nombre visible del equipo pasa a **Artifact** (bolsillo **Artifacts**).
Los identificadores C `GEAR_*` y el espacio de guardado **no cambian** para conservar compatibilidad.
Los once identificadores de Gear se añadieron en orden tras Escudo de paladín;
los IDs de objetos son **930–940**. Siguen equipándose en una ranura distinta al objeto ordinario.

| Artifact | Mecánica de combate | Cadencia |
| --- | --- | --- |
| Estandarte de guerra | Cuando el portador impone una condición de control *real* (estadísticas negativas, retroceso, estados o limitaciones volátiles), cura al aliado un 10 % de PS máximos y tira un 30 % para retirar una condición negativa. En individuales, se aplica al portador. No cura por intentos fallidos. | Por imposición de control; se registra cada objetivo afectado. |
| Torre de vigilancia | Borra las reducciones de Precisión y después intenta subirla un nivel (respetando Contrario/Simple). | Cada 3 turnos activos. |
| Faro | Durante el siguiente turno del adversario inhibe la contribución del clima al daño de movimientos y a las habilidades de Velocidad ligadas al clima; no elimina el clima. | Se prepara cada 2 turnos activos con clima presente. |
| Rompeolas | Reduce un golpe de Agua recibido 25 % o 40 % si es movimiento de área. | Máximo una aplicación cada 2 turnos activos. |
| Red de silos | Movimientos *balísticos* elegibles ejecutan 1–4 golpes extra al mismo objetivo. Los dos primeros ecos tienen 15 % del daño calculado y el tercero/cuarto, 10 %; nunca incluyen Pin Misil, Cañón Pincho, Recurrente, rayos, ondas, láseres, ataques múltiples nativos ni movimientos de estado. Los ecos no repiten secundarios ni consumen PP/Stamina. | Por movimiento elegible. |
| Tablero trifásico | Un movimiento Eléctrico de objetivo único rebota contra el segundo enemigo (si existe) con 25 % de daño y 10 % adicional de parálisis. No se replica a sí mismo. | Por movimiento elegible en dobles. |
| CRT | Duplica duración de Reflejo, Pantalla Luz y Velo Aurora (5→10, 8→16), y copia Protección al aliado vivo en dobles. | Al utilizar la pantalla o Protección. |
| Deshumidificador | Resta entre 1 y 2 turnos restantes de clima/niebla, sin dejar el temporizador en cero artificialmente. Los climas permanentes no se ven alterados. | Cada 2 turnos activos. |
| Bomba de ariete | Un ataque de Agua dirigido a un único objetivo añade una réplica del 25 %; si es el último de una cadena, se aplica a ese último movimiento. | Tras 2 turnos activos, consumido por el siguiente golpe elegible. |
| Glicerina | Toda habilidad de daño de Fuego puede quemar con un 20 % *adicional*. Se combina con la posibilidad nativa, sin repetir en ecos. | Por movimiento de Fuego que impacta. |
| Abono | Un Pokémon Planta recupera 1/16 de PS máximos al final del turno en terreno normal o campo de hierba; si tiene Enraizado recupera 2/16. | Una vez por turno; Bloqueo Cura lo impide. |

El motor de repeticiones utiliza la mecánica multigolpe sin ejecutar el movimiento desde cero;
el objetivo de Tablero sí pasa al segundo enemigo para el rebote. Las réplicas
siguen sujetas a inmunidad, sustituto y reglas de precisión como golpes normales.
Las activaciones de los efectos durante un movimiento generan el mensaje genérico
`[Pokémon] activó [Artifact]`; Estandarte usa un mensaje con la curación.

**Pendiente de QA en ROM:** pantalla visual, interacción completa de todos los
estados alterados, precisión de la protección en dobles y clima desde habilidades
que no calculan daño/Velocidad. No hay sprites únicos ni recetas para el lote:
se reutilizan iconos existentes temporalmente para poder probar la mecánica.
