# Unholy Emerald — interfaz nativa 240 × 160

La referencia conceptual es «Pokewatch UI – Unholy Emerald». La organización de Crafting, Horizon y el Poké Watch sirve de referencia para los demás menús de Unholy Emerald.

## Estándar de organización

| Elemento | Criterio |
| --- | --- |
| Contexto | El encabezado identifica la pantalla o el paso actual. El nombre, saldo o contador se muestran allí cuando ayudan a decidir. |
| Selección | El borde y el relleno muestran el foco. El estado bloqueado, agotado o incompleto lleva un texto, un símbolo o una cantidad que se pueda interpretar en ambos temas. |
| Detalle | El cuerpo de la pantalla agrupa la información de la selección. Crafting pone el objeto sobre sus materiales, el Simulador alinea estadísticas en columnas y una incursión muestra el jefe y las probabilidades debajo de la dificultad. |
| Acciones | El pie nombra lo que hace cada botón en ese paso. Si la selección está bloqueada, explica el requisito cerca del contenido. Una confirmación muestra el coste, el efecto o el riesgo antes de aplicar la acción. |

Usar cuadrícula cuando varias opciones se comparan de un vistazo, lista con ficha cuando importa inspeccionar cada opción y tabla cuando las mismas cifras se comparan entre filas. En cada caso, reservar espacio para nombres largos, cifras, iconos y estados. Las ayudas describen acciones reales y las cantidades van junto a su etiqueta.

`UnholyUi_Frame`, `UnholyUi_Footer`, `UnholyUi_ChoiceRow` y `UnholyUi_Card` comparten el marco, la zona de acciones y el estilo de selección. Mantener las precauciones de tiles, sprites y paletas de cada menú que los usa. Verificar a 240 × 160 el foco, la lectura y el efecto de los botones en Eclipse y Daylight, incluyendo lista vacía, bloqueo, confirmación, cancelación y retorno.

## Componentes y resolución

- Ventanas 4 bpp, texto real del motor, bordes de un píxel y selección con borde y relleno. No se reduce una imagen del PDF ni se dibuja texto dentro de un recurso gráfico.
- `unholy_ui` comparte recuadros, barras, texto acotado por anchura y préstamos de paleta. Cada ventana de campo que toma la paleta 14 devuelve sus 16 colores al cerrarse.
- Eclipse y Daylight conservan geometría, datos, navegación e instrucciones. El tema se elige en Opciones y dispone de vista previa; el reloj del juego sigue siendo independiente.
- Cabecera de 19 px; cuerpo con filas de 11–17 px según el contexto; pie separado para instrucciones. Se usan las fuentes existentes de 12 px y 8 px. Los nombres largos se acotan por medida tipográfica.

## Pantallas del PDF y adaptación

| Referencia | Implementación |
| --- | --- |
| 1, 6 · Poké Watch | Diez servicios en una cuadrícula de dos columnas y hasta seis tarjetas por página. El servicio bloqueado conserva el foco y muestra su requisito real. Al regresar se conserva el servicio elegido. |
| 2 · Horizon | Cinco categorías existentes: Pokémon, Discos, Objetos, Huevos y Movimientos. Las seis ofertas Pokémon y cinco huevos reutilizan su compra, confirmación, stock y RNG persistentes. |
| 3 · PokéNav | Se conserva el mapa real y sus funciones. El panel de ubicación adopta la paleta común y limita los nombres a su espacio. |
| 4 · Pokélocator | Catálogo 3×2 por especie, categorías presentes/desbloqueadas, ficha previa y progreso. Obtención gratuita individual/ALL/NEW con confirmación y capacidad, Search nativo, objetivo R persistente y Señal existente. Topes globales por medallas. Eclipse/Daylight comparten datos y navegación. Véase [Pokélocator](POKELOCATOR_SEARCH.md). |
| 5 · Mutator | Equipo en tarjetas con foco y bloqueos. Cada plan alinea coste, probabilidad de éxito y nivel resultante. La confirmación muestra que se cobra también si falla. |
| 7 · Huevos | Rango y calidad se identifican por separado. La compra sigue ocultando la especie. No hay una acción de reinicio del premio. |
| 8–11 · Resumen | Cuatro pestañas: INFO, STATS, MOVES, GEAR. Incluye sprite real, PS, stamina máxima, IV/EV, ranuras de objeto/Gear independientes, las nueve mutaciones combinables y bonos positivos/negativos calculados por estadística. |
| 12 · Crafting | Recetas en tarjetas a la izquierda. A la derecha van el objeto, su icono y debajo los materiales con cantidades disponibles y necesarias. Los materiales que faltan se marcan al lado de la cantidad. |
| 13, 19 · Combate | Se conservan las tarjetas de PS, estado, anillos e iconos de Gear existentes. En selección de movimientos, una franja muestra dos posiciones de cadena, nombres, coste previsto y stamina disponible. SELECT se indica para marcar/cancelar; A conserva las validaciones existentes. La franja se oculta con descripciones y al salir. |
| 14 · START | WATCH sigue siendo el acceso al reloj; se conservan las rutas y el retorno del menú existente. |
| 15 · Opciones | Se muestran las opciones reales de la rama con tipografía de 12 px y vista previa Eclipse/Daylight. |
| 16 · Mochila | Selector L+R desplazable con los diez nombres reales y recuentos de pilas, foco independiente del contenido y paleta restaurada al salir. Se mantienen cursores, ordenación, intercambio y restricciones de contexto. |
| 17 · Equipo | Las tarjetas originales conservan PS, estados y marca de Gear. El mensaje de selección en el campo distingue PS actuales/máximos de STA MAX. No se inventa una stamina persistente fuera del combate. |
| 18 · Diálogo | Las cajas usan la paleta común; el nombre del hablante se ajusta a su fila reservada. Se conservan marcos, paginación y opciones del motor. |

## Servicios complementarios del reloj

### Fabricación: lectura de descripciones largas

La ventana de Fabricación e Inyectores aprovecha **224 × 144 px** dentro de la
pantalla nativa de 240 × 160. El resumen reserva dos líneas de 8 px, con
separación real respecto a MATERIALES y tres filas de ingredientes. Pulsar
**START** abre la ficha de descripción a todo el ancho; **A** avanza entre
páginas y **START/B** vuelve a la lista. El selector de recetas, cantidades,
confirmación y equipamiento por SELECT mantienen sus controles.

QA visual pendiente: en Eclipse y Daylight, Velo Críptico, nombres de materiales
largos, tres materiales y una medalla, la tienda de inyectores y la paginación
de descripciones mayores de siete líneas. Nada debe invadir el pie de página.

- **PC:** abre la organización existente de cajas y regresa al reloj; conserva sus operaciones y selección.
- **Archivo:** nueve tarjetas de mutación con su tier y una marca en las registradas.
- **Señal mutante:** tarjeta de estado con el aumento, los encuentros restantes o la condición para activarla.
- **Simulador:** equipo en tarjetas y comparación alineada de IV, EV, estadísticas actuales y previstas. El coste y el dinero disponible se leen debajo antes de pagar.
- **Incursiones:** dificultad y recompensas en tarjetas. El cuerpo muestra el requisito, el jefe y su equipo, la probabilidad de captura o el detalle de la recompensa elegida.

## Controles de Resumen

- START abre Trainer Tools: ramas evolutivas con progreso, gestor de movimientos de nivel/preevolución/huevo/MT/tutor, objeto, Gear, Simulador y cambio de mote. Véase [Preparación del equipo](TEAM_QOL.md).
- Izquierda/derecha cambia pestaña. Arriba/abajo cambia Pokémon en Info, Stats y Gear; en Movimientos recorre las cuatro ranuras.
- L/R alterna Stats → IV → EV y Objeto/Gear → Mutaciones → Bonos.
- A en Info o Movimientos abre el detalle original, incluidas sus operaciones existentes. B en ese detalle vuelve al nuevo Resumen y conserva el Pokémon elegido.
- El aprendizaje, sustitución y borrado de movimientos, contextos de enlace y Resumen durante combate conservan la pantalla original. Las vistas normales de equipo y PC usan la nueva composición; la selección especial `PC_MON_CHOSEN` conserva su tratamiento original.

## Validación

Comprobar ambos temas en 240 × 160, nombres largos, huevos, ranuras vacías, varias mutaciones, penalización de Velocidad de Titán, IV/EV, objeto y Gear simultáneos, listas vacías, funciones bloqueadas y fondos después de cerrar ventanas.

En combate: movimiento marcado y sin marcar, segundo movimiento distinto/protección/sin PP, coste superior a stamina, cancelación, descripción, Megaevolución y dobles. La franja de cadenas solo aparece cuando el motor permite encadenar.

Comprobar diez ciclos de apertura/cierre del reloj y selector, regreso desde cada servicio, compra/cancelación sin regenerar stock y guardar/cargar. Las pruebas de datos verifican los requisitos del reloj y que el snapshot del Resumen conserva Pokémon, objetos y bonos reales.

## Correcciones de offsets del localizador ampliado

El menú de movimientos reserva el panel inferior para dos posiciones de cadena y SELECT, los cuatro movimientos y tres líneas independientes de PP, tipo y stamina prevista/disponible. La vista ya no cubre la tarjeta de PS. Al cambiar a descripción o salir se limpian ambas ventanas auxiliares.

Resumen distingue ranuras de objeto/Gear y muestra una tarjeta explicativa cuando no hay mutaciones. La cuadrícula de mutaciones separa iconos y nombres. La mochila usa franjas suaves según Eclipse/Daylight, diez etiquetas visibles (incluidas Gears, Correo y Miscelánea) y un selector con 14 píxeles por fila; su paleta se conserva entre aperturas del selector. El icono original de Exp. Share ya tiene aspecto de casco/máquina: no era un sprite de Pokémon cargado por error.

El HUD reserva ahora el espacio entre nombre y nivel para la mutación compacta. El Gear se presenta en vertical frente a la tarjeta, con icono arriba y fase/recarga debajo; la columna cabe en los 25 píxeles de separación de los dobles. Véase [Iconos de mutación](MUTATION_ICONS.md).

## Paleta y marco del concept

El distintivo de Artefacto en el equipo y el resumen es ahora **A** (antes «G»); el bolsillo correspondiente de la mochila utiliza **AR**. No cambia el equipo ni el inventario.

Las 16 entradas de la paleta compartida (`sHorizonPaletteEclipse` / `sHorizonPaletteDaylight` en `shop.c`) siguen los colores del concept «Pokewatch UI»: violeta nocturno con acento cian en Eclipse y papel verde claro con acento verde bosque en Daylight. Los roles de cada índice no cambian, de modo que todas las pantallas nativas, la mochila, los diálogos y el PokéNav heredan el aspecto sin tocar su geometría.

| Índice | Rol | Eclipse | Daylight |
| --- | --- | --- | --- |
| 1 | Panel / fondo de ventana | `#181039` | `#fffff7` |
| 2 | Texto principal | `#efe7ff` | `#212918` |
| 3 | Línea y pista de barras | `#42317b` | `#a5b594` |
| 4 / 9 | Relleno de selección (Eclipse / Daylight) | `#104a63` | `#bdefb5` |
| 5 / 7 | Borde de selección y acento | `#5af7e7` | `#29734a` |
| 6 | Acento de cabecera (Daylight) | — | `#318452` |
| 11 | Texto atenuado (Eclipse) | `#ad9cd6` | — |
| 13 | Advertencia | `#ff3131` | `#b52118` |

`DrawUnholyUiPanelAccents` dibuja ahora el marco con la línea del tema, una regla de acento bajo la cabecera con un píxel de halo, una regla discreta sobre el pie y tachuelas de acento en las esquinas. La geometría (2 px de borde, cabecera de 19 px, pie de 19 px) es la misma que antes.

Contraste calculado (WCAG): texto sobre panel ≥ 14:1 en ambos temas; texto sobre selección ≥ 8:1 en Eclipse y ≥ 11:1 en Daylight; acento sobre panel ≥ 5,7:1. No verificado en mGBA: ver la lista de QA anterior.

## Iconos y detalles vivos

- `UnholyUi_Icon` dibuja iconos de 8×8 píxeles (un bit por píxel) con rectángulos de ventana: sin sprites, tiles ni paletas nuevas. El Poké Watch muestra un icono por servicio (tienda, mapa, crafteo, mira, cajas, señal, incursión, archivo, mutador, simulador) en el color de acento, atenuado y con un candado cuando el servicio está bloqueado.
- `UnholyUi_Footer` convierte los pares `TECLA:acción` en teclas con esquinas redondeadas (chip de acento con el nombre del botón y la acción al lado). Si el pie no tiene esa forma o no cabe, se imprime como antes, así que ninguna instrucción se pierde.
- `DrawUnholyUiPanelAccents` añade bajo la cabecera una trama de ajedrez de un píxel que degrada el halo hacia el panel.
- Las cabeceras de los paneles del reloj, Mutator, Simulador y búsqueda de PC llevan su icono a la derecha; el Pokélocator muestra la mira junto al nombre del mapa y Horizon una moneda junto al saldo.

## Crafteo (rediseño)

- Lista: cada receta lleva un icono de estado (● lista, ◐ faltan materiales, ○ sin materiales, candado rojo si falta medalla). La cabecera muestra el chip `LISTAS n` con las recetas que ya se pueden crear.
- Detalle: tarjeta con arte, nombre y descripción; debajo, una única barra de acción (`UnholyUi_Button`) que dice qué hará A (CREAR OBJETO / COMPRAR INYECTOR), pide confirmación, confirma el resultado o explica por qué no se puede (faltan materiales, medalla, dinero, mochila llena). Los materiales muestran ● o ○ rojo y `tienes/necesitas`; en Inyectores hay una fila de precio con moneda. Las filas terminan antes de la línea del pie (antes cruzaban la línea del pie). 
- Pies con teclas (`A:CREAR  START:INFO  SEL:EQUIP  B:SALIR`); el modo equipar usa el chip `EQUIPAR`.
- Neón: `UnholyUi_PulseAccent` hace respirar los dos colores de acento (5/7 Eclipse, 6/7 Daylight) hacia el blanco con ciclo de ~1 s, solo en paleta; se restaura al cerrar. No verificado en mGBA.

## Horizon Shop: huevos y tienda de ítems/discos

- Huevos: las calidades RADIANT y PRISMATIC llevan halo punteado y la tarjeta seleccionada se enmarca con su color. `HorizonEggPulse` anima los slots 15 (dorado respirando) y 12 (arcoíris lento) solo mientras el panel posee la paleta 14.
- Ítems y Discos (pantalla de compra): ya no usa la imagen de fondo ni el mapa de la tienda cuando Horizon está activo. Ventanas propias (`sHorizonBuyWindowTemplates`): cabecera de 2 filas con pestañas y dinero, lista de 7 filas (punto ● asequible / ○ sin dinero / candado agotado, nombre, precio, carril de scroll), panel de detalle (arte, nombre, descripción, y para MTs tipo, POW, ACC, PP; además `IN BAG`) y pie con teclas. Las paletas 13/14 (marco y diálogo) cargan la paleta del tema, y el color de fondo coincide con el panel.
- Presupuesto de tiles BG0: los bloques de ventana deben quedar por debajo de 0x300 (ahí empiezan los tilemaps); el último es el Sí/No en 0x2D2.
- El flujo de compra (cantidad, confirmación, dinero, bonus de Premier Ball) no cambia. Los Pokémarts normales conservan su pantalla clásica.
- Controles: arriba/abajo ±1, izquierda/derecha ±7, L/R cambia de pestaña. No verificado en mGBA.
