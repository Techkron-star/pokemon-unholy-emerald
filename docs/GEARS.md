# Sistema de Gears — Unholy Emerald

> Documentación técnica del sistema de crafteo y equipamiento de Gears.
> Basado en `docs/GEARS.md` del repositorio `Techkron-star/pokemon-unholy-emerald`.

---

## 1. Visión general

Los **Gears** son ítems fabricables y equipables que otorgan bonificaciones en batalla y fuera de ella. Cada Pokémon puede equipar un Gear a la vez, y el Gear ocupa un slot independiente (no consume espacio en la mochila una vez equipado).

---

## 2. Gears implementados (6 en total)

| Gear | ID ítem | ID enum | Efecto | Requisito |
|------|---------|---------|--------|-----------|
| Visor Cósmico | 877 | GEAR_VISOR_COSMICO | Sp. Atk +20% (1 turno), +10% residual (3 turnos) | Ninguno |
| Corazón de Titán | 878 | GEAR_CORAZON_TITAN | Atk +20% (1 turno), +10% residual (3 turnos) | Ninguno |
| Ski Mask | 879 | GEAR_SKI_MASK | Tipo Oscuro +30% (1 turno), +21% residual (3 turnos) | Ninguno |
| Cetro Siniestro | 880 | GEAR_CETRO_SINIESTRO | Oscuro SE vs Psíquico/Hada/Normal/Lucha | Medalla Mente (7.ª insignia) |
| Impulso Cósmico | 881 | GEAR_IMPULSO_COSMICO | Tipo Fantasma SE vs Normal/Psíquico/Hada | Ninguno |
| Permafrost Fragment | 882 | GEAR_PERMAFROST_FRAGMENT | Hielo: ×2 probabilidad de congelación; cura 1/16 PS si es Hielo | Ninguno |

---

## 3. Flujo de crafteo (Panel Eclipse/Daylight)

El panel de crafteo se accede desde el menú START → **CRAFTING** (requiere `FLAG_CRAFTING_UNLOCKED`).

### Controles

| Tecla | Acción |
|-------|--------|
| L / R | Navegar entre recetas |
| A + A | Confirmar fabricación |
| SELECT | Equipar Gear seleccionado |
| Arriba / Abajo | Seleccionar Pokémon del equipo |

### Reglas

- El Gear consume una copia del material del inventario.
- El Gear antiguo (si hay uno equipado) regresa a la mochila.
- Cada Gear usa un slot independiente; no ocupa espacio de objeto equipado normal.
- Si la mochila está llena al devolver el Gear antiguo, el cambio no se aplica.

---

## 4. Recetas de crafteo

| Gear | Materiales | Cantidades |
|------|-----------|------------|
| Visor Cósmico | 1 Gafas de Sol, 2 Piedras Lunar, 1 Escama Metálica | — |
| Corazón de Titán | 1 Piedra Óxido, 3 Carbón, 1 Escama Metálica | — |
| Ski Mask | 1 Tela Oscura, 2 Piedras Umbral, 1 Gema Fantasma | — |
| Cetro Siniestro | 1 Gafas de Sol, 2 Hechizos, 1 Negra-Mente | Medalla Mente |
| Impulso Cósmico | 1 Piedra Sol, 1 Carbón, 1 Gema Fuego | — |
| Permafrost Fragment | 2 Piedras Hielo, 1 Hielo Eterno, 1 Agua Mistica | — |

> **Nota:** Los nombres exactos de los ítems de material pueden variar según la localización. Verificar en `include/constants/items.h`.

---

## 5. Efectos en batalla

| Gear | Efecto durante sobrecarga | Efecto residual |
|------|--------------------------|-----------------|
| Visor Cósmico | Sp. Atk +20% | +10% durante 3 turnos |
| Corazón de Titán | Atk +20% | +10% durante 3 turnos |
| Ski Mask | Movimiento Oscuro +30% | +21% durante 3 turnos |
| Cetro Siniestro | Oscuro SE vs Psíquico/Hada/Normal/Lucha | — |
| Impulso Cósmico | Fantasma SE vs Normal/Psíquico/Hada | — |
| Permafrost Fragment | Hielo: ×2 congelación | Cura 1/16 PS si es tipo Hielo |

---

## 6. Guardado y persistencia

Los datos de Gears se almacenan en **SaveBlock1** (ampliado en `feat/expand-saveblock`):

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `equippedGearSlot` | u8 | Slot de Gear equipado (0-5, 0xFF = ninguno) |
| `gearCharges[6]` | u16[6] | Cargas restantes por Gear |
| `gearUnlockFlags` | u8 | Gears desbloqueados/creados (bitfield) |
| `_gearPad` | u8 | Padding de alineación |

Además, cada Pokémon almacena su Gear equipado en `PokemonSubstruct1.gear` (5 bits), que viaja con el Pokémon entre equipo, PC y guardería.

---

## 7. Gráficos

### Iconos de bolsa (24×24)
Ubicados en `graphics/items/icons/`:
- `visor_cosmico.png` + `.pal`
- `corazon_titan.png` + `.pal`
- `ski_mask.png` + `.pal`
- `cetro_siniestro.png` + `.pal`
- `impulso_cosmico.png` + `.pal`
- `permafrost_fragment.png` + `.pal`

Formato: PNG indexado 4bpp, 24×24 píxeles, 16 posiciones de paleta JASC, transparencia en índice 0.

### Iconos de batalla (16×32) — PENDIENTE
Ubicación prevista: `graphics/gears/<nombre>/icon.png` + `.pal`

Cada Gear necesita un icono 16×32 con dos frames de 16×16 apilados para mostrar en la caja de vida durante la batalla. **Actualmente faltan estos archivos** — el código del sprite existe en `src/battle_interface.c` pero `iconGfx`/`iconPal` son NULL, por lo que ningún icono se dibuja en combate.

---

## 8. Archivos de código fuente

| Ruta | Contenido |
|------|-----------|
| `include/constants/gears.h` | `enum Gear` (GEAR_NONE…GEAR_COUNT), `GEAR_MAX_COUNT=32` |
| `include/config/gears.h` | `GEARS_ENABLED`, offsets de icono HUD |
| `include/gear.h` | `struct GearInfo`, `enum GearTrigger`, `enum GearPhase`, API declarada |
| `src/data/gears.h` | `gGearsInfo[GEAR_COUNT]` — tabla con 6 entradas |
| `src/gear.c` | Lógica de ciclo: `StartCycle`, `StartRecharge`, `Prepare`, `Gear_OnMoveUsed`, `Gear_EndOfTurn`, `Gear_GetPowerModifier` |
| `src/start_menu.c` | Estructura `UnholyCraftRecipe`, menú de crafteo, skins Eclipse/Daylight |

---

## 9. Checklist de implementación pendiente

- [ ] Crear iconos 16×32 de batalla para los 6 Gears en `graphics/gears/`
- [ ] Conectar `.iconGfx`/`.iconPal` en `src/data/gears.h`
- [ ] Verificar compatibilidad FRLG para `FLAG_BADGE07_GET` (Cetro Siniestro)
- [ ] Agregar equipamiento desde el resumen de Pokémon y desde la mochila
- [ ] Validar en emulador GBA (Emerald + FRLG)

---

*Última actualización: noviembre 2026 — consenso de PRs #9 y #12.*