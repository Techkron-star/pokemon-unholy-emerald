# Unholy Emerald — Artifacts del casino, rutas y crafteo (lote propuesto)

**Estado (19 de noviembre de 2026, RD): diseño preliminar.** Son 12 nombres solicitados por Camilo; sus efectos, cifras, precios y ubicaciones de esta ficha son **propuestas**, no decisiones finales ni contenido jugable. **No se han añadido IDs, sprites, scripts de recogida ni hooks de combate.**

## Regla de distribución

- **Casino de Malvalona:** canje por fichas en un mostrador separado de MT y adornos; sus premios no deben sustituir el catálogo original. Mantener los límites de fichas de la ROM.
- **Poké Balls del mapa:** priorizar objetos visibles que de verdad estén vacíos o mal conectados, tras un censo de `object_events` y pruebas con A. No reemplazar Poké Balls válidas, regalos, recompensas de Wild Boss ni otras colocaciones reservadas de Artifact.
- **Crafteo:** dos Artifacts hechos con materiales originales obtenidos explorando; los materiales se registran como ítems reales y no deben crear Poké Balls mudas.
- Cada Artifact es un **Gear permanente de la ranura independiente**, no una poción consumible. Sus efectos se aplican al portador y respetan las reglas de encadenamiento, Stamina, dobles y cooldown por **turno completo**.
- Preservar IDs persistidos: agregar nuevos enums al final de `include/constants/gears.h` (antes de `GEAR_COUNT`) y de `include/constants/items.h`; nunca reordenar los anteriores. Localización ES/EN.

## Los 12 candidatos

| Nombre solicitado | Nombre provisional ES | Procedencia sugerida | Efecto propuesto, pendiente de balance |
|---|---|---|---|
| Eclipse Umbrella | Sombrilla Eclipse | Casino, **6800 fichas** | Si hay clima, reduce **25 %** del **primer daño directo** recibido en una activación; **3 turnos** de enfriamiento. No cancela el clima ni sus efectos sobre terceros, a diferencia del Faro. |
| Moon Sword | Espada Lunar | Casino, **8200 fichas** | El primer movimiento **cortante** de una activación gana **18 %** de daño final (**25 %** de noche); recarga **2 turnos**. No eleva etapas y es distinta de Piedra de Afilar. |
| Runic Arch | Arco Rúnico | **Crafteo**: dos Fragmentos Rúnicos en exploración | Al completar un movimiento de **apoyo** dirigido a sí mismo o a un aliado, el siguiente daño directo recibido por el beneficiario se reduce **20 %**; recarga **3 turnos**. No permite encadenar movimientos de apoyo. **Confirmar:** «arch» aquí significa arco/portal mágico, no un arma de arquería. |
| Vital Tablet | Tableta Vital | **Crafteo**: Tableta Agrietada y otro material | Una vez por combate, si sobrevive a una acción con **35 % de PS o menos**, recupera **15 % de PS máximos** y **10 % de Stamina máxima**. No revive ni evita un KO. |
| Nosferatu Tooth | Colmillo Nosferatu (nombre temporal) | Poké Ball en **Monte Pírico** | El **primer golpe de contacto** que cause daño por turno cura al portador **12 % del daño efectivamente infligido**, limitado a **8 % de sus PS máximos** por turno; respeta anticuración, Sustituto y múltiples impactos. |
| Portal Gun | Pistola Portal | Casino, **9900 fichas** | **Una vez por combate**, al hacer un cambio voluntario, el Pokémon entrante ignora **ese ingreso** las trampas y recupera **20 % de Stamina máxima**; no elimina las trampas ni crea una acción extra. |
| White Candle | Vela Blanca | Poké Ball en **Cueva Granito** | Niega el **siguiente estado principal** nuevo recibido durante una activación, con **4 turnos** de enfriamiento; no cura estados existentes. |
| Black Candle | Vela Negra | Casino, **2500 fichas** | Tras el primer golpe dañino del ciclo, el blanco recibe **30 % menos curación** durante los **dos turnos siguientes**; recarga **3 turnos**. Menos severa que Muramasa. |
| Blue Candle | Vela Azul | Poké Ball en **Ruta 124** (tras Surf) | Reduce **20 %** del primer daño directo **especial** recibido en su ciclo; **3 turnos** de recarga. |
| Yellow Candle | Vela Amarilla | Poké Ball en **Ruta 110** | Cuando falla su propio movimiento dañino, devuelve **50 % del coste de Stamina** (no PP) y gana **+10 % de precisión** en el siguiente ataque; recarga **3 turnos**, no aplicable a movimientos OHKO. |
| Green Candle | Vela Verde | Poké Ball en **Bosque Petalia** | Tras un movimiento de apoyo propio que realmente mejore o cure al usuario o a su aliado, restaura **6 % de PS máximos al beneficiario**, recarga **3 turnos**; una sola instancia aunque haya múltiples objetivos. |
| Red Candle | Vela Roja | Poké Ball en **Senda Ígnea** | Tras recibir daño directo, el siguiente movimiento ofensivo de cualquier categoría gana **15 %** de daño; **3 turnos** de recarga y una aplicación por movimiento, no por golpe. |

### Materiales para los dos de crafteo

1. **Fragmento Rúnico** (dos unidades): repartir en dos Poké Balls auténticas de la **Ruta 114** y **Monte Pírico**, o reasignar dos Poké Balls vacías que el censo identifique en lugares similares.
2. **Tableta Agrietada**: una Poké Ball de la **Ruta 117** o zona de progresión equivalente; la segunda pieza de receta será un material ya existente, **a decidir después de balancear**. No exigir incursiones para estos dos Artifacts.

Estas ubicaciones son zonas objetivo, **no coordenadas aprobadas**: requieren comprobar accesibilidad, objetos previos, flags y ritmo de medallas.

## Auditoría obligatoria de Poké Balls vacías

**Error comunicado por Camilo:** siguen existiendo muchas Poké Balls visibles que no entregan nada al pulsar A, además de las reservadas para futuros Artifacts. Esto es un bug independiente de las nuevas recompensas.

1. Inventariar todas las `OBJ_EVENT_GFX_ITEM_BALL` de los mapas Emerald realmente jugables: **mapa, coordenadas, item, script, bandera, situación actual**, y distinguir las de FRLG y las que ya se recogieron.
2. Identificar `ITEM_NONE`, IDs que no existan, scripts sin comando de entrega, objetos con flag incorrecta/duplicada y problemas de activación. Revisar también el caso de objetos que parecen Poké Balls pero no son eventos de tipo item.
3. Clasificar cada una como **pickup original válido**, **Artifact existente**, **recompensa reservada**, **nuevo material**, o **bug sin asignación**. Dejar de usar objetos vacíos como promesas implícitas.
4. Reparar en una tanda separada los pickups **no reservados**: restaurar su objeto original si se perdió, o asignar un consumible/material útil escalado a la ruta. No inventar sustituciones de recompensas especiales.
5. Probar con A antes y después de recoger, con mochila llena, guardado/carga y partida previa. Verificar que flags nuevas no oculten/dupliquen objetos antiguos. Documentar un censo de ubicaciones y registrar al menos una prueba por tipo.

**Reservas preexistentes que deben respetarse:** Botón de Pánico/Ruta 119; Casco Hoplita o Torre de Vigilancia/Ruta 120 según cierre de diseño; Túnica Rúnica/Ruta 123; Rompeolas/Ruta 126; Escudo Aegis de Registeel/Calle Victoria B2F; futuras recompensas de Wild Bosses. Evitar duplicar el premio de un jefe con una Poké Ball salvo autorización expresa.

## Reglas de implementación y verificación

- Convertir cada propuesta ratificada en: ID de Item + Gear, sprite 24×24 y HUD 16×32 con paleta, nombres/descripciones ES/EN, inventario/EQUIP, lógica y pruebas específicas.
- Casino: reutilizar la mecánica real de `MauvilleCity_GameCorner/scripts.inc`; verificar `checkcoins`, `checkitemspace`, `removecoins`, confirmación, cancelación y límites de fichas. Evitar descuentos de fichas si la mochila está llena.
- Evitar doble activación por multigolpe, dobles, cambio o cadena; las seis velas comparten convenciones de enfriamiento y se diferencian por condiciones de activación.
- Los objetos exclusivos de casino no deben aparecer a la vez en Poké Balls; los materiales de crafteo pueden ser explorables sin depender de raids.
- **No colocar una Poké Ball con un nuevo ID sin que ese ítem tenga su definición, imagen, flujo de obtención y flag funcionales.**
- Primero obtener CI verde de Emerald, pruebas dirigidas y QA de mGBA/Pizza Boy; después integración con el PR de Wild Bosses, no directo a `master`.

## Referencias del propio repositorio

- [Gears: reglas de slot, persistencia y ciclos](GEARS.md)
- [MASKARA y arsenal pendiente](GEARS_MASKARA_ARSENAL.md)
- [Rebalance de Gears y recompensas existentes](GEARS_BATCH8_REBALANCE.md)
- [Wild Bosses V2 y reservas de recompensa](UNHOLY_WILD_BOSSES_V2_DESIGN.md)
- [Código original del casino](../data/maps/MauvilleCity_GameCorner/scripts.inc)
