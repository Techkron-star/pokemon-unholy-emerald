# Unholy Emerald — Ecosistemas y encuentros ampliados de Hoenn

## Objetivo

Ampliar **todas las rutas de Hoenn** por tandas, sin eliminar los Pokémon originales.
Cada especie nueva debe tener sentido para el hábitat, el método de encuentro,
el tramo de la aventura y su potencial evolutivo. Los Pokémon que normalmente
aparecen como regalos o encuentros especiales (por ejemplo, Kecleon) pueden
tener apariciones raras, pero no deben invadir los primeros caminos.

El proyecto usa actualmente **12 casillas de hierba, 5 de Surf y 10 de pesca**
por mapa. Esta primera tanda reutiliza casillas ocupadas por especies repetidas:
conserva las probabilidades de cada casilla, niveles y categorías de caña.
Así, aumenta la variedad sin modificar el motor ni perder los encuentros nativos.
Si las siguientes zonas requieren más especies únicas que casillas disponibles,
se diseñará una ampliación del motor, no una sustitución de especies originales.

## Tanda 1 — rutas iniciales

| Zona | Hábitat | Nuevos encuentros | Variedad terrestre final |
| --- | --- | --- | ---: |
| Ruta 101 | praderas de inicio | Sentret, Bidoof, Starly, Rookidee, Scatterbug | 8 |
| Ruta 102 | praderas, flores y río | Lillipup, Bounsweet, Sewaddle, Nymble; Surf: Wooper, Dewpider y Surskit; pesca: Poliwag, Barboach, Chewtle y Tympole | 10 |
| Ruta 103 | praderas junto al mar | Shinx, Nickit, Lillipup, Pidove y Ducklett; Surf: Buizel y Frillish; pesca: Shellder, Remoraid y Clauncher | 8 |
| Ruta 104 | costa y entrada al bosque | Fletchling, Bounsweet, Sewaddle y Combee; Surf: Buizel, Ducklett y Frillish; pesca: Krabby, Tentacool, Shellder, Horsea, Remoraid, Clauncher y Corsola | 9 |
| Bosque Petalia | sotobosque y árboles | Sewaddle, Scatterbug, Pineco y Phantump | 11 |
| Ruta 116 | pastizal rocoso y obras del túnel | Diglett, Roggenrola, Timburr y Dwebble | 10 |

- **Phantump** en Bosque Petalia aparece en la casilla rara del 1 %, como
  encuentro extraordinario coherente con un bosque.
- Las especies de evolución muy poderosa se introducen en etapas tempranas
  solo cuando su primera fase es razonable; no hay legendarios, míticos ni
  formas finales añadidos a las primeras rutas.
- Surf y cañas siguen requiriendo los desbloqueos normales. Los niveles ya
  configurados para estos métodos no cambian.

El manifiesto es `tools/unholy/hoenn_ecology_wave1.py`: ejecutarlo con
`--write` aplica esta selección determinista y sin cambiar niveles ni pesos.
Sin argumentos, comprueba los 48 cambios y garantiza la supervivencia de las
especies originales en las 12 tablas afectadas.

## Siguientes tandas

1. **Rutas 105–109**: costas, islas, Surf y pesca; fauna propia de playas y mar.
2. **Rutas 110–118**: praderas, desierto, ceniza volcánica, montaña y túneles.
3. **Rutas 119–123**: selvas, lluvia, zonas pantanosas y encuentros singulares.
   Evaluar Kecleon raro en 119/120 sin romper los eventos del Devon Scope.
4. **Rutas 124–134**: mar abierto, arrecifes, corrientes, submarinismo y pesca.
5. **Cuevas, bosques y mazmorras** de Hoenn: catálogo propio por profundidad y
   bioma, con especies comunes y apariciones especiales limitadas.

## Validación de juego pendiente

Comprobar en mGBA/Pizza Boy que los niveles, probabilidades y desbloqueos de
Surf/pesca son correctos; que siguen apareciendo todas las especies nativas;
que las formas y sprites añadidos se muestran bien; y que el Pokélocator y los
encuentros con mutaciones funcionan para especies nuevas.
