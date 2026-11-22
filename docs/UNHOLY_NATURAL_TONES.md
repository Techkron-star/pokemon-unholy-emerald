# Identidad cromática individual — Algoritmo V1 (inmutable)

Cada Pokémon se colorea de forma determinista según su **identidad
individual**, que combina los dos campos ya persistidos en el `.sav`:

- `personality` (PID de 32 bits): identifica al ejemplar, no la especie.
- `otId` (OT ID de 32 bits): entrenador original; no cambia al intercambiarlo.

El algoritmo aplica dos permutaciones de 32 bits y forma un código de 64 bits
sin perder información de ninguno de los campos. Así, **dos pares PID+OT
distintos tienen códigos distintos**. Se derivan de él, sin usar el RNG del
juego, los parámetros de recoloración. Clones que comparten ambos campos
son indistinguibles sin introducir un serial adicional en el guardado.
Que dos códigos sean distintos **no garantiza** que sus paletas RGB555
resultantes sean visualmente distintas: el hardware tiene colores finitos.

## Distribución inicial

| Variante | Normales | Shinies | Operación |
| --- | ---: | ---: | --- |
| Matiz cercano | 90 % | 88 % | Giro continuo de aproximadamente −75° a +75° |
| Complementaria | 9 % | 9,5 % | Giro cromático de aproximadamente 160° a 200° |
| Inversión de luminosidad | 1 % | 2,5 % | Oscuros neutros → grises claros/blancos, manteniendo contornos |

Adicionalmente, el código individual determina una variación de saturación
de ±6 % y luminosidad de ±1 paso RGB555. Para evitar que la reducción a
15 colores colapse cientos de identidades en una misma paleta, cada índice
de color intermedio recibe una **microvariación fija de ±1 paso RGB555**.
Los colores acromáticos ajustan su brillo y los saturados desplazan
suavemente rojo/azul, sin tocar transparencia, contornos o altas luces.
No es ruido aleatorio por frame: usa el mismo código PID + OT ID e índice
de paleta en todas las pantallas. Ya no hay once categorías fijas.
No se altera la bandera ni la probabilidad de shiny. La base de un shiny
es siempre su paleta shiny propia de la especie.

**Ejemplo:** un Charizard shiny con cuerpo oscuro y la variante de
inversión tonal puede mostrar el cuerpo blanco-grisáceo, sin convertir
los negros profundos de sus contornos en blanco. La transformación
es genérica sobre colores de paleta, no una textura o sprite especial
para Charizard; depende de los índices de color que use cada especie.

## Permanencia y compatibilidad

El color nunca se sortea de nuevo al entrar en batalla, cambiar de caja,
guardar, cargar o intercambiar: se recalcula a partir del mismo PID y OT ID,
incluido el contexto de combate. Al evolucionar se conserva el patrón
individual, aplicado a la paleta propia de la evolución.

**Contrato de compatibilidad:**
`NATURAL_TONE_ALGORITHM_VERSION = 1`. Congelar la función de identidad,
pesos, hue y operaciones de color. Los vectores de prueba fijos impiden
cambiar accidentalmente el aspecto de Pokémon existentes en una próxima
build. Modificaciones futuras necesitarían migración o compatibilidad con V1.

- No se cambia la estructura `BoxPokemon`, ni el formato `.sav`, ni se usa
  RAM extendida. Se usa la OT ID ya guardada en equipo/PC/resumen.
- Se recolorean sprites de combate, retrato ampliado del PC clásico,
  resumen y ciertos sprites temporales de animaciones.
- Los iconos de 16×16 del PC y los overworlds comparten paletas; todavía
  conservan los colores de especie para evitar cambiar otros Pokémon.
- Se preservan entrada transparente 0, contornos muy oscuros, blancos
  casi puros y bits de paleta reservados.
- No se alteran IV, EV, stats, habilidades, mutaciones ni captura.

**Experimental:** comprobar visualmente en Pizza Boy Pro cambios de forma,
Transform, Illusion, combates dobles, shinies, evolución, PC y fades.
En una simulación de 1.000 Heatran normales y 1.000 Charizard shiny
con PID distintos del mismo entrenador, la versión con microvariaciones
produjo 0 paletas exactamente repetidas en cada lote (frente a centenares
sin ese refinamiento). Esta es una muestra reproducible, no una garantía
universal. RGB555 todavía puede producir tonos visualmente parecidos.
