# Especificaciones de Gráficos — Unholy Emerald

> Documentación técnica con especificaciones exactas para todos los gráficos que faltan crear.
> Útil para artistas y contribuyentes que quieran aportar assets al proyecto.

---

## 1. Badges de Mutación (16×16) — Crítico

Ubicación: `graphics/mutations/badges/`

| Badge | Archivo PNG | Archivo PAL | Descripción | Colores |
|-------|-------------|-------------|-------------|---------|
| Hyper | `hyper.png` | `hyper.pal` | Doble flecha verde ascendente, centrada | Verde predominante |
| Titán | `titan.png` | `titan.pal` | Fortaleza de piedra con núcleo dorado, centrada | Gris piedra + dorado |
| Overloaded | `overloaded.png` | `overloaded.pal` | Rayo celeste con borde morado, fondo transparente | Celeste + morado |

### Especificaciones técnicas

- **Dimensiones:** 16×16 píxeles nativos
- **Formato:** PNG indexado 4bpp
- **Paleta:** 16 entradas JASC-PAL
- **Transparencia:** Índice 0
- **CRCs:** Deben cotejarse localmente para verificar integridad
- **Ubicación:** `graphics/mutations/badges/`

### Fuentes editables

- `graphics/mutations/badges/source_grid.json` — Índices de píxel 4bpp
- `graphics/mutations/badges/SOURCE.md` — Documentación de fuentes

### Estado

**PENDIENTE** — Los recursos de arte están subidos pero sin integración runtime. Falta conectar la carga/dibujo con las mutaciones en UI de Resumen/Combate. (PR #18)

---

## 2. Iconos de Gears para Bolsa (24×24)

Ubicación: `graphics/items/icons/`

| Gear | Archivo PNG | Archivo PAL | Descripción |
|------|-------------|-------------|-------------|
| Visor Cósmico | `visor_cosmico.png` | `visor_cosmico.pal` | Visor astral turquesa con estrella dorada |
| Corazón de Titán | `corazon_titan.png` | `corazon_titan.pal` | Piedra con núcleo dorado |
| Ski Mask | `ski_mask.png` | `ski_mask.pal` | Máscara violeta con ojos rojos |
| Cetro Siniestro | `cetro_siniestro.png` | `cetro_siniestro.pal` | Cetro con gema oscura |
| Impulso Cósmico | `impulso_cosmico.png` | `impulso_cosmico.pal` | Fragmento de energía cósmica |
| Permafrost Fragment | `permafrost_fragment.png` | `permafrost_fragment.pal` | Fragmento de hielo con núcleo cian |

### Especificaciones técnicas

- **Dimensiones:** 24×24 píxeles nativos
- **Formato:** PNG indexado 4bpp
- **Paleta:** 16 entradas JASC-PAL
- **Transparencia:** Índice 0
- **Ubicación:** `graphics/items/icons/` y `graphics/items/icon_palettes/`

### Estado

**PARCIAL** — PR #12 trajo 3 de 6 iconos. Faltan 3 iconos por completar.

---

## 3. Iconos de Gears para Batalla (16×32) — Faltante Crítico

Ubicación: `graphics/gears/<nombre>/`

Cada Gear necesita un icono 16×32 con **dos frames de 16×16 apilados** para mostrar en la caja de vida durante la batalla.

| Gear | Directorio | Archivo PNG | Archivo PAL | Descripción |
|------|------------|-------------|-------------|-------------|
| Visor Cósmico | `graphics/gears/visor_cosmico/` | `icon.png` | `icon.pal` | Visor astral turquesa |
| Corazón de Titán | `graphics/gears/corazon_titan/` | `icon.png` | `icon.pal` | Piedra con núcleo dorado |
| Ski Mask | `graphics/gears/ski_mask/` | `icon.png` | `icon.pal` | Máscara violeta |
| Cetro Siniestro | `graphics/gears/cetro_siniestro/` | `icon.png` | `icon.pal` | Cetro con gema oscura |
| Impulso Cósmico | `graphics/gears/impulso_cosmico/` | `icon.png` | `icon.pal` | Fragmento de energía |
| Permafrost Fragment | `graphics/gears/permafrost_fragment/` | `icon.png` | `icon.pal` | Fragmento de hielo |

### Especificaciones técnicas

- **Dimensiones:** 16×32 píxeles (dos frames de 16×16 apilados verticalmente)
- **Formato:** PNG indexado 4bpp
- **Paleta:** 16 entradas JASC-PAL
- **Transparencia:** Índice 0
- **Contorno:** Índice 1 debe ser contorno oscuro
- **Ubicación:** `graphics/gears/<nombre>/icon.png` + `icon.pal`

### Estado

**FALTANTE** — Los 6 iconos de batalla no existen. El código en `src/battle_interface.c` tiene `CreateGearIconSprite`, `UpdateGearIcon`, `SpriteCB_GearIcon` listos, pero `iconGfx`/`iconPal` son NULL en `src/data/gears.h`, por lo que ningún icono se dibuja en combate.

---

## 4. Archivos de Código Fuente Relacionados

| Ruta | Contenido | Estado |
|------|-----------|--------|
| `include/constants/gears.h` | `enum Gear`, `GEAR_MAX_COUNT=32` | ✅ Existe |
| `include/config/gears.h` | `GEARS_ENABLED`, offsets de icono HUD | ✅ Existe |
| `include/gear.h` | `struct GearInfo`, `enum GearTrigger`, `enum GearPhase` | ✅ Existe |
| `src/data/gears.h` | `gGearsInfo[GEAR_COUNT]` — tabla con 6 entradas | ✅ Existe |
| `src/gear.c` | Lógica de ciclo de Gears | ✅ Existe |
| `src/battle_interface.c` | `CreateGearIconSprite`, `UpdateGearIcon`, `SpriteCB_GearIcon` | ✅ Existe (iconos NULL) |
| `src/start_menu.c` | Menú de crafteo, Eclipse/Daylight | ✅ Existe |

---

## 5. Checklist de Implementación de Gráficos

- [ ] Crear `graphics/mutations/badges/hyper.png` + `hyper.pal` (16×16, 4bpp, índice 0 transparente)
- [ ] Crear `graphics/mutations/badges/titan.png` + `titan.pal` (16×16, 4bpp, índice 0 transparente)
- [ ] Crear `graphics/mutations/badges/overloaded.png` + `overloaded.pal` (16×16, 4bpp, índice 0 transparente)
- [ ] Crear 3 iconos de Gear 24×24 faltantes en `graphics/items/icons/`
- [ ] Crear 6 iconos de Gear 16×32 en `graphics/gears/<nombre>/`
- [ ] Conectar `.iconGfx`/`.iconPal` en `src/data/gears.h`
- [ ] Validar en emulador GBA (Emerald + FRLG)

---

## 6. Formato de Archivos

Todos los gráficos deben cumplir con las reglas de `graphics_file_rules.mk`:

- PNG indexado 4bpp
- Paletas JASC-PAL con 16 entradas
- Índice 0 = transparente
- Índice 1 = contorno oscuro (para iconos de batalla)
- Dos frames comparten la misma paleta (para iconos 16×32)

---

*Última actualización: noviembre 2026*
