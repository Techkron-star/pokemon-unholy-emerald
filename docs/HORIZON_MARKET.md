# Horizon Shop — seis ofertas Pokémon y huevos con calidad RNG

**Estado:** PR #13 experimental, apilado sobre #11 → #10 → #9 → #8. No fusionar a `master` antes de compilar, comprobar guardados y pasar QA en emulador.

## UI de tiendas y cierre seguro

Las cinco tiendas se presentan horizontalmente arriba: **PKMN · DISCS · ITEMS · EGGS · MOVES**. A abre la tienda seleccionada; izquierda/derecha o L/R cambia de pestaña; B reconstruye el campo y regresa al menú START. Las seis ofertas Pokémon usan una cuadrícula 3×2 y los cinco huevos una hilera de tarjetas horizontales en el PR #30. La salida desde Pokémon o Huevos con B cancela primero una compra en confirmación y luego vuelve a la barra superior.

No se modifica la ruleta de huevos, los rangos, el stock, el precio ni las probabilidades. El cierre libera el sprite/paleta temporal del huevo y las ventanas antes de llamar a `CB2_ReturnToFieldWithOpenMenu`, en vez de dejar `ShowReturnToFieldStartMenu` pendiente sin ejecutar.

**Validación pendiente:** compilación, retorno desde DISCS/ITEMS/MOVES, ciclos repetidos de apertura y cierre y prueba visual a 240×160 en Eclipse/Daylight.

## Interfaz horizontal de compra (revisión 11/11/2026)

**Corrección tras las capturas de Pizza Boy:** los paneles de **ITEMS y DISCS** mantienen el área de ítems nativa de 15×16 tiles (120×128 píxeles) para no tapar el dinero, los iconos, las descripciones ni las ventanas de compra. Dentro aparecen **dos tarjetas horizontales**, el artículo seleccionado y el siguiente, con nombre recortado por ancho, coste y marcas de catálogo; L/R recorre las ofertas, A compra y B sale. Con un solo producto no se muestra una segunda tarjeta duplicada. Los nombres completos se muestran debajo de las tarjetas. Continúan intactos los precios, la cantidad, el icono y la lógica de compra de PokéMart.

La navegación horizontal se activa **solo** cuando `sHorizonActive` está habilitado. Las tiendas tradicionales y su lista vertical no cambian. El cursor de la vista Horizon es transitorio: **no vuelve a sortear stock ni modifica las variables del mercado**. Se debe validar render a 240×160 y probar compras, cancelaciones, cambio de categorías, Eclipse/Daylight y retorno a START.

## Renovación visual de Horizon (PR #30 — 12/11/2026)

- Pokémon: cuadrícula de seis fichas, marcadores de mutación y precios individuales. El texto completo de selección se muestra por debajo y no se superpone a otras ofertas.
- Huevos: cinco tarjetas en línea con huevo pixelado realzado según calidad, textos compactos y precio/estado de la selección; la ruleta de especie sigue completamente oculta.
- MOVES: dos tarjetas visibles con selección y ayudas; se conservan las rutas previas de recordar/olvidar.
- ITEMS/DISCS: marcas de posición y bordes seleccionados dentro del panel PokéMart nativo. No se modifica el precio ni la lógica de compra.
- El cursor se conserva por pestaña hasta cerrar Horizon; los cambios visuales no generan RNG nuevo ni alteran el guardado.

**Sin validar todavía:** prueba visual directa en Pizza Boy, estabilidad en ambos temas, retorno a START, compra/venta, múltiples visitas, textos largos y disponibilidad de sprites de huevo con OBJ saturado.

## MOVES: recordar y olvidar (revisión 11/11/2026)

La quinta pestaña ahora abre una selección **RELEARN / FORGET** dentro del panel Horizon. **RELEARN** usa el recordador de movimientos y sus categorías, mientras **FORGET** abre el selector de Pokémon y la pantalla de movimientos existente. Elegir con A elimina el ataque elegido, salvo si es el último movimiento del Pokémon. B cancela sin alterar datos. No hay costes ni servicios externos de NPC añadidos.

**Pendiente de comprobar en emulador:** Pokémon de 1, 2 y 4 movimientos, huevos, cancelar en todos los niveles, ataques MO, menú de combate después de olvidar y persistencia tras guardar/cargar. No fusionar hasta validar estas rutas y el artefacto.

## Discos y Objetos: rotación persistente de stock (PR apilado)

El catálogo `DISCS` ofrece **seis MT distintas** de una lista elegible por medallas y ciclo día/noche; `ITEMS` incluye **tres suministros permanentes** (Poké Ball, Poción y Cuerda Huida) y **cinco artículos rotativos** (Poké Balls especiales, curación, repelentes, artículos de combate o especiales). Entre los objetos más avanzados se incluyen Caramelo Raro y PP Up, bloqueados hasta más medallas. **Infinity Ball y Gears no se venden** por este sistema: mantienen sus propias reglas.

La tienda de objetos y discos **no vuelve a tirar RNG al abrir/cerrar ni al pasar de pestaña**. La selección se genera de manera determinista a partir de dos mitades de una semilla guardadas en `VAR_HORIZON_GOODS_SEED_A` (`0x40B8`) y `VAR_HORIZON_GOODS_SEED_B` (`0x40BB`). Esas dos variables se actualizan al **rotar realmente** el mercado general (día, período o nueva medalla). En partidas anteriores cuyos Pokémon/huevos ya tienen stock válido, se inicializan las semillas sin volver a sortear sus ofertas.

Se preserva la compra normal del PokéMart: **puedes comprar varias unidades** de un artículo o una MT mientras permanezca en stock; aquí no existen flags de agotado por artículo. Los precios y restricciones particulares siguen sujetos a balance. Las seis MT cambian independientemente de los ocho objetos, pero ambas tiendas utilizan el mismo período persistido. Todas las listas terminan con `ITEM_NONE`, omiten duplicados y no ofrecen artículos que incumplen sus requisitos.

Código: `src/horizon_goods.c` y `include/horizon_goods.h`; pruebas de determinismo, cantidad y filtros en `test/horizon_goods.c`. No se ampliaron los SaveBlocks.

**QA de esta entrega:** entrar/salir repetidamente; abrir ambas tiendas antes y después del cambio de día/noche; comprobar desbloqueos por medallas, compras múltiples, estabilidad del stock tras guardar/cargar, regreso correcto al menú horizontal y ausencia de ítems repetidos. Comprobar en emulator con las paletas Eclipse y Daylight antes de fusionar.

## Catálogo Pokémon

- **Seis ofertas simultáneas e independientes** por período de mercado, una unidad por plaza. Se guardan especie, mutación, estado vendido y aviso leído, sin aumentar `SaveBlock1`.
- Rango máximo **Ancestral**. El catálogo de Pokémon **nunca** genera **Titan, Ascendido ni Omega**; tampoco especies legendarias o Infinity Ball.
- Son posibles Alpha, Super, Enraged, Hyper, Overloaded y Ancestral, con los denominadores originales de `include/config/special_badges.h` y las medallas mínimas asignadas. Cada plaza hace su tirada; una mutación como máximo por individuo vendido.
- Al entrar en START, punto rojo junto a Horizon si existen mutantes **no vistos**. Otro punto rojo en la pestaña Pokémon; cada una de las seis filas muestra un marcador para las ofertas mutantes. Al abrir de verdad las ofertas se reconoce el aviso, no se elimina la mutación ni se agota el stock.
- Precios iniciales sujetos a balance. A confirma (dos pulsaciones), B cancela o retrocede. Sin cobrar cuando faltan fondos o espacio de equipo y PC.

## Huevos — cinco rangos y calidad visual por RNG

En la pestaña **Huevos**, el catálogo muestra **huevos reales** (se usa el icono nativo de huevo de Pokémon), no una especie revelada. Cada rango tiene una plaza y un roll independiente de calidad al regenerar el stock:

| Rango del huevo | Precio estándar provisional | Prob. calidad Radiante* | Prob. Prismática |
|---|---:|---:|---:|
| Básico | 1,500 | 1/32 | 1/400 |
| Medio | 6,000 | 1/26 | 1/320 |
| Alto | 20,000 | 1/20 | 1/240 |
| Mítico | 75,000 | 1/15 | 1/160 |
| Legendario | 140,000 | 1/12 | 1/120 |

*La tirada de Radiante se ejecuta solo si la de Prismática ha fallado. Ambas son RNG: la calidad no está garantizada por el nombre del rango.*

- **Estándar:** precio base y probabilidades normales.
- **Radiante:** calidad resaltada; precio ×2 y probabilidad de premio raro/mutaciones mayor.
- **Prismático:** calidad excepcional; precio ×4 y mejores probabilidades, sin garantizar Omega ni legendarios.
- La calidad de **cada huevo** se decide y persiste con el mercado, antes de comprar. No se puede rerollear abriendo/cerrando. El icono visible sigue siendo de huevo; el nombre de especie y los badges se ocultan.
- **Al comprar**, otra tirada decide especie y hasta **tres mutaciones combinadas**. Puede tocar un Pokémon normal, un mítico, un legendario, un Omega, un legendario Omega o un legendario con varias mutaciones. Incluso el rango Básico puede obtener un legendario por un jackpot extremadamente infrecuente.
- Las mutaciones de huevos incluyen las nueve existentes (entre ellas Titan, Ascendido y Omega), **independientemente del límite de Ancestral del catálogo Pokémon normal**. Se aplican al individuo al crearse el huevo y persisten hasta eclosionar.
- La ruleta actual es **lógica de premios RNG**, no una animación de carrete. Incorporar una animación legible y cancelación solo después de QA de funcionalidad, con revelación diferida hasta eclosión.
- Cada plaza de huevo se compra una vez por período; las compras no descuentan dinero si falta espacio o fallan.

## Guardado

Variables ya existentes reutilizadas sin ampliar estructuras: las ocho `0x40F7–0x40FE`, además de `0x40FF, 0x40DB, 0x40DC, 0x40E5, 0x40A1` y `0x40A8`. Se reservan en `include/constants/vars.h`; **auditar conflictos con FRLG y scripts antiguos** antes de fusionar.

- Seis especies usan seis variables de 16 bits; dos variables adicionales guardan seis códigos de mutación de cuatro bits; otra guarda seis flags de comprado y un bit de notificación vista.
- Día, período, contador de medallas y versión controlan la regeneración. El cambio a seis plazas incrementó la versión del stock para invalidar el formato antiguo.
- Un último `u16` guarda los cinco grades (dos bits por huevo) y sus cinco flags de vendido.

## QA bloqueante

- [ ] Compilar Emerald, FireRed y LeafGreen; distinguir fallas nuevas de pruebas heredadas y comprobar el tamaño de `SaveBlock1`.
- [ ] Verificar las **seis** plazas y sus tiradas independientes (sin falsos positivos de marcador ni especies bloqueadas).
- [ ] Comprobar que nunca salgan Titan, Ascendido u Omega en Pokémon normal, pero sí puedan salir de huevos.
- [ ] Confirmar que los cinco rangos muestran huevos, y las calidades Estándar/Radiante/Prismática se representan sin desbordar 240×160.
- [ ] Guardar/cargar después de generar stock, comprar y eclosionar; verificar que la criatura recibida conserve la especie y todas sus mutaciones.
- [ ] Probar legendarios con `eggCycles=0`, combinaciones Omega+otras, premios normales, PC lleno, sin dinero, cancelar, comprar una única vez.
- [ ] Comprobar límites de GBA (sprites, paletas, tiles, ventanas) y fuga de icono tras abrir/cerrar Horizon repetidamente.
- [ ] Evitar rerolls infinitos por abrir/cerrar, ajustar reloj hacia atrás, entrar en una pestaña o salir de ella.
