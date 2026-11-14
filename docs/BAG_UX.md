# Unholy Emerald — Mochila de diez bolsillos · UX

**Alcance:** mejora visual y navegación, no cambios al almacenamiento de objetos. Incluye las categorías virtuales de Gear, Correo y Miscelánea de la rama actual.

## Problema

La imagen de la mochila y sus indicadores originales fueron diseñados para cinco bolsillos. Unholy Emerald ya tiene **diez**: Objetos, Poké Balls, MT/MO, Bayas, Objetos Clave, Objetos de Combate, Medicina, Gears, Correo y Miscelánea. Las tres últimas son categorías virtuales sobre los almacenes existentes. Los bolsillos añadidos eran funcionales pero poco visibles; navegar diez categorías con L/R y animaciones consecutivas resultaba incómodo.

## Solución

- **Cinta persistente de diez categorías** arriba de la lista de objetos. Etiquetas de dos letras con posición fija por el enum original: `IT` (Objetos), `PB` (Poké Balls), `TM` (MT/MO), `BR` (Bayas), `KY` (Clave), `BT` (Combate), `MD` (Medicina), `GR` (Gears), `ML` (Correo) y `MX` (Miscelánea).
- El bolsillo activo usa **subrayado completo y estilo diferenciado**; los demás muestran una marca si contienen objetos. No se confunde vacío con bloqueado.
- **L/R sigue funcionando como antes**, con su animación.
- **L+R juntos abre un selector rápido desplazable de siete filas visibles para las diez categorías**: se muestran nombres completos y cantidad de pilas por bolsillo (sin contar la fila «Cerrar mochila»). Arriba/abajo o izquierda/derecha mueve el foco, **A salta directamente** al bolsillo seleccionado **sin recorrer animaciones intermedias** y **B vuelve** sin cambiar nada.
- Cada bolsillo conserva sus propios cursores y scroll; se reconstruye el list menu y sus flechas de navegación sin alterar objetos ni su clasificación.
- El selector no se activa en tutoriales de captura, combates/eventos con cambio de bolsillo deshabilitado, modo especial Victory Catch ni contextos de enlace, para respetar sus controles existentes.

## Consideraciones técnicas

- `WIN_POCKET_TABS` se sitúa en la franja superior derecha (BG0 x14, y0, 15×2 tiles, base 0x260). Ningún texto debe pisar la lista, que comienza en y2.
- El selector usa BG1 x7, y2, 17×17 tiles, base 0x2A0 y paleta Eclipse/Daylight; se libera al cerrarlo.
- No se modifican `struct Bag`, `SaveBlock1`, `BagPocket`, `POCKETS_COUNT`, rutas de uso de ítems, slots de inventario ni el mecanismo de ordenación por START o intercambio con SELECT.

## QA obligatorio

- [ ] Compila Emerald, FireRed y LeafGreen.
- [ ] Las diez etiquetas, sobre todo `BT` y `MD`, son legibles y visibles en pantalla real 240×160; no se tapan por el sprite de la bolsa ni las paletas.
- [ ] El modal cabe sin ocultar contadores; funciona con L+R, flechas, A, B.
- [ ] El salto directo preserva cursor/scroll por categoría y refresca icono, nombre, descripción, números y flechas.
- [ ] Abrir/cerrar modal muchas veces no deja ventanas, tareas o paletas huérfanas.
- [ ] Los cambios de bolsillo tradicionales con L/R siguen intactos y el menú de ordenar con START, intercambiar con SELECT, usar, dar, vender y depositar continúan operativos.
- [ ] Wally, enlaces y contextos de bolsillo fijo no muestran accesos prohibidos.
- [ ] No aumenta SaveBlock ni cambian ítems ni cantidades tras guardar/cargar.

El cambio se limita al UX de la mochila; los errores generales heredados de la suite de pruebas necesitan una revisión separada antes de fusionar el conjunto de PRs.

La paleta del tema se adquiere después de cargar las ventanas nativas y se libera al salir de la mochila. Cerrar el selector no devuelve prematuramente su paleta. Cada fila reserva 14 píxeles para texto de 12 píxeles reales; abrir/cerrar y desplazarse conserva el tema.
