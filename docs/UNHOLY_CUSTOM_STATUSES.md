# Estados personalizados de Unholy

Implementación sobre la rama de bounty, sort simple y Alpha +20% (PR #98).

## Reglas y balance inicial

El diseño anterior fijaba los nombres de los nueve estados y las reglas centrales de Encadenado, Desterrado y las afinidades. Rooted, Ingravidez, Confinado, Cegado y Aterrado todavía no tenían efectos completos confirmados. Sus cifras, las duraciones de afinidad y los multiplicadores de STAB siguientes son **balance inicial de implementación**, ajustable tras jugar; no se presentan como decisiones anteriores del usuario.

| Estado | Efecto | Duración inicial |
| --- | --- | --- |
| Enraizado / Rooted | Fija al suelo e impide retirada voluntaria conforme a las excepciones normales del motor. Permite ataques Tierra contra Voladores o Levitación. Cancela Ingravidez. No cura como Arraigo. | Resto del turno y 3 turnos completos siguientes |
| Ingravidez | Levita y evita Tierra. Gravedad, Bola Férrea, Derribo y Arraigo conservan prioridad de anclaje; no se puede aplicar mientras esté Enraizado o bajo Gravedad. | Resto del turno y 3 turnos completos siguientes |
| Confinado | Restringe retirada voluntaria sin causar daño. Conserva las excepciones normales de Fantasma, Muda Concha y movimientos de pivote; no bloquea ataques ni cadenas. | Resto del turno y 2 turnos completos siguientes |
| Cegado | Precisión final ×0.75. No modifica movimientos que nunca fallan. Vista Lince y Ojo Mental lo impiden. | Resto del turno y 3 turnos completos siguientes |
| Aterrado | Ataque y Ataque Especial efectivos ×0.80; no cambia etapas, no causa retroceso ni impide actuar. Bélico es inmune. | Resto del turno y 2 turnos completos siguientes |
| Encadenado | Cancela la segunda acción de la cadena antes de gastar sus PP/stamina. Conserva el ataque individual, multigolpes y réplicas internas. Usa el mismo estado que Grillete. | Resto del turno y siguiente turno completo |
| Desterrado | Ver sus reglas específicas abajo. | 1–5 turnos, incluido el de aplicación, elegidos al aplicarlo |
| Santificado | Favorece el STAB de afinidad positiva y perjudica el de afinidad negativa. | Resto del turno y 4 turnos completos siguientes |
| Demonificado | Invierte las afinidades de Santificado. | Resto del turno y 4 turnos completos siguientes |

Son efectos volátiles independientes de quemadura, veneno, parálisis y otros estados principales. Pueden coexistir. Santificado y Demonificado se sustituyen mutuamente. Los efectos se limpian al cambiar o caer; no se transfieren con Relevo. El contador de Desterrado es la excepción: permanece con el individuo en el banquillo hasta que termine.

No se renuevan acumulando el mismo estado sobre sí mismo. Los contadores avanzan una vez por turno completo, no por golpe, réplica ni segundo movimiento encadenado.

## Afinidades y STAB

| Afinidad | Tipos |
| --- | --- |
| Positiva | Psíquico, Hada, Normal, Lucha |
| Negativa | Veneno, Fantasma, Dragón, Fuego |
| Sin clasificar | Todos los demás, incluido Siniestro |

Se considera el **tipo efectivo del movimiento y su STAB**, no una suma de las afinidades de los dos tipos del Pokémon. Si no había STAB, el estado no inventa uno. Los tipos sin clasificar conservan el modificador original.

El STAB favorable suma 0.25 al multiplicador normal: ×1.50 pasa a ×1.75 y Adaptabilidad ×2.00 pasa a ×2.25. El STAB negativo vale ×0.75. Se integra también en el cálculo de teracristalización; no altera la tabla de efectividades ni elimina inmunidades de tipo.

## Desterrado

- **Individuales:** obliga al objetivo a retirarse y abre la selección normal de sustituto. El jugador elige su reemplazo; el rival usa su controlador habitual. El sustituto no hereda la acción pendiente ni los estados del anterior.
- Un Pokémon desterrado no puede regresar mientras dure el contador, ni por selección normal, IA, Relevo, objetos de cambio o arrastre aleatorio como Rugido.
- **Dobles:** conserva su posición y PS, no puede ejecutar movimientos ni retirarse, incluso con Muda Concha. Queda fuera del conteo de individuos capaces de continuar. Si cae el último compañero y no quedan reservas utilizables, su equipo pierde aunque el desterrado conserve PS.
- Se purgan los estados principales y los efectos volátiles del objetivo al aplicarlo. Los cambios de etapas no se consideran estados y permanecen en dobles; un cambio normal los limpia como siempre.
- No recibe ataques, estados nuevos, daño de área ni daño residual. Ni Indefenso ni movimientos de impacto garantizado u OHKO atraviesan el destierro. No puede capturarse mientras esté desterrado.
- En individuales sin un reemplazo legal, el movimiento falla y no concede ni consume la inmunidad permanente. Esta regla cierra el caso antes abierto sin permitir un menú de cambio imposible.
- **Una vez por individuo para siempre:** un bit dentro del espacio anteriormente reservado de su estructura cifrada registra haber sido desterrado. Viaja con el Pokémon al guardarlo, copiarlo, capturarlo o intercambiarlo. Curarse o cambiar no lo elimina. No crece `BoxPokemon`, no se modifica el formato `.sav` y no se reserva una lista externa limitada de individuos.
- Los combates de enlace/repetición y Arena de Batalla no admiten Desterrado en esta implementación. El arnés de pruebas usa controladores de enlace grabado para simular combates locales y dispone de una excepción exclusiva de `TESTING`.

## Aplicación, aprendizaje y curación

Cada estado dispone de un movimiento propio. Todos usan el motor normal de precisión, Protección, reflejo de Capa Mágica y Sustituto. Los estados ofensivos también respetan el bloqueo del Escudo de Paladín. Los nueve movimientos son de estado y no se encadenan.

El catálogo de movimientos de Horizon / Resumen incorpora las técnicas según tipos y nivel. No sustituye movimientos previos ni añade automáticamente un ataque al equipo.

| Movimiento | Tipos que lo pueden aprender | Nivel | Precisión / PP |
| --- | --- | ---: | --- |
| Enraizar | Planta | 24 | 90 / 15 |
| Ingravidez | Psíquico, Fantasma | 28 | 100 / 15 |
| Confinar | Psíquico, Fantasma, Siniestro | 28 | 90 / 15 |
| Niebla Cegadora | Veneno, Siniestro | 24 | 90 / 15 |
| Mirada Aterradora | Fantasma, Siniestro, Dragón | 32 | 90 / 15 |
| Encadenar | Acero | 30 | 90 / 15 |
| Desterrar | Fantasma, Psíquico | 50 | 85 / 5 |
| Santificar | Hada, Lucha | 36 | 100 / 10 |
| Demonificar | Siniestro, Fuego | 36 | 100 / 10 |

Ingravidez elige al usuario o a su aliado; los demás eligen objetivo. La afinidad de Santificar/Demonificar puede beneficiar o perjudicar al objetivo según su movimiento.

**Enfermero** conserva su 30% original: primero puede curar un estado principal; cuando no hay uno, puede retirar un estado personalizado negativo por activación. No quita beneficios ni interrumpe Desterrado. El motor ofrece además una API única para futuras habilidades, artefactos y curas; Grillete ya utiliza esa misma aplicación.

La aplicación anuncia el nombre y duración. El panel de movimientos muestra el estado personalizado prioritario y su contador, conservando la visualización normal del estado principal. Los nombres y descripciones de movimientos están en español e inglés.

## Validación

`test/battle/unholy_status.c` prueba aplicación y coexistencia, inmunidades, anclaje y Gravedad, precisión real de Cegado, daño físico/especial bajo Aterrado, STAB real de ambas afinidades, caducidad, limpieza por cambio, Sustituto, Enfermero, guardado cifrado de inmunidad y acceso al catálogo.

Para Desterrado incluye cambio obligatorio en individuales, purga, daño de área y residual, Indefenso/OHKO, inmunidad tras expirar, bloqueo de arrastre de reservas y derrota por caída del último compañero en dobles.

También se ejecuta regresión del Enfermero original, Levitón y el catálogo de habilidades de Unholy. La inspección visual en Pizza Boy, los menús de cambio del jugador y las animaciones reutilizadas requieren prueba manual antes de fusionar.
