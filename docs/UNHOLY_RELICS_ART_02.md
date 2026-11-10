# Unholy Emerald — paquete de reliquias · arte 02

Paquete visual de **19 iconos originales de 24×24** con paletas JASC-PAL de 16 entradas y transparencia de índice 0. Se generaron y revisaron a escala nativa, con previews Eclipse/Daylight en el paquete entregado al usuario.

## Esferas (2)
- **Overdrive Ball**: núcleo de sobrecarga y disipadores de energía naranja. En `DISENO.md` sustituye al nombre Hyper Ball; efecto aún no definido.
- **Alpha Ball**: cierre plateado, núcleo azul y geometría de sellos. Efecto sin definir.

**Omega Ball e Infinity Ball**: arte aprobado previamente y conservado, sin alteraciones en esta entrega; Infinity es recompensa posjuego y no debe venderse en Horizon.

## Gears (8)
`teseracto`, `visor_cosmico`, `corazon_titan`, `ancla_realidad`, `impulsor_cosmico`, `cetro_siniestro`, `ski_mask`, `permafrost_fragment`.

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
