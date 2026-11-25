# Gears con sprites nuevos · lote 15/11/2026

**Base de integración:** `feat/unholy-combat-rebalance` (#58). Los tres lotes de arte proceden de `art/unholy-five-gear-sprites`, con imágenes 24×24 para mochila y 16×32 (dos fotogramas) para combate. Los IDs se añaden después de `GEAR_SHACKLE` / `ITEM_GEAR_SHACKLE`, sin reordenar los antiguos.

## Efectos conectados

| Gear | Implementación de este lote | Límites y QA específico |
|---|---|---|
| Escudo de Aegis | +n etapas positivas de Defensa/Def. Esp. se calculan como +2n, sin modificar niveles almacenados. +6 visible equivale a multiplicador ×7, sin limitar el efecto al multiplicador nativo de +6. | Críticos, Ignorante y movimientos que ignoran aumentos permanecen normales. Probar dobles y movimientos que usan Defensa en ataques especiales. |
| Ariete Náutilo | Movimiento dañino de contacto reduce una etapa de Defensa por evento de impacto válido, con motor nativo de bajadas. | Verificar mensajería, habilidades contrarias y cada impacto de multigolpe en emulador. |
| Guantes Cohete | +1 prioridad en movimientos clasificados como puñetazos; se suma a la prioridad innata y de habilidades. | Dobles, terreno Psíquico y ataques ya prioritarios. |
| Túnica Rúnica | Bloquea trampas de entrada del lado propio y elimina las existentes al entrar; si elimina alguna, +1 At. Esp. hasta el límite habitual. Niega daño ambiental de clima, Mar de Llamas y daño residual de tipo del campo. | La elección de limpiar solo el lado propio y de activar +1 únicamente al limpiar trampas es **provisional**. Otros efectos residuales de atrapamiento requieren ampliación. |
| Cartel de Búsqueda | Bonificación de daño según suma de etapas positivas del objetivo. | **Provisional:** +5 % por etapa, máximo +60 %. No se computan etapas negativas ni IV/EV. |
| Nihil Engine | Cada tres finales de turno activo elimina una trampa del lado rival, sin borrar todas a la vez. | La limpieza de campos ordinarios y su procedencia no está implementada; los efectos especiales no deben borrarse sin clasificación explícita. |
| Anillo Feérico | Portadores de tipo Hada reciben menos daño directo de ataques Veneno. | **Provisional:** reducción 25 %. El daño residual de envenenamiento aún no está cubierto. |
| Piedra de Afilar | Un crítico con daño real a un rival restaura cualquier etapa negativa de Ataque directamente a +1. Sin efecto con Ataque neutro o positivo; no requiere un movimiento cortante. | No activa por fallo, inmunidad, sustituto o daño a aliados. La restauración fija no se multiplica ni invierte por Simple o Contrary. Verificar mensajería y multigolpes. |
| Batería de Reserva | Si tras regenerar queda bajo 50 % de STA, recupera 20 % máximo y entra en recarga; durante recarga recupera 4 % adicional por final de turno. | Números tomados del catálogo preliminar; probar agotamiento, paralizar y cadenas con STA límite. |
| Botas del Umbral | Al entrar con trampas, una carga anula todas las trampas de **esa entrada**. Durante recarga reduce 50 % del daño de trampas, pero no sus estados ni bajadas. | Probar interacción con Botas Gruesas, Toxic Spikes y cambios sucesivos. |
| Velo Críptico | Una carga niega un efecto secundario rival que realmente hubiera ocurrido; en recarga baja 25 % relativo su probabilidad si no es garantizado. | No anula efectos primarios ni autobeneficios. Efectos secundarios de habilidades/objetos fuera de la lista del movimiento requieren revisión. |
| Amuleto Inquebrantable | Niega una bajada de estadísticas rival y entra en recarga. En recarga, una reducción de 2 o más etapas se atenúa en una. | No anula autopenalizaciones; revisar Intimidación, Espejo Mágico y mensajes. |
| Crisol Espiritual | Sobre la propuesta «Crisol Antídoto»: cura un estado principal al final del turno una vez por ciclo. En recarga, disminuye 25 % el daño por quemadura y veneno. | **Equivalencia de nombre propuesta**, no ratificada. Añadir anuncio de curación e inspeccionar sincronización del estado en hardware. |
| Lente de Certeza | El primer ataque dañino con chequeo ordinario de precisión ignora etapas de evasión enemiga; después, +10 % relativo de precisión hasta recargar. | No afecta KO directo, Protecciones ni inmunidades. Probar cálculos de IA y dobles. |
| Módulo Vectorial | Ataques con `EFFECT_HIT_ESCAPE` (p. ej., Ida y Vuelta, Voltiocambio) consumen 25 % menos STA al activar y 8 % menos al recargar. | Los cambios normales y los movimientos rápidos no cuentan como pivote. |

## Integración de objetos

- Los 15 Gears tienen arte de batalla, icono de mochila, ID estable de Gear e ítem, y conversión bidireccional para equipamiento independiente.
- Tienen recetas iniciales en `src/start_menu.c`, con barreras de 2 a 6 medallas. **Ingredientes, cantidades y medallas son provisionales** hasta la revisión económica del autor.
- Comparten ciclo y estado por Pokémon dentro del combate, no el slot de objeto normal.

## Verificación obligatoria antes de habilitar para una ROM estable

1. Compilar Emerald con `UNUSED_ERROR=1 DEPRECATED_ERROR=1` y ejecutar `test/unholy_recent_gears.c`.
2. Ejecutar pruebas de batalla reales de los 15 efectos, IA rival, dobles y estados con sustituto.
3. Ver iconos en mochila, Crafting, Resumen y HUD en Eclipse y Daylight, tanto listos como en recarga.
4. Confirmar mensajería en pantalla de cambios de etapas y curaciones, y el mantenimiento de ciclos al cambiar de Pokémon.
5. Probar compatibilidad de guardado y la amplitud de los nuevos ID (Gear hasta 255, sin modificar `BoxPokemon`).

**Verificado localmente:** `host_save_schema.sh`, `host_pc_search.sh`, `validate_mutation_badges.py` e `inclusive_summary.py` pasan. **Pendiente:** compilar y ejecutar la batería de tests de batalla de GBA (sin `arm-none-eabi-gcc` en el entorno local), además de QA visual en emulador. No considerar los 15 efectos totalmente validados por el solo registro del sprite.
