# DISEÑO — Unholy Emerald

> Especificación de diseño del proyecto **Unholy Emerald**, una base de ROM hack de Pokémon Emerald construida sobre `pokeemerald-expansion` (RHH) y la decompilación de `pret/pokeemerald`.
>
> Este documento consolida las decisiones de diseño tomadas en los PR abiertos (#9 a #18) y establece las reglas que deben respetar nuevas contribuciones.

---

## 1. Alcance y principios fundamentales

### 1.1. Qué es Unholy Emerald

Es una **base de desarrollo** para crear ROM hacks de Pokémon Emerald, no un juego jugable por sí mismo. Proporciona:

- Mecánicas modernas (nuevas generaciones de Pokémon, objetos, movimientos, habilidades).
- Sistema de **mutaciones** con tiers y badges visuales.
- Mercado **Horizon** con objetos, MTs, Pokémon y huevos con stock persistente.
- Sistema de **Gears** fabricables y equipables.
- UX mejorada de bolsos (siete bolsillos, selector rápido L+R).

### 1.2. Principios de diseño

| Principio | Descripción |
|-----------|-------------|
| **No ampliar SaveBlock*** | Los sistemas nuevos deben reutilizar variables de guardado existentes o usar espacio sin usar (ej. `0x40B8`, `0x40BB`, `0x40F7–0x40FE`). No cambiar el tamaño de `SaveBlock1` ni `SaveBlock2`, salvo excepción documentada (ver §9.1). |
| **Compatibilidad Emerald + FRLG** | El código debe compilar en Pokémon Emerald, FireRed y LeafGreen. Las banderas específicas de FRLG deben declararse para evitar errores de compilación (ej. EXP Share). |
| **No romper mecánicas heredadas** | Precios base, IDs de objetos existentes, clasificaciones, SaveBlock, bonos porcentuales y denominadores de RNG no se alteran sin justificación. |
| **Determinismo y persistencia** | Los sistemas con RNG (mercado, huevos, mutaciones) deben usar semillas persistentes para reproducir exactamente el mismo estado al abrir/cerrar, guardar/cargar y cambiar de pestaña. |
| **UI segura** | Las pantallas nuevas no deben dejar callbacks pendientes, ventanas residuales ni congelar al jugador. El cierre debe liberar recursos y reconstruir el campo correctamente (`CB2_ReturnToFieldWithOpenMenu`). |
| **Sin dualidad de callbacks verticales** | Mientras se navega en menús horizontales (Horizon), no usar `Menu_ProcessInputNoWrap` ni `Menu_MoveCursor` (pensados para menú vertical). |

> ***Nota 9.1 — Excepción al principio "No ampliar SaveBlock":** El SaveBlock1 se amplió en +44 bytes para persistir datos de mutaciones, stamina, mercado Horizon y Gears (campos `mutationTier`, `mutationMask`, `mutationBadges`, `staminaCurrent`, `staminaMax`, `staminaCooldown`, `horizonSeedDiscs`, `horizonSeedItems`, `horizonPokemonOffer1/2`, `equippedGearSlot`, `gearCharges[6]`, `gearUnlockFlags`, `_gearPad`). Los campos se añadieron al final de `struct SaveBlock1` sin desplazar offsets existentes. SaveBlock1 pasó de 15568 a 15612 bytes; cabe en la holgura existente de 304 bytes sin tocar el mapa de flash. Esta es una excepción deliberada al principio #1; cualquier futura ampliación requiere justificación y actualización de este documento. Véase `docs/SAVEBLOCK_EXPANSION.md`.*

---

## 2. Sistema de mutaciones

### 2.1. Taxonomía de tiers

Las nueve mutaciones se clasifican por potencia en cinco tiers:

| Tier | Sigla | Mutaciones |
|------|-------|------------|
| C | Alpha / Hyper | Alpha, Hyper |
| B | Super / Overloaded | Super, Overloaded |
| A | Enraged / Ancestral | Enraged, Ancestral |
| S | Titan / Ascendido | Titan, Ascendido |
| Ω | Omega | Omega |

### 2.2. Funciones centrales

Se proporciona una API central para trabajar con tiers:

- `MutationTierFromBadge(badge)` — devuelve el tier a partir del badge.
- `MutationTierFromMask(mask)` — devuelve el tier a partir de la máscara de mutación.
- `MutationTierCanAppearInMarket(tier)` — indica si un tier puede aparecer en el mercado Horizon.
- `MutationTierGetName(tier)` — devuelve el nombre legible del tier.

### 2.3. Badges visuales (16×16)

Los tres badges compactos aprobados para acompañar nombres de Pokémon en Resumen/Combate:

| Badge | Descripción | Archivos |
|-------|-------------|----------|
| **Hyper** | Doble flecha verde ascendente, centrada. | `graphics/mutations/badges/hyper.png`, `.pal` |
| **Titán** | Fortaleza de piedra con núcleo dorado, centrada. | `graphics/mutations/badges/titan.png`, `.pal` |
| **Overloaded** | Solo un rayo celeste con borde morado, fondo transparente. Sin círculo/medalla/placa adicional. | `graphics/mutations/badges/overloaded.png`, `.pal` |

**Formato:** PNG indexado 4bpp nativo 16×16, 16 entradas de paleta JASC, transparencia en índice 0. Fuentes editables en `graphics/mutations/badges/source_grid.json` y `SOURCE.md`.

> **Estado:** recursos de arte subidos, **sin integración runtime todavía**. Falta conectar la carga/dibujo con las mutaciones en UI de Resumen/Combate y verificar visualmente en emulador. (PR #18)

### 2.4. Reglas de aparición

- Huevos incluyen **todas** las mutaciones, independientemente de calidad o rango visual.
- Mercado Horizon de 6 Pokémon: límite **tier A** aplicado por función central. Los tiers **S y Ω** no pueden filtrarse desde stock antiguo.
- Máximo una mutación por oferta en el formato inicial del mercado.
- Omega solo desde 8 insignias.

---

## 3. Mercado Horizon

### 3.1. Estructura de pestañas

Horizon usa **cinco pestañas horizontales**: PKMN / DISCS / ITEMS / EGGS / MOVES.

- Navegación izquierda/derecha o L/R; A abre, B regresa.
- Root propia de 27×3 tiles, panel inferior de 27×14.
- Textos de stock, calidad de huevo, confirmación y avisos mutantes distribuidos en dos columnas.

### 3.2. Pestaña DISCS (MTs)

- Ya no vende cinco MT fijas: muestra **seis MT distintas** seleccionadas por medallas y período día/noche.
- Stock persistente mediante dos semillas en variables sin uso `0x40B8` y `0x40BB`.
- Listas sin duplicados, terminadas en `ITEM_NONE`, compatibles con compras repetidas del PokéMart.
- No vende Infinity Ball ni Gears.

### 3.3. Pestaña ITEMS

- Ofrece **tres básicos permanentes** (Poké Ball, Poción, Cuerda Huida) más **cinco objetos de RNG** con límites por progreso y horario.
- No vende Infinity Ball ni Gears.
- Semillas persistentes para reproducir inventario exacto al abrir/cerrar, guardar/cargar y cambiar de pestaña.

### 3.4. Pestaña PKMN (Pokémon)

- **Dos ofertas persistentes** en variables de guardado `0x40F7–0x40FE` (sin ampliar SaveBlock1).
- Sujeto a medallas y horario diurno/nocturno.
- RNG de rareza según denominadores de mutaciones existentes.
- Alpha y variantes progresivas por medallas; Omega solo desde 8 insignias.
- Máximo una mutación por oferta en este formato inicial.
- Compra funcional con A+A, costo variable, validación de saldo, almacenamiento en equipo/PC. Cada plaza se agota tras su compra.

### 3.5. Pestaña EGGS (huevos)

- **Cinco rangos de huevos:** Básico, Medio, Alto, Mítico y Legendario.
- Cada huevo visible recibe su propio roll de calidad (Estándar / Radiante / Prismática), persistente por período.
- Huevos con **icono real** en Horizon, sin revelar la especie.
- Al comprar, una tirada de contenido puede dar normales, míticos, legendarios y hasta **tres mutaciones**, Omega incluido.
- Huevo básico también tiene jackpot legendario muy excepcional.
- Calidad alta mejora chances pero **no garantiza** un resultado.

### 3.6. Pestaña MOVES

- Catálogo de movimientos (lista interna, por definir detalle de stock).

### 3.7. Alertas de stock raro (mutantes)

Jerarquía de alertas verídicas:

1. Punto rojo junto a HORIZON en el menú START.
2. Punto rojo en la pestaña Pokémon de Horizon.
3. Texto `!!! RARE STOCK` dentro de Horizon.
4. Detalle de mutación en la ficha del Pokémon.

El aviso se reconoce al entrar en el catálogo Pokémon, no solo al abrir Horizon. Abrir/cerrar el menú no regenera ofertas.

> **Excepción a "solo Pokémon normales":** El mercado Horizon permite la venta de Pokémon con mutaciones. Esta es una excepción intencional al principio de "solo Pokémon normales" y debe documentarse explícitamente en `diseño.md`. (PR #11)

### 3.8. Variables reservadas (sin ampliar SaveBlock)

| Variable | Uso |
|----------|-----|
| `0x40B8`, `0x40BB` | Semillas persistentes de stock DISCS / ITEMS |
| `0x40F7–0x40FE` | Ofertas de mercado PKMN |
| `0x40DB`, `0x40DC`, `0x40E5`, `0x40A1`, `0x40A8` | Variables de mercado/huevos (Horizon) |

---

## 4. Sistema de Gears

### 4.1. Descripción

Seis Gears fabricables con combinaciones de materiales y cantidades diferentes. Solo el **Cetro Siniestro** exige Medalla Mente.

### 4.2. Flujo de uso

- Panel Eclipse/Daylight: L/R recetas, A+A confirmación, SELECT equipamiento, arriba/abajo Pokémon.
- El Gear consume una copia del inventario y usa su slot independiente; el antiguo regresa a la mochila.
- Seis IDs nuevos de ítems (877–882) sin renumerar antiguos.
- Seis iconos pixel art preparados (24×24, indexed 4bpp, 16 posiciones de paleta, índice 0 transparente).

### 4.3. Gears (lote 1-3)

| Gear | ID | Nota |
|------|-----|------|
| (Gear 1) | 877 | — |
| (Gear 2) | 878 | — |
| (Gear 3) | 879 | — |
| (Gear 4) | 880 | — |
| (Gear 5) | 881 | — |
| Cetro Siniestro | 882 | Exige Medalla Mente |

Recetas detalladas en `docs/GEARS.md`. Fuentes editables en `docs/art_sources/gear_batch3_source_grid.json`.

---

## 5. UX del bolso (mochila)

### 5.1. Siete bolsillos claros

- Cinta persistente de **siete pestañas identificadas** arriba de la lista.
- Resaltado del bolsillo actual y puntos discretos para bolsillos con contenido.
- No altera clasificación de objetos ni SaveBlock.
- Conserva cursores, desplazamiento, animación L/R, ordenación START y cambio de posición SELECT.

### 5.2. Selector rápido L+R

- Abre un selector que muestra los siete bolsillos con nombres completos y recuentos de pilas.
- A salta directamente al bolsillo seleccionado; B cancela.
- Navegación arriba/abajo o izquierda/derecha.

### 5.3. Restricciones

- Contextos de batalla, tutorial y link no afectados.
- QA documentada en `docs/BAG_UX.md`. (PR #15)

---

## 6. Navegación y cierre de menús

### 6.1. Horizon horizontal (PR #16)

- Cierre de Horizon libera ventanas, icono de huevo y paleta temporal, luego ejecuta `CB2_ReturnToFieldWithOpenMenu` para reconstruir el campo y el menú START.
- Antes se dejaba un callback pendiente que no reconstruía correctamente el menú.

### 6.2. Pestañas Coming Soon (PR #10)

- Pestañas POKÉMON y EGGS visibles como **Coming Soon**.
- Al confirmar con A no destruyen el menú ni reconstruyen ventanas mediante callbacks de mensajes.
- Se reproduce un sonido de acción no disponible, sin salir de Horizon.
- No habilita aún compras de Pokémon/Huevos.

---

## 7. Reglas de compatibilidad y restricciones

1. **Emerald, FireRed y LeafGreen** deben compilar y pasar los tests sin regresiones heredadas.
2. **SaveBlock** no cambia de tamaño.
3. **Denominadores de RNG** no cambian; se bumps la versión de stock para evitar reinterpretación de catálogos antiguos.
4. **IDs de objetos antiguos** no se renumeran.
5. **Gráficos de juego:** PNG 4bpp indexado, paletas JASC, índice 0 transparente.
6. **PRs en borrador:** no fusionar a `master` hasta validación de CI y QA en emulador GBA.

---

## 8. Estado de PRs abiertos (referencia)

| PR | Título | Estado | Pendiente clave |
|----|--------|--------|-----------------|
| #18 | Badges de mutación 16×16 (Hyper, Titán, Overloaded) | Draft | Integración runtime en UI de Resumen/Combate |
| #17 | Horizon: objetos y discos con rotación real | Draft | QA en emulador, auditoría IDs UNUSED |
| #16 | Horizon horizontal y cierre seguro a START | Draft | Compilación Emerald/FRLG, QA visual |
| #15 | Mochila UX: 7 bolsillos + selector L+R | Draft | Compilación, QA visual 240×160 |
| #14 | Tiers unificados para 9 mutaciones | Draft | CI y QA de GBA |
| #13 | Horizon: 6 Pokémon + 5 huevos calidad RNG | Draft | Compilación GBA, test de guardados |
| #12 | 3 sprites de Gears 24×24 | Draft | Compilación CI, verificación visual |
| #11 | Horizon market real + alertas rojas | Draft | QA, revisión de DISENO.md (este archivo) |
| #10 | Evitar bloqueos en pestañas Coming Soon | Draft | Compilación Emerald/FRLG |
| #9 | Crafteo y equipamiento de 6 Gears + FRLG | Draft | Compilación, QA en emulador, 118 errores CI previa |

---

## 9. Convenciones para contribuciones

- Los nombres de funciones y structs usan `PascalCase`.
- Las variables y campos usan `camelCase` (prefijo `g` para globales, `s` para estáticos).
- Macros y constantes usan `CAPS_WITH_UNDERSCORES`.
- Archivos `.c` y `.h`: 4 espacios, sin tabs.
- Comentarios explican el _porqué_, no el _qué_.
- Todo PR nuevo debe actualizar este `diseño.md` si introduce cambios de arquitectura, nuevas variables de guardado, nuevas pestañas de UI o excepciones a los principios listados.

---

*Última actualización: consenso de PRs #9–#18 (rama `feat/unholy-*`, noviembre 2026).*
