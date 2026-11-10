# Tiers de mutaciones · Unholy Emerald

**Estado:** clasificación de jugabilidad propuesta e implementada en PR apilado sobre #13. Ajustable tras pruebas de balance.

## Tabla única

| Tier de poder | Mutaciones | Horizon Pokémon | Huevos Horizon |
|---|---|---|---|
| C — Común | Alpha, Hyper | Sí | Sí |
| B — Superior | Super, Overloaded | Sí | Sí |
| A — Élite | Enraged, Ancestral | Sí | Sí |
| S — Excepcional | Titan, Ascendido | **No** | Sí, raro |
| Ω — Anomalía | Omega | **No** | Sí, extremadamente raro |

El **tier describe nivel de poder/acceso**, no la rareza numérica del RNG. Por eso una mutación tier A puede tener un denominador de aparición menor que otra tier B. Los odds siguen en `include/config/special_badges.h` y no se alteran al clasificar.

**Regla global del catálogo normal:** el máximo permitido es `MUTATION_TIER_A`. Se rechaza cualquier mutación nueva sin clasificación (fail-closed). No se venden Titan, Ascendido ni Omega, aunque haya ocho medallas.

**Regla de huevos:** la ruleta utiliza las nueve mutaciones y puede generar premios S u Ω incluso desde un huevo Básico, con probabilidades bajas; el *rango* del huevo (Básico/Medio/Alto/Mítico/Legendario) y su *calidad visual* (Estándar/Radiante/Prismática) son conceptos **distintos del tier de mutación**. Calidad superior mejora odds pero nunca garantiza un tier determinado.

## Mutaciones múltiples

- Se conservan los badges individuales y sus **bonificaciones porcentuales se suman, nunca se multiplican entre sí**.
- El **tier principal** es el más alto entre las mutaciones presentes. Ej.: Alpha + Titan = tier S; Omega + Ancestral = tier Ω.
- Esta regla **no borra** las otras mutaciones ni introduce un multiplicador de estadísticas adicional por tier. Es una clasificación, no una tercera fuente de buffs.
- Máximo tres mutaciones por huevo en la ruleta actual; otros encuentros conservan sus límites y probabilidades existentes.

## API

`include/mutation_tiers.h` y `src/mutation_tiers.c`:

- `MutationTierFromBadge`: clasifica una mutación concreta.
- `MutationTierFromMask`: obtiene el tier principal de un conjunto de mutaciones.
- `MutationTierCanAppearInMarket`: política central, solo C/B/A.
- `MutationTierGetName`: etiqueta compacta para interfaz.

La tabla de Horizon usa la política compartida y también valida las lecturas del stock persistido. Los huevos consultan la misma clasificación sin excluir tiers S/Ω. Tests en `test/mutation_tiers.c`.

## Validación obligatoria

- Compilaciones Emerald, FireRed, LeafGreen.
- Pruebas de tiers, mercado y etiquetas; tests de guardado de versiones previas.
- Verificación en emulador del texto `!!! A ENRAGED` y similares, sin desbordar 240×160.
- Confirmar que la clasificación no cambia odds, estadísticas, combinaciones ni consumo de RNG de otras mecánicas.
- Hasta completar la validación, no fusionar a `master`.
