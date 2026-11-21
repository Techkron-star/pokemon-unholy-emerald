# Tonos naturales de Pokémon — prototipo

Cada Pokémon no shiny puede tener una variación ligera de color definida por su
`personality` de 32 bits. No es otra mutación, no aumenta estadísticas ni cambia
la probabilidad de shiny.

- 40 %: paleta original.
- 15 %: tono cálido (ligeramente más rojo y menos azul).
- 15 %: tono frío (ligeramente menos rojo y más azul).
- 15 %: más luminoso (un paso de RGB de 5 bits por canal).
- 15 %: más oscuro (un paso de RGB de 5 bits por canal).

Solo se modifican los colores de la paleta de 16 entradas que acaba de
cargarse. Se respetan la transparencia, los contornos oscuros y las luces casi
blancas. Los shiny y los huevos usan siempre su paleta original.

## Alcance inicial

- Sprites de combate y su paleta de fondo.
- Retrato de Pokémon en la pantalla de resumen.
- Retrato ampliado del PC clásico.
- Sprites creados temporalmente para animaciones de combate.

Los iconos pequeños compartidos del PC, los overworlds y los sprites de otros
personajes siguen usando sus colores habituales: recolorearlos individualmente
exigiría bancos de paletas adicionales y comprobaciones de compatibilidad.

Este sistema no añade datos a los archivos `.sav` ni usa RAM extendida. Un
Pokémon conserva la variante al evolucionar porque se deriva de su personalidad.
Es una **prueba experimental**: debe verificarse visualmente en el emulador,
especialmente durante fades, combates dobles, transformaciones y cambios de forma.
