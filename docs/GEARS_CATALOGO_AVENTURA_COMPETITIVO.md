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
| **Módulo Vectorial** | En el siguiente movimiento de pivote compatible (Ida y Vuelta, Voltiocambio, Acua Jet NO), reduce su coste de STA un **25 %**. | Reduce 8 % del coste de STA de pivotes compatibles. | Pivot, manejo de equipos, control de recursos. |
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

## Otras ideas anteriores (aún sin aprobación/implementación)

Se conservan las ocho ideas previamente propuestas; **NO mezclarlas con la lista de Gears implementados**: Espejo de Némesis (reflejo), Aguja de Cronos (drenaje de STA), Núcleo de Entropía (ruptura de pantallas), Orbe de Resonancia (multigolpe), Colmillo de Hidra (drenaje de PS), Prisma Anómalo (reducción de daño supereficaz), Grillete Astral (bloqueo temporal de cambio) y Sello de Cero (anular boosts defensivos).

Evitar solapamiento entre **Ancla de Escape** y **Grillete Astral**: ambos deben poder contrarrestarse de forma determinista. Evitar que los dos efectos de supervivencia consecutiva hagan invulnerable al portador en combinación con objetos.

## Orden de desarrollo recomendado

- **P0:** resolver amplitud y migración de IDs persistentes; completar interfaz de equipar, obtención real y QA de los Gears existentes. No prometer sprites o recetas no presentes.
- **P1:** implementar 18 Sangres mediante el modificador de potencia existente, con pruebas de cada uno de los 18 tipos y de las dos fases del ciclo. Anadir un catálogo claro en crafteo.
- **P2:** Batería de Reserva, Botas del Umbral, Velo Críptico y Amuleto Inquebrantable: cuatro puntos de entrada distintos del motor (STA, hazards, secundarios, cambios de estadísticas).
- **P3:** Crisol Antídoto, Lente de Certeza, Módulo Vectorial y Ancla de Escape.
- **P4:** Bastión Gemelo y Escudo del Último Aliento, tras QA intensiva de dobles y antiexploits.

### Límite de IDs y guardado

La rama base tiene `GEAR_MAX_COUNT = 32` y **5 bits** en `PokemonSubstruct1.gear`; por ello solo admite **31 Gears + GEAR_NONE**. Proyección: 8 Gears existentes/acordados + 18 Sangres + 8 ideas previas + 10 de este documento = **44** (sin contar Colmillo de Rayquaza si se convierte en Gear).

**No basta subir `GEAR_MAX_COUNT` a 128.** Se necesita una migración del campo persistente a, como mínimo, **6 bits** (hasta 63 identificadores, 62 Gears) o idealmente **7 bits** (hasta 127 identificadores, 126 Gears), y una auditoría de ocupación de bits/estructura, checksum, acceso `MON_DATA_GEAR`, Caja/PC y guardados antiguos. No sobrescribir bits usados por mutaciones, stamina u otros flags. Valorar una estructura de guardado ampliada y migración versionada, puesto que el proyecto admite incompatibilidad controlada entre compilaciones, siempre con respaldo de save.

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
- `make`, tests que procedan, emulador visual e inspección de guardado; **ningún check se declara aprobado sin ejecutarlo**.

## Fuentes de inspiración (no equivalen a cifras de balance)

- Smogon, catálogo de objetos SV: https://www.smogon.com/dex/sv/items/
- Smogon, Covert Cloak: https://www.smogon.com/dex/sv/items/covert-cloak/
- Referencia de tendencias de sets OU (septiembre 2026, no norma oficial): https://pokekipe.com/sv/ou?elo=all&tab=items

**Estado al crear este documento:** catálogo de diseño; sin efectos nuevos conectados al motor, sin assets nuevos, sin recetas registradas ni QA de ROM.
