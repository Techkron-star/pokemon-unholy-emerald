# Unholy Emerald — interfaz nativa 240 × 160

La referencia conceptual es el PDF «Pokewatch UI – Unholy Emerald». Esta entrega adapta su composición al motor GBA de la rama `feat/unholy-cosmic-trainer-gears`. Las mecánicas, precios, probabilidades, progresión y datos de guardado siguen perteneciendo a los proveedores existentes.

## Componentes y resolución

- Ventanas 4 bpp, texto real del motor, bordes de un píxel y selección con borde y relleno. No se reduce una imagen del PDF ni se dibuja texto dentro de un recurso gráfico.
- `unholy_ui` comparte recuadros, barras, texto acotado por anchura y préstamos de paleta. Cada ventana de campo que toma la paleta 14 devuelve sus 16 colores al cerrarse.
- Eclipse y Daylight conservan geometría, datos, navegación e instrucciones. El tema se elige en Opciones y dispone de vista previa; el reloj del juego sigue siendo independiente.
- Cabecera de 19 px; cuerpo con filas de 11–17 px según el contexto; pie separado para instrucciones. Se usan las fuentes existentes de 12 px y 8 px. Los nombres largos se acotan por medida tipográfica.

## Pantallas del PDF y adaptación

| Referencia | Implementación |
| --- | --- |
| 1, 6 · Pokéwatch | Diez servicios, cinco filas visibles, desplazamiento y contador. La fila bloqueada conserva el foco y muestra el requisito real. A nunca lanza un servicio sin acceso. Se conserva el servicio elegido al regresar. |
| 2 · Horizon | Cinco categorías existentes: Pokémon, Discos, Objetos, Huevos y Movimientos. Las seis ofertas Pokémon y cinco huevos reutilizan su compra, confirmación, stock y RNG persistentes. |
| 3 · PokéNav | Se conserva el mapa real y sus funciones. El panel de ubicación adopta la paleta común y limita los nombres a su espacio. |
| 4 · Pokélocator | Catálogo 2×2 por especie, categorías de encuentro, filtros de colección y progreso. Ficha Habitat/Stats/Traits, elección de método, revisión previa al combate e integración de la Señal existente. Eclipse/Daylight comparten datos y navegación. Véase [Pokélocator](POKELOCATOR_SEARCH.md). |
| 5 · Mutator | Lista del equipo, planes, confirmación y resultado comparten marco, paleta y ayudas. Coste, riesgo y requisitos siguen calculándose en el servicio existente. |
| 7 · Huevos | Rango y calidad se identifican por separado. La compra sigue ocultando la especie. No hay una acción de reinicio del premio. |
| 8–11 · Resumen | Cuatro pestañas: INFO, STATS, MOVES, GEAR. Incluye sprite real, PS, stamina máxima, IV/EV, ranuras de objeto/Gear independientes, las nueve mutaciones combinables y bonos positivos/negativos calculados por estadística. |
| 12 · Crafteo | Nombre y descripción de receta, ingredientes y cantidades poseído/requerido en columnas distintas, estados OK/NEED, requisito de medalla y confirmación existente. |
| 13, 19 · Combate | Se conservan las tarjetas de PS, estado, anillos e iconos de Gear existentes. En selección de movimientos, una franja muestra dos posiciones de cadena, nombres, coste previsto y stamina disponible. SELECT se indica para marcar/cancelar; A conserva las validaciones existentes. La franja se oculta con descripciones y al salir. |
| 14 · START | WATCH sigue siendo el acceso al reloj; se conservan las rutas y el retorno del menú existente. |
| 15 · Opciones | Se muestran las opciones reales de la rama con tipografía de 12 px y vista previa Eclipse/Daylight. |
| 16 · Mochila | Selector L+R con los siete nombres reales y recuentos de pilas, foco independiente del contenido y paleta restaurada al salir. Se mantienen cursores, ordenación, intercambio y restricciones de contexto. |
| 17 · Equipo | Las tarjetas originales conservan PS, estados y marca de Gear. El mensaje de selección en el campo distingue PS actuales/máximos de STA MAX. No se inventa una stamina persistente fuera del combate. |
| 18 · Diálogo | Las cajas usan la paleta común; el nombre del hablante se ajusta a su fila reservada. Se conservan marcos, paginación y opciones del motor. |

## Servicios complementarios del reloj

- **PC:** abre la organización existente de cajas y regresa al reloj; conserva sus operaciones y selección.
- **Archivo:** nueve registros con nombre, tier y estado, contenidos en una ventana de 24 × 18 tiles.
- **Señal mutante:** muestra estado diario, encuentros restantes y activación existente.
- **Simulador:** selección de Pokémon, edición de IV/EV, comparación de estadísticas y revisión del pago. Las modificaciones se aplican solo al confirmar.
- **Incursiones:** dificultad y requisito, jefe y equipo, probabilidad de captura, ronda actual y elección de recompensa. El panel no adelanta el estado de la incursión por dibujarse.

## Controles de Resumen

- Izquierda/derecha cambia pestaña. Arriba/abajo cambia Pokémon en Info, Stats y Gear; en Movimientos recorre las cuatro ranuras.
- L/R alterna Stats → IV → EV y Objeto/Gear → Mutaciones → Bonos.
- A en Info o Movimientos abre el detalle original, incluidas sus operaciones existentes. B en ese detalle vuelve al nuevo Resumen y conserva el Pokémon elegido.
- El aprendizaje, sustitución y borrado de movimientos, contextos de enlace y Resumen durante combate conservan la pantalla original. Las vistas normales de equipo y PC usan la nueva composición; la selección especial `PC_MON_CHOSEN` conserva su tratamiento original.

## Validación

Comprobar ambos temas en 240 × 160, nombres largos, huevos, ranuras vacías, varias mutaciones, penalización de Velocidad de Titán, IV/EV, objeto y Gear simultáneos, listas vacías, funciones bloqueadas y fondos después de cerrar ventanas.

En combate: movimiento marcado y sin marcar, segundo movimiento distinto/protección/sin PP, coste superior a stamina, cancelación, descripción, Megaevolución y dobles. La franja de cadenas solo aparece cuando el motor permite encadenar.

Comprobar diez ciclos de apertura/cierre del reloj y selector, regreso desde cada servicio, compra/cancelación sin regenerar stock y guardar/cargar. Las pruebas de datos verifican los requisitos del reloj y que el snapshot del Resumen conserva Pokémon, objetos y bonos reales.
