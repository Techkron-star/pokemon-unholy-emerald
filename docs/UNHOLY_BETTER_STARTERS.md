# Unholy Emerald · Better Starter por tipo favorito

## Experiencia de juego

1. Al terminar la conversación obligatoria de la televisión, la madre pregunta: **«¿Cuál es tu tipo de Pokémon favorito?»**.
2. El jugador puede desplazarse por los **18 tipos** disponibles (seis visibles por página). No se permite cerrar el selector sin escoger un tipo.
3. El juego sortea **tres Pokémon diferentes** entre ocho candidatos seleccionados a mano para ese tipo. Los tres se muestran en las Poké Balls originales de la mochila del profesor Abedul, con su nombre, sprite y grito correspondiente.
4. El rescate, la batalla introductoria y el resto del inicio de Hoenn conservan su flujo original. El Pokémon elegido es uno de esos tres sorteados.

## Equilibrio y compatibilidad

- Cada tipo tiene ocho candidatos de bajo poder inicial, con **400 puntos base de estadísticas como máximo**. No hay legendarios, singulares ni evoluciones finales. Algunos Pokémon de tipo Dragón son primeras etapas con evoluciones fuertes, pero necesitan entrenarse para alcanzarlas.
- **Frigibax** (Dragón/Hielo, 320 puntos base), la primera fase de Baxcalibur, puede aparecer en el sorteo de Dragón. Noibat permanece disponible mediante el tipo Volador.
- Los **tres resultados no se repiten** entre sí. En una partida nueva, quedan fijados desde la elección con la madre: cerrar y volver a abrir la mochila no los vuelve a sortear.
- Cuatro variables de evento de Hoenn que estaban sin utilizar guardan el tipo (índice + 1) y las tres especies. **No se modifica el formato del guardado**.
- Si una partida omite la conversación inicial (por ejemplo, un guardado anterior), la mochila pregunta el tipo favorito antes de abrir el selector de tres Pokémon.
- Las versiones FireRed y LeafGreen conservan sus starters originales. `VAR_STARTER_MON` mantiene el **índice 0/1/2** para que los guiones de encuentros y combates con el rival sigan funcionando, aunque por ahora **el equipo de los rivales sigue usando su selección original**, no se adapta a la especie sorteada.

## Verificación

`python3 tools/unholy/validate_better_starters.py` comprueba las 18 selecciones, los ocho candidatos por tipo, que el tipo coincide con los datos de especies, el presupuesto máximo de estadísticas y las conexiones de la madre, el menú, la mochila y el guardado. El validador se ejecuta en el trabajo `docs_validate` de CI.

Para aprobar la experiencia completa falta una partida de prueba en el emulador: escoger al menos un tipo, confirmar los tres nombres, rescatar a Abedul, guardar/cargar y verificar que la especie entregada es la correcta.
