# Horizon Shop — seis ofertas Pokémon y huevos con calidad RNG

**Estado:** PR #12 experimental, apilado sobre #11 → #10 → #9 → #8. No fusionar a `master` antes de compilar, comprobar guardados y pasar QA en emulador.

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
