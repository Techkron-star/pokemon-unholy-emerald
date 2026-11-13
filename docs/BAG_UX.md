# UX del Bolso (Mochila) — 7 Bolsillos — Unholy Emerald

> Documentación técnica de la mejora de UX del bolso con siete bolsillos claros y selector rápido L+R.
> Implementado en PR #15.

---

## 1. Visión general

El bolso original de Pokémon Emerald está diseñado para cinco bolsillos, lo que hace que dos categorías queden poco visibles. Esta mejora introduce **siete bolsillos identificados** con una cinta de pestañas persistente y un **selector rápido L+R** para navegar entre todos ellos eficientemente.

---

## 2. Cambios implementados

### 2.1 Cinta de pestañas persistente

- Siete pestañas identificadas mostradas arriba de la lista de ítems.
- Resaltado del bolsillo actual.
- Puntos discretos indican qué bolsillos tienen contenido.
- Las pestañas permanecen visibles mientras se navega por la lista.

### 2.2 Selector rápido L+R

- Presionar **L+R** abre un selector que muestra los siete bolsillos con:
  - Nombres completos de cada bolsillo.
  - Recuentos de pilas de ítems en cada uno.
- **A** salta directamente al bolsillo seleccionado.
- **B** cancela y regresa al bolsillo actual.
- Navegación arriba/abajo o izquierda/derecha dentro del selector.

### 2.3 Conservación de funcionalidad existente

- No altera la clasificación de ítems.
- No modifica SaveBlock.
- Conserva cursores, desplazamiento, animación L/R, ordenación START y cambio de posición SELECT.
- Contextos de batalla, tutorial y link no afectados.

---

## 3. Bolsillos

| # | Bolsillo | Contenido típico |
|---|----------|------------------|
| 1 | Ítems | Pociones, antidotos, objetos generales |
| 2 | Poké Balls | Poké Ball, Great Ball, Ultra Ball, etc. |
| 3 | MT/HM | Máquinas y Máquinas Ocultas |
| 4 | Bayas | Bayas para combate y plantar |
| 5 | Objetos Clave | Llaves, bicis, objetos de historia |
| 6 | Gears | Gears fabricables y equipables |
| 7 | Otros | Ítems sin categoría específica |

> **Nota:** Los bolsillos 6 y 7 son nuevos en esta implementación. El bolso en `master` base tiene 5 bolsillos; esta mejora los expande a 7.

---

## 4. Archivos de código fuente

| Ruta | Contenido |
|------|-----------|
| `include/bag.h` | (por crear) Definición de estructura de 7 bolsillos |
| `src/bag.c` | (por crear) Lógica de navegación y selector L+R |
| `src/item_menu.c` | Integración con menú de ítems existente |
| `graphics/bag/` | Gráficos de pestañas y selector |

---

## 5. Reglas de compatibilidad

- **No ampliar SaveBlock:** Los datos del bolso usan el espacio existente.
- **No romper mecánicas heredadas:** Precios, IDs de objetos y clasificaciones no cambian.
- **Emerald + FRLG:** Debe compilar en ambas versiones.

---

## 6. Checklist de QA (PR #15)

- [ ] Compilación Emerald + FireRed + LeafGreen
- [ ] Prueba visual en GBA 240×160
- [ ] Controles rápidos (L+R, arriba/abajo, izquierda/derecha)
- [ ] Casos límite (bolsillo vacío, mochila llena)
- [ ] No fugas de ventanas ni memoria
- [ ] QA de ítems y guardado

---

*Última actualización: noviembre 2026 — PR #15.*