# Horizon Nebula — especificación de UI para Unholy Emerald

**Fecha:** 2026-11-09  
**Rama de implementación actual:** `feat/unholy-horizon-ui-polish-20261112` (PR #30, apilado sobre #29)  
**Estado:** las pantallas base están programadas; falta validación visual 240×160 en emulador y la suite de CI. Lo restante se marca como propuesta.

## Referencias verificadas

- `DISENO.md`, sección 5 (Horizon Shop): cinco categorías, progresión por medallas, RNG y ciclo de día/noche.
- `Techkron-star/Nebulous/src/engine/nebula/themes.ts`, temas existentes `eclipse` y `daylight`.
- `src/shop.c`: implementa cinco pestañas, mercado de Pokémon/huevos, objetos/MT y MOVES; el PR #30 introduce cuadrícula, tarjetas horizontales y controles por contexto. No se considera probado visualmente hasta QA.
- `data/scripts/crafting.inc`: la receta Super Exp. Share funciona mediante un menú de script básico, aún sin interfaz de receta visual.
- `src/option_menu.c`: configuraciones existentes; necesita una nueva preferencia persistente para tema de UI.

## 1. Lenguaje visual y restricciones de hardware

Resolución interna: **240×160 píxeles**; tamaños de iconos y tipografía adaptados a la pantalla real, no al tamaño de los mockups ampliados.

- Unificar contornos (1–2 píxeles), jerarquía de títulos, barra de pestañas, listas, estados, recuadros de información y controles de regreso.
- Mantener legibilidad por encima de adornos: no solapar sprites, textos, información económica o números de unidades.
- No usar imágenes de interfaz predibujadas con letras que se distorsionen o que no respondan al estado del juego. Dibujar texto real del motor sobre ventanas.
- Paletas compartidas de fondos, sprites de ítems y acentos; respetar BG/OBJ VRAM, OAM, tiles, límites de 4bpp y budget de paletas.
- Transiciones opcionales mediante paleta/tiles y pocos frames; **sin WebGL**, bloom real ni animación que ralentice combate o menú. Los temas imitan el arte de Nebula Engine, no su motor de render.
- A/D-pad para seleccionar; L/R cambia pestaña; A confirma; B retrocede; SELECT muestra detalles si es viable. En los prompts siempre nombrar la acción real.
- El estilo no cambia el RNG, los precios, los requisitos ni la información funcional. El texto de los dos temas es idéntico.

### Eclipse (identidad Nebulous)

Fuente: Nebulous `themes.ts`: «Umbra de eclipse total, corona cian, perlas de luz y lente gravitatoria».

| Token | Dirección |
|---|---|
| Fondo | Azul negruzco, círculos umbra muy tenues |
| Panel | Pizarra oscura y acero violeta |
| Bordes activos | Cian eléctrico tenue, ribete plateado |
| Selección | Azul petróleo con foco cian, sin estrobo |
| Texto | Marfil frío, contraste alto |
| Motivo visual | Un eclipse discretamente dibujado en cabecera y halo estático detrás de icono |

### Daylight (identidad Nebulous)

Fuente: Nebulous `themes.ts`: «Marfil cálido, nácar estriado y reflejos de concha, con tipografía y controles de alto contraste».

| Token | Dirección |
|---|---|
| Fondo | Marfil/nácar |
| Panel | Blanco cálido con bandas crema |
| Bordes activos | Dorado pálido y turquesa suave |
| Selección | Verde salvia claro con contorno contrastado |
| Texto | Grafito oscuro |
| Motivo visual | Estrías perladas y reflejo corto en el encabezado, sin distraer |

### Opciones y persistencia

Nueva entrada visible en Opciones: `UI THEME: ECLIPSE / DAYLIGHT`. Guardar una selección válida en variable persistente de partida o campo seguro en save (definir después de auditar mapa de variables y tamaño real de SaveBlock). No reutilizar flags temporales. Aplicar al volver a abrir menús, preferiblemente sin reiniciar la ROM. El tema no sigue el reloj: la rotación día/noche del catálogo es **independiente**. El sistema tiene fallback funcional con paleta estándar si falla la carga del tema.

## Catálogos rotativos de objetos y discos (10/11/2026)

Se sustituye el stock de prueba fijo por ofertas persistentes: **ocho objetos (tres básicos y cinco aleatorios)** y **seis MT distintas**. Se filtran por progreso y hora del juego; dos variables de semilla de 16 bits permiten regenerar la misma selección sin llamar al RNG del juego cada vez que se abre la tienda. El catálogo cambia solo cuando rota Horizon por día, período o nueva medalla. No modifica compras ni huevos de otras pestañas; los precios son los normales del catálogo base.



Los PR apilados implementan la barra horizontal de cinco pestañas (PKMN / DISCS / ITEMS / EGGS / MOVES). La entrega de **PR #30** convierte las seis ofertas Pokémon a una **cuadrícula 3×2**, los cinco huevos a una **hilera horizontal de cinco tarjetas** con calidad visual y MOVES a dos tarjetas de servicio. Las flechas y L/R se adaptan al contexto; A compra o elige y B cancela o retrocede.

La raíz usa una ventana horizontal BG0 de 27×3 tiles desde (2,1), y el panel detallado otra de 27×14 desde (2,5). El cursor se dibuja de forma propia y **no utiliza** el procesador de menú vertical del PokéMart. El cierre de Horizon destruye su ventana, el icono de huevo y su paleta temporal, y usa `CB2_ReturnToFieldWithOpenMenu` para reconstruir correctamente el campo y START. Los PokéMart convencionales conservan su ruta original.

**QA bloqueante:** abrir/cerrar Horizon diez veces desde START, entrar/salir de Pokémon y huevos con B, comprar en DISC/ITEMS y volver, cambiar de pestaña con ambos esquemas de controles, y confirmar que la tienda no deja la UI congelada ni paletas erróneas en Eclipse/Daylight. El PR está en borrador; la corrección requiere comprobarse en emulador.


## Renaissance Horizon UI — entrega PR #30 (12/11/2026)

Diseño implementado mediante ventanas GBA 4bpp y fuentes reales, **sin imágenes generadas** ni efectos que comprometan recursos del motor. La estructura evita los solapamientos visibles en las capturas de Pizza Boy.

| Pantalla | Geometría en píxeles locales | Controles y comportamiento |
|---|---|---|
| Pestañas raíz | BG0 27×3 tiles en (2,1); cinco celdas de 42 px con línea de selección | Izquierda/derecha o L/R; A entra; B vuelve a START |
| Ficha principal | BG0 27×14 tiles en (2,5), 216×112 px | Título/tema arriba y ayudas abreviadas abajo |
| Pokémon | Cuadrícula 3 columnas × 2 filas, tarjetas de 66×22 px, nombres recortados por medida tipográfica, precio individual y indicador mutante | Izquierda/derecha/L/R recorre columnas, arriba/abajo cambia fila; A selecciona y otra A confirma; B cancela antes de salir |
| Huevos | Cinco tarjetas de 38×42 px, desde x=6 con paso de 41 px; miniaturas de 14×13 con sombra y borde según calidad | L/R o izquierda/derecha elige; detalle muestra grado, calidad y coste; B cancela o vuelve; especie oculta |
| MOVES | Dos tarjetas de 92×40 px (RELEARN/FORGET); texto descriptivo recortado | Flechas y L/R alternan; A entra, B vuelve |
| ITEMS/DISCS | Se conserva la ventana del PokéMart de **15×16 tiles** (120×128 px); seleccionado y siguiente, precio, nombre completo recortado y marcas de posición | L/R recorre stock; A compra; B sale; **nunca** invade dinero, descripción o cantidad |

La posición se conserva durante una **misma sesión Horizon**, por separado para Pokémon, Huevos, Discos, Objetos y MOVES. Solo se reinicia al abrir una sesión nueva. El catálogo y el RNG **no** cambian al moverse entre tarjetas; las compras, mutaciones y grados siguen las funciones persistentes existentes.

Los anchos de nombre se ajustan con `GetStringWidth(FONT_NARROW, ...)`, no mediante caracteres fijos. Los acentos de selección usan los colores existentes de Eclipse/Daylight. Las miniaturas de huevo se pintan con `FillWindowPixelRect` y el icono nativo se reserva en el lateral inferior derecho.

**QA bloqueante:** emulador a 240×160, comprobar especies y MT con nombres largos, saldo de seis dígitos, compra agotada, calidad Prismática/Legendaria, precio y botón de confirmación, paletas día/noche, doble B, cancelación, selección persistente, volver de MOVES y 10 ciclos de entrada/salida. No fusionar hasta compilar y verificar visualmente.

## 2. Horizon Shop — pantalla raíz y pestañas

Cinco pestañas siempre navegables:

1. `PKMN`: **Pokémon**. Seis ofertas persistentes, cada una con RNG propio. Máximo Ancestral: **sin Titan, Ascendido ni Omega**. La ficha muestra especie, mutación, precio y compra con confirmación; entrega a equipo o PC.
2. `DISCS`: **Discos**. Catálogo de MT/discos desbloqueados. Icono de disco, nombre del movimiento, tipo, compatibilidad/información, precio y restricciones. No inventar un tipo nuevo de disco sin definición en diseño.
3. `ITEMS`: **Objetos**. Consumibles, objetos especiales y mercancía normal permitida según progresión. Mostrar icono, precio y cantidad. Infinity Ball **excluida**: premio posjuego/cheat, no mercancía.
4. `EGGS`: **Huevos**. Cinco rangos Básico, Medio, Alto, Mítico y Legendario, visualizados como **huevos** con icono nativo. Cada uno tiene calidad visible Estándar/Radiante/Prismática tirada por RNG de forma independiente, guardada durante el período. Al comprar se realiza una ruleta de premios oculta con posibles legendarios, míticos, Omega y hasta tres mutaciones; ninguna recompensa garantizada.
5. `MOVES`: **Movimientos**. Dos acciones internas, `LEARN` y `FORGET`; primero elegir Pokémon, después movimiento. Reutilizar lógica del recordador/borrador y comprobaciones de HM y compatibilidad ya existentes.

### Layout interno propuesto (coordenadas indicativas, validar con fuente real)

- `y=0–15`: encabezado `HORIZON`, icono sutil de Eclipse/Daylight, saldo compacto.
- `y=16–35`: tira de pestañas de cinco segmentos, 48 px aprox. cada uno; etiquetas abreviadas pero distinguibles.
- `y=36–115`: lista desplazable (izquierda / centro); iconos y cantidades sin tapar el precio. Si es Pokémon, reservar retrato/sprite de especie.
- `y=116–145`: panel fijo de descripción de selección; cambiar texto cuando cambia foco. Errores y avisos de confirmación deben usar diálogo encima o pantalla de confirmación, nunca superponer dos cajas sin limpiar.
- `y=146–159`: ayudas `A BUY`, `B BACK`, `L/R TABS` (o sus equivalentes por servicio).

**Nota:** un mockup en alta resolución puede mostrar paneles simultáneos que no caben a 240×160; el implementador debe mantener la información disponible por selección, scroll o pantalla de detalle, no reducirla hasta volverla ilegible.

### Flujo

`Start Menu → Horizon Shop → pestaña → producto/servicio → información/requisitos → confirmar → resultado → regresar a la misma pestaña y posición`.

Reglas:
- B retrocede exactamente un nivel; sin reentradas duplicadas ni pantallas negras.
- El cursor y scroll de cada pestaña se conservan durante la misma sesión.
- La mercancía de Pokémon se genera y **persiste** por día/período día-noche o nuevo hito de medallas; no se vuelve a tirar al entrar/salir del menú. No se permite usar ajustes regresivos del reloj para regenerar ofertas.
- Los precios y denominadores en `src/horizon_market.c`, `src/horizon_eggs.c` y `include/config/special_badges.h` son **valores provisionales**. Seis plazas Pokémon y cinco de huevos, una unidad por plaza; cada compra agota esa plaza hasta la siguiente rotación válida.
- En compras con equipo/caja llenos, dinero insuficiente, bolsillo lleno, objeto no permitido o habilidad incompatible, mostrar causa y volver a lista en buen estado.
- Garantizar que los servicios no cobren si cancelan.

## 3. Crafting — menú con requisitos visibles

Se usará el mismo sistema de interfaz, con lista de recetas a la izquierda y panel de ficha a la derecha; **aunque solo haya una receta real**, las filas vacías no son recetas falsas.

- Receta inicial confirmada: `SUPER EXP. SHARE`.
- Requisitos: `EXP. SHARE ×1` (se consume) y `STONE BADGE` (solo comprobación, no se consume).
- Cada requisito muestra icono y estado `1/1`, `0/1`, `OK` o `LOCKED`.
- Seleccionar una receta actualiza ficha, descripción, materiales, requisitos, icono del resultado y estado de botón `CRAFT`.
- A abre confirmación; B cancela sin consumo. Solo retirar el objeto base después de comprobar materiales, espacio y aceptar. Si falla una validación, no cambiar inventario.
- Tras elaborar, actualizar lista y panel en la misma sesión; si un menú externo se abre/cierra, liberar ventanas, tiles, sprites y callbacks.
- Las futuras recetas usan una tabla de datos (id de receta, materiales, requisitos, resultado, count, desbloqueos, textos, icono). No bifurcar scripts para cada receta.

## 4. Sprites originales y glint mutado

Ítems personalizados existentes que necesitan icono reconocible y coherente a tamaño original:
- `SUPER EXP. SHARE`: núcleo metálico con 3 nodos luminosos; no reutilizar el sprite del Exp. Share normal.
- `OMEGA BALL`: emblema Ω, carmesí/negro.
- `INFINITY BALL`: oscuro con motivo de infinito, prestigio posjuego. **Nunca venderla en Horizon Shop**.
- `OVERDRIVE BALL`, `ALPHA BALL` y Gears: solo cuando sus reglas se definan.
- Los concepts opcionales `Horizon Pass`/`Crafting Kit` no constituyen objetos del juego hasta tener definición jugable.

Cada icono requiere lámina de píxeles limpia, paleta autorizada, transparencia, bordes coherentes y prueba con fondo claro/oscuro. Preferencia por assets fuente reproducibles (PNG con píxeles exactos) e integración en el sistema de iconos del proyecto.

Para Omega: reutilizar el **timing y mecanismo de aparición del glint shiny** pero con nueva forma de estrella de **seis puntas**, interior rojo intenso y borde negro. Mantener el shiny original para Pokémon shiny; el Omega se activa por badge de mutación independientemente de shininess. Si un Pokémon es Omega y shiny, permitir ambos efectos sin fuga de sprites/paletas y sin animación infinita. No confundir el glint con el indicador `O` del resumen.

## 5. Propuesta de arquitectura de implementación

1. Extraer un componente `UnholyMenuSkin` en C: inicialización de BG/window tiles, paletas Eclipse/Daylight, text colors, cursor, botón, marco, teardown seguro.
2. Almacenar preferencia `UI THEME` desde Opciones y dar fallback seguro en ambas rutas de guardado y nueva partida.
3. Crear `HorizonMenu` con estado `tab, selectedIndex[5], scroll[5], action`; refrescar solo paneles necesarios y nunca conservar punteros de ventana destruidos.
4. Integrar primero **tab Objetos** con el catálogo actual. Mantener el resto de pestañas visibles con mensaje honesto `COMING SOON` hasta tener funcionalidad, sin compras falsas.
5. Implementar Pokémon/Discos/Huevos/Movimientos de forma incremental; conservar oferta/sesión y comprobar límites en cada ruta.
6. Sustituir el menú de script de crafting por ventana de recetas y panel de materiales, manteniendo la receta validada existente.
7. Integrar nuevos sprites custom, el Omega glint separado de shiny, e instrumentos de QA.

### QA y criterios de aceptación

- Temas con contraste legible: nombres largos, iconos especiales, mensajes de error, saldo de hasta su límite y scroll extenso.
- A/B/L/R no dejan bloqueada la pantalla; abrir/cerrar diez veces seguidas no consume VRAM ni duplica tareas.
- El tema persiste al guardar/cargar; cambiar paleta no modifica resultados de RNG.
- Comprar un Pokémon entrega exactamente su mutación visible persistida; comprar un huevo realiza un roll de contenido distinto de la calidad visual y conserva especie/badges hasta eclosión. Infinity Ball nunca se vende. Un pedido entrega y cobra una sola vez.
- Craftear comprueba requisitos y consume solo el material permitido, una vez.
- La versión shiny sin badge conserva brillo normal; Omega usa estrellas rojas de seis puntas incluso sin ser shiny; combinados se prueban expresamente.
- Build y pruebas reales en mGBA/otro emulador; una imagen generada no equivale a sprite o pantalla ya implementada.

## Progreso

- [x] Inspeccionado `DISENO.md` y las paletas conceptuales de `Eclipse` / `Daylight` de Nebulous.
- [x] Navegación, temas y contratos de pantallas definidos.
- [ ] Implementación persistente de UI Theme en la ROM.
- [x] UI temática de Horizon con cinco pestañas implementada en C, incluyendo rediseño PR #30; pendiente de QA real en 240×160.
- [x] Catálogos avanzados y MOVES RELEARN/FORGET implementados en ramas apiladas; todavía deben superar QA y pruebas de guardado.
- [x] Menú real de crafting (una receta) con panel de materiales y medalla, estados dinámicos 1/1 y 0/1, estilo Eclipse/Daylight y teclas A/B. Reutiliza script de validación y confirmación, pendiente de revisión visual en emulador.
- [x] Se agregaron sprites indexados 24x24 con iconos dedicados para Omega Ball, Infinity Ball y Super Exp. Share, y el recurso de estrella Omega. El glint todavía necesita conectarse al combate.
- [ ] Compilación, QA visual e integración aprobada.

**Glint Omega:** conectado a `TryShinyAnimation` reutilizando órbitas y duración shiny, pero con estrella roja/negra distinta y etiqueta de recurso independiente. La finalización espera ambas animaciones si el Pokémon es shiny y Omega; requiere QA en emulador y CI.
