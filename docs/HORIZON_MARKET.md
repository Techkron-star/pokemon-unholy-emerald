# Mercado Horizon — Unholy Emerald

> Documentación técnica del mercado Horizon con cinco pestañas horizontales y stock persistente.
> Implementado en PRs #11, #13, #16, #17.

---

## 1. Visión general

**Horizon** es un mercado accesible desde el menú START que ofrece Pokémon, MTs, ítems, huevos y movimientos con stock persistente. A diferencia del PokéMart convencional, Horizon usa semillas persistentes para reproducir exactamente el mismo inventario al abrir/cerrar, guardar/cargar y cambiar de pestaña.

---

## 2. Estructura de pestañas

Horizon usa **cinco pestañas horizontales**:

| Pestaña | Contenido |
|---------|-----------|
| **PKMN** | Pokémon en oferta (con mutaciones) |
| **DISCS** | MT (Movimientos Técnicos) |
| **ITEMS** | Ítems básicos y de RNG |
| **EGGS** | Huevos de diferentes calidades |
| **MOVES** | Catálogo de movimientos |

### Navegación

- **L/R** o flechas izquierda/derecha: cambian de pestaña.
- **A**: abre la pestaña seleccionada o confirma una oferta.
- **B**: regresa al menú anterior o cierra Horizon.

### UI

- **Root:** 27×3 tiles (pestañas arriba).
- **Panel inferior:** 27×14 tiles.
- Textos de stock, calidad de huevo, confirmación y avisos mutantes distribuidos en dos columnas.

---

## 3. Pestaña DISCS (MTs)

- Muestra **seis MT distintas** seleccionadas por medallas y período día/noche.
- Ya no vende cinco MT fijas.
- Stock persistente mediante semillas en variables `0x40B8` y `0x40BB`.
- Listas sin duplicados, terminadas en `ITEM_NONE`.
- Compatibles con compras repetidas del PokéMart.
- **No vende:** Infinity Ball ni Gears.

---

## 4. Pestaña ITEMS

- **Tres básicos permanentes:** Poké Ball, Poción, Cuerda Huida.
- **Cinco objetos de RNG** con límites por progreso y horario.
- Stock persistente mediante semillas.
- **No vende:** Infinity Ball ni Gears.

---

## 5. Pestaña PKMN (Pokémon)

- **Dos ofertas persistentes** en variables `0x40F7–0x40FE` (sin ampliar SaveBlock1).
- Sujeto a medallas y horario diurno/nocturno.
- RNG de rareza según denominadores de mutaciones existentes.
- Alpha y variantes progresivas por medallas.
- **Omega solo desde 8 insignias.**
- Máximo una mutación por oferta en este formato inicial.
- Compra con **A+A**, costo variable, validación de saldo.
- Almacenamiento en equipo/PC. Cada plaza se agota tras su compra.

---

## 6. Pestaña EGGS (Huevos)

- **Cinco rangos de huevos:** Básico, Medio, Alto, Mítico y Legendario.
- Cada huevo visible recibe su propio roll de calidad (Estándar / Radiante / Prismática), persistente por período.
- Huevos con **icono real** en Horizon, sin revelar la especie.
- Al comprar, tirada de contenido puede dar: normales, míticos, legendarios y hasta **tres mutaciones** (Omega incluido).
- Huevo básico tiene jackpot legendario muy excepcional.
- Calidad alta mejora chances pero **no garantiza** resultado.

---

## 7. Pestaña MOVES

- Catálogo de movimientos (detalle de stock por definir).

---

## 8. Alertas de stock raro (mutantes)

Jerarquía de alertas verídicas:

1. **Punto rojo** junto a HORIZON en el menú START.
2. **Punto rojo** en la pestaña Pokémon de Horizon.
3. Texto **`!!! RARE STOCK`** dentro de Horizon.
4. **Detalle de mutación** en la ficha del Pokémon.

El aviso se reconoce al entrar en el catálogo Pokémon, no solo al abrir Horizon. Abrir/cerrar el menú **no regenera ofertas**.

---

## 9. Variables reservadas (sin ampliar SaveBlock)

| Variable | Uso |
|----------|-----|
| `0x40B8`, `0x40BB` | Semillas persistentes de stock DISCS / ITEMS |
| `0x40F7–0x40FE` | Ofertas de mercado PKMN |
| `0x40DB`, `0x40DC`, `0x40E5`, `0x40A1`, `0x40A8` | Variables de mercado/huevos (Horizon) |

> **Nota:** Estas variables están fuera de SaveBlock1 (2516 bytes). Deben verificarse como libres en el mapeo de SRAM/WRAM del juego antes de usarse.

---

## 10. Pestañas Coming Soon

- **POKÉMON** y **EGGS** visibles como **Coming Soon** inicialmente.
- Al confirmar con **A** no destruyen el menú ni reconstruyen ventanas mediante callbacks.
- Se reproduce un sonido de **acción no disponible**, sin salir de Horizon.
- No habilitan aún compras de Pokémon/Huevos. (PR #10)

---

## 11. Cierre seguro

El cierre de Horizon:
1. Libera ventanas, ícono de huevo y paleta temporal.
2. Ejecuta `CB2_ReturnToFieldWithOpenMenu` para reconstruir el campo y el menú START.
3. No deja callbacks pendientes. (PR #16)

---

## 12. Excepción de diseño

El mercado Horizon permite la venta de **Pokémon con mutaciones**. Esta es una **excepción intencional** al principio de "solo Pokémon normales" y está documentada explícitamente en `diseño.md` (§3.7). (PR #11)

---

## 13. Archivos de código fuente

| Ruta | Contenido |
|------|-----------|
| `src/horizon.c` | (por crear) Lógica de mercado Horizon |
| `include/horizon.h` | (por crear) Cabecera de Horizon |
| `src/start_menu.c` | Entrada desde menú START |
| `src/shop.c` | Referencia de tienda (PokéMart NPC) |

---

## 14. Checklist de QA

- [ ] Compilación Emerald + FireRed + LeafGreen
- [ ] QA en emulador GBA (240×160)
- [ ] Prueba de guardado/carga (persistencia de semillas)
- [ ] Auditoría de colisión de variables `0x40xx` con scripts FRLG
- [ ] Prueba de compras, PC lleno, saldo insuficiente
- [ ] Prueba de cambio día-noche y cambio de medallas
- [ ] Verificar que el punto rojo no tapa HORIZON
- [ ] Verificar que textos caben en 240×160

---

*Última actualización: noviembre 2026 — PRs #11, #13, #16, #17.*