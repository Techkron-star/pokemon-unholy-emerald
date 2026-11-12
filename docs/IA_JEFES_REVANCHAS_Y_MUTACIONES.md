# Unholy Emerald · IA justa, jefes tácticos y revanchas dinámicas

**Estado:** especificación + primera entrega parcial, 9-11-2026. No confundir el diseño pendiente con funcionalidades compiladas.  
**Rama:** `feat/unholy-trainer-ai-foundation` (basada en `feat/chain-ko-qa-20261109`).  
**Documento complementario:** `docs/MUTACIONES_ADITIVAS_Y_ENTRENADORES_PERSISTENTES.md` en `docs/unholy-mutation-stacking-persistence`.

## 1. Intención

Los entrenadores de ruta deben ofrecer una resistencia razonable, pero los rivales recurrentes, líderes, Alto Mando, campeón y jefes deben obligar a evaluar cambios, coberturas, objetos, stamina y encadenamientos. No se trata de construir equipos competitivos de Smogon ni de dar información secreta al motor.

**Principios de justicia:** nadie recibe acciones ocultas, PP infinito, precisión artificial, lectura exacta de los movimientos no revelados del jugador, objetivos que no existen ni acceso a mecánicas prohibidas para el jugador. El rival usa las mismas reglas de stamina y las mismas restricciones de cadena.

## 2. Niveles de IA

| Categoría | Comportamiento |
| --- | --- |
| Entrenadores comunes | IA básica completa: descarta ataques inútiles, valora efectividad y oportunidad de KO; no predice de forma sobrenatural |
| Líderes de gimnasio y rivales especiales | Evalúan 2HKO, PS, cuándo cambiar y cuándo preservar su Pokémon estrella; sin omnisciencia |
| Alto Mando y campeón | Estrategias explícitas de equipo, cambios inteligentes, control de campo y cierre de combate; siguen sin ver datos privados |
| Jefes de historia | Distintas estrategias según identidad, recursos limitados y fases argumentales; pueden aprovechar megas al desbloquearse |

**Primera entrega programada en esta rama:** se sustituye `AI: Check Bad Move` por `AI: Basic Trainer` para 637 entrenadores comunes; 40 encuentros de líderes, cuatro del Alto Mando, uno de campeón y 17 rivales/jefes nombrados pasan a `Basic Trainer / Try To 2HKO / HP Aware / Smart Switching / PP Stall Prevention / Ace Pokemon`. El perfil deliberadamente **no incluye `Omniscient`**, ni previsión perfecta. Debe validarse en ROM antes de fusionarlo.

**Trabajo posterior de IA:** añadir criterios conscientes del gasto de stamina, segunda acción encadenada, vulnerabilidad tras el KO, objetivos de cambio, desgaste por clima personalizado y recursos de Mega; comprobar que las nuevas prioridades no hacen que la IA se quede inmóvil o cambie eternamente.

**Primer ajuste de Stamina en integración:** si varias opciones para la primera acción de un entrenador tienen la misma puntuación, elige entre las que dejan Stamina disponible. Si todas las opciones empatadas agotan al Pokémon, conserva la selección habitual. Los movimientos con una puntuación superior mantienen su prioridad. En dobles el desempate se aplica a los movimientos para un mismo objetivo. La estimación usa el coste del motor y la Stamina del propio combatiente. La segunda acción de una cadena conserva sus reglas y puede agotar al usuario.

## 3. Equipos de gimnasio: uno o más Pokémon extraordinarios

**Regla solicitada:** cada uno de los ocho líderes tiene **al menos un legendario o miembro de una familia seudolegendaria**, ajustado al nivel del resto del equipo y con ataques jugables en ese momento. No regalarles estadísticas máximas, mutaciones obligatorias o megapiedras antes de que el jugador tenga herramientas comparables. Se priorizan sinergias temáticas y debilidades deliberadas.

### Pauta propuesta para diseñar equipos (requiere validación)

| Gimnasio | Ancla extraordinaria | Concepto táctico |
| --- | --- | --- |
| Roxanne | **Pupitar** (implementado, nivel 13, sustituye un Geodude) | Roca/Arena; vulnerable a Agua/Planta, conserva Nosepass como as |
| Brawly | **Hakamo-o** (primer pase implementado) | Lucha/Dragón con Bulk Up y contraataque controlado |
| Wattson | **Dragonair** (primer pase implementado) | Thunder Wave y cobertura eléctrica; conserva Manectric como as |
| Flannery | **Shelgon** (primer pase implementado) | Dragón resiste Agua; golpes de Fuego limitados y apoyo de sol |
| Norman | **Type: Null** (primer pase implementado; legendario de tipo Normal) | Potencia física contenida, intercambio de tempo; Slaking sigue siendo su as |
| Winona | **Salamence** (primer pase implementado, set contenido) | Volador/Dragón, presión aérea con ventanas reales de respuesta |
| Tate y Liza | **Metang** (primer pase implementado) | Psíquico/Acero y cooperación en doble combate |
| Juan | **Sliggoo** (primer pase implementado) | Dragón con apoyo de lluvia; Kingdra sigue siendo su as |

**Primer pase implementado en esta rama para los ocho gimnasios**: se ha sustituido un miembro por el ejemplar de la tabla y se han asignado sets de cuatro movimientos simples y niveles próximos al resto. **Siguen pendientes la compilación, las pruebas reales en emulador y el balance fino**; no se ha aplicado a sus revanchas ni se han añadido Megas por defecto. Los ases originales siguen presentes.

## 4. Megapiedras de entrenadores

- **Implementado en datos:** el quinto equipo de revancha de cada uno de los ocho gimnasios tiene una megapiedra asignada a un integrante compatible. Roxanne–Aerodactyl, Brawly–Medicham, Wattson–Manectric, Flannery–Camerupt, Norman–Kangaskhan, Winona–Altaria, Tate y Liza–Metagross, Juan–Slowbro. Estos dos últimos sustituyen un miembro de sus equipos finales para disponer de una Mega.
- Los entrenadores importantes pueden equipar megapiedras cuando la historia y el acceso del jugador justifiquen que aparezcan.
- Una megaevolución como máximo por bando y combate, usando el sistema base de `pokeemerald-expansion`.
- La IA debe elegir una oportunidad razonable para megaevolucionar, no desperdiciarla con un Pokémon que no corresponda ni usarla después de un KO.
- Respetar la existencia del objeto en la especie correspondiente y confirmar que la función está habilitada en esa modalidad.
- **Pendiente de QA:** el ítem equipado no prueba por sí mismo que la IA active la Mega en combate. Verificar al menos una Mega por líder en sus revanchas finales y corregir las condiciones de activación si hiciera falta.

## 5. Revanchas escaladas por medallas

**Implementado en la generación de equipos, pendiente de QA:** al desafiar de nuevo a un líder, **todo su equipo** se ajusta al **nivel máximo de la última medalla que tiene el jugador** (no al nivel del Pokémon más fuerte del jugador). `src/battle_setup.c` detecta `isRematch` en líderes y ajusta la experiencia, recalcula estadísticas y recupera PS completos a ese cap. Los encuentros iniciales permanecen a nivel original y la identidad mutante se calcula aparte. **Todavía falta probarlo con revanchas reales y confirmar su disponibilidad antes de la Liga.**

Referencia para un primer pase, basada en los ases actuales de los ocho gimnasios; es una **tabla de diseño por validar**, no un límite global del juego:

| Última medalla obtenida | Nivel de referencia |
| --- | ---: |
| 1.ª — Piedra | 15 |
| 2.ª — Puño | 19 |
| 3.ª — Dinamo | 24 |
| 4.ª — Calor | 29 |
| 5.ª — Equilibrio | 31 |
| 6.ª — Pluma | 33 |
| 7.ª — Mente | 42 |
| 8.ª — Lluvia | 46 |

Al ganar una medalla nueva, la siguiente revancha se actualiza al nuevo tramo. **No modificar la primera batalla** con cada líder. Las revanchas deben estar disponibles por una vía explícita, sin obligar al jugador a completar la Liga si el diseño permite solicitarla antes; el código actual de `src/gym_leader_rematch.c` todavía contiene una puerta por `FLAG_SYS_GAME_CLEAR`.

### Mutaciones en revanchas: dos veces más probables

- Si el Pokémon del líder no tiene mutación, la probabilidad de obtener una en la revancha es **2× la probabilidad base que le corresponda**, sin superar el 100 %.
- Si tiene exactamente una mutación, la oportunidad de conseguir una **segunda** en la revancha también se duplica. Esta bonificación es sobre la oportunidad adicional, **no una garantía**.
- Si ya tiene dos o más, no inventar una bonificación multiplicativa que vuelva triviales las triples; se aplicarán las puertas de rareza correspondientes.
- **Persistencia:** una mutación antigua **nunca se pierde ni se vuelve a sortear** al repetir la pelea, evolucionar o guardar/cargar.
- **Anti-farmeo (propuesta):** evaluar una nueva tirada de mutación por Pokémon **solo cuando el entrenador recibe un nuevo tramo de revancha** (por ejemplo, tras adquirir una nueva medalla). Repetir la misma revancha en el mismo tramo no permite tirar sin límite.
- La ruleta de mutaciones de entrenadores normales sigue prohibiendo Omega; rivales, gimnasios y jefes sí pueden ser Omega. Los porcentajes Omega de jefes todavía necesitan su propia tabla.

### Identidad estable por individuo

Usar `saveSeed + stableTrainerId + stablePokemonLineageId + rematchTier` para decidir la **oportunidad nueva**, conservar en el guardado lo obtenido, y mantener los mismos modificadores concretos de Omega/Titan/Ancestral. La identidad del Pokémon sobrevive a nuevas posiciones, evolución y reordenamientos de equipo.

**Implementación inicial de persistencia (cuatro badges actuales):** `src/trainer_util.c` reemplaza `Pokemon_AssignEncounterBadges(mon, TRUE)` para Pokémon de entrenadores por una ruleta reproducible derivada del OTID de la partida + identidad de entrenador (nombre y clase) + especie base de la línea evolutiva + ordinal de duplicados de esa línea. Las revanchas de gimnasio usan el número de medallas como tramo estable: las tiradas anteriores se conservan matemáticamente y el nuevo tramo puede añadir mutaciones con las oportunidades 2× acordadas. Repetir el mismo tramo no vuelve a sortear nada; la generación NO consume el RNG global. **No se añaden campos al guardado.**

**Limitaciones explícitas de esta fase:** solo soporta Alpha, Super, Enraged y Omega (4 bits). Las probabilidades numéricas siguen siendo las configuradas actualmente; todavía hay que migrar la tabla de nueve mutaciones. Si el Pokémon de un equipo se sustituye por otra familia evolutiva en una revancha, se trata como otra identidad; si su especie evoluciona dentro de la misma familia, conserva su conjunto. Un miembro duplicado de la misma familia se distingue por su ordinal dentro de esa familia. Las nuevas posibilidades de revancha se derivan de los tramos de medallas, incluso si no se combatió en todos los tramos anteriores. Una política estricta de «solo una oportunidad cuando se solicita una revancha» requerirá un registro adicional en guardado.

## 6. HUD de varias mutaciones

Mostrar la **insignia principal como prefijo del nombre** (Omega: pastilla roja con `Ω`) y colocar un contador en **la esquina superior de esa insignia**:

| Cantidad real | Visualización |
| ---: | --- |
| 0 | Solo el nombre, sin insignia |
| 1 | `[Ω] Nombre` |
| 2 | `[Ω⁺¹] Nombre` (en juego: pequeño `+1` sobre la esquina) |
| 3 | `[Ω⁺²] Nombre` |
| 4 | `[Ω⁺³] Nombre` |

El contador significa **mutaciones adicionales**, no bonificación de estadísticas, stacks temporales ni nivel de insignia. La insignia principal puede escogerse mediante prioridad visual estable (por ejemplo Omega > Ascendido > Titan > el resto), **sin borrar la mutación original ni su orden persistente**. El resumen muestra **todas** las mutaciones y sus efectos reales.

**Limitaciones actuales:** el battle HUD solo muestra un carácter como sufijo y el formato de datos tiene cuatro bits; implementarlo bien obliga a actualizar datos de Pokémon, fuente pequeña/sprite del contador, anchura del nombre y GBA OAM/VRAM, con pruebas para nombres largos y dobles batallas. No fingir que el gráfico existe aún.

## 7. Pruebas obligatorias antes de fusionar

- [ ] Compilación Emerald y tests pertinentes.
- [ ] Primer gimnasio con Pupitar del nivel acordado, ataques legales y vulnerabilidades explotables.
- [ ] Normal vs líder: diferencia de criterio perceptible sin información privada.
- [ ] Rivales conservan cadenas y stamina sin bucles de cambios, acciones dobles infinitas o ataques a la nada.
- [ ] Megapiedra equipada por jefe avanzado: una sola Mega y solo cuando las reglas lo permiten.
- [ ] Revanchas tras 1, 3, 5 y 8 medallas: equipos completos al cap de medalla; sin reescalar el primer encuentro.
- [ ] Revancha con Pokémon sin mutación: 2× oportunidad base de una; con una mutación: 2× oportunidad de segunda.
- [ ] Repetir revancha y cargar guardado no cambia una mutación concedida.
- [ ] Rival con evolución mantiene su historial de mutaciones.
- [ ] HUD muestra `+1/+2/+3` para dos/tres/cuatro mutaciones sin solapar nombre, sexo, nivel o PS.
- [ ] En combates dobles, Megas, cambios e intimidación no producen regressiones.

## 8. Estado y secuencia recomendada

**Hecho en esta rama (sujeto a QA):** refuerzo de perfiles AI para entrenadores y jefes; un legendario o seudolegendario en los ocho primeros equipos de gimnasio (Pupitar, Hakamo-o, Dragonair, Shelgon, Type: Null, Salamence, Metang, Sliggoo) y una megapiedra adecuada en el quinto equipo de revancha de cada líder.

**Pendiente:** probar y balancear equipos, confirmar activación real de Megas, QA de escalado dinámico por medallas, validación de rareza 2× determinista, ampliar a nueve mutaciones, persistencia estricta por oportunidad de revancha y sprite de insignia con contador.

No fusionar automáticamente a main hasta que CI y pruebas de emulador validen este primer paquete; las especificaciones pendientes no autorizan implementar sistemas ficticios.

**Fuentes del repositorio:** `src/data/trainers.party`, `src/trainer_util.c`, `src/pokemon.c`, `src/battle_setup.c`, `src/gym_leader_rematch.c`, `src/battle_ai_main.c`, `src/battle_interface.c`, `docs/tutorials/ai_flags.md`.
