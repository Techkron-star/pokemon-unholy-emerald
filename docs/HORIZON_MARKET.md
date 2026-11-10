# Horizon Shop · ofertas mutantes y notificaciones

**Estado:** implementación experimental en PR #11 (apilado sobre #10 → #9 → #8), NO integrada en master. Los tests y el emulador deben aprobar antes de fusionar.

## Contrato de producto

- El mercado Pokémon muestra **dos ofertas reales** por período; cada oferta admite una compra.
- Las especies se extraen de una lista segura, gated por número de medallas y horario diurno/nocturno. No aparecen legendarios ni Infinity Ball.
- Cada oferta tira independientemente sus probabilidades de mutación usando los denominadores de `include/config/special_badges.h`. En esta versión se guarda **una** mutación como máximo por oferta; los Pokémon salvajes pueden acumular varias según su propia mecánica.
- Medallas mínimas para cada rareza: Alpha 0, Hyper 1, Super/Overloaded 2, Titan 3, Enraged 4, Ancestral 5, Ascended 7, Omega 8.
- Los costos iniciales van desde `2000 + medallas × 1200` para normales hasta un recargo de 180000 para Omega. Son **valores de balance provisionales**.
- El comprador recibe un Pokémon con exactamente el badge de la oferta; `CalculateMonStats` aplica los efectos y las reglas de genética Omega/Ascended existentes.
- Comprar solo ocurre con doble A. B cancela primero la confirmación y después retrocede. No se descuenta dinero si no se puede entregar el Pokémon al equipo o PC.

## Contrato de persistencia

Se usan ocho variables previamente sin uso del guardado, todas en `include/constants/vars.h`: `0x40F7–0x40FE`. No se incrementa el tamaño de ningún SaveBlock.

| Variable | Propósito |
| --- | --- |
| `0x40F7` | Día de RTC del catálogo |
| `0x40F8` | Período (diurno/nocturno) |
| `0x40F9`, `0x40FA` | Especies de los dos slots |
| `0x40FB` | Dos códigos de mutación de cuatro bits + bits 8 y 9 de agotado |
| `0x40FC` | Estado de aviso leído |
| `0x40FD` | Versión/marcador de inicialización |
| `0x40FE` | Número de medallas de la rotación |

La primera consulta en un guardado anterior inicializa el catálogo. El stock se recalcula si avanza el día, pasa de diurno a nocturno o se obtiene una nueva medalla. Retroceder el reloj dentro de un día no genera un nuevo roll; abrir/cerrar el menú tampoco. Si no hay RTC operativo hay que validar en emulador el fallback antes de considerar lista la función.

## Jerarquía de señales

1. **START:** aparece un punto rojo de 3×3 píxeles al lado de HORIZON, usando el rojo que ya existe en la paleta de menú estándar. No se asigna una nueva paleta.
2. **HORIZON:** aparece un punto rojo de 4×4 al lado de la pestaña POKÉMON. Las paletas Eclipse y Daylight reservan su color 13 para rojo.
3. **Panel:** un aviso `!!! RARE STOCK`, seguido de `!!! ALPHA`, `!!! OMEGA`, etc. al inspeccionar la ficha seleccionada.

**La señal solo se marca como vista al pulsar A en la pestaña POKÉMON** y abrir sus ofertas. Un mutante no comprado sigue en stock, pero no dispara otra alerta hasta que una nueva rotación lo sustituya.

## QA bloqueante

- [ ] Compilan Emerald, FireRed y LeafGreen y no crecen las estructuras de guardado.
- [ ] En las dos paletas, los puntos rojos son realmente visibles y los textos largos no invaden la interfaz 240×160.
- [ ] A, B, L/R, arriba/abajo funcionan en Horizon sin bloquear menús y sin tareas duplicadas; salida normal con B desde submenú y desde raíz.
- [ ] No hay ningún aviso si las dos ofertas son normales, fueron compradas o ya fueron inspeccionadas.
- [ ] Guardar/cargar conserva ofertas, estado de compra y estado de leído; abrir/cerrar diez veces no cambia el RNG de stock.
- [ ] Día y noche actualizan stock una vez; ajustar el reloj hacia atrás no produce ofertas infinitas.
- [ ] Cambiar una medalla actualiza especies elegibles y ninguna especie inaccesible aparece al principio.
- [ ] Equipo completo entrega a PC; equipo y PC completos rechazan sin cobrar.
- [ ] Saldo insuficiente, cancelación y reintento no entregan ni cobran.
- [ ] Mutaciones almacenadas, incluidos Omega y Ascended, conservan sus buffs y datos genéticos.
- [ ] El CI heredado de los PR #8/#9 debe corregirse o identificarse explícitamente por causa; no ignorarlo.
