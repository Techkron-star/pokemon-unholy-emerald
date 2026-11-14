# Lista de sprites de Gears — Unholy Emerald

Especificaciones para entregar a una IA generadora de imágenes.
Una vez generados, pásamelos y yo los integro en el código y el build.

## Especificaciones técnicas comunes

- **Formato:** PNG con fondo transparente
- **Profundidad:** 4 bpp (16 colores), índice 0 = transparente
- **Convertidor:** `gbagfx` (ya configurado en el Makefile)
- **Paleta:** 16 colores compartidos por todos los gears (definir una paleta maestra y usarla en todos)

---

## Ítems de equipamiento (ícono en mochila, 24×24 px)

| ID | Nombre | Descripción para la IA |
|----|--------|------------------------|
| `ITEM_VISOR_COSMICO` | Visor Cósmico | Gafas futuristas con lentes que reflejan estrellas y nebulosas, marco metálico plateado, estilo sci-fi retro |
| `ITEM_CORAZON_TITAN` | Corazón de Titán | Corazón mecánico dorado con engranajes visibles, rodeado de iones eléctricos azules, estilo épico |
| `ITEM_SKI_MASK` | Ski Mask | Pasamontañas negro con solo los ojos visibles, tela elástica, aspecto misterioso y urbano |
| `ITEM_CETRO_SINIESTRO` | Cetro Siniestro | Cetro de ébano con una cabeza de búho negro en la cima, gemas púrpuras brillantes, energía oscura alrededor |
| `ITEM_IMPULSO_COSMICO` | Impulso Cósmico | Gafas de deportista con lentes violetas que emiten destellos de energía cósmica, marco blanco brillante |
| `ITEM_PERMAFROST_FRAGMENT` | Fragmento Permafrost | Fragmento de hielo azul cristalino con escarcha blanca, partículas de nieve flotando, frío azul intenso |
| `ITEM_TESERACTO` | Teseracto | Cubo azul translúcido de energía pura, partículas moviéndose en su interior, brillo interior azul eléctrico |
| `ITEM_ANCLA_REALIDAD` | Ancla de Realidad | Ancla marina de hierro oxidado con runas grabadas, cadenas rotas, aspecto pesado y antiguo |
| `ITEM_COLMILLO_RAYQUAZA` | Colmillo Rayquaza | Colmillo verde esmeralda enorme y afilado, con escamas de dragón en la base, brillo verde draconico |

---

## Gears Sangre — Inyecciones sanguíneas (18 ítems, uno por tipo)

| ID | Nombre | Color principal | Descripción para la IA |
|----|--------|-----------------|------------------------|
| `ITEM_SANGRE_NORMAL` | Sangre Normal | Gris plata | Ampolla de vidrio con líquido gris plateado, etiqueta blanca minimalista |
| `ITEM_SANGRE_LUCHA` | Sangre Lucha | Rojo naranja | Ampolla con líquido rojo anaranjado, vendaje deportivo en el cuello |
| `ITEM_SANGRE_VOLADOR` | Sangre Volador | Azul cielo | Ampolla con líquido azul celeste, plumas blancas alrededor del cuello |
| `ITEM_SANGRE_VENENO` | Sangre Veneno | Verde púrpura | Ampolla con líquido verde tóxico, burbujas, calavera pequeña en la etiqueta |
| `ITEM_SANGRE_TIERRA` | Sangre Tierra | Marrón ocre | Ampolla con líquido marrón terroso, partículas de tierra flotando |
| `ITEM_SANGRE_ROCA` | Sangre Roca | Gris piedra | Ampolla con líquido gris opaco, pequeños fragmentos de roca suspendidos |
| `ITEM_SANGRE_BICHO` | Sangre Bicho | Verde lima | Ampolla con líquido verde brillante, pequeñas alas de insecto en el cuello |
| `ITEM_SANGRE_FANTASMA` | Sangre Fantasma | Púrpura oscuro | Ampolla con líquido púrpura translúcido, forma etérea dentro del líquido |
| `ITEM_SANGRE_ACERO` | Sangre Acero | Gris acero | Ampolla con líquido gris metálico, tornillos y engranajes diminutos dentro |
| `ITEM_SANGRE_FUEGO` | Sangre Fuego | Rojo naranja brillante | Ampolla con líquido rojo ardiendo, llamas diminutas en la superficie |
| `ITEM_SANGRE_AGUA` | Sangre Agua | Azul profundo | Ampolla con líquido azul océano, burbujas de aire subiendo |
| `ITEM_SANGRE_PLANTA` | Sangre Planta | Verde hoja | Ampolla con líquido verde hoja, pequeñas hojas y enredaderas dentro |
| `ITEM_SANGRE_ELECTRICO` | Sangre Eléctrico | Amarillo eléctrico | Ampolla con líquido amarillo, chispas eléctricas dentro del vidrio |
| `ITEM_SANGRE_PSIQUICO` | Sangre Psíquico | Violeta claro | Ampolla con líquido violeta, ondas cerebrales tenues visibles dentro |
| `ITEM_SANGRE_HIELO` | Sangre Hielo | Azul cristal | Ampolla con líquido azul congelado, cristales de hielo en el interior |
| `ITEM_SANGRE_DRAGON` | Sangre Dragón | Naranja dorado | Ampolla con líquido naranja dorado, escamas de dragón diminutas flotando |
| `ITEM_SANGRE_SINIESTRO` | Sangre Siniestro | Negro púrpura | Ampolla con líquido negro con ribetes púrpuras, energía oscura dentro |
| `ITEM_SANGRE_HADA` | Sangre Hada | Rosa brillante | Ampolla con líquido rosa brillante, destellos de polvo de hada dentro |

---

## HUD de batalla (16×16 px, dos frames)

Cada gear tiene dos estados en el HUD circular de batalla:
- **Frame 1 (normal):** Ícono del gear cuando está listo/cargando
- **Frame 2 (sobrecargado):** Mismo ícono pero con brillo/energía adicional para indicar fase de sobrecarga

| Ítem | Frame 1 | Frame 2 |
|------|---------|---------|
| Visor Cósmico | Gafas cósmicas normales | Lentes brillando con energía |
| Corazón de Titán | Corazón mecánico normal | Corazón latiendo con energía dorada |
| Ski Mask | Pasamontañas normal | Ojos brillando con energía siniestra |
| Cetro Siniestro | Cetro normal | Gema púrpura brillando intensamente |
| Impulso Cósmico | Gafas normales | Lentes violetas con destello cósmico |
| Fragmento Permafrost | Hielo normal | Hielo con escarcha activa |
| Teseracto | Cubo azul normal | Cubo azul con energía sobrecargada |
| Ancla de Realidad | Ancla normal | Ancla con runas brillando |
| Colmillo Rayquaza | Colmillo normal | Colmillo con energía verde draconica |
| Sangre <tipo> | Ampolla normal | Ampolla con líquido brillando |

---

## Cómo entregármelos

1. Guarda cada PNG con el nombre del ID (ej: `visor_cosmico.png`, `sangre_fuego.png`, etc.)
2. Dime la carpeta donde están
3. Yo los agrego al build (`graphics/items/icons/` y `graphics/unholy_concepts/icons/`), actualizo las tablas C y el Makefile, y hago commit

---

## Notas de balance (para tu referencia, no para la IA)

Todos los valores están en `src/gear_data.c` en partes por mil (‰):
- Gears únicos: sobrecarga y residual según el DISENO.md
- Gears Sangre: +400‰ (40 %) en sobrecarga, +100‰ (10 %) residual
