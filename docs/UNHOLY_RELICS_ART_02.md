# Unholy Emerald — paquete de reliquias · arte 02

Paquete visual de **19 iconos originales de 24×24** con paletas JASC-PAL de 16 entradas y transparencia de índice 0. Se generaron y revisaron a escala nativa, con previews Eclipse/Daylight en el paquete entregado al usuario.

## Esferas (2)
- **Overdrive Ball**: núcleo de sobrecarga y disipadores de energía naranja. Se sitúa entre Ultra Ball y Master Ball sin reemplazar ninguna esfera existente. Multiplicador, obtención y límites frente a mutaciones especiales pendientes de decisión (×4 es una propuesta no aprobada).
- **Alpha Ball**: cierre plateado, núcleo azul y geometría de sellos. Efecto sin definir.

**Omega Ball e Infinity Ball**: arte aprobado previamente y conservado, sin alteraciones en esta entrega; Infinity es recompensa posjuego y no debe venderse en Horizon.

## Gears (8)
`teseracto`, `visor_cosmico`, `corazon_titan`, `ancla_realidad`, `impulso_cosmico`, `cetro_siniestro`, `ski_mask`, `permafrost_fragment`.

## Inyecciones (4)
`sangre_flamigera`, `sangre_demoniaca`, `sangre_divina`, `sangre_escamada`.

## Especiales / dispositivos (5)
`colmillo_rayquaza`, `overloaded_exp_share`, `poke_simulator`, `pokelocator`, `mutator`.

Los tres últimos no se declaran objetos de mochila: **Poké Simulator y Mutator son máquinas**, y Pokélocator es una herramienta prevista. Estas son miniaturas conceptuales que pueden emplearse en sus futuras interfaces.

### Ubicación y estado
- PNG: `graphics/unholy_concepts/icons/*.png`
- JASC: `graphics/unholy_concepts/palettes/*.pal`
- Todos **visuales solamente**. No crear constantes, handlers, recetas, tiendas ni costes ficticios. Los efectos de Alpha Ball y la mayoría de Gears siguen pendientes de diseño.
- En una integración futura, mover solamente los ítems aprobados a `graphics/items/...` y registrar sus referencias en el motor, sin tocar números de guardado.
- **QA necesario:** confirmar PNG indexed + 16 colores + índice 0 transparente, conversión 4bpp en compilación, inspección real en emulador y contraste con ambos temas. El atlas no sustituye una prueba en emulador.

Referencia: `DISENO.md`, secciones Poké Balls, Gears y Progresión/QoL; `docs/UI_HORIZON_NEBULA.md`.

## Tanda 2 — actualización aprobada (2026-11-09)

El usuario aprobó el diseño de los cuatro Gears de esta tanda y corrigió el nombre a **Impulso Cósmico** (NO Motor Warp Cósmico / Impulsor Cósmico). Se redibujaron **Corazón de Titán, Ancla de Realidad, Impulso Cósmico y Cetro Siniestro** a **24 × 24 píxeles reales**, en PNG **indexado de 4 bpp (16 entradas de paleta, índice 0 transparente)** con paletas JASC-PAL. Se sustituyeron los cuatro bocetos anteriores, sin modificar las otras 15 reliquias ni los sprites aprobados Infinity, Omega y Super Exp. Share. El diseño visual de Impulso Cósmico representa un **motor warp en miniatura** con núcleo azul, pero su **nombre de objeto sigue siendo Impulso Cósmico**.

Estos cuatro sprites son assets visuales que NO se han conectado todavía a `src/data/items.h`, lógica de batalla ni interfaz final. Compilación y QA en emulador siguen pendientes.

## Tanda 2 — redibujo fiel a la ilustración (2026-11-09)

Los cuatro Gears (Corazón de Titán, Ancla de Realidad, Impulso Cósmico, Cetro Siniestro) se redibujaron a mano a 24 × 24 siguiendo la ilustración original: cristal púrpura con grieta rosa y banda dorada; lente dorada con estrella azul y anillos cruzados; motor warp en miniatura con núcleo azul y órbita púrpura; maza de espinas con ojo rojo y cinta carmesí. Mismo formato: PNG indexado, 16 entradas (índice 0 transparente, índice 1 contorno oscuro) y JASC-PAL. Siguen siendo solo visuales; QA en emulador pendiente.
