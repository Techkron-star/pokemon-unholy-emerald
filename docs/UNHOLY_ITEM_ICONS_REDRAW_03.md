# Unholy Emerald — redibujo de iconos de ítems · arte 03

Seis iconos que no se entendían a tamaño real se redibujaron a **24×24** con siluetas más claras. Mismo formato que el resto: PNG indexado 4 bpp, 16 entradas de paleta JASC-PAL, índice 0 transparente, índice 1 contorno oscuro. Los nombres de archivo no cambian.

| Ítem | Antes | Ahora |
|---|---|---|
| Super Exp. Share | Mancha oscura con puntos amarillos | Dispositivo de acero con bisel dorado, orbe cian con cruz, LEDs y barra de EXP |
| Impulso Cósmico | Motor muy cargado, ilegible | Motor warp horizontal: tobera dorada, aletas, anillo orbital púrpura, cúpula azul y estela |
| Fragmento de Permafrost | Un cristal fino con líneas | Tres cristales de hielo sobre nieve, con destellos |
| Colmillo de Rayquaza | Forma ambigua tipo flecha | Colmillo curvo marfil con collar verde y dorado |
| Corazón de Titán | Mancha púrpura | Corazón facetado brillante con grieta rosa sobre soporte dorado |
| Cetro Siniestro | Maza confusa | Orbe con púas y ojo rojo, bastón y cinta carmesí |

## Dónde están
- En juego: `graphics/items/icons/*.png` y `graphics/items/icon_palettes/*.pal` (Super Exp. Share, Impulso Cósmico, Permafrost, Corazón de Titán, Cetro Siniestro).
- Copias conceptuales: `graphics/unholy_concepts/icons/*.png` y `graphics/unholy_concepts/palettes/*.pal` (esas cinco más Colmillo de Rayquaza, que aún no es ítem de mochila).

## Gears conceptuales nuevos (solo `graphics/unholy_concepts/`)
Muramasa, Fragmento de Muramasa, Espada Carmesí, Espada Plateada, Necro Espada, Mira de Francotirador, Guante de Midas y Bufanda Siniestra (24×24, 4bpp, 16 colores, índice 0 transparente, índice 1 contorno). Aún no son ítems de mochila: no se tocan constantes ni tablas.

## Alcance y pendiente
- Solo arte: no cambia constantes, nombres, recetas, efectos ni IDs de guardado.
- Pendiente: compilar, ver los iconos en mGBA con los estilos Eclipse y Daylight, y confirmar contraste en la mochila y el panel de Crafting.
