# Unholy Emerald · Sistema de Gears

Un **Gear** es equipamiento de un slot propio, distinto del objeto equipado ordinario. Habrá pocos, con identidades marcadas. En vez de «más stats», cada Gear funciona en **ciclos**: una **sobrecarga** potente y, mientras se **recarga**, una **bonificación residual** modesta. El Gear no se consume y se reactiva solo.

**Estado:** el *sistema* está implementado; **todavía no existe ningún Gear** (solo `GEAR_NONE`). Los valores de balance de cada Gear son decisión de diseño y no se inventan aquí (ver `DISENO.md`, sección 4).

## Cómo funciona

| Pieza | Dónde |
| --- | --- |
| IDs de Gear (`enum Gear`) | `include/constants/gears.h` |
| Ajustes (posición del icono, on/off) | `include/config/gears.h` |
| Estructura `GearInfo` y API | `include/gear.h` |
| Lógica del ciclo | `src/gear.c` |
| **Tabla de Gears (aquí se crean)** | `src/data/gears.h` |
| Icono en la caja de vida | `src/battle_interface.c` (`CreateGearIconSprite`) |

**Guardado.** El Gear equipado ocupa 5 bits antes sin usar de la subestructura de movimientos del Pokémon (`PokemonSubstruct1.gear`), accesible con `MON_DATA_GEAR`. No cambia el tamaño de la estructura ni el formato del guardado; las partidas existentes lo leen como `GEAR_NONE`. Límite: **31 Gears** (`GEAR_MAX_COUNT` = 32 con el vacío).

**Ciclo en combate.** El estado vive en `gBattleStruct` y se reinicia cada vez que el Pokémon entra al campo (igual que la stamina; cambiar de Pokémon reinicia el ciclo, punto de balance abierto).

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

## Cómo crear un Gear

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
[GEAR_SKI_MASK] =
{
    .name = COMPOUND_STRING("Ski Mask"),
    .description = COMPOUND_STRING("Potencia los movimientos\nde tipo Siniestro."),
    .iconGfx = INCBIN_U32("graphics/gears/ski_mask/icon.4bpp"),
    .iconPal = INCBIN_U16("graphics/gears/ski_mask/icon.gbapal"),
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
`Gear_SetEquipped(mon, GEAR_...)` lo equipa y `GEAR_NONE` lo quita; `Gear_GetEquipped(mon)` lo lee. Aún no hay interfaz para equipar ni objetos que los otorguen (ver «Pendiente»). Para probar, llama a `Gear_SetEquipped` desde código de prueba o un script.

## Cómo se ve en combate

El icono es un sprite de 16×16 pegado a la **esquina superior derecha de la caja del Pokémon**, tanto del jugador como del rival. Sigue la visibilidad de la barra de vida (se oculta en menús y al caer el Pokémon) y cambia de fotograma al pasar a recarga. Si en el emulador no encaja, ajusta `GEAR_ICON_X_OFFSET_PLAYER`, `GEAR_ICON_X_OFFSET_OPPONENT` y `GEAR_ICON_Y_OFFSET` en `include/config/gears.h`.

## Límites de la GBA

- Cada Pokémon con Gear en el campo usa **un hueco de paleta de sprites** (hasta 4 en dobles). Si no queda ninguno libre, ese icono no se dibuja; el Gear sigue funcionando.
- Hasta 31 Gears por el ancho del campo guardado.

## Lista de comprobación para cada Gear nuevo

- [ ] La constante está antes de `GEAR_COUNT` y no se tocó ninguna anterior.
- [ ] El PNG es 16×32, indexado, 16 colores, índice 0 transparente, y la paleta tiene 16 entradas.
- [ ] La ROM compila y el icono aparece en la esquina en el emulador, en el lado del jugador y del rival, en singles y dobles.
- [ ] La sobrecarga se aplica una sola vez y el residual aparece al recargar; el icono cambia de fotograma.
- [ ] Dos movimientos encadenados avanzan el ciclo **una** vez.
- [ ] Cambiar de Pokémon y volver reinicia el ciclo como se espera.
- [ ] Guardar, cargar y mover el Pokémon por el PC conserva el Gear.

## Pendiente (fuera de este sistema)

- Interfaz para equipar/quitar (resumen del Pokémon y menú de equipo) y mostrar `name`/`description`.
- Cómo se obtiene cada Gear (objeto, recompensa de jefe, tienda). No definido.
- Entrenadores que usen Gears (campo nuevo en los datos de equipo del entrenador).
- Mostrar el turno restante de recarga y el Gear en el resumen en combate.
- Casos abiertos del diseño: sobrecarga con dos movimientos encadenados compatibles, y si cambiar de Pokémon debe reiniciar el ciclo.
