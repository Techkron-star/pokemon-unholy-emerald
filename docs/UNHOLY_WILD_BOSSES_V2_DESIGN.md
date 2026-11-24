# Unholy Emerald — Wild Bosses: expansión temprana y media (diseño aprobado)

> Estado: **diseño registrado; encuentros NO implementados todavía**. El PR #70 contiene únicamente el primer encuentro jugable propuesto de Registeel Alfa en Calle Victoria B2F, y requiere compilación/QA. Este documento no convierte los cinco nuevos jefes en encuentros de la ROM.

## Catálogo aprobado

| Orden | Zona | Jefe | Nivel objetivo | Mutación garantizada | Perfil | Recompensa al vencer |
| --- | --- | --- | ---: | --- | --- | --- |
| 1 | Ruta 102 | Poochyena — Colmillo Feral | 9 | Alfa | tutorial, presión moderada | **Colmillo Feral** (nuevo Artifact, pendiente de ID) |
| 2 | Bosque Petalia | Shroomish — Raíz Corrupta | 15 | Enraged | desgaste, drenaje, control | **Raíz Espinosa** (nuevo Artifact, pendiente de ID) |
| 3 | Ruta 110 | Electrike — Relámpago Salvaje | 26 | Overloaded | velocidad y encadenamientos ofensivos válidos | **Condensador de Tormenta** (nuevo Artifact, pendiente de ID) |
| 4 | Ruta 113 | Skarmory — Centinela de Ceniza | 32 | Titán | resistencia física y hazards | **Piedra de Afilar** (`ITEM_GEAR_PIEDRA_AFILAR`, existente; comprobar obtención en otras zonas) |
| 5 | Ruta 120 | Absol — Presagio de la Muerte | 43 | Super | críticos y castigo a objetivos debilitados | **Mira Francotirador** (concepto; pendiente de ID y diferenciación frente a Scope Lens/Lente Certeza) |

Los niveles son objetivos de diseño; ajustar al ritmo real de medallas, disponibilidad de especies y stamina antes de fijarlos en scripts.

## Jefe introductorio: Poochyena, Ruta 102

- Visible en overworld mediante `OBJ_EVENT_GFX_POOCHYENA`, ya definido; colocar en una casilla lateral accesible, **sin bloquear el camino principal**. Elegir coordenadas solo al inspeccionar el mapa en Porymap y emulador.
- Interacción voluntaria con A; diálogo breve: «Este Pokémon desprende una presencia anormal…». Explicar **ANTES** del combate: «Puedes capturarlo y conservar su mutación, o vencerlo y conseguir el Artifact. No obtendrás ambos».
- Encuentro nivel 9, Alfa garantizado. Diseño inicial: ×1,6 PS **solo durante la pelea**, +15% Ataque **solo durante la pelea**, sin trasladar aumentos al ejemplar capturado. No usar estas cifras literalmente hasta disponer de un modificador temporal de PS seguro; si solo hay etapas de stats, calibrar una versión equivalente.
- Estrategia didáctica: ataques físicos sencillos y una reducción de precisión o defensa. Sin segunda fase, sin OHKO, sin cadenas abusivas ni imposibilidad de huir.
- Concepto de Colmillo Feral: +10% de daño contra objetivos que tengan alguna estadística reducida. Efecto y balance pendientes de aprobación de implementación/catálogo.

## Identidad de los otros cuatro

- **Shroomish Enraged**: Leech Seed/Absorb o equivalentes que existan en la build; enseña a romper la recuperación y el control. Raíz Espinosa: propuesta de castigo leve a ataques de contacto, pendiente de balance.
- **Electrike Overloaded**: presión por velocidad y Stamina; cadenas solo con movimientos que la normativa de encadenamiento admita, sin excepciones para movimientos de campo/soporte.
- **Skarmory Titán**: defensa, trampas y desgaste; evitar inmunidad efectiva que obligue a una composición de equipo específica. Reutiliza Piedra de Afilar, pero revisar que no quite una recompensa reservada en otra ubicación.
- **Absol Super**: críticos y castigo a rivales con pocos PS; el Artifact de puntería debe distinguirse de `ITEM_SCOPE_LENS` y `ITEM_GEAR_LENTE_CERTEZA`, ya disponible en Ruta 110.

## Reglas comunes (alineadas con Wild Bosses V1)

1. Un sprite de Pokémon visible en overworld, interacción con A, sin encuentros aleatorios forzados; capturable con Poké Balls normales según reglas vigentes.
2. Derrota del jefe = un Artifact; captura = conservar el Pokémon y su mutación real, **renunciando** al Artifact. Mejoras de jefe (PS/stages/efectos) exclusivas de batalla.
3. Huir o perder no marca el jefe como completado. Mochila llena no debe eliminar permanentemente la recompensa.
4. Solo después de superar la Superliga: limpiar bandera al **volver a entrar en el mapa**, y permitir capturas y recompensas nuevas sin límite de una copia por partida.
5. Los jefes no modifican encuentros aleatorios, rivales, entrenadores, raids ni Pokémon capturados de la misma especie.
6. Recompensas no duplicadas con el **Escudo de Aegis** del Registeel Alfa ya definido ni **Rompeolas** reservado a Ruta 126. No sustituir automáticamente especies o Artifacts de ese encuentro.
7. Mantener traducciones ES/EN y los textos de pantalla breves, sin lenguaje de desarrollo.

## Dependencias de implementación

- **Gráficos:** solo `OBJ_EVENT_GFX_POOCHYENA` está registrado entre estos cinco. Shroomish, Electrike, Skarmory y Absol requieren nuevos overworld sprites 4bpp, paletas y registros `event_objects`, o una solución visual fiel. **No sustituirlos por sprites de otras especies**.
- **Artifacts:** Colmillo Feral, Raíz Espinosa y Condensador de Tormenta no existen en `include/constants/items.h`; Mira Francotirador tampoco está registrada como Gear. Antes de conectar recompensas crear IDs, datos, descripción ES/EN, inventario/equipamiento, iconos y efectos. No dar por creados esos objetos mediante documentación.
- **Gameplay:** extender el despacho por ID de `UnholyWildBoss_Arm` y `UnholyWildBoss_ApplyBattleBonuses`, sin copiar el hook del PR #70 por cada especie. Garantizar las mutaciones manteniendo otras tiradas naturales, limpiar el ID al cerrar combate y validar persistencia.
- **Mapas:** añadir scripts/eventos y banderas específicas para cada zona, integrando la lógica de `MAP_SCRIPT_ON_TRANSITION` preexistente (Rutas 110, 113 y 120 ya tienen transiciones propias). Seleccionar coordenadas mediante Porymap.
- **QA:** compilar GBA; reproducir cada batalla en emulador; capturar con equipo lleno, huir, perder, ganar con mochila llena; visitar antes/después de la Superliga; verificar reapariciones repetidas, ausencia de duplicación de premios, mutaciones y sprites correctos.

## Encuentros ya existentes o reservados

- **Registeel Alfa, Calle Victoria B2F, nivel 62:** implementado en PR #70 como guardián del Escudo de Aegis, pendiente de compilación y QA. **No reemplazar con Hariyama.**
- **Ruta 126:** reservar Rompeolas para el encuentro acuático previsto; especie/ID y efecto no cerrados en el PR #70. **No crear un segundo premio Rompeolas.**

Fuente técnica: árbol de la rama `feat/unholy-wild-bosses-v1-20261118` de este repositorio, más los acuerdos de diseño de Unholy Emerald del 18-11-2026.
