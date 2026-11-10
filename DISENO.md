# Pokémon Unholy Emerald — Documento de diseño

**Estado:** documento de diseño y seguimiento de implementación; las tablas de estado distinguen funciones compiladas, funciones no probadas en hardware/emulador y diseño pendiente.

**Base técnica sugerida:** `pokeemerald-expansion` (ya trae muchas mecánicas hechas, como Mega Evolución) + Porymap para mapas.

## Visión

Un Esmeralda donde el combate es un juego de **tempo y riesgo**: cada KO te da ventaja, puedes encadenar movimientos, pero todo cuesta stamina y el rival tiene las mismas herramientas. Si te confías, te barren el equipo. Encima, Pokémon especiales (alpha, super, enraged y Omega) que valen la pena cazar, un post-game durísimo y legendarios que vuelven en versión Omega.

---

## Estado de implementación (rama `feat/stamina`)

| Pieza | Estado |
|---|---|
| Menú de debug (ítems, dinero, Pokémon) | Hecho (el de pokeemerald-expansion, activado en el menú de inicio) |
| Stamina en combate | Implementado y compilado. Máximo = 40 + nivel/2 + EV HP/8 + IV HP/2. Costo relativo a stamina máxima (14% base + potencia efectiva/10 puntos porcentuales). Recarga configurable por turno. A 0 queda agotado y pierde acciones hasta recuperar el umbral |
| Barra de stamina | Implementada y compilada; clon de la barra de vida con colores y animación. Falta validación visual en emulador/hardware (`STAMINA_BAR_Y_OFFSET`) |
| Coste e información de stamina | Implementados en la selección de movimientos: coste del movimiento y stamina actual/máxima |
| Moxie y habilidades de snowball | Implementado. Restauran stamina por KO (Moxie, Beast Boost, Chilling/Grim Neigh, As One) |
| Movimientos encadenados | Implementado en combates individuales: SELECT marca el primer movimiento, A confirma el segundo; el rival obtiene una acción extra y también puede encadenar |
| Señal visual del encadenamiento rival | Implementada como destello breve del sprite rival antes de su segundo movimiento |
| Velocidad de texto | Implementada en Opciones: NORMAL, FAST e INSTANT. Los valores antiguos SLOW se normalizan al abrir el menú |
| Inicio rápido | Tras fijar el reloj se omiten TV/casa del rival/habitación, apareciendo en Pueblo Raíz justo detrás de la niña antes de subir hacia la Ruta 101 |
| Repelente renovable | Implementado: recuerda el último repelente y permite activar renovación automática tras el primer uso posterior al desgaste |
| PC: cursor por caja | Implementado durante la sesión: se recuerda la última posición del cursor por caja. No se persiste todavía en el guardado |
| Movimiento acelerado | Implementado y configurable: el jugador corre automáticamente en exteriores e interiores; A mantiene el movimiento lento y se respetan los tiles que prohíben correr |
| Renovación automática de repelente | Implementada y configurable: reutiliza el último repelente/cebo disponible al agotarse, consume el objeto y conserva el flujo existente si no está disponible |
| Sistema de crafteo inicial | Implementada la receta Repartir EXP + Medalla Piedra -> Súper Repartir EXP. La medalla no se consume; el objeto base sí se reemplaza |
| Progresión de crafteo | Al iniciar se reciben Repartir EXP, correr y bicicleta; el menú de crafteo se desbloquea tras la primera medalla mediante Roxanne |
| Horizon Shop desde el menú | Corregido el retorno: ya no intenta reabrir el menú principal sobre la tienda y vuelve al campo correctamente |
| Captura de mutaciones | Corregida la captura de Enraged/Omega: ya no son imposibles; conservan su penalización de probabilidad |
| Identificación de mutaciones | El resumen del Pokémon muestra `MUT: A S E O` junto a la habilidad; Birch explica los símbolos tras el rescate |
| Encadenamiento rival | El segundo movimiento no puede repetir el mismo movimiento del primero; la selección muestra `▶2` y el coste de stamina se reorganiza sin solaparse |
| Badges de Pokémon especiales | Badges persistentes y combinables; aparición salvaje/enemiga y penalización de captura implementadas |
| Cambio de habilidades | Ability Capsule y Ability Patch se venden en todos los PokéMart estándar desde el primer acceso; conservan sus reglas normales |
| Pokédex+ | Activada la interfaz HGSS existente: búsqueda, información, movimientos, evoluciones, huevos y pantalla de área; variantes especiales y validación visual pendientes |
| Horizon Shop | Entrada en el menú principal y catálogo inicial fijo de objetos implementados; Pokémon, huevos, discos, servicios y rotación persistente pendientes |
| Gestión de movimientos | Exclusiva de Horizon Shop: se retiran LEARN del equipo y START RELEARN del resumen; el acceso de Horizon conserva el recordador existente |
| Balls, Pokémon especiales, gears, Super Liga | Omega Ball e Infinity Ball añadidas; balance y contenido de recompensa siguen pendientes |

### Cambios recientes verificados

- `0e7f5e3c10`: encadenamiento del rival y señal visual.
- `e4ab4be0bc`: inicio rápido hasta Route 101 y velocidades NORMAL/FAST/INSTANT.
- `7216d77fac`: cursor del PC por caja e información actual/máxima de stamina.
- La ROM `pokeemerald.gba` compiló correctamente después de estos cambios. La compilación muestra advertencias gráficas `libpng` existentes, pero no errores de compilación.
- La renovación automática usa `VAR_LAST_REPEL_LURE_USED` (`0x404E`) y se puede desactivar con `I_REPEL_AUTO_RENEW`.
- Los badges se almacenan en los bits libres del substruct seguro del Pokémon y pasan por `Get/Set(Box)MonData`, por lo que participan en el checksum normal.

Todos los números de balance están en `include/config/stamina.h`.

**Limitaciones conocidas de la v1:** la stamina se reinicia al máximo al volver a entrar un Pokémon; el agotamiento reutiliza el mensaje de "must recharge"; un Pokémon agotado no se puede cambiar (igual que tras Hyper Beam); los movimientos de estado cuestan solo el costo base.

---

## 1. Núcleo de combate

### Stamina
- Es una stat más del Pokémon. Su máximo sube con el **nivel, los EVs y los IVs**.
- Cada movimiento gasta stamina según su **potencia**.
- Al agotarse, hay que esperar a que se recargue; mientras tanto no se puede hacer nada, ni siquiera Struggle.

### Movimientos encadenados (compuestos)
- Se pueden ejecutar **máximo dos movimientos encadenados naturalmente**, con restricciones; **los dos cuentan como un solo turno**.
- Cada movimiento consume su stamina correspondiente. No se pueden repetir movimientos dentro de la misma cadena (regla previa del diseño).
- Los movimientos de protección o que anulan un ataque durante un turno (tipo Protect) **no se encadenan** con otros.
- Un turno con dos movimientos solo avanza **una vez** la recarga de Gears y efectos medidos por turnos.
- Resto de restricciones y orden de actuación frente al rival: pendientes de detallar.

### Habilidades de snowball
- **Moxie reworkeado:** al noquear a un Pokémon, restaura al instante un porcentaje de stamina, además de subir el ataque. Así los atacantes de mucho daño (Haxorus y similares) se vuelven requeridos.
- Más habilidades de este tipo: encadenar KOs, destrucción.
- **Contrapeso:** debuffs que quitan stamina y movimientos que reducen el gasto de stamina.
- El rival también puede encadenar: hay que pensar cada jugada, o te barren el equipo completo.

---

## 2. Pokémon especiales
- **Alpha, Super, Enraged y Omega** son características de individuos; pueden **combinarse** en un mismo Pokémon, con combinaciones extremas muy raras.
- **Alpha:** inspirado en los Alpha de Legends Arceus; detalles de estadísticas y presencia aún pendientes.
- **Super:** requiere menos EXP, gana más estadísticas por nivel (independientemente de IVs/EVs) y puede tener **varias habilidades simultáneamente**.
- **Enraged:** crecimiento ofensivo más alto por nivel, a cambio de menor crecimiento/valores defensivos.
- **Omega:** anomalías genéticas excepcionalmente raras, con IVs/EVs máximos en cada stat (incluida la excepción al límite total habitual de EVs) y multiplicadores positivos **individuales por estadística de ×1.2 a ×1.5**, sujetos a la fórmula definitiva. Un Omega con +50 % en sus seis estadísticas base es el extremo superior ilustrado por el Dragonite de Lance.
- Omega puede encontrarse de manera aleatoria en la naturaleza, con zonas de incidencia mayor (por ejemplo, cueva de Groudon) y mini-jefes. Rareza comparable o superior a shiny; probabilidades exactas pendientes.
- Se conserva la intención de que los especiales sean amenazas importantes, incluso con ventajas al inicio del encuentro, pero deben concretarse los buffs reales por variante.
- **Actualización del mercado (10/11/2026):** seis ofertas de Pokémon por RNG individual, con mutaciones **hasta Ancestral**. **Titan, Ascendido y Omega están excluidos del catálogo Pokémon**. Los huevos del mercado sí admiten estas mutaciones, incluida Omega y combinaciones múltiples; el **Mutator sigue sin poder crear Omega**.
- Los huevos de Horizon tienen RNG de especie y mutaciones independiente de los encuentros, con premios Omega extremadamente infrecuentes; esto no aumenta las tasas de Omega en huevos normales de crianza.

### Cambio de habilidades

- La **Ability Capsule** y el **Ability Patch** están disponibles para comprar en todos los PokéMart estándar desde el primer acceso, sin requisito de medallas.
- La Capsule alterna entre las dos habilidades normales de la especie y no tiene efecto si no existen dos habilidades normales distintas.
- El Patch alterna entre una habilidad normal y la habilidad oculta cuando la especie dispone de ella.
- Ambos objetos se aplican desde el menú del grupo, se consumen solo después de confirmar el cambio y no crean habilidades múltiples para Pokémon Super.
- Los badges Alpha, Super, Enraged y Omega no cambian estas reglas en esta fase.

### Gestión de movimientos

- **MOVE MANAGEMENT** aparece en el menú principal y abre la selección de Pokémon usando el flujo existente del recordador.
- El catálogo de movimientos conserva las categorías y restricciones ya configuradas (nivel, MT, tutor y huevo).
- La pantalla de resumen existente gestiona el reemplazo cuando el Pokémon ya tiene cuatro movimientos; no se crea una ruta paralela para borrar HMs.

---

## 3. Poké Balls

| Ball | Notas |
|---|---|
| **Overdrive Ball** | Reemplaza a la "Hyper Ball" (que es el nombre japonés de la Ultra Ball). |
| **Alpha Ball** | Idea propia; efecto por definir. |
| **Omega Ball** | Puede capturar Omegas, pero **no está garantizado**. |
| **Infinity Ball** | Captura a cualquier Pokémon, incluidos los especiales. Equivalente post-game de la Master Ball. No se compra en tienda; es de post-game (recompensa de la Super Liga) o de cheat codes. |

- La **Master Ball deja de ser 100%** con los especiales: su eficacia queda reducida al 75% contra Alpha, 35% contra Super, 12% contra Enraged y 3% contra Omega, antes de aplicar el resto de la fórmula de captura.
- Como capturar un Omega exige una ball de end game, encontrarse uno a mitad del juego no sirve de nada intentarlo.
- Los encuentros salvajes usan tasas bajas (Alpha 4%, Super 2%, Enraged 1%) y los enemigos tasas sustancialmente mayores (20%, 12%, 6%, Omega 2%). Cada badge se tira de forma independiente, por lo que las combinaciones son posibles.
- Enraged y Omega requieren Omega Ball o Infinity Ball. La Omega Ball no garantiza la captura; Infinity Ball sí. Master Ball nunca garantiza especiales.

---

## 4. Gears — decisiones y código (10/11/2026)

- **Slot de Gear propio**, independiente del objeto ordinario, persistente en el Pokémon; los ciclos son efímeros y no ocupan un nuevo bloque de guardado. Base del sistema: `feat/gears-system`.
- **Ciclo**: 1 turno sobrecargado + 3 turnos residuales en Visor, Corazón y Ski Mask; luego recomienza. Dos ataques encadenados cuentan como 1 turno. Cambiar de Pokémon no recarga el Gear gratis en el mismo combate (rama de integración).
- **Visor Cósmico:** +20 % a ataques especiales al sobrecargarse, +10 % residual; evolución de Gafas Especiales.
- **Corazón de Titán:** +20 % a ataques físicos al sobrecargarse, +10 % residual; evolución de Banda Músculo.
- **Ski Mask:** +30 % a ataques Siniestros al sobrecargarse y **+21 % constante durante recarga**. No altera efectividades.
- **Cetro Siniestro:** efectividad Siniestro ×2 por cada tipo objetivo Psíquico, Hada, Normal o Lucha. Siempre activo, sin afectar la tabla global. Contra Lucha/Hada, ×4.
- **Impulso Cósmico:** efectividad Fantasma ×2 por cada tipo objetivo Normal, Psíquico o Hada. Siempre activo.
- **Permafrost Fragment:** los movimientos de daño Hielo duplican la probabilidad de congelar o Frostbite. Si el portador es tipo Hielo, cura **1/16 de los PS máximos al final de cada turno**, incluso sin atacar, como Restos. La curación no es vampirismo.
- **Promedio de bonos coincidentes:** Gear +20 % y Gafas/Banda normal +10 % => **+15 %** total (no +32 %). Con fase residual +10 % y objeto +10 % => +10 %. Restos y Permafrost simultáneos de la misma magnitud no duplican curación.
- **Teseracto:** consumo único, como Gema equipada. Vuelve permanentes clima, campos, salas y trampas mientras no se eliminen explícitamente; NO se implementa sin cerrar disparador y todas las interacciones.
- **Ancla de Realidad:** elimina clima, campos, salas y trampas, **incluso los hechos permanentes por Teseracto**; lo eliminado no reaparece. Pendiente si bloquea nueva activación durante la presencia.
- **Colmillo de Rayquaza:** +10 % a estadísticas para Pokémon Volador/Dragón al derrotar a Rayquaza; todavía necesita reglas de ciclo y compatibilidad de mutaciones.
- **Alcance real en código:** seis Gears con IDs, fases, daño y efectividades definidos en `feat/unholy-gear-effects-20261110`; aún falta UI de equipamiento y sprite HUD 16×32. No están integrados Teseracto/Ancla, y la evolución del sistema exige QA de combates dobles y guardados.

### Familia de Gears: Inyecciones sanguíneas
- Inspiradas en las gemas de tipo, **sin reemplazarlas literalmente**.
- Proporcionan un **gran bonus al primer ataque compatible**, luego un **bonus de daño residual menor** que se mantiene hasta que se recargue la siguiente sobrecarga cada X turnos.
- La bonificación residual debe notarse, sin superar permanentemente objetos ofensivos especializados.
- **Nombres establecidos:** Sangre flamígera; Sangre demoníaca; Sangre divina; Sangre escamada.
- Afinidades tentativas —**no confirmadas por el autor**—: flamígera/Fuego; demoníaca/afinidad negativa; divina/afinidad positiva; escamada/Dragón.
- Queda por decidir cómo aplicar la sobrecarga al primer ataque compatible si se encadenan dos en un solo turno; no duplicar la recarga ni asumir multiplicadores.

### Estado de implementación (10 noviembre 2026)
- El sistema base está en `feat/gears-system`; **seis Gears tienen lógica registrada en rama experimental**, sin UI de equipamiento ni arte HUD 16×32; detalles en [`docs/GEARS.md`](docs/GEARS.md).
- Provisional hasta que se decida: con dos movimientos encadenados compatibles, la sobrecarga la lleva el primero y el segundo recibe el residual; el ciclo avanza una sola vez por turno.

---

## Diseño de interfaz — Horizon Nebula (9 noviembre 2026)

Se definió el sistema visual de Horizon Shop y los menús personalizados. Es **diseño aprobado para implementar, no pantalla ya integrada en la ROM**. Consulta [`docs/UI_HORIZON_NEBULA.md`](docs/UI_HORIZON_NEBULA.md) para layouts, navegación, paletas Eclipse/Daylight, persistencia, límites GBA, sprites personalizados y pruebas.

- Horizon Shop abre una **pantalla raíz con cinco pestañas**: Pokémon, Discos, Objetos, Huevos, Movimientos.
- La lista, el panel de descripción y los botones comparten estructura, pero cada pestaña utiliza reglas y flujos propios.
- Los temas de interfaz **Eclipse** y **Daylight** se eligen desde Opciones y permanecen guardados; no dependen del ciclo diurno de la partida.
- Reusar la composición y efectos discretos de los temas Nebulous de esos nombres, no trasladar WebGL o shaders a GBA.
- Horizon tiene seis ofertas Pokémon independientes limitadas hasta Ancestral y cinco huevos visuales de rango Básico/Medio/Alto/Mítico/Legendario; estos últimos sí pueden dar Omega. **Infinity Ball sigue excluida**. Precios y probabilidades aún sujetos a balance.
- Crafting adopta panel de receta, requisitos visibles (poseído / requerido) y confirmación antes de consumir materiales.
- Movimiento / aprender / olvidar reutiliza el flujo existente; se crea navegación uniforme, no una segunda lógica incompatible.

**Estado:** especificado, pendiente de implementación en C y de validación visual en ROM/emulador.

---

## 5. Horizon Shop (primera entrega implementada; catálogo avanzado pendiente)

Sistema de compras y servicios accesible desde el menú principal del juego. Permite preparar y ampliar el equipo sin depender exclusivamente de los encuentros y tiendas físicas.

Horizon Shop reutiliza el menú del PokéMart para objetos y discos. La rama `feat/unholy-horizon-six-stock-egg-roulette-20261110` desarrolla seis ofertas Pokémon persistentes y cinco rangos de huevos reales con calidad visual independiente por RNG. Compilación y pruebas de emulador pendientes.

### Catálogo y progresión
- La oferta de **Pokémon y objetos** se selecciona de forma **aleatoria (RNG)** y cambia en función del **ciclo de día y noche** del juego.
- **Las medallas** determinan qué Pokémon y mercancías pueden aparecer en el catálogo; el contenido disponible progresa junto con la aventura.
- El catálogo Pokémon realiza seis tiradas de mutación independientes pero no ofrece nada superior a **Ancestral**; excluye expresamente Titan, Ascendido y Omega. Cada plaza se compra una vez. El catálogo de huevos usa su propio RNG de calidades y premios, incluyendo Omega, legendarios y hasta tres mutaciones simultáneas.

### Secciones del mercado
| Sección | Contenido / servicio |
| --- | --- |
| **Pokémon** | Seis ofertas simultáneas, una tirada RNG y compra por plaza. Hasta Ancestral; nunca Titan, Ascendido ni Omega. |
| **Objetos** | Inventario aleatorio de objetos normales y especiales. |
| **Discos** | Venta de discos de movimientos; selección aleatoria dentro del catálogo habilitado. |
| **Huevos** | Cinco huevos visibles (Básico, Medio, Alto, Mítico, Legendario); cada uno tira su propia calidad visual Estándar/Radiante/Prismática. Tras comprar se sortea una especie oculta y hasta tres mutaciones; incluso legendario Omega. |
| **Movimientos** | Servicio para **aprender y borrar movimientos** de tus Pokémon desde el mismo menú. |

### Avisos de ofertas mutantes (10/11/2026)

Al aparecer por RNG una oferta mutante, la señal se propaga sin falsos positivos: **punto rojo junto a HORIZON en el menú START** (paleta estándar), **punto rojo junto a Pokémon en Horizon** y **!!! RARE STOCK** en el panel. Los indicadores desaparecen al **abrir de verdad la sección Pokémon**, no al abrir simplemente el menú Horizon. Solo una nueva rotación con oferta mutante reactivará los avisos. Si no hay oferta mutante, no aparecen.

Implementación apilada en los PR #8 → #9 → #10 → #11 → #13, **sin fusionar a master antes de CI y QA en emulador**. Seis ofertas Pokémon con una mutación máxima por individuo y cinco huevos con calidad persistente y premios de hasta tres mutaciones. El formato de guardado se describe en `docs/HORIZON_MARKET.md`.

### Decisiones pendientes
- Precios, cantidades, existencia de unidades limitadas y escalado de costos por medalla.
- La primera versión del catálogo Pokémon se regenera al avanzar de día o de período día/noche, o al obtener una medalla nueva; abrir/cerrar no repite la tirada. La selección queda guardada en variables reservadas sin ampliar `SaveBlock1`. Ajustar esta periodicidad tras QA y feedback.
- Qué discos, objetos especiales, especies y huevos entran en cada tramo de medallas y período del día.
- Costos y límites del servicio de aprender/borrar movimientos; posibles condiciones para acceder a determinados movimientos.
- Balance final de huevos (precios, tasas de jackpots y representación gráfica de la ruleta) aún por validar. La versión actual **no garantiza** legendarios ni mutaciones, incluso en calidad Prismática.

---

## 6. Post-game

- Para llegar hay que ser **campeón de Hoenn**. La **Super Liga** viene después de la liga normal, no la reemplaza.
- Antes de ella el jugador ya puede tener Pokémon especiales y con gears.
- En la Super Liga, los líderes llevan **entre 1 y 4 Omegas** en su equipo. Dificultad extrema: "reza para ganar". Ejemplo: Dragonite Omega de Lance con +50 % en todas las estadísticas base y Danza Dragón; no atribuirle Moxie ni Lethal Tempo por defecto.
- Ganarla da la **Infinity Ball**.
- Después de la liga, los **legendarios derrotados reaparecen en su versión Omega**, inspirados en la pelea final contra Ultra Necrozma de Sol y Luna: muerte y destrucción para el jugador.

### Rayquaza
- Al bajar a **75% de vida o menos, mega evoluciona** durante el combate.
- Es una **trampa a propósito**: la idea es que el jugador se confíe antes del giro.

---

## 7. Progresión y herramientas QoL

| Progreso | Desbloqueo | Regla |
| --- | --- | --- |
| **Inicio; al bajar y encontrarse con la madre** | Repartir EXP y otros objetos QoL | La madre entrega las herramientas básicas de conveniencia. Lista adicional pendiente. |
| **Primera medalla** | Súper Repartir EXP | Cada Pokémon elegible recibe **100 % de la EXP**, sin repartirse la cifra entre el equipo. |
| **Cuarta medalla** | Repartir EXP Sobrecargado / Overloaded (nombre WIP) | Reparte EXP íntegra, concede EXP adicional, acelera crecimiento de **IVs y EVs** y hace que la **EXP adelante la eclosión de los huevos** del equipo, además de los pasos habituales. |
| **Cuarta medalla** | Poké Simulator | Máquina para ajustar **IVs y EVs a voluntad en cada stat**, pagando Pokédólares. |
| **Octava medalla** | Acceso limitado al Mutator | Máquina de mutaciones genéticas aleatorias. |
| **Ruta de Ralts (pueblo siguiente, tras la captura guiada por el padre)** | **Pokélocator** | Ítem clave del menú. Muestra todos los Pokémon de la ruta actual y se desbloquean datos según las medallas. Puede **forzar mayor probabilidad de mutación** en los salvajes de esa ruta. |

### Pokélocator (idea nueva, sin implementar)
- Lo entrega el padre al llegar al siguiente pueblo, después de enseñar a capturar un Ralts.
- Funciona como el **Pokénav de Pokémon Elite Redux**: se abre desde el menú, muestra la lista de especies de la ruta y su información.
- **Por medallas:** cada medalla desbloquea más información (especies, tasas de encuentro, probabilidad de mutación).
- **Forzar mutación:** el jugador puede activar un modificador de probabilidad de mutación sobre los salvajes de la ruta. Propuesta de balance (no confirmada): consumo de un recurso por ruta o por encuentro, y topes por medalla.
- Pendiente: qué recurso consume, si el efecto dura una sesión de encuentros o un tiempo fijo, y si las mutaciones forzadas respetan la regla de Omega (sin Omega en el Mutator).

- El reparto sobrecargado se comparó con efectos de **plumas, pesos y Golden Nugget**; falta definir exactamente la contribución de cada uno.
- Conversión EXP → avance de huevos, fórmula de EXP extra, límites de estadísticas, costo del simulador y listado QoL inicial: **sin cuantificar**.
- Ajustar IVs/EVs en el simulador **no convierte** a un Pokémon normal en Omega.

---

## 8. Mutator

- Tras la octava medalla, el jugador puede introducir **un Pokémon propio** y pagar dinero para intentar aplicar una **mutación aleatoria**, entre las mutaciones permitidas (por ejemplo Alpha, Super o Enraged).
- **Más dinero invertido implica mayor probabilidad** de mutación; probabilidad y distribución final aún sin fórmula.
- **Omega está completamente excluido** del Mutator.
- El procedimiento provoca pérdida proporcional de niveles, **máximo 15**. Penalización por intento fallido y relación dinero/niveles: pendientes.
- **No se puede rerollear una mutación hasta derrotar la Super Liga.** Si sale Enraged, el jugador se queda con ese resultado hasta entonces.
- **La victoria en la Liga normal no habilita rerolls**: deben superarse los combates de la Super Liga. Los límites exactos de «acceso limitado» tras la octava medalla están por definir.

---

## 9. Estados alterados, afinidades y destierro

### Estados añadidos
**Rooted, Ingravidez, Confinado, Cegado, Aterrado, Encadenado, Desterrado, Santificado y Demonificado.**
Frostbite ya existe y no se define como estado inventado. **Rooted, Ingravidez, Confinado, Cegado y Aterrado** son nombres confirmados **sin efecto completo confirmado**.

### Encadenado
- Impide al afectado **encadenar movimientos**. Puede seguir atacando individualmente.
- Duración, formas de retirarlo y otras restricciones: pendientes.

### Desterrado
- Dura **1–5 turnos elegidos aleatoriamente**.
- **1v1:** retira al objetivo del campo y **obliga a abrir la pantalla de cambio**. El Pokémon desterrado no puede volver hasta que pase el efecto.
- **2v2:** el afectado **permanece ocupando su posición**, pero **no puede cambiarse**. Si solo quedan esos dos Pokémon y cae el compañero que puede seguir combatiendo, el jugador **pierde**, aunque el desterrado conserve PS.
- Al recibir Desterrado se **borran automáticamente los efectos secundarios** que el Pokémon tuviera.
- **Invulnerabilidad:** no se le puede causar daño de ninguna fuente mientras dure el destierro.
- **Una sola vez por individuo para siempre:** tras padecer el destierro, ese Pokémon adquiere **inmunidad definitiva** a futuros Desterrados. Se requiere guardarla de modo persistente.
- Abierto: reglas si en 1v1 no existe otro Pokémon disponible, posibilidad de actuar en el espacio ocupado de 2v2, conflictos con cambios forzados/campos y definición precisa de efectos purgados.

### Santificado / Demonificado y STAB
| Afinidad | Tipos confirmados | Santificado | Demonificado |
| --- | --- | --- | --- |
| **Positiva** | Psíquico, Hada, Normal, Lucha | STAB favorable | STAB negativo |
| **Negativa** | Veneno, Fantasma, Dragón, Fuego | STAB negativo | STAB favorable |

- Se clasifican por asociación con **creación/orden frente a destrucción**. Los otros tipos, incluido Siniestro, siguen **pendientes de clasificación explícita**.
- El **STAB negativo** perjudica el daño de ataques del tipo correspondiente; multiplicador exacto **sin definir**. ×0.75 fue solamente un ejemplo.
- Falta especificar tipos duales, compatibilidad simultánea de estados y efectos en movimientos sin STAB.

---

## 10. Habilidades propuestas y acciones adicionales

### Nombres reservados
**Militarista, Terrornauta, Core Overdrive, Corazón Primogenio, Semilla Original, Yggdrasil, Drake, Yamata no Orochi, Susanoo, Amaterasu, Fundición, Lancelot, Berserker, Devora Almas, Marea Roja y Midas.**

**Solo Terrornauta tiene efectos concretos definidos** entre estos nombres; los demás son nombres pendientes. Las ideas de efectos de conversaciones anteriores **no son reglas aprobadas**.

### Terrornauta — field «Hora de Halloween»
Terrornauta **activa automáticamente** el campo **«Hora de Halloween»**, con estas reglas aportadas por el autor:

1. **«Ningún Pokémon pueda salir»** mientras esté activo el campo: se impide salir. Aclarar si bloquea cambios voluntarios, forzados, la huida o todas esas salidas.
2. Los **Pokémon aliados de tipo Siniestro recuperan PS equivalentes al 30 % del daño que infligen**.
3. Los **«enemigos normales» tienen un 15 % de probabilidad de quedar Aterrados al contacto**. Falta precisar si «normales» se refiere al **tipo Normal** o a enemigos ordinarios/no Siniestro, y la dirección exacta del contacto.
4. **Regla incompleta, conservar literalmente:** «los tipo siniestros enemigos son incapacidades de ganarle a un tipo». Falta el tipo objetivo y la definición operativa de «ganarle». **No implementar ni completar por conjetura**.

Pendientes: duración, quién puede sustituir el campo, superposición con otros terrenos, si se puede volver a invocar, y su interacción con Desterrado (que fuerza cambios en 1v1).

### Lethal Tempo (habilidad previamente planteada)
- Si el usuario noquea usando un movimiento que coincide con **uno de sus tipos**, obtiene prioridad para su siguiente turno, **sin superar movimientos de prioridad naturalmente mayor**.
- Pensada para Pokémon rápidos como Sceptile; **no asignar a Dragonite** por defecto. Confirmar si una cadena de dos ataques cambia la activación.

### Esquivar (acción de combate)
- Acción distinta de atacar/defender, con probabilidad de evasión base asociada a la **Velocidad**.
- Tras cada **esquiva exitosa**, la probabilidad para la siguiente esquiva se **reduce a la mitad**.
- Cada acción de **atacar o defender sin esquivar** recupera **15 puntos porcentuales** hacia la probabilidad base.
- Fallar la esquiva deja al usuario vulnerable. Fórmula base, coste de stamina, prioridad y definición de defender quedan pendientes.

---

## Notas técnicas y puntos abiertos
*(Sugerencias surgidas al hablarlo, no decisiones.)*

**Técnico**
- En Esmeralda **no hay espacio para una séptima stat** en la estructura del Pokémon guardado. Opciones: derivar la stamina de stats existentes (la más fácil para empezar), reutilizar campos poco usados (como los de concursos) o ampliar la estructura.
- Stamina, Gear y Omega requieren **persistencia en guardado**; también la inmunidad permanente tras Desterrado, la progresión de desbloqueos y cualquier avance propio de incubación por EXP.
- Los stats base caben en un byte (máx. 255). El bonus Omega conviene aplicarlo como **multiplicador en el cálculo de stats con un flag**, no tocando los datos de cada especie.
- La **UI** (barra de stamina, ver la del rival) es de las partes que más trabajo dan.
- Los entrenadores necesitan **IA con reglas simples** para usar la stamina con sentido.

**Balance**
- +50% a las stats base es enorme: un Sceptile (530) pasaría a unos 795, más que un Mega Rayquaza (780). Valorar un multiplicador por tier (por ejemplo alpha +15%, super +30%, omega +50%) o bajar el Omega.
- Riesgo de ventaja del que pega primero (el primer KO decide el combate). Los drenajes deben ser baratos y con buena prioridad.
- Decidir: qué pasa si dos Pokémon se quedan sin stamina a la vez, si los movimientos de estado gastan stamina, y si cambiar de Pokémon cuesta stamina.
- Hyper Beam, Giga Impact y similares ya tienen recarga: sustituir por costo de stamina alto.
- Los Gears deben funcionar mediante **sobrecarga cíclica y mejora residual**, con interacciones con turnos encadenados, y no limitarse a «más stats». Los entrenadores fuertes deberían poder usarlos.
- Los Omega de entrenador quizá no deberían tener el mismo buff que los salvajes.

**Encuentros y pelea contra Rayquaza**
- Omega salvajes: aura o señal visible antes del combate, poder huir siempre, y algo que ayude a cazarlos en post-game.
- Rayquaza: pistas sutiles antes del giro, punto de guardado antes de la pelea. La versión Omega no debe repetir el mismo truco.
- Considerar si derrotar a Rayquaza hace que no se pueda capturar en ese momento (el dilema gear vs. Pokémon) y avisarlo antes del combate.

---

## Faltantes por prioridad

### P0 — Indispensable antes de una versión jugable

- Correr automáticamente, correr en interiores y opción de movimiento acelerado. Implementado con `OW_AUTO_RUN`; mantener A permite caminar despacio.
- Repelente automático/renovable implementado; falta decidir y balancear el modo de repelente infinito.
- Saltar escenas repetidas de gimnasios, jefes y reintentos; el salto inicial ya está implementado.
- PC avanzado: favoritos/protección, filtros, búsqueda y operaciones múltiples. El traslado múltiple y el cambio rápido de caja ya existen en la base.
- Cambio/reaprendizaje de movimientos desde el equipo y eliminación de la dependencia de enseñar MO. Implementado mediante **MOVE MANAGEMENT** y el recordador existente.
- Pokédex+ con hábitats, horarios, evoluciones, movimientos y variantes. La interfaz HGSS existente ya está activada; quedan por integrar explícitamente las variantes Alpha/Super/Enraged/Omega y validar textos/pantallas en hardware.
- Guardado fiable, versionado/migraciones y pruebas de persistencia.
- Validación visual de la barra de stamina, encadenamiento y textos en emulador/hardware.

### P1 — Progresión principal

- Repartir EXP básico, Súper Repartir EXP y Repartir EXP Sobrecargado.
- Crafteo inicial de Súper Repartir EXP implementado; faltan recetas adicionales, interfaz de recetas y balance económico.
- Poké Simulator para IVs/EVs, con costos, límites, previsualización y excepciones Omega definidos.
- HUD de batalla para mutaciones: las cajas de vida muestran la inicial de la mutación principal (`A`, `S`, `E` u `O`); el resumen conserva la lista completa.
- Acceso contextual `LEARN` desde el menú de acciones del Pokémon: implementado para abrir directamente el gestor con el Pokémon seleccionado; queda pendiente ampliar filtros y acciones avanzadas del gestor.
- Animación residual del profesor al saltar la introducción: pendiente de limpiar el estado visual del objeto tras el warp inicial.
- Horizon Shop con catálogo persistente, medallas, día/noche, compras múltiples y economía equilibrada.
- Crianza y huevos: progreso por pasos/EXP, recogida múltiple, almacenamiento y compatibilidad.
- Gears: sistema y icono de esquina hechos (`docs/GEARS.md`); faltan Gears concretos, interfaz de equipar, obtención, entrenadores y HUD del turno de recarga.
- Información ampliada de estados, campos, habilidades y registro de combate.

### P2 — Contenido avanzado

- Mutator, reglas de reroll y pérdida de niveles.
- Presets competitivos y preparación pre-jefe.
- Super Liga, entrenadores con Omegas y recompensa Infinity Ball.
- Estadísticas de combate, revancha de entrenadores y herramientas de búsqueda especializadas.

### P3 — Mejoras opcionales

- Personalización estética, Pokémon acompañantes, estadísticas históricas y otras mejoras que no bloquean la versión jugable.

## Orden de implementación sugerido

1. Cerrar P0 de movimiento, repelentes, PC, movimientos y Pokédex.
2. Diseñar y probar guardado/migraciones antes de añadir datos persistentes de Pokémon.
3. Implementar P1: EXP, Poké Simulator, crianza y catálogo avanzado de Horizon Shop.
4. Completar información de combate y Gears.
5. Implementar Mutator, Pokémon especiales y reglas de captura.
6. Implementar estados, campos, habilidades adicionales y casos límite.
7. Implementar Super Liga, Omegas de endgame y contenido P2/P3.

Antes de programar: simular a mano en papel combates con 5 o 6 Pokémon, con sus costos de stamina, para ver si el snowball se descontrola.
