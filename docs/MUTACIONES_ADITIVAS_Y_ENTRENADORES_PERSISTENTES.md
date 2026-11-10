# Unholy Emerald · Mutaciones acumulables y persistencia por partida

**Estado:** especificación de diseño confirmada el 9 de noviembre de 2026. **No implementada por este documento.**  
**Ámbito:** Pokémon salvajes, incursiones de HFC Watch, rival, líderes de gimnasio y jefes especiales.  
**Rama de documentación:** `docs/unholy-mutation-stacking-persistence`.  
**Referencia de código:** `feat/chain-ko-qa-20261109`, commit `dbeef5a4a3cf6448c4f9660359aab49dccfd9478`.

## 1. Principio de diseño

Las mutaciones son propiedades *individuales* del Pokémon. **Pueden coexistir sin límite artificial de una sola mutación por individuo.** Si un Chansey recibe Titan, Omega y Ancestral, **las bonificaciones por estadísticas se SUMAN en puntos porcentuales; no se multiplican unas con otras**.

Ejemplo: Titan +40 % PS, Omega +35 % PS y Ancestral +37.5 % PS:

```
multiplicador de PS = 1 + 0.40 + 0.35 + 0.375 = 2.125
PS finales = redondear(PS normales con IV/EV relevantes × 2.125)
```

**No** calcular `PS × 1.40 × 1.35 × 1.375`: esa sería otra regla distinta. La misma suma aditiva de bonos positivos rige para otras estadísticas; penalizaciones se registran como porcentajes negativos y también participan en la suma. El balance exacto de penalización Titan queda por definir.

El Pokémon puede superar 999 PS si la fórmula lo permite. No aplicar un tope arbitrario de 999 para la jugabilidad; actualizar en cambio las interfaces y la aritmética para cifras de cuatro dígitos o más.

## 2. Nueve mutaciones conocidas

| Mutación | Efecto confirmado | Particularidades |
| --- | --- | --- |
| **Alpha** | Ya definida en el diseño anterior; valores definitivos pendientes | Se acumula con las demás |
| **Super** | Menor EXP requerida, mayor crecimiento y posibilidad de múltiples habilidades | No inventar aquí un multiplicador |
| **Enraged** | Crecimiento ofensivo superior a cambio de defensas inferiores | Ajustes finales pendientes |
| **Hyper** | **+25 %** a todas las estadísticas | Incluir stamina por ser una estadística del hack |
| **Overloaded** | **+25 %** a Velocidad, Stamina y Ataque Especial | No aplicar al resto |
| **Titan** | Entre **+20 y +60 %** a PS, Defensa y Defensa Especial; Velocidad fuertemente reducida | Magnitud de reducción de Velocidad por definir |
| **Ancestral** | Entre **+35 y +40 %** a PS | No inferir otros bonos |
| **Ascendido** | **31 IV en cada estadística y EV máximos en las seis** | Requiere excepción a cap total de EV |
| **Omega** | **Ascendido + entre +20 y +50 % a cada estadística** | Bono por estadística, extraordinariamente raro |

Reglas complementarias:

- **Ascendido + Omega:** ambos garantizan IV y EV máximos, pero la asignación genética se realiza **una sola vez**. No añadir un "segundo bono de IV/EV" ficticio. Omega conserva su bono porcentual.
- **Hyper + Omega + Overloaded:** todos sus **bonos porcentuales distintos** suman; no se descarta ninguno.
- Los porcentajes de rango se sortean **una sola vez por individuo** al asignar su mutación; no se vuelven a sortear al combatir, guardar, cargar o evolucionar.
- **Omega:** respetar la intención previa de multiplicadores **individuales por estadística** dentro de +20 a +50 %. Titan y Ancestral pueden tener valores fijos por mutación o por estadística: determinarlo antes de fijar formato binario.
- La stamina se calcula a partir de su fórmula propia y después recibe solo los bonos de mutaciones aplicables a stamina. Evitar aplicar dos veces un mismo incremento originado por PS o EV.
- Los porcentajes se representan como enteros o valores de punto fijo, nunca con floats dependientes del compilador GBA.

### Ejemplo ilustrativo: Chansey extremo

Un Chansey de nivel alto con PS base calculados después de IV/EV puede alcanzar **1200 PS y mucho más** con Titan + Omega + Ancestral. Con +40 % Titan, +35 % Omega y +37.5 % Ancestral, el total es **+112.5 % PS**, o **×2.125**; si la base fuera 600 PS, resultaría en 1275 PS. **600 es solo un valor de ejemplo, no un dato de Chansey a cierto nivel.**

## 3. Ruleta reproducible y persistencia de entrenadores

Cada archivo de guardado establece una **semilla de mutaciones de esa partida**. El juego genera asignaciones reproducibles por **identidad estable del Pokémon del entrenador**, no por cada encuentro:

```
semilla_de_partida + ID_estable_entrenador + ID_estable_linea_del_Pokemon
    -> conjunto_de_mutaciones + bonos_individuales
```

**Obligatorio:** el resultado permanece estable durante toda la partida.

- Si el Combusken del rival es Omega, su Blaziken evolucionado sigue siendo Omega y retiene el mismo bono individual cuando corresponda.
- Una nueva partida puede asignar otras mutaciones (o ninguna) al mismo Pokémon rival.
- Guardar/cargar, escapar, perder, reiniciar, vencer al líder o solicitar una revancha no vuelve a tirar la ruleta.
- No identificar un miembro únicamente por la posición del equipo en un combate: las revanchas pueden añadir, quitar, reordenar o evolucionar Pokémon. Mantener **lineageId / stableMonKey** por equipo importante.
- Guardar los resultados reales (incluidas bonificaciones sorteadas) o usar una función determinista *versionada* con semilla y claves estables, con especial cuidado al actualizar el juego.
- No usar RNG de combate, animaciones o apertura de menús para tirar las mutaciones.

### Restricción de Omega por tipo de encuentro

**Pokémon de entrenadores:** Omega **solo** puede salir aleatoriamente en miembros de personajes especiales: rival persistente, líderes de gimnasio y jefes importantes (Alto Mando, campeón, jefes designados). **Nunca** en un entrenador común de ruta.

**Salvajes y raids:** conservar el sistema de rarezas o reglas específicas del contenido; la restricción anterior se refiere **a entrenadores** y no prohíbe futuros Omega salvajes, de cuevas especiales o de incursiones autorizadas por diseño.

### Probabilidades naturales de las nueve mutaciones

**Alcance:** tiradas base por cada Pokémon salvaje individual, independientemente de la especie. Una tasa 1/100 significa *una probabilidad del 1 % por encuentro de esa especie*, no que cada bloque de 100 individuos garantice un Alpha. Las tiradas se aplican solo en circunstancias donde la mutación sea elegible.

| Mutación | Frecuencia base por encuentro | Porcentaje | Estado |
| --- | ---: | ---: | --- |
| **Enraged** | **1/90** | **1.111 %** | **Confirmada por el autor** |
| **Alpha** | **1/100** | **1.000 %** | **Confirmada por el autor** |
| Super | 1/180 | 0.556 % | Propuesta |
| Hyper | 1/250 | 0.400 % | Propuesta |
| Overloaded | 1/350 | 0.286 % | Propuesta |
| Titan | 1/500 | 0.200 % | Propuesta |
| Ancestral | 1/750 | 0.133 % | Propuesta |
| Ascendido | 1/2500 | 0.040 % | Propuesta |
| **Omega natural** | **1/9000** | **0.0111 %** | **Confirmada por el autor; ligeramente más raro que shiny base** |

**No confundir Omega salvaje (1/9000 confirmado) con Omega del Mutator (10/65536 ≈ 1/6554 propuesto), ni con Omega en rivales y jefes (tabla especial por definir).** Los intentos del Mutator son comprados; las tiradas naturales son encuentros. La tasa shiny base de esta ROM es 1/8192: **Omega natural (1/9000) es aproximadamente un 9 % menos frecuente que shiny**, no más frecuente.

**Estado real del código:** `include/config/special_badges.h` aún fija Alpha salvaje en 4 %, Enraged salvaje en 1 % y Omega salvaje en 0 %, y solo soporta las cuatro mutaciones antiguas. Esta tabla es de **diseño**, no ha alterado la ROM ni las probabilidades actuales.

### Rareza extrema de acumulaciones naturales (regla confirmada)

Las mutaciones individuales pueden combinarse, pero **cada mutación adicional debe ser muchísimo más difícil de obtener naturalmente**. No basta con nueve probabilidades independientes de valor moderado, porque producirían demasiados Pokémon con dos o tres mutaciones.

- **Una mutación:** rara o poco frecuente según la especie, zona, jefe y tipo de mutación.
- **Dos mutaciones:** una combinación extremadamente afortunada y excepcional entre salvajes o Pokémon de entrenadores.
- **Tres mutaciones:** extraordinariamente improbable; el encuentro debe sentirse como un hallazgo casi irrepetible.
- **Cuatro o más mutaciones:** aún posibles, sin cap arbitrario, pero cada nivel adicional debe disminuir drásticamente la posibilidad. No confundir *teóricamente posible* con *frecuente*.
- Las raids o jefes pueden aplicar tablas especiales, **sin que eso vuelva comunes las acumulaciones**. La identidad persistente de los Pokémon rivales debe cubrir también la cantidad de mutaciones.

**Mecánica de combinación propuesta para simulación (todavía no aprobada):** realizar primero las nueve tiradas base; si salen varias, ordenarlas aleatoriamente con una semilla reproducible y asignar la primera. Para conservar una segunda, superar una puerta adicional de **1/12**; para la tercera, una puerta de **1/40**; para la cuarta, **1/100**, y puertas progresivamente más exigentes para las siguientes. Si una puerta falla, detener la acumulación. Estas puertas son **adicionales** a la rareza de cada mutación, no la sustituyen.

Con la tabla combinada (Alpha, Enraged y Omega confirmados; las otras seis tasas propuestas) y tiradas independientes **antes** de las puertas, dos o más mutaciones candidatas aparecerían cada ~1,826 encuentros y tres o más cada ~237,645. Las puertas 1/12 y 1/40 darían resultados efectivos del orden de 1 en ~21,918 para dos o más y 1 en ~114 millones para tres o más. **Son cálculos teóricos para esta propuesta de distribución, no resultados verificados en la ROM; confirmar con simulaciones y tests del algoritmo final.** Las tasas base declaradas se refieren a la tirada inicial; la frecuencia final de cada mutación puede variar ligeramente cuando se resuelven colisiones.

No reutilizar las tasas anteriores en código como si fueran las nuevas: hacen falta nuevos rangos RNG, bitmask ampliada, compatibilidad de guardado y pruebas de balance. Los entrenadores especiales y raids utilizarán reglas contextualizadas, sin hacer triviales las combinaciones.

### Mutator del endgame: acumulación conseguida mediante inversión

El **Mutator** es una máquina desbloqueada en el tramo final del juego para intentar rerollear mutaciones gastando cantidades elevadas de Pokédólares. Mediante uso repetido, el jugador puede llegar a obtener **varias mutaciones en un mismo Pokémon**, en contraste con su rareza natural extrema.

- El Mutator no regala combinaciones ni permite obtenerlas sin pagar: cada intento sigue siendo costoso y sujeto al azar.
- Buscar una segunda, tercera o cuarta mutación exige **una inversión muy grande**, que debe crecer significativamente con la dificultad del objetivo.
- No convertir la inversión en una garantía automática ni en una forma trivial de conseguir un Pokémon con todas las mutaciones.
- **Omega SÍ puede salir del Mutator.** Su probabilidad en **cada intento pagado** debe ser **ligeramente superior a la probabilidad shiny configurada en la ROM**, sin convertirse en una recompensa garantizada.
- **Referencia verificada de la ROM actual:** `SHINY_ODDS = 8` y la fórmula de shiny es `SHINY_ODDS / 65536`, es decir, **1/8192** sin modificadores. Como **propuesta de balance**, usar `10 / 65536` para Omega del Mutator (1.25 veces la tasa shiny; aproximadamente **1 entre 6554** intentos). Es un punto inicial **pendiente de aprobación**, no una probabilidad final acordada.
- La tirada Omega debe basarse en la tasa shiny **base**, no en bonificaciones temporales, amuletos u otros modificadores de shiny que pudieran crear un exploit.
- No permitir reiniciar o cancelar el mismo intento para volver a lanzar esa tirada gratuitamente; asociar cada intento a su pago y resultado persistentes.
- Los detalles de la operación —rerollear solo una mutación, sustituir una existente, añadir una nueva, proteger mutaciones anteriores o arriesgarse a perderlas— **no están cerrados**. Diseñar la UI y la transacción sin inventar ese comportamiento.
- Mantener las otras restricciones previamente contempladas para la máquina, incluida la penalización máxima de 15 niveles cuando corresponda, hasta que se ajuste el diseño.

**Objetivo del balance:** ver un Pokémon natural con dos mutaciones debe sorprender; con tres debe ser un acontecimiento extraordinario. El Mutator ofrece una ruta deliberada de progreso caro y una posibilidad remota pero real de obtener Omega, nunca una distribución natural alterada ni un atajo gratuito.

## 4. HFC Watch, incursiones y presentación

**HFC Watch** (nombre provisional), inspirada en el sistema de búsqueda del PokéNav de Elite Redux:

- Localizador de Pokémon por zona, con filtros y progresión dependiente de medallas.
- **PC portátil integrada en HFC Watch:** abrir las cajas reales del jugador, consultar sus Pokémon, moverlos, depositar, retirar y reorganizar equipo y cajas desde el menú del dispositivo. No será una PC paralela ni un catálogo ficticio: usa el almacenamiento persistente existente.
- La PC remota estará accesible fuera de combate y siempre que el mapa o una secuencia argumental no prohíban gestionar el equipo. No permitir abrirla durante batallas, selecciones forzadas del inicial, escenas guiadas ni transiciones críticas. La adquisición del dispositivo habilita la función; no se inventa un bloqueo por medallas para la PC sin decisión posterior.
- **Seguridad y calidad:** impedir depositar al último Pokémon utilizable cuando la lógica original lo prohíba, evitar duplicación de individuos, respetar objetos equipados y mutaciones persistentes, y restaurar correctamente la interfaz al cerrar. Aprovechar la interfaz actual de PC/almacenamiento, ampliada solo donde haga falta.
- Modificadores de probabilidad de mutaciones según medallas/recursos; no otorgar mutaciones garantizadas salvo decisión expresa.
- Acceso a **raids** con Pokémon muy fuertes, IA agresiva y posibilidad de múltiples mutaciones.
- El conjunto de mutaciones del Pokémon de una incursión se fija **al generar la instancia** y no se modifica para recompensar reinicios o reabrir el mismo menú.
- No confundir la persistencia de un jefe entrenador durante toda la partida con la de raids, cuya política exacta de rotación aún debe fijarse.

**UI:** prefijos visuales compactos anteriores al nombre del Pokémon. Omega se representa como una pastilla roja con `Ω`; otras mutaciones reciben símbolos y colores distintos. Para múltiples mutaciones, mostrar hasta el espacio disponible con orden fijo y acceder al listado completo en el resumen. No cortar nombres ni barras.

## 5. Deuda técnica obligatoria antes de implementar

### A. Ampliar representación de mutaciones

Hoy `struct BoxPokemon` usa `specialBadges:4` y los monstruos de batalla usan un campo `u8 specialBadges`. **Nueve mutaciones requieren al menos 9 bits**, es decir `u16` o una representación externa equivalente. No basta ampliar la interfaz: inventario, PC, equipo, rivales, guardado y carga necesitan los nuevos bits.

No cambiar sin auditoría el tamaño o desplazamiento de los substructs de Pokémon cifrados ni sobrescribir bits usados por ribbons u otros datos. Elegir formato compatible o mecanismo de migración de partidas. Actualizar serialización, resúmenes, captura, guardado, evoluciones y tests.

### B. EV e IV extraordinarios

En el código actual: `MAX_PER_STAT_IVS = 31`, `MAX_PER_STAT_EVS = 252` (en configuraciones modernas) y `MAX_TOTAL_EVS = 510`. Ascendido y Omega necesitan **252 EV en cada una de las seis estadísticas = 1512 EV en total**, sin relajar el límite 510 para los Pokémon normales.

Implementar una excepción dirigida **solo a individuos con Ascendido y/o Omega**, aplicada en generación, crecimiento, vitaminas, recalculo, evolución, validaciones, entrenamiento y UI. Asegurar que quitar una mutación (si alguna mecánica futura lo permite) no deja datos inválidos.

### C. Estadísticas grandes y aritmética segura

- Recalcular PS actuales proporcionalmente al cambiar PS máximos; nunca dejar PS actuales por encima del máximo.
- Usar intermediarios `u32`/aritmética segura y saturación explícita solo donde de verdad lo exija el formato.
- Soportar PS por encima de 999 en batalla, equipo, cajas, resumen, curación, intercambio, HUD de daño, IA y ventanas de cifras.
- Verificar golpes porcentuales, robo de vida, drenaje, daño residual, protección, resistencia, stamina y otras fórmulas que dependan de PS o stats.
- Un multiplicador se aplica **una vez** sobre la stat calculada con IV, EV, naturaleza y nivel. No multiplicar bonos entre sí; tampoco duplicarlos por un recálculo.

### D. Guardado persistente y compatibilidad

Agregar versión del esquema de datos de mutaciones. Probar iniciar partida nueva, reabrir un guardado existente, evolucionar, intercambiar, mover entre PC/equipo, volver a enfrentar al rival y entrar a una revancha. Si el guardado anterior no puede migrarse, documentar claramente la incompatibilidad **antes** de publicar.

### E. Balance, dificultad y comunicación

Aceptamos encuentros deliberadamente extremos —por ejemplo Chansey con PS de cuatro cifras—, pero la AI debe ser consciente de las mutaciones y la interfaz debe explicar sus efectos sin inventar datos. La combinación no debe impedir que el combate termine por fallos de cálculo, valores fuera de rango o estados imposibles.

## 6. Criterios de aceptación

1. Chansey Titan + Omega + Ancestral aplica **suma aditiva**, no producto de multiplicadores.
2. Hyper + Overloaded + Omega suma los bonos de cada stat aplicable y gestiona stamina una sola vez.
3. Ascendido y Omega asignan 31 IV y 252 EV en cada stat, sin violar las reglas de EV de Pokémon normales.
4. El rival conserva la identidad mutante y los porcentajes individuales a través de Combusken → Blaziken y de todas sus apariciones.
5. Una partida nueva puede producir variantes distintas y guardados de la misma partida reproducen las mismas variantes.
6. Un entrenador corriente no recibe Omega; rival, líderes y jefes sí pueden.
7. Cinco mutaciones adicionales caben en equipo, PC, batalla y guardado; la combinación no se pierde al capturar ni evolucionar.
8. HUD, resúmenes, bolsas/íconos y daño soportan números grandes sin solapamientos, truncamientos o corrupciones.
9. HFC Watch, PC remota y raids aparecen como **pendientes**, nunca como funciones falsas hasta que tengan flujo real; la PC remota debe operar sobre cajas reales sin duplicar ni perder Pokémon.
10. En simulaciones de encuentros naturales, las tasas de dos y tres mutaciones descienden fuertemente; no se trata de mutaciones sorteadas sin restricciones que accidentalmente produzcan combinaciones comunes.
11. El Mutator puede llegar a producir acumulaciones mediante intentos costosos y persistentes; **Omega está permitido**, con una probabilidad por intento apenas superior a la tasa shiny base, sin alterar retroactivamente las probabilidades naturales ni duplicar intentos gratis.

## 7. Decisiones abiertas (no asignar valores arbitrarios)

- Reducción exacta de Velocidad de Titan.
- Cómo se sortean valores dentro del rango de Titan y Ancestral: por instancia o por stat.
- **Enraged 1/90, Alpha 1/100 y Omega salvaje 1/9000 confirmados**. Las otras seis tasas de la tabla y las puertas extra (1/12, 1/40, 1/100) siguen siendo **propuestas** sujetas a simulación, además de las modificaciones por medallas, raids, tipo de encuentro y dificultad.
- Escalado de coste del Mutator por intento y por cantidad de mutaciones, posibilidades de éxito y tratamiento de las mutaciones ya existentes.
- Qué entrenadores especiales quedan incluidos además del rival/líderes/Alto Mando/campeón.
- Qué mutaciones de las cinco nuevas puede producir el Mutator y la **tasa definitiva de Omega** (debe ser ligeramente mayor que la shiny base); determinar si se mide por intento total o por resultado válido de mutación, garantizando que la UI explique esa diferencia.
- Política exacta de persistencia/rotación de raids de HFC Watch.
- Precedencia e interacción con Alpha, Super y Enraged cuando sus multiplicadores finales se concreten.

---

**Fuentes técnicas del repositorio:** `DISENO.md`, `include/pokemon.h`, `include/constants/pokemon.h`, `include/config/special_badges.h`, `src/pokemon.c`, `src/battle_stamina.c`.  
**Este documento solo registra las reglas: no modifica jugabilidad ni compila una ROM.**
