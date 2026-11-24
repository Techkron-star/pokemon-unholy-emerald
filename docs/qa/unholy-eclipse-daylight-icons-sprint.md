# Unholy Emerald · primer lote de Eclipse/Daylight y pixel art

> Rama de trabajo. No fusionar antes de compilar y probar en emulador.

## Qué se modificó

- Se reforzó el estilo visual de los paneles **Horizon Shop** y **Crafting** con bandas, separadores y esquinas a 1–2 píxeles sobre ventanas nativas de GBA.
- Las dos skins usan la preferencia persistente `UI THEME: ECLIPSE / DAYLIGHT` que ya existía en Opciones. **No** sigue el reloj del juego.
- Los detalles comparten una función `DrawUnholyUiPanelAccents`; colores y estado visual dependen de la misma bandera de guardado, sin tocar la RNG, el inventario ni los precios.
- Los tres objetos personalizados actualmente definidos cuentan ahora con arte **24×24 píxeles, PNG indexado de 16 colores y su paleta JASC correspondiente**:
  - `OMEGA BALL`: cápsula carmesí con núcleo Ω.
  - `INFINITY BALL`: cápsula oscura con detalles dorados.
  - `SUPER EXP. SHARE`: medallón hexagonal metálico y núcleo cian con tres nodos.

## Qué no se declara implementado

- Una renovación completa de cada menú vanilla (mochila, Pokédex, resumen, combate, PokéNav, menú principal): aún necesitan trabajo específico.
- Catálogo completo de Pokémon/huevos, todos los ítems y Gears futuros, nuevos sprites de objetos todavía no declarados en `items.h`.
- Transiciones gráficas avanzadas ni efectos fuera de presupuesto de GBA.
- Pruebas reales de compilación/emulación: pendientes en este entorno.

## QA antes de mezclar

1. Construir ROM con el Makefile del proyecto y verificar la conversión de `graphics/items/icons/*.png` y `graphics/items/icon_palettes/*.pal`.
2. Comparar los tres iconos en bolsa, entrega de objeto y crafting contra fondos claros/oscuros, revisando transparencia y colores.
3. Opciones → UI THEME: cambiar de ECLIPSE a DAYLIGHT, guardar, reiniciar, volver a Horizon y Crafting; comprobar persistencia y contraste.
4. Navegar A/B/L/R, abrir y cerrar Horizon y crafting al menos diez veces sin fugas de sprites/ventanas, freezes ni glitches.
5. Elaborar Super Exp. Share y cancelar: se ha de consumir solo Exp. Share al confirmar y conservar la Medalla Piedra.
6. Revisar la visualización de la barra lateral de Horizon con selección y avisos de sección pendiente; comprobar que el tema no cambia compras ni probabilidades.

Siguiente paso de diseño: una hoja de estilo reutilizable para mochila, menú del equipo, resumen y combate, con las dos paletas y un presupuesto de tiles/paletas por pantalla.
