# Horizon Nebula — especificación de UI para Unholy Emerald

**Fecha:** 2026-11-09  
**Rama objetivo:** `feat/stamina`  
**Estado:** especificación de interfaz; ninguna de las pantallas descritas se declara implementada por este documento.

## Referencias verificadas

- `DISENO.md`, sección 5 (Horizon Shop): cinco categorías, progresión por medallas, RNG y ciclo de día/noche.
- `Techkron-star/Nebulous/src/engine/nebula/themes.ts`, temas existentes `eclipse` y `daylight`.
- `src/shop.c`: actualmente ofrece un catálogo inicial fijo de objetos. No dispone de secciones de Pokémon/huevos/discos/servicios.
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

## 2. Horizon Shop — pantalla raíz y pestañas

Cinco pestañas siempre navegables:

1. `PKMN`: **Pokémon**. Dos ofertas persistentes por RNG, medallas y día/noche, con mutaciones extraordinariamente raras. Omega requiere ocho medallas. La ficha muestra especie, mutación, precio y compra con confirmación; destino al equipo o PC cuando hay espacio.
2. `DISCS`: **Discos**. Catálogo de MT/discos desbloqueados. Icono de disco, nombre del movimiento, tipo, compatibilidad/información, precio y restricciones. No inventar un tipo nuevo de disco sin definición en diseño.
3. `ITEMS`: **Objetos**. Consumibles, objetos especiales y mercancía normal permitida según progresión. Mostrar icono, precio y cantidad. Infinity Ball **excluida**: premio posjuego/cheat, no mercancía.
4. `EGGS`: **Huevos**. Huevos aleatorios con precio y condiciones de entrega. No revelar especie si es misteriosa; no prometer Omega ni mutación rara; sin mecánica especial no definida.
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
- Los precios y denominadores en `src/horizon_market.c` y `include/config/special_badges.h` son **valores iniciales sujetos a balance**. Dos plazas, una unidad por plaza; cuando se compra, queda agotada hasta la siguiente rotación válida.
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
- Comprar genera exactamente la mutación persistida en la oferta (si la hubiera), nunca una tirada oculta adicional; Infinity Ball no se vende. El Pokémon adquirido entra una sola vez en equipo o PC y el pago se descuenta una sola vez.
- Craftear comprueba requisitos y consume solo el material permitido, una vez.
- La versión shiny sin badge conserva brillo normal; Omega usa estrellas rojas de seis puntas incluso sin ser shiny; combinados se prueban expresamente.
- Build y pruebas reales en mGBA/otro emulador; una imagen generada no equivale a sprite o pantalla ya implementada.

## Progreso

- [x] Inspeccionado `DISENO.md` y las paletas conceptuales de `Eclipse` / `Daylight` de Nebulous.
- [x] Navegación, temas y contratos de pantallas definidos.
- [ ] Implementación persistente de UI Theme en la ROM.
- [ ] UI temática funcional de Horizon Shop con 5 pestañas.
- [ ] Catálogos avanzados y servicios funcionales.
- [x] Menú real de crafting (una receta) con panel de materiales y medalla, estados dinámicos 1/1 y 0/1, estilo Eclipse/Daylight y teclas A/B. Reutiliza script de validación y confirmación, pendiente de revisión visual en emulador.
- [x] Se agregaron sprites indexados 24x24 con iconos dedicados para Omega Ball, Infinity Ball y Super Exp. Share, y el recurso de estrella Omega. El glint todavía necesita conectarse al combate.
- [ ] Compilación, QA visual e integración aprobada.

**Glint Omega:** conectado a `TryShinyAnimation` reutilizando órbitas y duración shiny, pero con estrella roja/negra distinta y etiqueta de recurso independiente. La finalización espera ambas animaciones si el Pokémon es shiny y Omega; requiere QA en emulador y CI.
