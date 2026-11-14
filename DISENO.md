# Pokémon Unholy Emerald — Documento de diseño

**UI nativa:** la adaptación del concepto de Pokéwatch a 240 × 160, sus cuatro pestañas de Resumen, servicios del reloj, componentes Eclipse/Daylight y matriz de QA se documentan en [UI_NATIVE_240.md](docs/UI_NATIVE_240.md).

**Estado:** documento de diseño y seguimiento de implementación; las tablas de estado distinguen funciones compiladas, funciones no probadas en hardware/emulador y diseño pendiente.

**Base técnica sugerida:** `pokeemerald-expansion` (ya trae muchas mecánicas hechas, como Mega Evolución) + Porymap para mapas.

## Visión

Un Esmeralda donde el combate es un juego de **tempo y riesgo**: cada KO te da ventaja, puedes encadenar movimientos, pero todo cuesta stamina y el rival tiene las mismas herramientas. Si te confías, te barren el equipo. Encima, Pokémon especiales (alpha, super, enraged y Omega) que valen la pena cazar, un post-game durísimo y legendarios que vuelven en versión Omega.

---

## Plan de producción por tandas

Punto de partida: [PR #31, `678927a`](https://github.com/Techkron-star/pokemon-unholy-emerald/pull/31), que contiene el último pulido de Horizon. La [CI del PR #30](https://github.com/Techkron-star/pokemon-unholy-emerald/actions/runs/43134287219) compiló Emerald, FireRed, LeafGreen y release, con 112 pruebas fallidas. La [CI del PR #31](https://github.com/Techkron-star/pokemon-unholy-emerald/actions/runs/43137489382) falló antes de ejecutar la suite porque el compilador rechazó `HorizonPrintFit`, una función estática sin uso. La tanda 0 empieza retirando esa función. Registrar ambos resultados por separado y comparar las pruebas por nombre y motivo con la misma configuración.

| Tanda | Trabajo | Criterio de cierre |
| --- | --- | --- |
| **0 · ROM de referencia** | Recuperar la compilación de Horizon. Clasificar los fallos de la suite, incluidos agotamiento de sprites y casos de captura. Revisar Horizon, mochila, Stamina, cadenas y guardado. | Compilan Emerald, FireRed, LeafGreen y release. La suite se compara con #30 y se investigan las diferencias. Pruebas de compra, cancelación, guardado/carga y batallas individuales y dobles. Capturas de Horizon a 240×160 en Eclipse y Daylight. |
| **1 · QoL de aventura** | Eliminar la animación residual del profesor en la introducción rápida si se reproduce, agilizar diálogos repetidos de jefes y construir Pokélocator de consulta por ruta reutilizando los datos de encuentros. A continuación, filtros y protección del PC y datos de variantes en Pokédex+. | Verificar partida nueva, guardado existente, primer intento/reintento, rutas con distintos métodos y horarios de encuentro y navegación del PC. El modificador de mutaciones del Pokélocator se programa cuando estén definidos su coste, duración y límites. |
| **2 · Objetos, Gears y sprites** | Completar de punta a punta los seis Gears existentes: obtención, mochila, EQUIP, efecto y HUD. Después ampliar el catálogo por familias y balancear progresión y recetas. Conservar los iconos ya aprobados y producir los faltantes. | Probar adquisición, cancelación, inventario lleno, reemplazo del Gear, guardado y daño real. Iconos de objeto de 24×24 y HUD de dos cuadros de 16×16, en 4 bpp con índice 0 transparente, convertidos por `gbagfx` y comprobados en ambos temas. La familia Sangre tiene exactamente una entrada por cada uno de los 18 tipos. |
| **3 · IA y jefes** | Auditar [la primera entrega de IA](docs/IA_JEFES_REVANCHAS_Y_MUTACIONES.md) contra la rama de integración y rescatar solo los cambios faltantes. Ajustar valoración de KO, efectividad, coste de Stamina, encadenamiento, cambios y fase de Gear. Introducir equipos con objeto ordinario y Gear simultáneos. | Escenarios reproducibles de entrenadores comunes y jefes en individuales y dobles: elegir ataques asequibles, evitar cambios cíclicos y cadenas inválidas, respetar la información revelada y compartir las reglas de Stamina y Gear. Probar primero un jefe completo antes de extender perfiles a toda Hoenn. |
| **4 · Progresión y persistencia** | Implementar Repartir EXP Sobrecargado, Poké Simulator, crianza ampliada y Mutator según sus reglas. Desarrollar identidad persistente y migraciones donde la función lo requiera. | Verificar límites, costes y desbloqueos por medalla, reinicio tras guardar, recuperación ante datos dañados, traslado entre equipo/PC/guardería y exclusión de Omega del Mutator. Validar el backend extendido en una ROM de diagnóstico antes de guardar progreso allí. |
| **5 · Combate avanzado y post-game** | Completar habilidades, estados y Gears con interacciones complejas. Añadir revanchas, Super Liga y legendarios Omega. | Probar desde una partida de campeón de Hoenn, verificar recompensas y reapariciones, y medir balance de cadenas, Gears y mutaciones en equipos completos. |

**Orden de trabajo.** Partir de la última ROM verificada. Usar una rama y una revisión pequeña por entrega jugable. Para cada objeto, recorrer datos → obtención → uso → efecto → sprite → pruebas. Los sprites pueden prepararse en paralelo reutilizando las paletas e iconos existentes. La IA comparte los cálculos de coste y daño del combate; sus simulaciones deben ser de solo lectura. Antes de ampliar una familia, validar una muestra representativa en la ROM.

**Decisiones ya registradas.** El [catálogo de Gears del PR #19](https://github.com/Techkron-star/pokemon-unholy-emerald/pull/19) incluye las 18 Sangres y el equipamiento doble de NPC. La primera Roxanne del diseño usa **Larvitar Alfa con Teseracto y Tormenta de Arena**; su nivel, movimientos restantes y objeto ordinario siguen abiertos. Programar ese equipo cuando el Teseracto tenga activación y consumo definidos. Reaper corresponde a una segunda habilidad oculta, con su propia persistencia y elección aleatoria entre ocultas mediante el objeto apropiado. Mantener las cifras de balance propuestas como tales hasta cerrar sus reglas.

### Primera entrega tras la ROM de referencia

La [CI de referencia del PR #32](https://github.com/Techkron-star/pokemon-unholy-emerald/actions/runs/43139031519) compiló Emerald, FireRed, LeafGreen y release. La suite registró 112 fallos, el mismo total que el PR #30.

| Área | Implementado | QA |
| --- | --- | --- |
| QoL | Birch explica en el primer combate cómo marcar un movimiento con SELECT y elegir otro con A. El tutorial advierte que un KO o el agotamiento interrumpen el encadenamiento. | Cuatro páginas, cadena Scratch → Ember y regreso al laboratorio comprobados en mGBA. El warp tras ajustar el reloj se probó en mGBA y no reprodujo la animación residual reportada. |
| Gears | Los seis Gears de crafteo tienen icono propio de HUD con dos estados. La recarga se guarda durante el combate por entrenador y ranura de equipo, incluso al volver por otro puesto en dobles. | La prueba de regresión del ciclo cubre ambos puestos y dos entrenadores con la misma ranura. Revisar el HUD en cambios, KO y animaciones de dobles. |
| IA | Cuando dos primeras acciones puntúan igual, el entrenador prioriza las que evitan el agotamiento si tiene alguna. En dobles aplica el desempate entre movimientos para un mismo objetivo. | Probar el primer gimnasio y medir el efecto de las cadenas voluntarias antes de ajustar otras decisiones. |

Las pruebas locales de esta entrega pasan en mGBA: 17 casos del grupo Unholy, 25 presentaciones de entrenadores en multibatalla y el intercambio de Gear desde la mochila. Quedan la suite completa y la revisión del HUD en Pizza Boy Pro.

---

## Estado de implementación (rama de integración apilada, noviembre 2026)

| Pieza | Estado |
|---|---|
| Menú de debug (ítems, dinero, Pokémon) | Hecho (el de pokeemerald-expansion, activado en el menú de inicio) |
| Stamina en combate | Implementado y compilado. Máximo = 40 + nivel/2 + EV HP/8 + IV HP/2. Costo relativo a stamina máxima (14% base + potencia efectiva/10 puntos porcentuales). Recarga configurable por turno. A 0 queda agotado y pierde acciones hasta recuperar el umbral |
| HUD circular de Stamina | Anillo visible de 9×9 px en un OBJ de 16×16. En la tarjeta alta del aliado queda en el hueco inferior izquierdo. Las tarjetas compactas y rivales reservan espacio delante del nombre para dejar visibles PS, estado y nivel |
| Coste e información de stamina | Implementados en la selección de movimientos: coste del movimiento y stamina actual/máxima |
| Moxie y habilidades de snowball | Implementado. Restauran stamina por KO (Moxie, Beast Boost, Chilling/Grim Neigh, As One) |
| Movimientos encadenados | SELECT marca el primer movimiento y A confirma el segundo. Corregida la repetición automática: el jugador solo ejecuta una segunda acción si encadenó explícitamente. La IA conserva su réplica extra cuando el jugador encadena; pendiente validar en ROM |
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
| Horizon Shop | PR #30: pestañas horizontales, cuadrícula Pokémon 3×2, cinco huevos horizontales, MOVES en tarjetas y objetos/MT en panel nativo; cursor conservado por sección y stock persistente. Pendiente de compilar y comprobar en Pizza Boy |
| Gestión de movimientos | Horizon MOVES presenta dos acciones, RELEARN y FORGET. La última reutiliza selección de Pokémon y resumen, descarta cancelar y protege el último movimiento. QA en emulador pendiente |
| Balls, Pokémon especiales, gears, Super Liga | Omega Ball e Infinity Ball añadidas; balance y contenido de recompensa siguen pendientes |

### Cambios recientes y estado de verificación

- `0e7f5e3c10`: encadenamiento del rival y señal visual.
- `e4ab4be0bc`: inicio rápido hasta Route 101 y velocidades NORMAL/FAST/INSTANT.
- `7216d77fac`: cursor del PC por caja e información actual/máxima de stamina.
- La ROM `pokeemerald.gba` compiló correctamente después de estos cambios. La compilación muestra advertencias gráficas `libpng` existentes, pero no errores de compilación.
- La renovación automática usa `VAR_LAST_REPEL_LURE_USED` (`0x404E`) y se puede desactivar con `I_REPEL_AUTO_RENEW`.
- Los badges se almacenan en los bits libres del substruct seguro del Pokémon y pasan por `Get/Set(Box)MonData`, por lo que participan en el checksum normal.

### Integración actual — 11 de noviembre de 2026

**Rama de este lote:** `fix/unholy-chain-horizon-ui-moves-20261111`, basada en `feat/unholy-special-origin-mutations-20261111`. Las mejoras no están fusionadas con `master` ni se consideran visualmente aprobadas hasta superar CI y QA en Pizza Boy.

| Prioridad | Hallazgo/cambio | Estado real |
| --- | --- | --- |
| P0 | El jugador ejecutaba un ataque extra sin encadenar si la IA encadenaba. Se retiró la acción `CHAIN_ROLE_NORMAL` añadida implícitamente al jugador; las acciones extras se conceden solo a la IA cuando el jugador encadena | Código corregido; prueba de turnos pendiente |
| P0 | Horizonte ITEMS/DISCS usaba una ventana de 27×8 tiles superpuesta a dinero, descripción y cantidad. Se mantiene la ventana nativa de 15×16 y se muestran dos fichas horizontales, seleccionada/siguiente, dentro del panel | Código corregido; revisión 240×160 pendiente |
| P0 | Anillos de Stamina desalineados o superpuestos al nombre y a los estados | Rediseñado el hueco en cada variante de tarjeta. Los apodos anchos se condensan y conservan género e insignia de mutación al abreviarse. La matriz de QA está en [HUD circular de Stamina](docs/UNHOLY_STAMINA_HUD.md) |
| P1 | Horizon MOVES solo abría el recordador; ahora permite elegir RELEARN o FORGET, con selector de Pokémon y pantalla de movimientos | Implementado en código; validar cancelación, HM y último movimiento |
| P1 | Mutaciones de iniciales, regalos, guardería e intercambios NPC; bonus relativo del 15 % para tiers C/B | PR #27 apilado, pendiente de QA |
| P1 | Clasificación lógica de bolsillos y franja negra de abreviaturas | PR #25 apilado; **captura de MEDICINE vacío** requiere prueba con Debug Fill/obtención normal, no se marca como resuelto sin comprobarlo |
| P1 | Catálogos Horizon: rediseño raíz, seis Pokémon en cuadrícula 3×2, cinco huevos en carrusel, MOVES en tarjetas y objetos/MT con marcas | PR #30, implementado en código; QA visual/CI pendiente |
| P1 | Pulido de Horizon por capturas: marcos/precios de Pokémon, etiquetas y pie de Huevos/MOVES, contraste de selección Eclipse/Daylight y tipografías de 8/12/15 px | Rama `fix/unholy-horizon-ui-alignment-colors-20261112`; código implementado, requiere CI y capturas nuevas |
| P2 | Nuevos Gears, crafteos, IA avanzada, legendarios y animaciones | Continúan pendientes de implementación/integración completa |

**Regla del encadenamiento validable:** sin cadena del jugador, jamás hay segundo ataque suyo en el mismo turno; con cadena explícita, se ejecutan los dos movimientos elegidos si quedan vivos y no se agota Stamina; la IA puede responder con una acción adicional válida, nunca repitiendo su misma selección por defecto. Probar velocidades relativas (jugador antes/después), IA con y sin cadena, KO intermedio y cambio de Pokémon.

**Matriz de QA inmediata:** comprobar el marco y las etiquetas de Horizon en Eclipse/Daylight, cantidades e iconos de compra, cancelar y volver a las pestañas; anillos de ambos bandos en individual/doble y transiciones; Horizon MOVES con Pokémon de 1, 2 y 4 ataques, cancelaciones, HMs y guardado/carga; llenar MEDICINE con Debug y con pociones normales; verificar Game Boy Advance ROM final y artefacto descargable.

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

### Tiers de mutación (10 noviembre 2026)

Clasificación unificada por **poder/acceso**, distinta de las probabilidades RNG: **C** = Alpha y Hyper; **B** = Super y Overloaded; **A** = Enraged y Ancestral; **S** = Titan y Ascendido; **Ω** = Omega. Horizon Pokémon permite únicamente C/B/A; huevos admiten C/B/A/S/Ω por ruleta. Las combinaciones conservan todas sus mutaciones, el tier principal es el más alto y los bonos siguen siendo **aditivos**. Reglas, código y pruebas en [`docs/MUTATION_TIERS.md`](docs/MUTATION_TIERS.md).

### Cambio de habilidades

- La **Ability Capsule** y el **Ability Patch** están disponibles para comprar en todos los PokéMart estándar desde el primer acceso, sin requisito de medallas.
- La Capsule alterna entre las dos habilidades normales de la especie y no tiene efecto si no existen dos habilidades normales distintas.
- El Patch alterna entre una habilidad normal y la habilidad oculta cuando la especie dispone de ella.
- Ambos objetos se aplican desde el menú del grupo, se consumen solo después de confirmar el cambio y no crean habilidades múltiples para Pokémon Super.
- Los badges Alpha, Super, Enraged y Omega no cambian estas reglas en esta fase.

### Gestión de movimientos

- **Horizon Shop > MOVES** ofrece RELEARN y FORGET con el selector del equipo. El recordador conserva sus filtros; FORGET usa el selector de movimientos original y no permite eliminar el último movimiento del Pokémon.
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
- **Ciclo**: 1 turno sobrecargado + 3 turnos residuales en Impulso, Corazón y Ski Mask; luego recomienza. Dos ataques encadenados cuentan como 1 turno. Cambiar de Pokémon no recarga el Gear gratis en el mismo combate (rama de integración).
- **Visor Cósmico:** efectividad Fantasma ×2 por cada tipo objetivo Normal, Psíquico o Hada durante todo el combate.
- **Corazón de Titán:** +20 % a ataques físicos al sobrecargarse, +10 % residual; evolución de Banda Músculo.
- **Ski Mask:** +30 % a ataques Siniestros al sobrecargarse y **+21 % constante durante recarga**. No altera efectividades.
- **Cetro Siniestro:** efectividad Siniestro ×2 por cada tipo objetivo Psíquico, Hada, Normal o Lucha. Siempre activo, sin afectar la tabla global. Contra Lucha/Hada, ×4.
- **Impulso Cósmico:** +20 % a ataques especiales al sobrecargarse y +10 % residual. Se fabrica a partir de Gafas Especiales.
- **Permafrost Fragment:** los movimientos de daño Hielo duplican la probabilidad de congelar o Frostbite. Si el portador es tipo Hielo, cura **1/16 de los PS máximos al final de cada turno**, incluso sin atacar, como Restos. La curación no es vampirismo.
- **Promedio de bonos coincidentes:** Gear +20 % y Gafas/Banda normal +10 % => **+15 %** total (no +32 %). Con fase residual +10 % y objeto +10 % => +10 %. Restos y Permafrost simultáneos de la misma magnitud no duplican curación.
- **Teseracto:** consumo único, como Gema equipada. Vuelve permanentes clima, campos, salas y trampas mientras no se eliminen explícitamente; NO se implementa sin cerrar disparador y todas las interacciones.
- **Ancla de Realidad:** elimina clima, campos, salas y trampas, **incluso los hechos permanentes por Teseracto**; lo eliminado no reaparece. Pendiente si bloquea nueva activación durante la presencia.
- **Colmillo de Rayquaza:** +10 % a estadísticas para Pokémon Volador/Dragón al derrotar a Rayquaza; todavía necesita reglas de ciclo y compatibilidad de mutaciones.
- **Alcance real en código:** seis Gears registrados con lógica de combate, recetas, acción EQUIP desde la mochila e iconos de HUD de 16×32 en la rama de integración. Las incursiones Experta y Maestra y cinco combates de jefes usan Gear y objeto ordinario simultáneamente. Teseracto y Ancla siguen en diseño.

### Familia de Gears: Inyecciones sanguíneas
- Inspiradas en las gemas de tipo, **sin reemplazarlas literalmente**.
- Proporcionan un **gran bonus al primer ataque compatible**, luego un **bonus de daño residual menor** que se mantiene hasta que se recargue la siguiente sobrecarga cada X turnos.
- La bonificación residual debe notarse, sin superar permanentemente objetos ofensivos especializados.
- **Catálogo de 18 Gears:** una Sangre por tipo. La tabla del [PR #19](https://github.com/Techkron-star/pokemon-unholy-emerald/pull/19) asigna Flamígera a Fuego, Demoníaca a Siniestro, Divina a Hada y Escamada a Dragón.
- Queda por decidir cómo aplicar la sobrecarga al primer ataque compatible si se encadenan dos en un solo turno; no duplicar la recarga ni asumir multiplicadores.

### Estado de implementación
- Los seis Gears del sistema base y sus recursos de HUD están registrados en la rama de integración. Consultar [`docs/GEARS.md`](docs/GEARS.md) para equipamiento, recetas y pruebas.
- Los equipos de entrenador admiten el campo `Gear:` por Pokémon. La primera selección incluye a Tate y Liza, Phoebe, Glacia y Wallace. En el quinto combate de Tate y Liza, Metagross combina su megapiedra con Corazón de Titán. Las Sangres siguen pendientes de obtención, balance y pruebas.
- Provisional hasta que se decida: con dos movimientos encadenados compatibles, la sobrecarga la lleva el primero y el segundo recibe el residual; el ciclo avanza una sola vez por turno.

---

## Diseño de interfaz — Horizon Nebula (9 noviembre 2026)

Se definió el sistema visual de Horizon Shop y sus menús. **La pantalla raíz con cinco pestañas ya está implementada en ramas apiladas; estas revisiones todavía necesitan QA en emulador**. Consulta [`docs/UI_HORIZON_NEBULA.md`](docs/UI_HORIZON_NEBULA.md) para layouts, navegación, paletas Eclipse/Daylight, persistencia, límites GBA, sprites personalizados y pruebas.

- Horizon Shop abre una **pantalla raíz con cinco pestañas**: Pokémon, Discos, Objetos, Huevos, Movimientos.
- La lista, el panel de descripción y los botones comparten estructura, pero cada pestaña utiliza reglas y flujos propios.
- Los temas de interfaz **Eclipse** y **Daylight** se eligen desde Opciones y permanecen guardados; no dependen del ciclo diurno de la partida.
- Reusar la composición y efectos discretos de los temas Nebulous de esos nombres, no trasladar WebGL o shaders a GBA.
- Horizon tiene seis ofertas Pokémon independientes limitadas hasta Ancestral y cinco huevos visuales de rango Básico/Medio/Alto/Mítico/Legendario; estos últimos sí pueden dar Omega. **Infinity Ball sigue excluida**. Precios y probabilidades aún sujetos a balance.
- Crafting adopta panel de receta, requisitos visibles (poseído / requerido) y confirmación antes de consumir materiales.
- Movimiento / aprender / olvidar reutiliza el flujo existente; se crea navegación uniforme, no una segunda lógica incompatible.

**Estado:** pantalla raíz y catálogos implementados en C; editor avanzado de estilos, efectos y validación visual siguen pendientes.

---

## 5. Horizon Shop (primera entrega implementada; catálogo avanzado pendiente)

Sistema de compras y servicios accesible desde el menú principal del juego. Permite preparar y ampliar el equipo sin depender exclusivamente de los encuentros y tiendas físicas.

Horizon Shop reutiliza las compras del PokéMart para objetos y discos, con navegación horizontal propia; también integra seis ofertas Pokémon persistentes y cinco rangos de huevos reales con calidad independiente por RNG. Los cambios viven en PR apilados, con pruebas de emulador pendientes.

### Catálogo y progresión
- La oferta de **Pokémon y objetos** se selecciona de forma **aleatoria (RNG)** y cambia en función del **ciclo de día y noche** del juego.
- **Las medallas** determinan qué Pokémon y mercancías pueden aparecer en el catálogo; el contenido disponible progresa junto con la aventura.
- El catálogo Pokémon realiza seis tiradas de mutación independientes pero no ofrece nada superior a **Ancestral**; excluye expresamente Titan, Ascendido y Omega. Cada plaza se compra una vez. El catálogo de huevos usa su propio RNG de calidades y premios, incluyendo Omega, legendarios y hasta tres mutaciones simultáneas.

### Secciones del mercado
| Sección | Contenido / servicio |
| --- | --- |
| **Pokémon** | Seis ofertas simultáneas, una tirada RNG y compra por plaza. Hasta Ancestral; nunca Titan, Ascendido ni Omega. |
| **Objetos** | Tres suministros permanentes y cinco ofertas rotativas según medallas y día/noche, con stock guardado por semilla, sin reroll por abrir/cerrar. |
| **Discos** | Seis MT aleatorias distintas y desbloqueadas por medallas y día/noche; selección persistente por período. |
| **Huevos** | Cinco huevos visibles (Básico, Medio, Alto, Mítico, Legendario); cada uno tira su propia calidad visual Estándar/Radiante/Prismática. Tras comprar se sortea una especie oculta y hasta tres mutaciones; incluso legendario Omega. |
| **Movimientos** | Servicio para **aprender y borrar movimientos** de tus Pokémon desde el mismo menú. |

### Avisos de ofertas mutantes (10/11/2026)

Una oferta mutante nueva marca **WATCH** en START, **HORIZON** dentro del reloj y la pestaña Pokémon de Horizon. El panel muestra **!!! RARE STOCK**. Abrir la sección Pokémon limpia el aviso hasta la siguiente rotación que tenga una oferta mutante.

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
| **Cuarta medalla** | Repartir EXP Sobrecargado | Se fabrica en Crafteo con el Súper Repartir EXP y una Pepita. Se activa al fabricarlo y se puede apagar desde Objetos Clave. |
| **Quinta medalla, al recibir el HF** | Poké Simulator | Ajuste de IV y EV por estadística con vista previa y pago al confirmar. |
| **Octava medalla** | Acceso limitado al Mutator | Máquina de mutaciones genéticas aleatorias. |
| **Segunda medalla** | **Pokélocator** | Acceso desde el Super Poké Watch. Muestra los Pokémon de la ruta actual. La Señal mutante se abre con el HF. |

### Pokélocator
- El Super Poké Watch de Brawly habilita el acceso por historia. El objeto dedicado permanece disponible en Debug.
- Funciona como el **Pokénav de Pokémon Elite Redux**: se abre desde el menú, muestra la lista de especies de la ruta y su información.
- **Por medallas:** cada medalla desbloquea más información (especies, tasas de encuentro, probabilidad de mutación).
- **Forzar mutación:** el jugador puede activar un modificador de probabilidad de mutación sobre los salvajes de la ruta. Propuesta de balance (no confirmada): consumo de un recurso por ruta o por encuentro, y topes por medalla.
- Pendiente: qué recurso consume, si el efecto dura una sesión de encuentros o un tiempo fijo, y si las mutaciones forzadas respetan la regla de Omega (sin Omega en el Mutator).

#### Búsqueda y captura

El catálogo agrupa especies, combina categorías de encuentro con filtros de colección y cuenta progreso sin duplicar métodos. La ficha ofrece Habitat, Stats y Traits. **A en la ficha → revisar búsqueda → confirmar** inicia un combate salvaje con la especie y método elegidos en el mapa actual; permite capturar desde la Mochila. Tools integra la Señal mutante existente con requisitos, tiradas base, contador y confirmación. Revalida horario y requisitos de Surf/caña/Golpe Roca. Los detalles y la QA están en [Pokélocator: búsqueda y captura](docs/POKELOCATOR_SEARCH.md). Consultar y cancelar siguen siendo de solo lectura; solo confirmar Buscar consume las tiradas normales del encuentro.

#### Implementación v1 (rama `feat/unholy-pokelocator-v1`)

**Qué hace.** `ITEM_POKELOCATOR` (ID 883, ítem clave, `ITEM_USE_FIELD`) se usa desde la Mochila o registrado en SELECT, igual que la Buscaobjetos. Abre una ventana BG0 nativa (26×17 tiles) con las especies de la zona actual: icono, nombre, método, nivel, probabilidad y estado Pokédex (CAPTURADO / VISTO / SIN VER). Controles: ↑↓ cursor (con repetición y vuelta), **A** detalle de la especie (todos sus métodos), **B** atrás/salir, **L/R** filtro por método (solo métodos que existen en la zona), **←→** cambiar de mapa dentro de la misma sección del mapa regional (pisos de una cueva, etc.).

**De dónde salen los datos (sin catálogos duplicados).**
- Lee `gWildMonHeaders` in situ, con la tabla del horario activo (`GetTimeOfDayForEncounters`), igual que el motor de encuentros. Por eso el horario solo aparece reflejado porque el motor ya lo soporta.
- Las probabilidades salen de los mismos `ENCOUNTER_CHANCE_*` que usan `ChooseWildMonIndex_*`, expuestos como accesores de solo lectura en `wild_encounter.c` (`GetWildEncounterSlotCount/Weight/GroupTotal`, con `STATIC_ASSERT` de tamaños). Los slots de la misma especie y método se fusionan (suman peso, unen rango de niveles). Cada caña (Vieja, Buena, Súper) se calcula contra su propio total. Los encuentros ocultos no tienen probabilidad definida: se muestra `--`.
- **Clima, swarm/brote y Feebas no se muestran** porque no forman parte de las tablas leídas; el Pokélocator no los simula.
- Información secreta: los encuentros ocultos (DexNav) solo se listan si la especie ya fue vista (`Pokelocator_IsSpeciesRevealed`).

**Arquitectura.** `src/pokelocator_data.c` (lectura, filtros, cursor, zona; sin hardware ni RNG, probable de forma aislada) y `src/pokelocator.c` (pantalla). Se renderiza solo cuando cambia una entrada, no cada frame. Como máximo 3 sprites de icono; las paletas de iconos cargadas por la pantalla se liberan al salir, y la paleta 14 del tema se guarda/restaura. Estilo Eclipse/Daylight con `FLAG_UNHOLY_UI_DAYLIGHT` y los helpers compartidos de `shop.c`. Capacidad: 124 mapas reales verificados, máximo 19 filas por mapa (límite 40, con `STATIC_ASSERT`).

**Por qué un ítem clave y no una opción del menú de inicio.** El menú de inicio ya llega a 10-11 entradas y su ventana mide `2·n+2` tiles (con 9 o más ya supera los 20 tiles de pantalla), así que una entrada más lo empeoraría. El ítem clave sigue el diseño ("Ítem clave del menú") y no elimina ni reemplaza nada.

**Mutaciones: preparado y apagado.** `POKELOCATOR_MUTATION_MODIFIER_ENABLED` es `FALSE` (con `#error` si se activa sin proveedor). `Pokelocator_GetModifierStatus()` devuelve `UNDEFINED` y la pantalla no muestra nada. No hay costes, duración, probabilidades ni desbloqueos inventados, y consultar la pantalla no toca el RNG de encuentros ni el estado de guardado. La acción Buscar crea un encuentro normal y puede consumir una carga de Frecuencia ya activa, como los demás encuentros salvajes.

**Pendiente real (decisiones de diseño, no de código).**
- **Entrega por historia:** la escena "el padre entrega el Pokélocator tras la captura guiada de Ralts" no existe en los scripts. Queda `EventScript_GivePokelocator` (idempotente, en `data/scripts/pokelocator.inc`) listo para llamarlo desde esa escena. Hasta entonces **no se consigue en una partida normal**; los scripts de debug sí lo entregan. Partidas existentes: mismo caso.
- **Desbloqueo por medallas:** el diseño dice que cada medalla desbloquea más datos pero no fija qué dato con cuál. V1 muestra especies y probabilidades sin restricción por medalla; hay que definir los umbrales antes de aplicar filtros de progresión.
- **Modificador de mutaciones:** definir recurso, duración, topes y regla Omega; después añadir un proveedor de solo lectura en `sModifierProviders[]`.
- **Zonas por región:** la agrupación usa la sección del mapa regional; mapas con varias secciones no se agrupan.
- FRLG: el ítem y la pantalla son compartidos; la entrega por script solo existe en debug.

**Pruebas.** `test/pokelocator.c` (15 pruebas): suma de pesos = total por método y por caña, slots inválidos/ocultos, fusión de especies, chance en décimas, métodos de pesca, regla de revelado, filtros, ciclo L/R, cursor y listas vacías, modificador indefinido. No se tocó ningún test ni expectativa de la suite de los 112 fallos.

**QA manual en mGBA (no verificado por mí).** Esta sesión no tenía toolchain ARM ni emulador: la compilación Emerald/FRLG, `make check` y la pantalla real dependen del CI y de pruebas manuales. Revisar: abrir desde la Mochila y desde SELECT; ruta con hierba, Surf, pesca y cueva; filtros L/R; A/B; ←→ en zona multi-mapa; Eclipse y Daylight; mapa sin encuentros; que no queden iconos ni ventanas residuales al salir; que el mapa recupere sus colores; nombres largos de especie; 3 filas sin texto superpuesto; Pokédex con especies vistas/capturadas/ocultas.

- **Repartir EXP Sobrecargado, balance provisional.** Cada Pokémon elegible recibe la EXP íntegra una vez, con un 25 % adicional sujeto al límite de nivel. Duplica los EV de combate, incluidos los modificadores de objetos y Pokérus, con los topes habituales.
- Cada nivel que se gana en combate sube un punto el IV más bajo que todavía pueda subir. En empate se usa el orden PS, Ataque, Defensa, Velocidad, Ataque Especial y Defensa Especial. Se respetan los IV de 31, el entrenamiento extremo y las mutaciones Ascendido y Omega.
- Cada 256 puntos de EXP base generados por un rival adelantan un ciclo de cada huevo del equipo. El cálculo cuenta una vez por rival y conserva el sobrante mientras queden huevos por incubar. Los huevos listos eclosionan en los siguientes pasos seguros del campo.
- Ajustar IVs/EVs en el simulador **no convierte** a un Pokémon normal en Omega.

---

### HFC Watch

Dispositivo de muñeca en tres niveles. El HF Poke Watch es el completo; los otros son el mismo dispositivo con funciones recortadas (tabla `sFeatures` en `src/hfc_watch_data.c`, un `minTier` por función).

| Reloj | Entrega | Funciones |
|---|---|---|
| Poké Watch | Norman tras la captura guiada de Wally | Horizon Shop y PokéNav básico. Crafteo tras ganar a Roxanne. |
| Super Poké Watch | Brawly (Marcial) tras la segunda medalla | Añade Pokélocator y las cajas del PC. |
| HF Poké Watch | Norman tras la quinta medalla | Añade Archivo de anomalías, Señal mutante, Poké Simulator e incursiones. El Mutator aparece con la octava medalla. |

- **WATCH** en START reúne las funciones disponibles. Horizon Shop se abre desde WATCH a partir del primer Poké Watch. Crafteo incluye recetas y equipamiento de Gears. Las partidas que aún carezcan de reloj pueden entrar a Crafteo desde START. Las cajas usan la pantalla de organización del PC.
- Los relojes se pueden usar desde la mochila o registrar en SELECT. Cada mejora sustituye el reloj anterior y conserva el registro si lo tenía. Las partidas anteriores pueden recoger la mejora hablando de nuevo con Norman o Brawly tras el hito correspondiente.
- El PokéNav básico ofrece mapa y condición. Match Call y cintas siguen su progresión habitual. El Pokélocator también conserva su objeto dedicado.

**Incursiones y señal, balance inicial.**

- Una incursión por día, desde una ruta al aire libre. Tiene tres rondas: dos combates dobles y un jefe individual. Cada doble requiere dos Pokémon capaces de luchar. Se conservan PS y PP entre rondas. El intento se consume al iniciar.
- Cada dificultad tiene 20 jefes propios, para un total de 80 especies distintas con movimientos y equipo definidos. La rotación diaria de cada entrenador recorre los 20 jefes de su dificultad antes de repetir. Los dos dobles eligen cuatro especies diferentes de ese grupo y excluyen al jefe del día.
- El jefe tiene una mutación aleatoria de la tabla de dificultad, con tiers C–S y exclusión de Omega. Recibe al entrar Ataque, Defensa, Ataque Especial y Defensa Especial +1 en Estándar y Avanzada, y +2 en Experta y Maestra. Experta y Maestra también dan +1 de Velocidad.
- Al ganar se elige el lote de tres tipos de objetos o un intento de capturar al jefe. Una tirada fallida consume esa elección. La probabilidad varía cada día por dificultad, se muestra con dos decimales y se fija para las tres rondas. Si falta espacio, se puede liberar espacio y volver al panel durante la misma sesión.

| Dificultad | Equipo en dobles | Equipo del jefe |
|---|---|---|
| Estándar | Movimientos del catálogo | Aura y mutación |
| Avanzada | 50 % de llevar un objeto por rival | Objeto equipado |
| Experta | Objeto por rival, Gear al 50 % y tirada de Mega al 30 % por combate | Objeto y Gear, con tirada de Mega al 55 % |
| Maestra | Objeto por rival, Gear al 80 % y tirada de Mega al 65 % por combate | Objeto y Gear, con Mega cuando la especie es compatible |

Cada combate admite una megaevolución rival. La megapiedra ocupa la ranura de objeto y el Gear conserva su propia ranura. El jefe capturado llega en su forma base y conserva la mutación. El equipo de incursión pertenece al combate.

| Dificultad | Medallas | Niveles por ronda | Probabilidad de captura | Lote |
|---|---:|---|---:|---|
| Estándar | 5 | 34, 36, 40 | 20.00–35.00 % | 3 Super Balls, 2 Hiperpociones, 1 Caramelo Raro |
| Avanzada | 6 | 40, 42, 46 | 35.01–50.00 % | 3 Ultra Balls, 2 Pociones Máximas, 1 Caramelo Raro |
| Experta | 7 | 47, 49, 53 | 50.01–65.00 % | 4 Ultra Balls, 2 Restaurar Todo, 2 Caramelos Raros |
| Maestra | 8 | 54, 56, 60 | 65.01–80.00 % | 5 Ultra Balls, 3 Restaurar Todo, 3 Caramelos Raros |

- La señal se activa una vez al día con el HF. En la progresión actual, aumenta entre 35 % y 50 % **relativo** la probabilidad de mutaciones de tier C y B en los próximos diez Pokémon de encuentros normales de esa zona. Conserva los usos al guardar y cambia de día con el ciclo del juego. Mantiene las probabilidades originales de A, S y Omega.
- El archivo enumera las nueve mutaciones y su tier. Registra ejemplares al capturar, recibir, eclosionar o intercambiar y, al abrirlo, incorpora los que ya están en el equipo o en las cajas. El registro queda tras liberar el ejemplar.

**Poké Simulator.** WATCH permite elegir un Pokémon del equipo, preparar cambios de IV y EV y previsualizar sus estadísticas. La tarifa provisional cobra ₽300 por punto de IV y ₽20 por punto de EV cambiado al confirmar. Aplica los límites habituales del motor. Los huevos, Ascendidos y Omega tienen su edición bloqueada.

**Pendientes.** Incursiones multijugador o con aliados, recompensas adicionales e iconos propios del reloj. [Diseño previo del HFC Watch](https://github.com/Techkron-star/pokemon-unholy-emerald/blob/107a7f252be1ec9d6f705019c1398bd4f827a202/docs/HFC_WATCH_MUTATIONS_V2.md).

## 8. Mutator

El Mutator abre desde WATCH con el HF y ocho medallas. Se elige un Pokémon del equipo y se revisan el costo, la probabilidad y el nivel resultante antes de pagar. Los huevos y los Pokémon con Omega quedan excluidos.

| Plan provisional | Costo | Probabilidad de éxito | Pérdida de nivel en caso de éxito |
|---|---:|---:|---:|
| Básico | ₽12 000 | 35 % | 5 % |
| Reforzado | ₽30 000 | 60 % | 10 % |
| Intensivo | ₽60 000 | 85 % | 15 % |

- La pérdida se redondea hacia arriba, con un máximo de 15 niveles y un mínimo de nivel 1. Un fallo consume la tarifa y conserva al Pokémon. Cancelar evita el cobro y la tirada.
- En caso de éxito, la distribución provisional para un Pokémon sin mutaciones es Alpha 25 %, Hyper 20 %, Super 15 %, Overloaded 12 %, Enraged 11 %, Ancestral 9 %, Titan 5 % y Ascendido 3 %. El resultado queda registrado en el Archivo de anomalías.
- Con la octava medalla se puede mutar a Pokémon sin mutaciones previas. El reroll requiere el indicador persistente `FLAG_DEFEATED_SUPER_LEAGUE`, que activará el evento de victoria de la futura Super Liga. El reroll sustituye una mutación del tier más alto y conserva las demás. Los empates siguen el orden de la lista anterior. La tirada excluye las mutaciones que el Pokémon ya tiene. Al sustituir Ascendido, los EV se reducen proporcionalmente al límite de 510.

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
- PC avanzado: favoritos con protección frente a liberación y búsqueda combinada por especie seleccionada, tipo, mutación, favorito, shiny y Gear implementados; ver [controles y QA pendiente](docs/PC_QOL.md). El traslado múltiple y el cambio rápido de caja ya existen en la base. Falta validar navegación, guardado/carga y aspecto en emulador/hardware.
- Aprender/olvidar movimientos desde **Horizon MOVES**: código de ambas acciones integrado; todavía deben validarse cancelación, restricciones y guardado.
- Pokédex+ con hábitats, horarios, evoluciones, movimientos y variantes. La interfaz HGSS existente ya está activada; quedan por integrar explícitamente las variantes Alpha/Super/Enraged/Omega y validar textos/pantallas en hardware.
- Guardado fiable, versionado/migraciones y pruebas de persistencia.
- Validación visual de la barra de stamina, encadenamiento y textos en emulador/hardware.

### P1 — Progresión principal

- Repartir EXP básico, Súper y Sobrecargado integrados. Pendiente el balance económico y la prueba de progresión completa.
- Poké Simulator integrado en WATCH con previsualización de IV y EV, límites del motor y tarifa provisional.
- HUD de batalla para mutaciones: las cajas de vida muestran la inicial de la mutación principal (`A`, `S`, `E` u `O`); el resumen conserva la lista completa.
- Horizon MOVES: selector RELEARN/FORGET y protección del último ataque en código; filtros avanzados y QA pendiente. El acceso contextual `LEARN` antiguo se retiró del menú del Pokémon.
- Animación residual del profesor al saltar la introducción: pendiente de limpiar el estado visual del objeto tras el warp inicial.
- Horizon Shop con catálogo persistente, medallas, día/noche, compras múltiples y economía equilibrada.
- Crianza y huevos: progreso por pasos/EXP, recogida múltiple, almacenamiento y compatibilidad.
- Gears: seis piezas fabricables. El equipo y el resumen muestran el Gear equipado. El HUD de combate muestra su fase, destella al activarse o recargarse y cuenta los turnos restantes (`docs/GEARS.md`). Pendientes las Inyecciones sanguíneas y ampliar el equipamiento a más entrenadores.
- Información ampliada de estados, campos, habilidades y registro de combate.

### P2 — Contenido avanzado

- Conectar el desbloqueo del reroll del Mutator al evento de victoria de la Super Liga.
- Presets competitivos y preparación pre-jefe.
- Super Liga, entrenadores con Omegas y recompensa Infinity Ball.
- Estadísticas de combate, revancha de entrenadores y herramientas de búsqueda especializadas.

### P3 — Mejoras opcionales

- Personalización estética, Pokémon acompañantes, estadísticas históricas y otras mejoras que no bloquean la versión jugable.

El orden de entrega y los criterios de cierre se definen en el [plan de producción por tandas](#plan-de-producción-por-tandas).
