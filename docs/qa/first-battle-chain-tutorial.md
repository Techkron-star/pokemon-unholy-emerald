# QA — Tutorial de encadenados en el primer combate

## Comportamiento esperado

1. Iniciar **partida nueva en Esmeralda** y llegar al rescate de Birch en Ruta 101.
2. Hablar con la mochila de Birch. La transición a la pantalla para elegir al inicial continúa normalmente; **no** aparece el antiguo texto retrospectivo sobre la pantalla negra.
3. Elegir un inicial. Entrar en el primer combate contra el Zigzagoon.
4. **Una vez visibles los Pokémon en el campo**, el propio cuadro de texto del combate presenta la explicación del profesor:
   - «In this battle, you can CHAIN two different moves!»
   - «Each move costs STAMINA. The second follows immediately!»
   - Recordatorio breve sobre los símbolos de mutaciones.
5. Confirmar el texto en sus páginas y comprobar que la batalla pasa a permitir elegir movimientos. No debe haber pantalla negra de diálogo fuera del combate, congelamiento ni controles retenidos.
6. Terminar el combate y confirmar que siguen la escena de agradecimiento de Birch y el traslado al laboratorio.
7. Comprobar que la explicación **no** se repite en combates salvajes posteriores ni en los primeros combates del rival.
8. Probar derrota y reintento, si la ROM permite ese escenario sin reiniciar partida.

## Implementación

- `data/maps/Route101/scripts.inc`: eliminado `Route101_Text_BattleSystemsIntro` del bloque posterior al fade negro (y su definición obsoleta).
- `src/battle_main.c`: mensaje de Birch durante `BATTLE_INTRO_STATE_SET_DEX_AND_BATTLE_VARS`, tras el envío de Pokémon y antes de `TryDoEventsBeforeFirstTurn`, restringido a `BATTLE_TYPE_FIRST_BATTLE` **sin** `BATTLE_TYPE_TRAINER` ni `BATTLE_TYPE_LINK`.
- `include/constants/battle_string_ids.h` y `src/battle_message.c`: nueva cadena `STRINGID_FIRST_BATTLE_CHAIN_TUTORIAL`. Se usa el texto de batalla y sus comandos de paginación, no un `msgbox` sobre una pantalla de transición negra.

## Criterios de validación

- [ ] Build Emerald, FireRed y LeafGreen (el tutorial solo debe mostrarse en el rescate correspondiente).
- [ ] Pruebas automatizadas y límites de texto de batalla (dos líneas y ancho máximo de 208 px).
- [ ] QA visual real: texto sobre el fondo del combate, sprites visibles y correcta interacción A/B.
- [ ] Regresión de selección del inicial, batalla, victoria, retorno a Ruta 101 y progreso del guion.
