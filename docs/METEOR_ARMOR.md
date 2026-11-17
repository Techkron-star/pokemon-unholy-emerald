# Armadura Meteórica — Gear

## Obtención

- El profesor Cozmo entrega **MT27 (Retribución/Return)** gratis, una sola vez, después de los sucesos del Monte Cenizo. No exige ni retira el Meteorito del jugador.
- La receta de Crafting es **1 Revestimiento Metálico + 1 Meteorito**; ambos ingredientes se consumen al fabricar el Gear. No exige medallas adicionales. Se equipa en la ranura independiente de Gear.

## Combate

Si un **único impacto directo de un ataque de un turno** fuera a dejar al portador a 0 PS:

1. Cancela el KO y lo deja en **30 % de sus PS máximos**, redondeando hacia arriba. Si tenía menos de 30 %, también queda en ese valor.
2. Su Defensa y Defensa Especial bajan simultáneamente según el exceso de daño (*overkill*):

   | Exceso de daño respecto a los PS restantes | Bajada de cada defensa | Ceniza |
   | --- | --- | --- |
   | Menos del 25 % de PS máximos | 1 nivel | Quema al atacante |
   | Entre el 25 % y menos del 75 % | 2 niveles | Quema al atacante |
   | 75 % o más | 3 niveles | Quema al atacante y, en dobles, a su compañero |

3. La ceniza respeta inmunidades y estados existentes: nunca sobrescribe otro estado principal. Si un objetivo no puede quemarse, conserva su estado.

**Contrario (Contrary) invierte la penalización de ambas defensas:** el portador recibe +1, +2 o +3 niveles en Defensa y Defensa Especial en vez de perderlos. Se respetan los límites normales de etapas (−6 a +6).

Solo se activa **una vez por Pokémon y combate**; cambiarlo no restaura la carga. El Gear tampoco evita otros impactos del mismo turno. Se excluyen los movimientos de estado, de carga de dos turnos, los impactos diferidos, los movimientos multigolpe, OHKO directos, Bide y ataques potenciados por Parental Bond. Las interacciones defensivas anteriores, como Aguante, Robustez y Banda Focus, se resuelven primero. Los daños indirectos —clima, veneno, quemadura, trampas— no activan la armadura.

## Estado de arte y QA

- Temporalmente la mochila usa el icono del **Meteorito** y el HUD reutiliza el del **Casco Hoplita**. Queda pendiente un sprite propio de Armadura Meteórica (icono 24×24 y estado HUD 16×32).
- Incluye pruebas automatizadas para IDs, equipamiento independiente, exclusión de movimientos, supervivencia y tres niveles de overkill. Pendiente CI con compilador ARM y comprobación visual en mGBA/Pizza Boy Pro de mensaje, PS, defensas, quemaduras dobles e inmunidades.
