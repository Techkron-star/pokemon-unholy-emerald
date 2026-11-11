# Unholy Emerald — Catálogo ampliado de Gears: combate, aventura y competitivo

> **Diseño propuesto, 11-11-2026. No implica implementación.** Especificaciones para revisión, crafteo, motor de batalla y QA. No se alteran IDs ni partidas guardadas en esta rama. Los seis Gears registrados en `feat/unholy-gear-effects-20261110` siguen siendo la referencia de código; Teseracto y Ancla de Realidad tienen reglas acordadas pero implementación pendiente.

## Principios

1. **Gear y objeto ordinario ocupan slots diferentes.** Un Gear útil no debe anular sistemáticamente las opciones de objetos clásicos.
2. **Sobrecarga + recarga residual** es la norma; cualquier excepción se declara explícitamente.
3. **Un turno, un avance de recarga**, aunque se encadenen dos movimientos; sin activaciones dobles por multigolpe ni dobles.
4. **Antiexploit:** sacar y volver a introducir al Pokémon no rearma el Gear dentro de un combate. La fase debe seguir al Pokémon, incluso si cambia el puesto en dobles.
5. Los efectos que **potencian el mismo aspecto** que un objeto ordinario deben **promediar sus bonificaciones**, conforme a `docs/GEARS.md`; no multiplicar ni sumar automáticamente. Los efectos de distinta naturaleza se evalúan por separado.
6. Solo los Gears marcados **implementados y probados** pueden anunciarse como funcionales en ROM. En particular, no basta registrar un ID/nombre para afirmar que protege de trampas o efectos secundarios.

## Las Sangres: una por CADA tipo (18 en total)

La lista anterior de cuatro Sangres era **incompleta**, no un límite. Son dieciocho identidades distintas, una por cada tipo; los cuatro nombres anteriores se conservan.

| Tipo de movimiento | Gear / Sangre | Identificador sugerido |
|---|---|---|
| Normal | Sangre Pura | `GEAR_BLOOD_NORMAL` |
| Fuego | Sangre Flamígera | `GEAR_BLOOD_FIRE` |
| Agua | Sangre Abisal | `GEAR_BLOOD_WATER` |
| Eléctrico | Sangre Fulminante | `GEAR_BLOOD_ELECTRIC` |
| Planta | Sangre Primordial | `GEAR_BLOOD_GRASS` |
| Hielo | Sangre Glacial | `GEAR_BLOOD_ICE` |
| Lucha | Sangre Marcial | `GEAR_BLOOD_FIGHTING` |
| Veneno | Sangre Tóxica | `GEAR_BLOOD_POISON` |
| Tierra | Sangre Telúrica | `GEAR_BLOOD_GROUND` |
| Volador | Sangre Celestial | `GEAR_BLOOD_FLYING` |
| Psíquico | Sangre Psiónica | `GEAR_BLOOD_PSYCHIC` |
| Bicho | Sangre Quitinosa | `GEAR_BLOOD_BUG` |
| Roca | Sangre Pétrea | `GEAR_BLOOD_ROCK` |
| Fantasma | Sangre Espectral | `GEAR_BLOOD_GHOST` |
| Dragón | Sangre Escamada | `GEAR_BLOOD_DRAGON` |
| Siniestro | Sangre Demoníaca | `GEAR_BLOOD_DARK` |
| Acero | Sangre Férrea | `GEAR_BLOOD_STEEL` |
| Hada | Sangre Divina | `GEAR_BLOOD_FAIRY` |

**Balance preliminar, NO aprobado definitivamente:** sobrecarga del **primer movimiento dañino compatible: +40 %** de potencia; recarga de **3 finales de turno** con **+12 %** residual a movimientos dañinos de ese tipo. No funciona con estados; no altera efectividad ni inmunidades. Un movimiento encadenado compatible después del primero recibe solo el residual; la recarga avanza una vez por turno. El Gear no se consume. Los nombres no implican que el Pokémon deba pertenecer a ese tipo.

**Crafteo sugerido:** material genérico + **esencia del tipo** correspondiente, obtenido de encuentros, encargos y jefes sin exigir 18 sistemas separados. Los materiales y cantidades deben equilibrarse con la economía real y no declararse implementados antes de hacerlo.

## Gears útiles en aventura y competitivo (nuevos)

Todas las cifras son **propuestas para playtesting**, no valores aprobados/implementados. El disparador debe consumir la sobrecarga únicamente cuando ocurra el evento descrito; en ausencia del evento, el Gear permanece armado (sin forzar activaciones gratuitas cada turno). La recarga estándar propuesta es de **3 finales de turno** a partir de la activación, salvo donde se indique otra cosa. El residual se aplica durante esa fase.

| Gear | Evento de sobrecarga y efecto | Residual (durante recarga) | Utilidad |
|---|---|---|---|
| **Botas del Umbral** | En la primera entrada mientras está armado, anula daño, estados y bajadas de velocidad de las trampas de entrada **solo para el portador**. | Próximas entradas: reduce 50 % el **daño** de trampas; NO impide estados/bajadas. | Cambios frecuentes, aventura larga, control de hazards. |
| **Velo Críptico** | Niega el primer efecto **secundario** recibido de un ataque rival (incluye retroceso); no niega daño ni efectos primarios del movimiento. | Reduce un 25 % **relativo** la probabilidad de secundarios aleatorios (< 100 %); no altera efectos garantizados. | Antiflinch, apoyo en dobles, combates contra estados. |
| **Amuleto Inquebrantable** | Niega la primera **reducción de niveles** causada por rival, incluida Intimidación; no bloquea movimientos autodebilitantes. | Si un efecto rival bajaría >= 2 niveles de una estadística, reduce la bajada en un nivel. | Sweepers, Intimidación, equipos defensivos. |
| **Batería de Reserva** | Al acabar un turno con STA < 50 % y estar armado, restaura **20 % de STA máxima**, sin exceder el máximo. | **+4 % de STA máxima** adicional al finalizar cada turno. | Estrategia de STA, historia, cadenas, control de agotamiento. |
| **Ancla de Escape** | Permite realizar una elección válida de cambio a pesar de trampas (movimiento/habilidad) **una vez por ciclo**, solo en combates que permiten cambiar. | Reduce 15 % daño residual de efectos de aprisionamiento; **no** bloquea impedir cambio. | Antitrap, salvar Pokémon importantes. |
| **Crisol Antídoto** | Al adquirir un estado principal y estar armado, lo cura una vez al final del turno (sin reactivar múltiples veces ese turno). | Reduce 25 % daño residual de quemadura y veneno; no protege de sueño/parálisis en residual. | Aventuras largas, stall y defensas. |
| **Lente de Certeza** | El próximo ataque dañino que realiza una comprobación de precisión ignora modificadores de **evasión del objetivo** (no ignora inmunidades, Protección ni efectos externos de precisión). | +10 % relativo de precisión para movimientos dañinos ordinarios; no aumenta precisión de movimientos KO instantáneo. | Contra evasión, jefes, competitividad predecible. |
| **Módulo Vectorial** | En el siguiente movimiento de pivote compatible (Ida y Vuelta, Voltiocambio; Acua Jet NO), reduce su coste de STA un **25 %**. | Reduce 8 % del coste de STA de pivotes compatibles. | Pivot, manejo de equipos, control de recursos. |
| **Escudo del Último Aliento** | **Una vez por combate**, si recibe un golpe que le haría KO desde el 100 % de PS, sobrevive con 1 PS y queda con **0 STA**; no protege del daño posterior de multigolpes. | **Sin residual.** Excepción explícita, no cíclica. | Seguro de emergencia para historia / PvP sujeto a cláusula. |
| **Bastión Gemelo** | Durante la ventana armada de un turno en dobles reduce **20 %** el daño de movimientos de área al aliado; no bloquea daño propio. | Reducción de 5 % al aliado, mientras recarga. | Dobles y jefes con ataques de área. |

### Notas de balance y no solapamiento

- **Botas del Umbral:** las Botas Gruesas ordinarias siguen ofreciendo prevención continua de trampas. El Gear solo niega plenamente una entrada por ciclo; sin trampas no gana nada. Con botas ordinarias, no hay beneficio duplicado.
- **Velo Críptico:** la Capa Furtiva ordinaria protege de secundarios continuamente. No convertir el Gear en una versión permanente y gratis del objeto.
- **Amuleto Inquebrantable:** el Amuleto Puro ordinario niega bajadas de estadísticas; el Gear es una protección episódica.
- **Batería de Reserva:** si se combina con habilidades u objetos que restauran STA, sumar recuperaciones distintas solo después de aplicar topes y comprobar que no genera bucles infinitos de encadenamiento.
- **Ancla de Escape:** nunca permite huir de batallas guionizadas ni salta límites de combate; no se activa si ya puede cambiar normalmente. Distinguir cambio voluntario y cambios forzados.
- **Crisol Antídoto:** impedir que el efecto se active repetidamente con estados aplicados múltiples veces en un turno; documentar interacciones con Sincronía, Agallas y antídotos ordinarios.
- **Escudo del Último Aliento:** NO se apila con Banda Focus, Robustez o Aguante para generar varias supervivencias; en competitivo evaluar restricción o prohibición, por ser un segundo slot de seguro anti-KO.
- **Bastión Gemelo:** solo daño recibido por el aliado, no aporta nada en combates individuales; debe anunciarse así de forma clara.
- **Módulo Vectorial:** mantener una lista explícita de IDs de pivotes que incluyen cambio al ejecutar. No dar beneficios a ataques simplemente rápidos ni a teletransportes de historia.
- La **IA enemiga** debe considerar los Gears al planificar sin activar por accidente sus temporizadores durante la simulación de daño.

## Nueve Gears propuestos por el autor (11-11-2026)

**Los efectos centrales de los nueve Gears de esta sección son decisiones del autor**; los detalles que el autor no fijó (duración/cadencia, porcentajes en espadas y Berserker, reglas de prioridad) quedan claramente señalados para el balance. **No implementados todavía.**

| Gear | Efecto confirmado | Fase / detalle pendiente |
|---|---|---|
| **Nihil Engine** | Cada pocos turnos emite un **pulso de normalización** que elimina **una trampa rival** o **un efecto de campo ordinario** generado por habilidades o movimientos. Los campos/efectos **especiales NO se ven afectados**. | Periodicidad pendiente; **propuesta**: cada 3 finales de turno. Cada pulso elimina solo 1 objetivo elegible, no limpia toda la arena. Determinar prioridad trampa/campo y clasificación de «especial». |
| **Bufanda Siniestra** | **Cada vez que el portador causa un retroceso (flinch) efectivo al enemigo**, recupera **25 % de sus PS actuales**, no de sus PS máximos. | Pasivo por evento, sin enfriamiento inventado. Retroceso debe impedir realmente la acción; no curar por mero intento de aplicar flinch. Redondeo mínimo propuesto: 1 PS si tenía vida y cabe curación. |
| **Guante de Midas** | **Duplica los Pokédólares** recibidos **solo si el Pokémon que finalizó el combate** lo tiene equipado. | Efecto económico pasivo, fuera del ciclo. Comprobar portador en la conclusión válida del combate, antes de repartir premio **una sola vez**; sin premios en encuentros salvajes ni duplicación por dobles/encadenamiento. |
| **Mira de Francotirador** | **Sobrecarga:** multiplica por **3** la probabilidad de crítico y aumenta el **daño de los golpes críticos un 20 %**. **Recarga:** probabilidad ×**2**, daño crítico **+9 %**. | Multiplicar la probabilidad real calculada (con tope 100 %), no +3 niveles de crítico. No aumentar daño de golpes no críticos. Duraciones por decidir, con propuesta 1 turno de sobrecarga y 3 de recarga. |
| **Espada Carmesí** | Potencia **todos los movimientos de corte** e incorpora **quemadura** a sus impactos. | Potencia y probabilidad de quemadura no especificadas; **no convertir en 100 % sin aprobación**. Usar una sola lista de movimientos de corte, común a las tres espadas. |
| **Espada Plateada** | Igual que la Carmesí: potencia **todos los movimientos de corte**, pero incorpora **congelación** a sus impactos. | Potencia y probabilidad de congelación pendientes. La congelación puede anular turnos enteros; validar reglas de Frostbite/congelación de Unholy antes de fijar porcentajes. |
| **Necro Espada** | Potencia **todos los movimientos de corte** y **cura al portador cuando derrota a un enemigo**, calculando la curación a partir de **sus PS actuales** (no máximos). | Porcentaje de curación pendiente: el autor confirmó la base «vida actual», **no fijó una cifra**. Un único evento de cura por KO válido, sin reactivar por cada golpe de un multigolpe. |
| **Inyección Berserker** | Dispara las estadísticas **ofensivas** (Ataque y Ataque Especial) mientras **disminuye proporcionalmente las defensivas** (Defensa y Defensa Especial). El coste puede alcanzar una **defensa efectiva nula o negativa**. | Intensidades, acumulación y duración aún no especificadas. La semántica negativa debe conservarse como **vulnerabilidad por debajo de cero**, no descartarse con clamp a 1; usar cálculo seguro sin dividir por cero ni overflow. |
| **Anillo Feérico** (escrito inicialmente «Abillo faerico») | Si el portador **es tipo Hada**, recibe **menos daño de Veneno**. Para tipos que no sean Hada, no da beneficio. | Reducción % pendiente. Definir y probar por separado **movimientos de tipo Veneno** y **daño residual de envenenamiento/Tóxico**, sin crear inmunidad total. |

### Reglas de interacción y precisión

- **Nihil Engine no equivale al Ancla de Realidad.** El Ancla elimina clima, terrenos/campos, salas y trampas según su definición, incluso efectos que Teseracto hizo persistentes. Nihil hace **pulsos parciales**, limitados a trampas del lado enemigo o campos **ordinarios**. Para que «ordinario/especial» no sea arbitrario, cada efecto de campo debe registrar su **origen** (movimiento, habilidad, evento/script) y una clasificación inequívoca de protección especial. Si un campo ordinario fue hecho permanente por Teseracto, aún debe definirse si Nihil puede afectarlo; **no borrar por defecto las excepciones de Teseracto**.
- **Bufanda Siniestra:** curación igual al **25 % de PS actuales en el instante del flinch**, no al 25 % de PS máximos. Ejemplo: con 80/200 PS, cura 20 PS y queda en 100/200; el siguiente retroceso vuelve a calcular con la vida actual en ese momento. Si un objetivo ya no podía actuar por una causa anterior, no tratar el flinch como causado por el Gear. Se revisará competencia con objetos normales de retroceso.
- **Guante de Midas:** se evalúa al finalizar el combate, no al participar o ser el primero del equipo. **Interpretación técnica provisional**: Pokémon aliado activo responsable de cerrar el combate; en dobles o KO por daño residual, resolver quién cuenta como finalizador antes de implementar. El bono de premio no debe aplicar dos veces. Con Amuleto Moneda / Incienso Duplo, definir su coincidencia bajo la regla de promediar bonos equivalentes para evitar multiplicación libre de ×2×2.
- **Mira de Francotirador:** distinguir de **Periscopio / Garra Afilada**, que alteran niveles de crítico, y de la habilidad **Francotirador**, que aumenta el daño crítico. La habilidad Francotirador sigue su efecto original; toda coincidencia de un mismo bonus requiere regla explícita de promedio para que la Mira no produzca apilamiento descontrolado. **Ejemplo de probabilidad, solo ilustrativo:** base 1/24 ≈ 4.17 % → sobrecarga 3/24 = 12.5 %, residual 2/24 ≈ 8.33 %. Las tasas efectivas dependen de la generación/configuración concreta del hack. Los +20 %/+9 % modifican **daño solo cuando hay crítico**.
- **Las tres Espadas:** «de corte» debe comprobarse por metadatos/flags de movimiento compatibles con la clasificación de **Cortante (Sharpness)**, no por nombre ni por categoría física: algunos cortes son especiales. Sin daño de corte o sin impacto, no aplicar estado ni curación por KO. Verificar dobles, multigolpes, inmunidades de estado, objetivos con sustituto y KO simultáneos.
- **Inyección Berserker:** los stats del Pokémon en el guardado NO deben volverse negativos ni corromperse. La «Defensa negativa» se implementa como una **defensa efectiva de combate con déficit firmado** que amplifica el daño entrante cuando cruza cero, sin usar defensa cero/negativa como divisor de la fórmula tradicional. Esta representación conserva que sea peor que estar en cero y evita división por cero, bajo flujo definido y comprobable. Tanto Defensa como Defensa Especial requieren pruebas con golpes críticos, modificaciones de etapas, mutaciones y movimientos que ignoran defensas.
- **Anillo Feérico:** mantiene debilidades e inmunidades normales; **reduce daño**, no cambia la tabla de tipos, no cura Veneno y no anula Tóxico a menos que el diseño lo confirme. Identificar exactamente qué fuente de daño es «Veneno».

### Prioridad de implementación y pruebas de estos nueve

1. **P1 económico / simple:** Guante de Midas (ganancias de entrenadores y premio), Anillo Feérico (daño), Bufanda Siniestra (evento flinch). Ninguno debe heredar un temporizador de potencia que le impida funcionar.
2. **P2 de combate:** Mira de Francotirador, las tres Espadas (tres variantes con **una sola** definición de cortes, dos hooks de estado y uno de KO).
3. **P3 profundo del motor:** Nihil Engine (origen/tipo de campo especial/ordinario, trampas por lado y Teseracto); Inyección Berserker (cálculo de daño con vulnerabilidad firmada, mutaciones y casos de defensa efectiva <= 0).
4. Probar también uso por **entrenadores rivales e IA**, no solo por el equipo del jugador. Guardar/cargar, PC, batallas dobles, encadenamientos y límite de Gears.

**Estado:** diseños registrados por petición del autor, no presentes todavía como objetos funcionales, constantes, sprites, recetas o efectos en la ROM.


## Seis Gears adicionales definidos por el autor (11-11-2026)

**Diseño solicitado, NO implementado en la ROM.** Se conservan las mecánicas aportadas; activadores, excepciones y cantidades que no fueron especificadas quedan identificados como pendientes, no se inventan cifras definitivas.

| Gear | Efecto principal (confirmado por el autor) | Precisión técnica / pendientes |
|---|---|---|
| **Escudo de Aegis** | **Duplica la eficacia de cada nivel positivo de Defensa y Defensa Especial**, sin otorgar niveles adicionales. | Con +1 de Defensa se *muestra +1*, pero el **bono de esa etapa** vale por dos. No confundir con subir +2 etapas cada vez que se recibe un buff. Propuesta de cálculo: evaluar +n como +2n **solo al calcular el multiplicador defensivo**, sin alterar stages guardados ni exceder límites numéricos. Igual para Defensa Especial; etapas negativas normales. |
| **Ariete Nautilo** | **Cada golpe de contacto al rival rompe un nivel de Defensa**; si el rival no tiene aumentos, **continúa reduciendo su Defensa**. | Golpe válido: primero quita +1 del stage de Defensa positivo; desde 0 lo pasa a −1, y así hasta el suelo habitual de −6. En multigolpe, comprobar cada golpe de contacto válido; confirmar balance si se decide limitar a una activación por movimiento. No disminuir la estadística base. |
| **Guantes Cohete** | Los ataques **de puño reciben +1 de prioridad**, **aditiva** a la prioridad intrínseca del movimiento y a otras fuentes. | Usar flag de movimientos de puño (misma familia que Puño Férreo), incluidos los que ya son prioritarios. Calcular antes de ordenar acciones, aplicando el límite permitido por el motor sin desbordamiento. No afectar ataques que no sean de puño. |
| **Piedra de Afilar** | **Cada ataque cortante usado aumenta Ataque en +1 etapa**. Además, existe protección para que no te bajen Ataque «de 1≥» (literal del autor). | Interpretación operativa **provisional** de «1≥»: una vez alcanzado al menos **+1**, no puede reducirse mediante bajadas de Ataque enemigas **por debajo de +1** mientras lleve el Gear. Confirmar si la intención era bloquear cualquier descenso cuando se está en +1 o superior. No bloquear automáticamente Haze, robo de boosts o reseteos si no se define. Usar clasificación de corte compartida con las tres Espadas. Máximo normal +6. |
| **Túnica Rúnica** | El portador **no recibe daño de efectos ambientales ni trampas**; **destruye trampas** y obtiene **+1 etapa de Ataque Especial**. | La inmunidad de daño cubre clima dañino, peligros de entrada y efectos ambientales con daño (no implica inmunidad a daño directo de ataques). Falta fijar si limpia trampas de su propio lado, ambas partes, o solo cuando entra; también si el +1 ocurre al equiparla/entrar/al destruir trampas. **No otorgar +1 ilimitado por cambiar y volver** antes de definir activación. |
| **Cartel de Búsqueda** | Cuantas **más subidas de estadísticas tenga el rival**, **más daño le inflige el portador**. | Solo contar **etapas positivas activas del objetivo** (no IV/EV, stats base ni buffs pasivos invisibles) salvo aprobación expresa. Propuesta pendiente: bono porcentual por etapa positiva, con límite de daño a equilibrar. Stage de Ataque/Defensa/Velocidad/At. Esp./Def. Esp./Precisión/Evasión: definir exactamente cuáles cuentan antes de implementar. |

### Reglas generales y pruebas

- **Aegis:** un +1 de Defensa comúnmente da factor ×1.5; con interpretación propuesta, el +1 rinde como +2 en *el cálculo*, es decir, ×2. La interfaz continúa mostrando +1 y efectos que ignoran etapas defensivas siguen tratándose como corresponde. La idea no es «aumenta dos niveles», sino **cada nivel vale el doble**. Probar también +2..+6 y crítico/Unaware/Cuerpo Puro.
- **Nautilo:** desciende el stage de Defensa incluso si ya estaba en 0; si había +2, un impacto lo lleva a +1. La bajada está sujeta a inmunidades ordinarias a reducciones y límites de stages salvo decisión posterior. Considerar interacción con Escudo de Aegis.
- **Cohete:** la prioridad se **suma**, no reemplaza. Probar prioridad natural del movimiento, prioridades por habilidades, Trick Room, Psychic Terrain y encadenamiento. No todos los movimientos de Lucha son de puño.
- **Afilar:** distinguir subidas reales de stage por uso de un movimiento cortante de doble activación en multigolpes. **Propuesta:** +1 por uso de movimiento cortante válido, no +1 por cada golpe dentro de ese mismo movimiento, para evitar explosión instantánea de +6. La protección de Ataque es una interpretación marcada, a confirmar.
- **Rúnica:** inmunidad de daño no significa eliminar clima global, campos especiales ni trampas enemigas si no se especifica; documentar con claridad cuáles trampas se destruyen y la regla que entrega +1 At. Esp. Evitar bucle de entrada/salida.
- **Cartel:** requiere que el rival tenga bonificaciones **positivas**; las reducciones negativas no se suman como «más stats subidas». No afectar al aliado en dobles ni aumentar daño solo por mutaciones Omega/Titan sin boosts de etapas.

**QA:** combate individual y doble, cambios de posición, switches, turnos compuestos, boosts de etapas, límites ±6, estados protegidos de bajadas, golpes multigolpe, movimientos de puño/corte y sus flags, prioridades sumadas, campo psíquico, pérdida de bonus al quitar el Gear, entrada con trampas/clima y comparación de daño vs. rival con 0/+1/+6 boosts.

## Muramasa — reliquia de raids solicitada por el autor

**Concepto confirmado:** **Muramasa** es un objeto/Gear anticuración y antidefensa que **reduce los efectos curativos o defensivos del rival independientemente de la fuente**. Es **fabricable con exactamente 10 Fragmentos de Muramasa**, y esos fragmentos se **obtienen en raids**. No se ha especificado una medalla como ingrediente ni se debe exigir una por defecto.

### Receta confirmada

| Resultado | Cantidad | Material requerido | Fuente de obtención |
|---|---:|---|---|
| **Muramasa** | ×1 | **Fragmento de Muramasa ×10** | Recompensas de **raids** (frecuencia y dificultades pendientes) |

El crafteo debe consumir los **10 fragmentos** solo tras confirmar la elaboración, verificar materiales antes de gastar y registrar Muramasa como Gear equipable por el sistema existente, con objeto de inventario/icono/HUD cuando se implemente. **No existe todavía receta funcional ni probabilidad de drop confirmada**. No añadir costes monetarios ni materiales adicionales por su cuenta.

### Alcance del efecto: «independiente de todo»

- **Sin excepciones por procedencia:** el efecto ha de actuar frente a curaciones y mitigación/protección generadas por **movimientos, habilidades, objetos equipados ordinarios, Gears, clima, campos y otros efectos de batalla**, siempre que beneficien al rival afectado. El nombre de la fuente **no** debe ser una vía para eludir Muramasa.
- **Curación:** incluye recuperación directa de PS, drenaje, curación por turno y recuperación provocada por efectos defensivos o habilidades. Muramasa reduce la **cantidad efectivamente recuperada** por el rival, no la cantidad de daño inicial.
- **Defensa:** incluye bonificaciones temporales de Defensa/Defensa Especial, pantallas, barreras, escudos y **reducciones porcentuales de daño** de cualquier procedencia. El objetivo es debilitar la **ventaja defensiva que generan los efectos**, sin alterar las estadísticas base del Pokémon por defecto.
- **Protecciones binarias** (p. ej., Protección, inmunidad temporal por una habilidad, aguantar a 1 PS) **requieren una regla de resolución especial**: no se pueden «reducir un porcentaje» como si fueran curación. **Pendiente de concretar cómo las afecta Muramasa** sin convertir en comportamiento implícito una ruptura total de inmunidades o del guion de combates.
- La regla **no depende del tipo del portador, del rival, del tipo del ataque ni de la fuente del beneficio defensivo**. Sí debe comprobarse quién es el rival afectado para no debilitar accidentalmente a los aliados del portador en dobles.
- **No confundir reducción con cancelación:** el autor confirmó «reducir», **no** anular al 100 %; el porcentaje de reducción de curaciones, aumento de defensa y escudos **queda pendiente**. Cuando se fije, una propuesta de cálculo es aplicar Muramasa **una vez por evento de recuperación/mitigación** y nunca una vez por cada componente que lo causó, para evitar multiplicación involuntaria.
- **Fase de Gear pendiente:** dado que Muramasa se especificó por efecto permanente sobre las fuentes del rival y no por pulso/recarga, **no asignar arbitrariamente 1+3 turnos**. Propuesta a valorar: efecto pasivo mientras lo porte el Pokémon en campo, con condiciones explícitas para el rival. El usuario no ha confirmado aún ese detalle.

### Herida de Muramasa: bloqueo de pociones y reducción general de curación (11-11-2026)

**Decisión del autor (ampliada):** cuando un Pokémon que lleva **Muramasa** acierta a un **rival con un movimiento de contacto**, le aplica una herida temporal. Durante **1–2 turnos posteriores al ataque**, el rival **no puede restaurar PS mediante pociones**, y **TODAS las fuentes de recuperación de PS** que sí pueden operar durante la herida quedan **reducidas aleatoriamente entre 25 % y 75 %**, sin importar su procedencia: movimientos de recuperación, robo de vida, bayas, objetos equipados, Restos, habilidades, Gears, clima, terreno y efectos de turno. Este rango **no es exclusivo del drenaje**. Se aplica a PS recuperados, **no al daño infligido**.

- **Activación precisa:** el portador de Muramasa debe acertar un ataque de **contacto efectivo** al rival. No se activa si falla, es bloqueado, no hay contacto o si es el portador quien recibe contacto. En dobles, se aplica solo a cada objetivo rival realmente golpeado. Consultar flags y supresiones de contacto del motor.
- **Duración:** 1 o 2 turnos **siguientes** al del impacto. La selección aleatoria equiprobable de 1 o 2 es una **propuesta**, no una distribución confirmada. Avanza solo una vez por turno (incluidos turnos con movimientos encadenados). Impactos posteriores pueden **renovar** el estado, no apilarlo. Asociar la duración al Pokémon, no a la ranura de combate.
- **Pociones:** mientras dure la herida, la recuperación de PS por **medicinas curativas de combate** queda bloqueada al **100 %** (Poción, Superpoción, Hiperpoción, Poción Máxima, Restaurar Todo, etc.). No gastar el objeto si la curación está prohibida; mostrar mensaje. Para medicinas mixtas, bloquear su componente de PS y definir de manera explícita el componente de estados, sin dejar que se salte el bloqueo. No impedir objetos curativos **fuera del combate**.
- **Todas las demás recuperaciones de PS:** cada evento que normalmente curaría un total **H** pasa a curar **H × (1 − r)**, con **r aleatorio entre 0.25 y 0.75, ambos extremos incluidos**. Ejemplos: Recuperación, Amortiguador, Deseo, Síntesis, Puño Drenaje, Gigadrenado, bayas Aranja/Zidra, Restos, Regeneración, Piel Seca, Gears de curación o habilidades que otorguen PS. Recomendación técnica: generar **una única reducción por evento de curación** (sin nueva tirada por fuente intermedia), con redondeo consistente y tope de PS máximos. La distribución del RNG queda propuesta como uniforme entre porcentajes enteros hasta validación.
- **Ejemplo ilustrativo:** un movimiento, baya o habilidad que normalmente recuperaría **100 PS** recupera entre **25 y 75 PS** durante la herida; una poción recupera **0 PS** porque está bloqueada, no porque le toque una tirada del 75 %.
- **Sin apilamiento involuntario:** la reducción general anticuración de Muramasa definida antes (porcentaje no concretado fuera de la herida) **no se debe aplicar otra vez** a una recuperación ya reducida por la herida. Durante la herida, la reducción 25–75 % es la única penalización de Muramasa a **esa** curación. Los efectos defensivos de Muramasa se tratan por separado.
- **Compatibilidad:** este efecto **reduce**, pero no deshabilita, la curación de movimientos, bayas, habilidades, Gears y efectos del campo; la prohibición absoluta está limitada a **pociones/medicinas curativas en combate**. No afectar la recuperación de STA ni cambios de estados que no otorguen PS, salvo una regla futura.
- **Persistencia y seguridad:** el estado dura lo restante si el Pokémon cambia y vuelve en el mismo combate; se elimina al acabar la batalla. No generar curación negativa ni superar los PS máximos; no gastar pociones bloqueadas. La IA rival debe detectar la herida y evitar intentar curar con pociones.
- **QA adicional:** contacto efectivo/no contacto, fallo y Protección, sustituto, multigolpes, encadenamiento, 1/2 turnos, cambio y regreso, dobles, pociones sin consumo, medicinas mixtas, Recuperación/Deseo, ataques drenantes, bayas curativas, Restos, regeneración por habilidad, Gears, curación continua por clima/terreno, recuperación cero, RNG límites 25/75, redondeo y comprobación de que no hay **dos** reducciones de Muramasa en un mismo evento de curación.

**Matriz de QA necesaria:** Recuperación, Deseo, Restos, Regeneración, drenaje, pantalla Reflejo/Luz/Velo Aurora, aumentos defensivos, Protección, Sustituto, Foco Interno u otros defensivos pertinentes, Robustez/Banda Focus, clima/campos, Gears, dobles (enemigo vs. aliado), IA, encadenamiento y enfrentamientos donde el efecto defensivo proviene de una combinación de fuentes. No alterar inmunidades de tipo o reglas de batalla especiales sin especificación, y no afirmar que está implementado antes de probarlo.

**Estado:** especificación de diseño y receta confirmada, **sin sprites, fragmentos dados de alta, loot de raids, entrada de crafteo ni efecto de combate funcional**.


## Otras ideas anteriores (aún sin aprobación/implementación)

Se conservan las ocho ideas previamente propuestas; **NO mezclarlas con la lista de Gears implementados**: Espejo de Némesis (reflejo), Aguja de Cronos (drenaje de STA), Núcleo de Entropía (ruptura de pantallas), Orbe de Resonancia (multigolpe), Colmillo de Hidra (drenaje de PS), Prisma Anómalo (reducción de daño supereficaz), Grillete Astral (bloqueo temporal de cambio) y Sello de Cero (anular boosts defensivos).

Evitar solapamiento entre **Ancla de Escape** y **Grillete Astral**: ambos deben poder contrarrestarse de forma determinista. Evitar que los dos efectos de supervivencia consecutiva hagan invulnerable al portador en combinación con objetos.

## Orden de desarrollo recomendado

- **P0:** resolver amplitud y migración de IDs persistentes; completar interfaz de equipar, obtención real y QA de los Gears existentes. No prometer sprites o recetas no presentes.
- **P1:** implementar 18 Sangres mediante el modificador de potencia existente, con pruebas de cada uno de los 18 tipos y de las dos fases del ciclo. Anadir un catálogo claro en crafteo.
- **P2:** Batería de Reserva, Botas del Umbral, Velo Críptico y Amuleto Inquebrantable: cuatro puntos de entrada distintos del motor (STA, hazards, secundarios, cambios de estadísticas).
- **P3:** Crisol Antídoto, Lente de Certeza, Módulo Vectorial y Ancla de Escape.
- **P4:** Bastión Gemelo y Escudo del Último Aliento, tras QA intensiva de dobles y antiexploits.
- **P5 (nuevos Gears):** seguir los niveles P1–P3 específicos de la sección «Nueve Gears propuestos por el autor»; no mover Berserker ni Nihil a «implementado» hasta terminar los ajustes del motor.
- **Seis Gears nuevos (diseño confirmado):** implementar Aegis/Nautilo en etapas defensivas, Cohete en cálculo de prioridad, Afilar en movimientos cortantes, Rúnica en eventos ambientales/trampas y Cartel en evaluación de buffs positivos; verificar reglas abiertas antes de codificar.
- **Muramasa (diseño confirmado):** implementar obtención de Fragmentos en raids y receta ×10; motor de curación/mitigación unificado por fuente, sin excepciones por procedencia, más pruebas de protecciones binarias antes de habilitarlo.

### Límite de IDs y guardado

La rama base tiene `GEAR_MAX_COUNT = 32` y **5 bits** en `PokemonSubstruct1.gear`; por ello solo admite **31 Gears + GEAR_NONE**. Proyección: 8 Gears existentes/acordados + 18 Sangres + 8 ideas previas + 10 utilitarios + 9 añadidos por el autor + Muramasa + 6 Gears adicionales = **60** (sin contar Colmillo de Rayquaza si se convierte en Gear).

**No basta subir `GEAR_MAX_COUNT` a 128.** Se necesita una migración del campo persistente a, como mínimo, **6 bits** (hasta 63 Gears + GEAR_NONE) o idealmente **7 bits** (hasta 127 Gears + GEAR_NONE), y una auditoría de ocupación de bits/estructura, checksum, acceso `MON_DATA_GEAR`, Caja/PC y guardados antiguos. No sobrescribir bits usados por mutaciones, stamina u otros flags. Valorar una estructura de guardado ampliada y migración versionada, puesto que el proyecto admite incompatibilidad controlada entre compilaciones, siempre con respaldo de save.

### Casos de aceptación obligatorios

- Se comprueban las **18** Sangres: máscara de tipo correcta, sin aplicar a estados; daño y residual; objeto normal + Gear con promedio correcto.
- Dos movimientos encadenados provocan un solo avance de recarga; no se disparan dos sobrecargas en el mismo turno.
- Conservar temporizador al cambiar y volver; también cambios forzados, `U-turn` y cambios de slot en dobles.
- Entrada con Trampa Rocas, Púas, Púas Tóxicas y Red Viscosa en fase armada/residual y con Botas Gruesas ordinarias.
- Probar efectos secundarios aleatorios y garantizados; Velo no debe bloquear daño principal ni funcionamiento de habilidades ajenas.
- Probar Intimidación, reducciones multietapa, autodebuffs y casos con `Clear Amulet`.
- Probar STA en valores 0, 1, <50 %, 50 %, máximo, encadenamiento, buffs por KO y cambio.
- Probar estados principales, una curación por turno y daño residual con Crisol.
- Probar escape válido/prohibido, trampas, cambios forzados y reglas de combate contra entrenador.
- Probar el Escudo con Focus Sash, Sturdy, Endure, hazards, daño residual y movimientos multigolpe (y el modo competitivo, si aplica).
- Probar IA rival y entrenador de nivel alto con los mismos Gears que el jugador; sin ventaja oculta.
- Probar **Aegis, Nautilo, Cohete, Afilar, Rúnica y Cartel** con stages ±6, multiplicadores reales vs. niveles visibles, contacto, puños, cortes, prioridad adicional, trampas, clima y daño frente a buffs positivos.
- Probar **herida de Muramasa** con ataques de contacto y sin contacto, pociones bloqueadas sin consumo, 1–2 turnos posteriores y reducción aleatoria de **25–75 % a TODA curación** (movimientos, drenaje, bayas, objetos, habilidades, Gears, terreno, clima), sin doble reducción, cambios de Pokémon y combate doble.
- Probar **Nihil** contra trampas rivales, campos ordinarios/especiales, habilidades, ataques, Teseracto y Ancla; **Bufanda** con flinch efectivo e infructuoso, PS actuales; **Midas** con finalizador distinto del que inicia, dobles, veneno/KO residual, Amuleto Moneda; **Mira** con probabilidades por etapa e inmunidad de críticos; **Espadas** con cada corte, estados, crítico, multigolpe y curación por KO; **Berserker** en defensa cero/negativa sin desbordamiento; **Anillo Feérico** en Hada, no Hada, ataques Veneno y Tóxico.
- `make`, tests que procedan, emulador visual e inspección de guardado; **ningún check se declara aprobado sin ejecutarlo**.

## Fuentes de inspiración (no equivalen a cifras de balance)

- Smogon, catálogo de objetos SV: https://www.smogon.com/dex/sv/items/
- Smogon, Covert Cloak: https://www.smogon.com/dex/sv/items/covert-cloak/
- Referencia de tendencias de sets OU (septiembre 2026, no norma oficial): https://pokekipe.com/sv/ou?elo=all&tab=items

**Estado al crear este documento:** catálogo de diseño; sin efectos nuevos conectados al motor, sin assets nuevos, sin recetas registradas ni QA de ROM.
