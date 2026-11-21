# Unholy Emerald — catálogo de 37 habilidades nuevas y revisadas

**Estado:** diseño acordado con Camilo, **no equivale a implementación ni a compilación**. Última revisión: 21 de noviembre de 2026.

Este documento conserva las **37 habilidades** definidas por tandas. Las cifras confirmadas se distinguen de los detalles que todavía requieren una decisión antes de programar. No asignar habilidades a especies ni modificar las originales de los iniciales hasta completar la fase de distribución y reequilibrio.

## Reglas compartidas

1. **Una habilidad activa por Pokémon, por ahora.** La posibilidad futura de múltiples habilidades para mutaciones Super no forma parte de esta entrega. Una habilidad puede producir varios efectos propios; eso no cuenta como portar dos habilidades.
2. **Movimientos originales frente a convertidos.** Dragonborn, Abisal y Malvado transforman movimientos originalmente Normales. Sus efectos adicionales se aplican **solo** a movimientos que ya eran Dragón, Agua o Siniestro, respectivamente, antes de la conversión.
3. **Una ejecución no equivale a cada impacto.** Salvo reglas expresas como Escama Acero o Black Hole, las probabilidades de un movimiento se comprueban una vez por ejecución, no por cada golpe interno. **Encadenamiento** sí permite nuevas comprobaciones por cada movimiento ejecutado, cuando proceda.
4. **Clasificaciones compartidas.** Los ataques **cortantes/de filo** deben usar el catálogo común de cortes (incluidos cortes especiales); *Doble Filo* no es un corte. Los **proyectiles** usan una clasificación propia, sin incluir automáticamente rayos, ondas, láseres ni púas. Los **puñetazos** requieren otra lista explícita. Las definiciones concretas se auditan antes de implementar.
5. **Daño mixto.** Un movimiento con proporción física/especial reparte la potencia original entre ambos cálculos y suma sus resultados **como un solo ataque**. No ejecuta dos movimientos ni duplica efectos secundarios. **GodHand es 50 % físico + 50 % especial, no doble daño.**
6. **Objetos ordinarios y Artifacts son sistemas distintos.** Una prohibición de usar objetos ordinarios no elimina automáticamente el Artifact. Los casos de copia, robo y restricciones se detallan por habilidad.
7. **Inmunidades y precisión.** No asumir que una conversión de tipo, inversión de efectos o alteración de efectividad atraviesa inmunidades o precisión, salvo que la habilidad lo indique. Respetar la prioridad y el coste de Stamina de cada movimiento salvo modificación explícita.
8. **Habilidades existentes:** **Cuerpo Magma** puede reutilizar/reformar `ABILITY_MAGMA_ARMOR` y **Gula**, `ABILITY_GLUTTONY`, evitando dos IDs con el mismo concepto. Revisar las habilidades oficiales homónimas antes de crear nuevas constantes.

## Tanda I — 1 a 5

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 1 | **Dragonborn** | Convierte los movimientos originalmente **Normal** en **Dragón**. Los ataques originalmente Dragón tienen **20 % de paralizar** al objetivo. Los movimientos convertidos **no** reciben esa probabilidad. |
| 2 | **Reaper** | Al debilitar a un Pokémon enemigo con uno de sus ataques, recupera **⅓ de sus PS máximos y ⅓ de su Stamina máxima**. Si debilita a dos rivales en dobles, puede activarse por cada KO; se respetan los máximos de cada recurso. |
| 3 | **Dual Wield** | Los ataques clasificados como **proyectiles** pasan a afectar al lado enemigo. El objetivo original recibe el daño normal y su compañero recibe una réplica de **50 % de potencia**. En individuales, no añade objetivos; nunca golpea al aliado propio por este efecto. |
| 4 | **Maratonista** | Cada nivel **positivo** de Velocidad reduce **10 %** el coste de Stamina de las acciones, hasta una reducción de **60 %** con +6 Velocidad. Las bajadas de Velocidad no producen descuentos. |
| 5 | **Reactor** | Al finalizar cada turno gana **+1 Ataque Especial**, hasta +6. Si alcanza +6 y pasa **un turno completo** manteniéndolo, su recuperación normal de Stamina aumenta al **150 %** mientras conserve ese nivel. Si pierde el +6, se pierde el bono y deberá volver a cumplir la condición. |

## Tanda II — 6 a 10

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 6 | **Cuerpo Magma** | Al recibir un ataque de contacto, tiene **35 % de quemar** al atacante (**cifra propuesta**, ligeramente superior a Cuerpo Llama). **Evapora los ataques de tipo Agua:** no recibe su daño. No se ha aprobado que esa inmunidad cure al portador. |
| 7 | **Árbol Mundial** | Cada movimiento **dañino de Planta** que impacte aplica **Drenadoras** al objetivo si es elegible. Se comprueba una vez por objetivo/ejecución: no se acumula Drenadoras y conserva la inmunidad de los Pokémon Planta. |
| 8 | **Abisal** | Convierte los movimientos originalmente **Normal** en **Agua**. Los ataques originalmente Agua tienen **10 % de congelar**. Los convertidos desde Normal no pueden congelar por esta habilidad. El **10 %** es la cifra propuesta en la conversación; confirmar antes del balance final. |
| 9 | **Super Batería** | Los movimientos Eléctricos que requieren **turno de carga** se ejecutan sin esa espera, conservando sus demás costes. La recuperación normal de Stamina aumenta al **150 %** de su valor base (**cifra propuesta**). |
| 10 | **Bélico** | Los ataques Acero tienen **25 % de colocar Púas de Acero** en el campo enemigo. Se propuso **una capa**, con daño de entrada modificado por efectividad Acero y retirada por limpieza habitual de trampas. Es inmune a **Intimidación** y **Aterrado**, no a cualquier reducción estadística. La probabilidad y la capa siguen siendo propuestas pendientes de cierre explícito. |

## Tanda III — 11 a 15

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 11 | **Slayer** | Los ataques de filo/corte ganan **+1 nivel de crítico adicional** al +1 general previsto para los cortes. Así, un corte que ya tenía +1 llega a **+2** con Slayer. |
| 12 | **Knight** | Cada vez que entra en combate obtiene **+1 Defensa y +1 Defensa Especial**. Los aumentos se someten a los límites habituales y se reinician normalmente al cambiar. |
| 13 | **Wizard** | Los ataques Psíquico que impactan tienen **20 % de hacer retroceder** (*flinch*) al rival cuando pueda actuar después (**cifra propuesta**). El Ataque Especial del portador **no puede disminuir**. |
| 14 | **Malvado** | Convierte los movimientos originalmente Normal en **Siniestro**. Los ataques originalmente Siniestro tienen **20 % de obtener +1 de prioridad** en su ejecución. Los convertidos no reciben la prioridad adicional. El 20 % sigue siendo cifra propuesta. |
| 15 | **Greedy** | Al ejecutar un **movimiento de control de campo**, tiene **30 % de robar** el objeto ordinario equipado del enemigo. Puede robarlo y conservarlo, pero **no puede activar sus efectos**: por ejemplo, jamás consume una baya robada. Las restricciones exactas de slot lleno y el alcance sobre Artifacts están pendientes; el robo de Artifacts **no** está aprobado. |

**Greedy — movimientos de campo:** se prevén clima, terrenos, pantallas, trampas y efectos similares. Cerrar la lista de movimientos elegibles y el destinatario del robo en combates dobles antes de programar.

## Tanda IV — 16 a 20

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 16 | **Gula** | **Duplica solo los valores numéricos** de los efectos de las bayas: curación, Stamina, PP y aumentos de estadísticas cuando corresponda. Curar un estado alterado es una operación no numérica: sucede una vez, no dos. |
| 17 | **Envy** | **Imita** los efectos del objeto ordinario y del Artifact del enemigo, sin robarlos. Los objetos y Artifacts propios **no funcionan** con esta habilidad. En dobles, la selección del enemigo al que imita está pendiente; se propuso el rival directamente enfrente. |
| 18 | **Colérico** | Cada movimiento de **puño** que impacte da al usuario **+1 Ataque**. Un movimiento multigolpe da una subida por ejecución, no una por cada impacto. |
| 19 | **Manthra** | Alterna un turno de acción con **uno de meditación obligatoria**, durante el que no ataca. En cada meditación recupera **⅓ de sus PS máximos** y, si tiene un estado alterado, tiene **40 % de curarlo**. La curación de PS no requiere tener un estado. |
| 20 | **Fallen Angel** | Los ataques originalmente Hada infligen **daño mixto por tipo**: **50 % de la potencia como Hada y 50 % como Siniestro**. Cada mitad calcula su efectividad por separado y ambas se combinan en **una única instancia de movimiento**, sin duplicar sus efectos adicionales. |

## Tanda V — 21 a 25

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 21 | **Samurai** | **Cada ejecución** de un movimiento de corte tiene **50 % de activar Sablazo**. Si hay dos cortes en una cadena, se hacen **dos tiradas independientes**. Sablazo absorbe el **100 % del daño** de un ataque **de contacto** y lo devuelve al atacante. Opera en el turno del corte tanto si el contacto enemigo llega antes como después del corte; no se conserva como escudo de turnos posteriores. No funciona si el movimiento propio no era de corte ni contra ataques sin contacto. |
| 22 | **Gélido** | Convierte movimientos Agua en **Hielo**. Tiene **20 % de causar Frostbite**, **no congelación completa**, al realizar un ataque de contacto o al recibir uno. La aplicación sigue las inmunidades/reglas del estado Frostbite. |
| 23 | **Banshee** | Los ataques Fantasma obtienen **+1 nivel de probabilidad de sus efectos secundarios**. Ejemplo conceptual del autor: subir un nivel la probabilidad de veneno de *Poison Sting*; **ese movimiento no pasa a ser afectado por Banshee**, porque no es Fantasma. **Falta definir la escala matemática** del +1; no sustituirla todavía por doble tirada ni por +10 puntos porcentuales. |
| 24 | **Oni** | Los ataques **físicos Siniestro** infligen daño mixto de categoría: **50 % con Ataque/Defensa y 50 % con Ataque Especial/Defensa Especial**. Los Siniestro ya especiales permanecen íntegramente especiales. Es un movimiento, no dos impactos. |
| 25 | **Drake** | Los movimientos Dragón **ignoran el 20 %** de la Defensa o Defensa Especial del objetivo según la categoría del daño. No equivale a un aumento permanente de estadísticas. |

**Samurai y orden de acciones:** la reacción puede afectar a contacto previo o posterior al corte en ese mismo turno. El diseño técnico debe determinar cómo reservar/verificar la activación sin aplicar curación o devolución retroactiva después de un KO. Si dos cortes activan Sablazo, queda por precisar si pueden reflejar dos ataques de contacto distintos o si uno sustituye al otro.

## Tanda VI — 26 a 30

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 26 | **Monje** | Los movimientos dañinos Psíquico se calculan como **45 % de potencia física y 55 % especial**. **Ninguna estadística del portador puede bajar** por efectos de combate. La interpretación de ataques Psíquico que originalmente ya eran físicos se revisará con pruebas de categoría mixta. |
| 27 | **Escama Acero** | **Cada contacto enemigo** contra el portador provoca daño al atacante equivalente a **⅛ de los PS máximos del atacante**, modificado por la **efectividad ofensiva de Acero**. El portador ignora efectos secundarios de movimientos de contacto, por ejemplo la bajada de Defensa de Golpe Roca; el daño principal del golpe sí se recibe. |
| 28 | **Luz de Alba** | Al final de cada turno, si tiene al menos un estado alterado, **30 % de curar uno**. Solo si se cura de verdad, gana **+1 Ataque Especial** y recupera **⅓ de sus PS máximos**. Sin estado que eliminar no concede ese bono. |
| 29 | **Gran Eclipse** | Mientras esté activo, los ataques de **Hada, Psíquico, Eléctrico y Planta** aplican **efectividad neutra ×1** aunque normalmente fueran muy eficaces o resistidos, **sin romper inmunidades ×0**. Ejemplo explícito: **Eléctrico → Agua = ×1**; **Eléctrico → Tierra = ×0**. **Impide establecer clima soleado**. Queda por delimitar si el efecto de tipos alcanza a ambos bandos o solo los ataques relacionados con el portador. |
| 30 | **GodHand** *(exclusiva de Arceus)* | **Antes de recibir cada ataque**, adapta el tipo del portador para ser **inmune** a ese movimiento, si existe un tipo que lo permita; en otro caso escoge la resistencia disponible más favorable. **Antes de atacar**, adapta el tipo del movimiento al **más eficaz** contra su objetivo, sin importar el original. Sus movimientos usan **daño mixto de categoría 50 % físico + 50 % especial**, con **potencia total normal**: **no hay daño duplicado ni dos ataques**. |

**Gran Eclipse:** no convertir arbitrariamente en neutro un tipo ajeno a la lista (por ejemplo, Tierra → Eléctrico). La aclaración final del autor se refería a **un atacante Eléctrico golpeando a un Pokémon Agua**, no a recibir un ataque Tierra.

## Tanda VII — 31 a 37

| N.º | Habilidad | Efecto acordado |
| --- | --- | --- |
| 31 | **Time Realm** *(secreta de Dialga)* | Adelanta aleatoriamente **1 o 2 turnos** los temporizadores de uso, espera, recarga y efectos de movimientos que conciernen al portador y sus aliados: Artifacts, objetos, movimientos y cuentas regresivas. **También acelera efectos negativos**, incluido **Canto Mortal/Réquiem**, aunque eso adelante el KO. **Prolonga la duración de climas** aliados; cantidad y momento exacto pendientes. |
| 32 | **Space Realm** *(Palkia)* | Despliega una burbuja espacial: el portador y su aliado **no reciben daño de movimientos de área** (por ejemplo, Terremoto). Los ataques de contacto que les afecten infligen **30 % menos daño**. Los ataques de objetivo único sin contacto conservan su comportamiento. |
| 33 | **Anti Realm** *(Giratina Origen)* | Sus ataques recibidos que serían **supereficaces pasan a neutros ×1**; las inmunidades se conservan. Los ataques supereficaces contra su **aliado también pasan a neutros**, no se anulan. El portador **flota** y obtiene la inmunidad a Tierra correspondiente mientras la levitación sea válida. |
| 34 | **God Realm** *(Ultra Necrozma)* | Mientras el portador esté en combate mantiene **Pantalla de Luz, Reflejo y Velo Sagrado** sobre su lado. Los ataques del portador **rompen las barreras enemigas convencionales**, pero **no atraviesan Protección** ni sus equivalentes que se definan expresamente. Las pantallas creadas por la habilidad dejan de depender de una duración por turnos mientras permanezca activo. |
| 35 | **Hiperbárico** | Despliega **Viento Afín** para su lado mientras permanezca en combate. Los **proyectiles enemigos se desvían y fallan**, **no** impactan contra un objetivo aleatorio. No clasificar rayos u ondas como proyectiles por defecto. |
| 36 | **Antithesis** | **Invierte los efectos positivos del enemigo** y los efectos perjudiciales de **estados alterados aliados** mientras la habilidad esté activa. Ejemplos: una baya enemiga que curaría 25 % de PS hace perder 25 %; +2 Ataque enemigo se convierte en −2; veneno o quemadura sobre un aliado **cura** la cantidad de PS que habría quitado. **Excepción expresa:** la **regeneración normal de Stamina por turno NO se invierte** ni se vuelve drenaje; **recuperaciones instantáneas** de Stamina por objeto, baya o efecto puntual **sí** se invierten. Las incapacidades sin inversión numérica y el alcance de efectos positivos como barreras permanecen pendientes. |
| 37 | **Black Hole** *(Guzzlord)* | Atrae hacia sí los **proyectiles enemigos que no sean supereficaces contra el portador**, incluso en dobles y aunque fueran dirigidos al aliado (similar a Pararrayos para selección de objetivo). **Anula por completo su daño** y recupera **⅛ de los PS máximos del portador por proyectil absorbido**. **Puede activarse múltiples veces por turno**, sin límite de activaciones aprobado. Los proyectiles supereficaces no son atraídos por esta habilidad. |

## Semántica especial de Antithesis

| Evento mientras Antithesis permanece activo | Transformación prevista |
| --- | --- |
| Curación de PS enemiga por baya, movimiento u objeto | **Daño** por la misma cantidad de PS que se habría recuperado. |
| Subida de estadísticas enemiga (+1, +2…) | **Bajada** de igual magnitud (−1, −2…). |
| Recuperación **instantánea** enemiga de Stamina por un objeto o efecto directo | **Pérdida instantánea** equivalente de Stamina. |
| Regeneración automática de Stamina al finalizar turno | **Sin cambios**; nunca invertirla. |
| Daño residual por veneno, veneno grave o quemadura sobre el portador/aliado | **Curación** de la misma magnitud; el veneno grave puede escalar esa curación con sus turnos. |
| Reducciones estadísticas asociadas a un estado alterado del portador/aliado | **Bonos** equivalentes mientras el estado esté sometido a Antithesis. |
| Parálisis, sueño, congelación, miedo u otros estados que bloquean acciones | **Pendiente de definición**: no asignar automáticamente turnos extra ni efectos inventados. |

La habilidad **no elimina** los estados alterados: invierte sus consecuencias aplicables. Al dejar de estar presente, los estados que sigan vigentes vuelven a sus efectos normales. Evitar convertir la inversión de un beneficio en una nueva fuente que se reinvierta infinitamente.

## Casos de QA obligatorios al implementarlas

1. **Dragonborn / Abisal / Malvado:** Normal convertido frente a ataque originalmente elemental; verificar que el efecto secundario no se concede al convertido.
2. **Samurai:** corte único, dos cortes encadenados, contacto enemigo anterior/posterior, ataques sin contacto y varios golpes; verificar daño devuelto y no duplicación accidental.
3. **Slayer / Colérico / Banshee:** corte con crítico innato, golpes multigolpe, un puño bloqueado y un ataque Fantasma con dos secundarios posibles.
4. **GodHand / Oni / Monje / Fallen Angel:** daño mixto con estadísticas físicas y especiales desiguales; confirmar **ausencia de doble potencia y doble activación**.
5. **Escama Acero / Gélido / Black Hole:** ataques de contacto multigolpe, proyectiles dirigidos al aliado, ataques supereficaces y cambios de tipo.
6. **Greedy / Envy / Gula:** baya, objeto permanente, Artifact y dobles; demostrar que no se usan objetos prohibidos ni se duplica una cura no numérica.
7. **Time Realm / Manthra / Reactor:** duraciones y contadores exactos en turnos alternos, turnos perdidos, activaciones encadenadas y cambio de Pokémon.
8. **God Realm / Gran Eclipse / Space Realm / Anti Realm / Hiperbárico:** alternar entradas y salidas, clima vigente, Pantallas, Área, Protección e inmunidades.
9. **Antithesis:** baya enemiga, subidas de stats, veneno grave aliado, recuperación **natural** de Stamina y recuperación **instantánea**, siempre en individuales y dobles.
10. **Persistencia:** habilidades nuevas obtenidas por cápsula/parche o especies asignadas, intercambio equipo/PC, guardado/carga y compatibilidad con mutaciones. No añadir una segunda habilidad simultánea por error.

## Decisiones todavía pendientes — no inventar valores

- **Banshee:** fórmula de «+1 nivel» en la probabilidad de efecto secundario; el usuario **no confirmó** ni una tirada adicional ni +10 puntos porcentuales.
- **Antithesis:** inversión de estados de incapacidad (sueño, parálisis, congelación, etc.), efectos positivos no numéricos como barreras, retroactividad sobre boosts ya existentes y alcance exacto en dobles.
- **Gran Eclipse:** si la neutralización de los cuatro tipos afecta a **ambos bandos** o solo al portador/sus movimientos; qué sucede con sol establecido antes de entrar.
- **Samurai:** protección ante un contacto que ocurre **antes** de la ejecución del corte y resolución de dos Sablazos activados dentro de una misma cadena.
- **Greedy / Envy:** selección de objetivo en dobles, destino de un objeto robado con slot ocupado, condiciones de copia de efectos consumibles y tratamiento de Artifacts propios de Greedy.
- **Cuerpo Magma / Super Batería / Wizard:** los porcentajes **35 % de quemadura / 150 % de regeneración / 20 % de retroceso** se propusieron, pero no recibieron confirmación numérica expresa.
- **Bélico / Abisal / Malvado:** los porcentajes **25 % / 10 % / 20 %** se propusieron pero no recibieron confirmación numérica expresa; la cantidad de capas de Púas de Acero debe cerrarse.
- **Time Realm:** qué cronómetros se pueden reducir hasta cero, duración adicional concreta de climas y comportamiento de temporizadores de terceros que beneficien indirectamente a aliados.
- **Combate general:** categorías de corte/puño/proyectil, coexistencia de reacciones con Encadenamiento y efectos activados por cada objetivo de movimientos de área.

**Próxima fase:** cerrar los puntos abiertos, asignar estas habilidades a especies/formas y después diseñar los reworks de **Torrente, Mar Llamas y Espesura**. Este catálogo por sí solo no modifica el comportamiento de ninguna ROM.
