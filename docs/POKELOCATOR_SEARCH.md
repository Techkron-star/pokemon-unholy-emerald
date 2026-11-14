# Pokélocator: catálogo, búsqueda y captura

El localizador de Unholy ofrece un catálogo de especies de la zona, colección y datos de especie, elección de método, búsqueda y captura nativa. La referencia funcional es Elite Redux; la composición, navegación y Señal mutante pertenecen a Unholy. No se habilita el DexNav de rastreo del motor.

## Catálogo y controles

- Cuatro tarjetas visibles en una cuadrícula 2×2. Cada especie aparece una sola vez aunque exista en hierba, Surf y varias cañas.
- **L/R:** categorías ALL, GRASS/CAVE, SURF, RODS y OTHER. Las tres cañas conservan sus slots y requisitos independientes en la ficha.
- **SELECT:** colección ALL → MISSING → CAUGHT. Se combina con la categoría de encuentro. El progreso cuenta especies únicas y los filtros vacíos muestran su estado explícitamente.
- **Cruceta:** recorrer tarjetas y páginas, incluida la última página incompleta. **A:** abrir la ficha. **B:** volver al menú de origen.
- **START → Tools → Browse area:** consultar otros pisos/mapas de la misma zona con L/R. Solo se buscan encuentros en el mapa donde está el jugador.

## Ficha y búsqueda

La ficha tiene tres pestañas mediante L/R: **HABITAT**, **STATS** y **TRAITS**. Izquierda/derecha recorre especies sin volver al catálogo. Habitat permite elegir con arriba/abajo un método real de esa especie, con nivel, tasa y requisito pendiente. Stats muestra las seis estadísticas base; Traits muestra las dos habilidades ordinarias y la oculta de la especie. Estos datos no son IV, habilidad ni mutaciones predichas de un ejemplar aún no generado.

**A → Prepare Search** revisa especie, método, nivel, Balls disponibles, destino equipo/PC, almacenamiento lleno y cargas de Señal. **B** cancela sin generar un Pokémon ni consumir RNG o cargas. **A → Begin Search** revalida e inicia un combate salvaje normal; se captura usando una Ball desde la Mochila. No se capturan especies automáticamente. Sin Balls se advierte que solo se podrá combatir; llenar el almacenamiento también muestra una advertencia.

Al reabrir en el mismo mapa se recuperan categoría, filtro, especie y método si siguen disponibles. Esta selección dura la sesión y se reconstruye contra las tablas actuales; no añade campos al guardado ni conserva datos de una tabla antigua. Si una captura elimina el objetivo del filtro MISSING, se selecciona otra especie válida o el estado vacío.

## Señal mutante

**START → Tools → Mutant Signal** muestra los nueve símbolos de mutación, sus tiradas base `1/N`, el estado y las cargas disponibles. Se indica que las combinaciones pasan por compuertas adicionales. Las cifras proceden de `config/special_badges.h`: no se presentan como probabilidad final de una combinación.

**A** abre una confirmación con las reglas del servicio existente: HF Poké Watch, medallas y reloj, una activación diaria, diez encuentros de ese mapa y mejora relativa de las tiradas C/B. A/S/Omega conservan sus tasas. **B** no consume el uso diario. Confirmar llama a `HfcWatch_ActivateFrequency`; el localizador no duplica sus flags, balance o contadores. La vista obtiene el boost guardado al activar, no uno recalculado al ganar otra medalla. Dibujarla nunca consume cargas. La búsqueda confirmada consume una carga mediante el mismo generador que los encuentros normales.

El modificador independiente de mutaciones pendiente de diseño permanece desactivado. No se inventan costes, duración, probabilidades ni desbloqueos por medalla adicionales a los del WATCH.

## Requisitos y datos

| Método | Requisito para Buscar |
| --- | --- |
| Hierba/cueva | Estar en tierra y tener un Pokémon apto para combatir |
| Surf | Estar surfeando o bajo el agua |
| Cañas | Poseer la caña elegida y estar en una posición donde el motor permite pescar |
| Golpe Roca | Movimiento de campo desbloqueado y un miembro del equipo que lo conozca |
| Ocultos | Haber visto la especie; también se aplica tierra/agua según su tabla |

Se comprueban mapa, horario y disponibilidad al confirmar y después de cerrar la ventana. Se elige un slot de la especie respetando los pesos de su método/caña; el nivel se obtiene con `ChooseWildMonLevel`. No se inventan niveles intermedios uniendo slots. Consultar, filtrar y rechazar búsquedas no cambia el RNG ni el equipo enemigo. Buscar evita la tirada al caminar y el repelente como acción manual. Safari, Pirámide y bloqueos de encuentros/captura impiden Buscar.

La ventana devuelve la paleta del campo y libera iconos al salir o pasar al combate. Los textos reservan 12 píxeles reales por línea, aunque la métrica de la fuente estrecha anuncie 8; los tiles terminan antes del marco estándar.

## Verificación

- `test/pokelocator.c`: cuatro pruebas de búsqueda y cuatro de catálogo adicionales a las 15 originales. Comprueban slots separados, cañas, entradas inválidas, rechazo sin efectos, deduplicación, filtros combinados, cuadrícula impar/vacía y lectura sin cambiar tablas/RNG.
- `test/hfc_watch.c`: lectura del boost almacenado sin consumir cargas, mapa distinto y cargas agotadas.
- Emerald compila con `UNUSED_ERROR=1`. Escena de campo preparada en mGBA: catálogo, pestañas, requisitos de Señal, confirmación, cancelación y activación nativa con 10 cargas y boost de 35 %. Al cerrar no quedan paletas de los nueve símbolos.
- La búsqueda anterior verificó Poochyena capturado como segundo miembro, 10 → 9 Master Balls, resultado `B_OUTCOME_CAUGHT`, Pokédex CAUGHT y regreso al campo. La comprobación del flujo ampliado y la suite nativa se registran en el PR.

Pendientes de QA: partida natural, todos los mapas/métodos, equipo y cajas llenos, hardware.

Referencia de funciones: [código del localizador de Elite Redux](https://github.com/Elite-Redux/eliteredux/blob/master/src/dexnav.c). No se importa código ni arte de esa referencia.
