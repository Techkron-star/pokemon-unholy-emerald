# Variantes cromáticas de Pokémon — prototipo

Cada Pokémon tiene una variante de color **permanente** derivada de su
`personality` de 32 bits. Además de cambios leves de temperatura/luz,
ahora puede girar su **matiz** por el círculo cromático sin alterar el
brillo máximo, el mínimo ni la saturación de los colores principales.

Por ejemplo, un Heatran rojo podría mostrar partes principales magenta o
violetas con un giro de -60° o -90°. Los giros de 30° a 90° buscan tonos cercanos, mientras que una variante
menos frecuente gira 180° hacia el color complementario. No se invierten
los contornos ni las luces. Se preservan grises,
contornos casi negros, reflejos casi blancos y el color transparente.

| Tono | Pokémon normales | Shinies | Efecto |
| --- | ---: | ---: | --- |
| Original | 25 % | 25 % | Colores originales |
| Cálido | 10 % | 5 % | +2 rojo, -2 azul |
| Frío | 10 % | 5 % | -2 rojo, +2 azul |
| Claro | 7 % | 5 % | +1 en cada canal |
| Oscuro | 8 % | 5 % | -1 en cada canal |
| Matiz +30° | 9 % | 9 % | Giro cromático suave |
| Matiz -30° | 9 % | 9 % | Giro cromático suave |
| Matiz +60° | 6 % | 9 % | Giro cromático moderado |
| Matiz -60° | 6 % | 9 % | Giro cromático moderado |
| Matiz +90° | 3 % | 7 % | Giro cromático notable |
| Matiz -90° | 3 % | 7 % | Giro cromático notable; rojos pueden verse violetas |
| Complementario 180° | 4 % | 5 % | Invierte el matiz hacia un color opuesto, sin invertir grises ni luces |

Los shinies usan como base **su propia paleta shiny**, y pueden recibir
también una variante de matiz. Conservan su estado shiny, sus destellos y
su probabilidad normal de aparición. No hay variantes perladas/blancas especiales: se quitó esa idea para
mantener cada especie reconocible. Huevos sin cambios.

Los porcentajes son distribuciones esperadas entre muchos Pokémon, no
tiradas repetibles al entrar en un menú.

## Implementación y límites

- Paletas GBA RGB555 de 16 entradas; solo se ajustan los colores con cierta
  saturación. Se preserva el índice transparente 0.
- Sprites de combate, retrato de resumen, retrato grande del PC clásico y
  sprites temporales de algunas animaciones de combate.
- Los iconos pequeños compartidos del PC y los sprites de overworld no se
  recolorean por individuo.
- No se añaden datos a los archivos `.sav` ni se usa RAM extendida.
  Cada individuo mantiene su variante al evolucionar, cargar o capturarse.
- No se modifican IV, EV, habilidades, mutaciones, estadísticas ni mecánicas
  de captura.

**Experimental:** comprobar con Pizza Boy Pro transiciones de paleta,
combates dobles, cambios de forma y cambios de Pokémon. El algoritmo
recolorea colores y no regiones específicas del sprite: si alas y cuerpo
comparten un mismo color, los dos se desplazarán.
