# Variantes cromáticas de Pokémon — prototipo

Cada Pokémon recibe un tono visual **estable** según su `personality` de 32 bits.
Ahora hay cambios sutiles y cambios fuertes sobre los colores principales de
cada especie. Los shinies también pueden tener variantes cromáticas: parten
siempre de la paleta shiny propia de la especie, conservan sus destellos y
siguen siendo shinies. Ningún tono altera estadísticas o probabilidades de
captura/shiny.

| Tono | Pokémon normales | Shinies | Efecto |
| --- | ---: | ---: | --- |
| Original | 30 % | 30 % | Paleta intacta |
| Cálido | 10 % | 5 % | +2 rojo, -2 azul |
| Frío | 10 % | 5 % | -2 rojo, +2 azul |
| Claro | 10 % | 5 % | +1 en cada canal |
| Oscuro | 10 % | 5 % | -1 en cada canal |
| Giro verde | 10 % | 12 % | R/G/B → B/R/G, solo colores saturados |
| Giro violeta | 10 % | 12 % | R/G/B → G/B/R, solo colores saturados |
| Complementario | 7 % | 13 % | Invierte el matiz principal, sin invertir grises |
| Perlado | 3 % | 13 % | Convierte colores principales a blancos, grises o marfiles claros, preservando sombras |

Por ejemplo, un Charizard shiny con tono perlado puede tener el cuerpo
blanquecino o plateado y acentos ligeramente teñidos; otros shiny conservan su
color original o pueden ser violetas/verdes según sus paletas. La variante
perlada es genérica, no una forma o sprite especial reservado para Charizard.
Los porcentajes son distribuciones esperadas entre muchos Pokémon; no son
tiradas repetibles al entrar a un menú.

La recoloración opera sobre la paleta de 16 entradas en RGB555 **ya cargada**.
No altera el color transparente (índice 0), los contornos casi negros, los
destellos casi blancos ni el bit alto reservado. Los tonos más intensos
recoloran solamente colores con suficiente saturación. Huevos sin cambios.

## Alcance

- Sprites de combate, incluido el fondo de batalla de Pokémon.
- Retratos en la pantalla de resumen y en el PC clásico.
- Sprites temporales de algunas animaciones de combate.

Los iconos pequeños compartidos del PC, los overworlds y otros personajes
conservan sus colores habituales. No se añaden bytes al `.sav` ni se usa
RAM extendida. El tono se conserva al evolucionar y al cargar la partida.
Los Pokémon shiny mantienen su bandera shiny; su paleta resultante puede
diferir de la de otros shinies de la misma especie.

**Experimental:** probar en Pizza Boy Pro fades, combates dobles, evoluciones,
transformaciones y cambios de forma. El algoritmo no reconoce regiones del
sprite, solo colores de la paleta; un mismo color usado en alas y cuerpo se
recolorea en ambas zonas. Se puede ajustar la distribución y la intensidad.
