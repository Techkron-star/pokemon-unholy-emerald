# Unholy Emerald — HFC Watch y cinco nuevas mutaciones

**Fecha:** 2026-11-09  
**Estado:** especificación técnica y de diseño; NO implementado  
**Base:** `feat/chain-ko-qa-20261109`  
**Origen:** definiciones expresas del autor. Mantener otras magnitudes como pendientes, no inventarlas.

## 1. Las cinco mutaciones nuevas (definiciones confirmadas)

| Mutación | Efectos confirmados | Pendiente de decidir |
|---|---|---|
| Hyper | Multiplicador **×1.25** a **todas las seis estadísticas**: PS, Ataque, Defensa, Velocidad, Ataque Especial y Defensa Especial | Frecuencia, icono, obtención y reglas de combinación |
| Overloaded | Multiplicador **×1.25** a **Velocidad, Ataque Especial y stamina máxima** | Frecuencia, icono y combinación |
| Titan | **PS, Defensa y Defensa Especial** elevados mediante multiplicadores en el intervalo **×1.20–×1.60**; **Velocidad muy reducida** | Valor/intervalo de reducción de Velocidad y si las tres bonificaciones son independientes |
| Ancestral | **PS ×1.35–×1.40**, sin la penalización de Velocidad de Titan | Si tendrá ventajas adicionales; rareza |
| Ascendido | **IV perfectos (31) y EV máximos (252) en cada una de las seis estadísticas** | Rareza, procedimiento, reentrenamiento y compatibilidad de combinaciones |

**Importante:** Las nuevas mutaciones son *características individuales del Pokémon*. La implementación del nombre como prefijo visual tipo Nebulous es una capa de presentación separada. Si coexisten varias mutaciones, no inventar jerarquías ni apilamientos antes de definir las reglas de balance.

### Ascendido vs Omega

`DISENO.md` ya describe **Omega** con IV/EV perfectos en cada estadística, excepción al tope global de EV y multiplicadores independientes por estadística (×1.2–×1.5). **Ascendido** comparte el rasgo de IV/EV máximos, pero **no recibe por defecto esos multiplicadores extraordinarios de Omega**. Esta diferencia es el punto de partida para que no sean sinónimos.

- Normal: hasta 252 EV por estadística y 510 EV totales.
- Ascendido: 252 × 6 = **1512 EV efectivos/totales**, fuera de la restricción normal; 31 IV en los seis atributos.
- Omega: conservar por separado las reglas previas del documento, incluyendo los multiplicadores propios que aún requieren implementación.
- No cambiar globalmente `MAX_TOTAL_EVS` para permitir Ascendido: solo la excepción del ejemplar que posea la mutación. Auditar todos los caminos de entrenamiento y generación, incluida la opción de combinación Ascendido+Omega.

## 2. Arquitectura de persistencia y estadísticas

### 2.1 Representación

Actualmente `include/pokemon.h` define cuatro bits para `specialBadges`, en `PokemonSubstruct0`, y `MON_DATA_SPECIAL_BADGES` devuelve estos cuatro. **No basta con sumar cinco máscaras nuevas a un campo de cuatro bits**.

Propuesta: ampliar las máscaras a por lo menos **9 bits** y repartir las cinco nuevas marcas entre bits reservados de `PokemonSubstruct0` (hay campos `unused_02:2` y `unused_04:3` que se deben auditar primero), manteniendo la **misma longitud binaria de BoxPokemon**. Ofrecer una única API `Get/Set(Box)MonData` que componga/descomponga la máscara extendida, y continuar usando los mecanismos de checksum/cifrado existentes.

Antes de modificar nada:
1. Verificar que los campos reservados siguen realmente libres en todos los formatos importados/exportados.
2. Comparar `sizeof(struct BoxPokemon)` y offsets con la versión actual.
3. Probar guardados antiguos, cajas, intercambio, huevos, captura, evolución y datos de batalla.
4. Usar como lectura un formato `u16` (no `u8`) para que no se trunquen los cinco indicadores nuevos.

### 2.2 Aplicación matemática

Actualmente `CalculateMonStatsCont` es la función central para recalcular estadísticas; las insignias existentes aún necesitan integrarse correctamente a los modificadores reales. Crear una capa central **determinista** de modificación de stats de Pokémon, reutilizada donde corresponda por ficha, batalla, IA, recálculo, nivel y evolución.

- Basarse en los stats normales resultantes de especie/IV/EV/naturaleza y aplicar los modificadores especiales después, siguiendo una prioridad única que todavía se debe aprobar.
- HP máximo y PS actuales se actualizan de modo que no revivan Pokémon debilitados ni generen PS negativos/extraños al entrar o salir de combate.
- `Overloaded` afecta **stamina máxima** dentro del cálculo actual en `src/battle_stamina.c`; el **coste del movimiento** continúa creciendo porcentualmente junto con el máximo.
- Los valores de Titan y Ancestral pueden generarse por individuo, con semilla/persistencia estable; **nunca re-randomizar por consulta de pantalla o recálculo**.
- Si faltan bits para almacenar la intensidad particular, planificar almacenamiento adicional compatible o elegir un valor ligado a una característica persistente y documentar su independencia del RNG de batalla.
- Revisar compatibilidad de varias mutaciones combinadas para impedir desbordamientos de PS, daño, estadísticas y velocidad.

### 2.3 Regla de EVs de Ascendido

El motor usa `MAX_TOTAL_EVS = 510` y `MAX_PER_STAT_EVS = 252`, con controles en `MonGainEVs`, vitaminas y otras operaciones. Las seis cifras **caben** en los seis campos EV `u8` actuales, pero **el tope global debe exceptuarse para el individuo Ascendido**.

Se recomienda una única función de decisión centralizada para la excepción del límite total del ejemplar, usada por la generación y todos los métodos de modificar EV. No dejar el indicador sin efecto ni fingir 1512 EV en la UI si el motor sigue aplicando solo 510.

## 3. HFC Watch (High Frequency) — Pokélocator / PokéNav evolucionado

**Nombre provisional elegido:** HFC Watch. Inspiración funcional: buscador por hábitat/encuentro tipo PokéNav; **no copiar contenido propietario ni prometer clon exacto**.

### Menús propuestos

- **Radar de ruta:** especies disponibles, horarios/condiciones, intensidad de señal; datos reales de las tablas de encuentros.
- **Frecuencia mutante:** aumentar probabilidad de encuentros con variantes conforme se obtengan **medallas**. Escalado, porcentajes, duración y costo **pendientes**, sin rerolls ilimitados.
- **Raids:** acceso a incursiones con Pokémon fuertes, posibilidades de mutaciones especiales, condiciones de desbloqueo por medallas, y una IA deliberadamente agresiva con mecánicas legítimas, sin trampas invisibles ni bucles.
- **Archivo de anomalías:** información y símbolos de las mutaciones registradas, mostrando las nuevas **Hyper, Overloaded, Titan, Ancestral y Ascendido**, además de Alpha, Super, Enraged y Omega.

### Principios de obtención

- Mantener lo ya acordado en `DISENO.md`: Horizon Shop solo vende Pokémon **normales** y no vende Omega ni Infinity Ball.
- No suponer que HFC pueda generar Omega o Ascendido indiscriminadamente; la curva de rareza, desbloqueos por medallas y acceso a raids deben fijarse de manera explícita.
- La IA de raids puede explotar cobertura, cambio, stamina, predicción y encadenamientos, pero debe respetar las mismas reglas que el jugador salvo que el encuentro señale modificaciones concretas.

## 4. Interfaz / prefijos

- Una insignia pequeña, tipo *pastilla* con símbolo/imagen a la **izquierda del nombre** del Pokémon, desplazando el nombre sin cortarlo; mantener sprites/píxeles apropiados para 240×160 de GBA.
- Omega: `Ω` blanca en pastilla roja. Nuevas mutaciones: **iconos, colores y prioridad visual aún por definir**.
- Incluir leyenda/explicación en el resumen y opción de revisar combinaciones sin agotar el ancho del nombre.
- Comprobar portabilidad del campo de insignias a equipo, cajas, captura y resumen.

## 5. Etapas y aceptación

1. **Documentar y asegurar el esquema persistente** con pruebas de guardados antiguos; sin tocar `master`.
2. Implementar insignias extendidas y lectura/escritura; pruebas de checksum, evolución, cajas y niveles.
3. Implementar Hyper, Overloaded, Titan y Ancestral en cálculo central y stamina; pruebas de combinaciones, rangos y redondeo.
4. Implementar Ascendido real (31 IV/252 EV todos los atributos), incluso a través de objetos/entrenamiento; validar total 1512, evolución y recálculo.
5. Implementar UI de prefijos y ficha; evitar cortes de nombres largos.
6. HFC Watch/radar y desbloqueos por medallas; después raids e IA avanzada.
7. Compilar + QA en emulador antes de considerar cierre; las ideas no están implementadas por crear este archivo.

## Fuentes técnicas del repositorio

- `DISENO.md`: características actuales de Alpha, Super, Enraged, Omega, Pokélocator y Mutator.
- `include/pokemon.h`: `specialBadges:4`, `Get/Set(Box)MonData`, `BoxPokemon`.
- `include/constants/pokemon.h`: máximos de EV/IV.
- `src/pokemon.c`: `CalculateMonStatsCont`, `MonGainEVs`, `Pokemon_AssignEncounterBadges`.
- `src/battle_stamina.c`: stamina máxima y costes relativos.
