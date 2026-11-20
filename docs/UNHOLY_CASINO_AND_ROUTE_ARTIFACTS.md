# Unholy Emerald — 12 Artifacts: diseño definido por Camilo

**Estado:** efectos especificados directamente por Camilo durante el 19 de noviembre de 2026 (RD). **Diseño; no implementado en la ROM.** No hay IDs nuevos, sprites, scripts de recogida, precios ni efectos de combate confirmados por QA. Las propuestas de balance y ubicaciones anteriores del PR #71 quedan **sustituidas** por esta ficha.

## 1. Eclipse Umbrella

- **Activación:** los movimientos de tipo **Siniestro** del portador tienen **10 %** de probabilidad de imponer atrapamiento al enemigo afectado.
- **Persistencia:** el bloqueo de cambio dura mientras el **portador del Artifact permanezca en combate**; no tiene duración fija por turnos. Al salir el portador, termina el atrapamiento creado exclusivamente por este Gear.
- **Sinergia:** contra objetivos que ya estén atrapados por cualquier fuente válida —por ejemplo Trampa Arena, un movimiento de atrapamiento u otro efecto— los movimientos **Siniestro y Fantasma** del portador infligen daño adicional.
- **Pendiente:** cuantía del daño adicional; compatibilidad con excepciones nativas de cambio, inmunidades y movimientos Siniestro sin objetivo enemigo; evaluar si el golpe que causa atrapamiento recibe el daño extra o solo ataques posteriores.

## 2. Moon Sword

- Los movimientos de tipo **Psíquico, Siniestro y Hada** del portador se consideran **movimientos de corte** aunque originalmente no sean cortantes.
- Además, tienen **alta probabilidad de crítico**.
- **Cuentan como movimientos de contacto**, **incluso si son especiales**: el contacto es real y activa las interacciones nativas correspondientes, como Piel Tosca y Casco Dentado.
- **Pendiente:** aumento exacto de probabilidad de crítico y especificar si la conversión incluye movimientos de estado o solo movimientos que golpean objetivos.

## 3. Runic Arch

- El portador gana **+1 de Ataque Especial por cada evento** que cumpla uno de estos disparadores **independientes**:
  1. **Anular** un movimiento.
  2. Aplicar **control (CC)** al enemigo.
  3. **Proteger** a un aliado.
- Pueden producirse **varios eventos en el mismo turno** y conceder varias subidas.
- **Pendiente:** catálogo exacto de qué cuenta como anulación/CC/protección y cómo evitar contar dos veces un mismo evento. No se ha autorizado un límite nuevo distinto del límite nativo de etapas.

## 4. Vital Tablet

- Cada **3 turnos completos** emite un **pulso curativo**.
- En **individuales**, el portador recupera **15 % de sus PS actuales** (no de sus PS máximos).
- En **dobles**, el pulso cura a **ambos Pokémon aliados**; aplicar la regla de **15 % de los PS actuales de cada uno** como interpretación de la cifra indicada, sujeta a confirmación si Camilo desea otra base.
- **Pendiente:** si el contador avanza estando el portador fuera de campo y redondeos a PS mínimos.

## 5. Nosferatu Tooth (nombre provisional)

- Los ataques de **contacto** curan al portador por el **10 % del daño efectivo causado**.
- Si ese daño es **crítico**, la curación se **duplica al 20 %**.
- **Pendiente:** aplicación en multigolpes, ataques de área, Sustituto, inmunidades y bloqueo de curación.

## 6. Portal Gun

- **Cada 2 turnos** permite al portador **o a su aliado** cambiar de Pokémon **aunque el cambio esté restringido** por un efecto de atrapamiento o bloqueo.
- **Pendiente:** momento exacto de activación, si es cambio normal que consume acción, qué pasa en dobles y excepciones técnicas que deben preservarse para evitar cambiar Pokémon fuera de combate.

## 7. White Candle

- **Al salir al combate**, el portador cura **un estado alterado**.
- Si está en un **combate doble**, cura en cambio **un estado alterado del compañero**.
- **Pendiente:** si se puede activar cada vez que entra o una vez por combate; qué ocurre si el compañero no tiene estado, o hay dos aliados con más de un estado.

## 8. Black Candle

- Diseño **inverso** a White Candle: al entrar en combate perjudica a un enemigo.
- El efecto elegido será **solo uno** de estas tres categorías, **nunca las tres a la vez**:
  1. Imponer un **estado alterado aleatorio**.
  2. Aplicar una **reducción de estadísticas**.
  3. Provocar **retroceso (flinch)**.
- **Pendiente:** probabilidades de las categorías, objetivo en dobles, intensidad de bajada, inmunidades y duración/viabilidad del flinch según momento de entrada.

## 9. Blue Candle

- Aumenta **pasivamente un 8 %** la regeneración de **Stamina** del portador **y del aliado**.
- **Pendiente:** precisar si el +8 % son ocho puntos porcentuales de Stamina máxima por turno (p. ej., 18 % → 26 %) o un incremento relativo de la regeneración base.

## 10. Yellow Candle

- Aumenta en **12 % todos los efectos del clima soleado y sus variantes similares** que reciben los Pokémon.
- **Pendiente:** clasificar qué climas y efectos cubre (sol normal, climas primordiales, cambios a poderes de movimientos, curación, velocidad, daño, habilidades, etc.) y si beneficia a ambos bandos. No crear clima por sí misma sin instrucción.

## 11. Green Candle

- En **Campo de Hierba**, las estadísticas de **todos los Pokémon de tipo Planta o Bicho** reciben una bonificación de **6 %**.
- **Pendiente:** qué estadísticas incluye (en particular PS y Stamina, que tienen tratamiento distinto de las estadísticas efectivas de combate) y alcance sobre ambos bandos.

## 12. Red Candle

- Cada **ataque de contacto que acierte y cause daño** recupera **6 % de PS y 6 % de Stamina** del portador.
- Si el ataque de contacto es **supereficaz**, recupera **10 % de PS y 10 % de Stamina**.
- **Pendiente:** precisar si los porcentajes se calculan sobre máximos o actuales y si se aplica una vez por ejecución o por golpe de multigolpe.

## Distribución: todavía sin decidir

Camilo quiere obtener parte de este lote en el **casino**, otra parte en **Poké Balls de mapas**, o mediante **materiales de crafteo recogidos en ellas**. **No se han aprobado precios, ubicaciones concretas, materiales, recetas ni asignaciones a estos tres canales.** No conservar como oficiales los precios y mapas que se propusieron antes.

No usar estos IDs en objetos del mapa hasta que existan los Item/Gear, sprites, nombres/descripciones ES/EN, scripts de recogida, flags y efectos reales. Mantener ranuras independientes (Artifact + objeto ordinario), sin reordenar IDs persistidos y respetando el sistema de cooldown por turnos completos.

## P0 — Poké Balls vacías reportadas

Camilo confirma que **siguen existiendo Poké Balls visibles sin contenido funcional**, incluso descontando las reservadas para Artifacts futuros. Es un defecto **independiente** de este nuevo catálogo.

1. Auditar los `OBJ_EVENT_GFX_ITEM_BALL` de Hoenn, con mapa/coordenadas/ítem/script/flag y diferenciar los de FRLG.
2. Identificar ítems `ITEM_NONE`, IDs faltantes, scripts que no entregan ítems, flags duplicadas o incorrectas y objetos decorativos que aparentan ser recogibles.
3. Preservar recompensas ya asignadas o reservadas de Artifacts y Wild Bosses: no reemplazarlas por un material genérico.
4. Reparar Poké Balls **no reservadas** con su objeto original o un contenido apropiado para la ruta; registrar el censo final.
5. Validar pulsando A antes/después de recoger, mochila llena, guardar/cargar y revisitar el mapa, en mGBA y Pizza Boy Pro.

## Referencias técnicas

- [Ranura de Gear, ciclos y guardado](GEARS.md).
- [Recompensas asignadas previamente](GEARS_BATCH8_REBALANCE.md).
- [Wild Bosses y exclusividad de recompensas](UNHOLY_WILD_BOSSES_V2_DESIGN.md).
- [Casino real de Malvalona](../data/maps/MauvilleCity_GameCorner/scripts.inc).

**QA pendiente:** implementar y probar en ROM solo después de cerrar las dudas marcadas. Esta ficha no implica que el PR esté compilado o sus efectos sean jugables.
