# Unholy Emerald — Octava tanda y redistribución de Gears

Estado: implementación apilada sobre el PR #62. Valores sujetos a QA de combate.

## Gears jugables de la octava tanda

| Gear | Efecto real | Obtención |
| --- | --- | --- |
| Amplificador | Primer ataque sonoro de daño cargado: +25 % de potencia y +10 % de coste de stamina. Recarga de 3 turnos. | Crafteo desde medalla 3. |
| Headphones | -40 % de daño sonoro recibido; bloquea efectos secundarios de esos movimientos dirigidos al portador. | Crafteo desde medalla 3. |
| Botón de Pánico | Una vez por combate: tras sobrevivir a un golpe que deja menos del 30 % de PS, restaura 18 % de stamina máxima, gana una etapa de Velocidad y cancela el segundo ataque encadenado pendiente. No evita el KO. | Poké Ball de Ruta 119. |
| Núcleo Hydra | Hasta dos veces por combate: tras un golpe que lo deja por debajo de 50 % de PS, sana 8 % de PS máximos al final de cada uno de los siguientes 3 turnos, sin revivir ni evitar KO. Recarga 4 turnos. | Incursión Maestro (rotación diaria). |
| Casco Hoplita | En dobles, el compañero recibe 12 % menos daño directo; en individuales, usar con éxito Protección restaura 12 % de stamina máxima, una vez cada 3 turnos. | Poké Ball de Ruta 120. |

El crafteo no ofrece Pánico, Hydra ni Hoplita para conservar el valor de explorar/pelear incursiones.
Las Poké Balls reutilizan posiciones conocidas, pero **tienen indicadores de recogida nuevos**: una partida que ya recogió los objetos anteriores puede recoger las reliquias una vez tras actualizar. Los objetos antiguos no se borran del inventario.

## Rebalance de los 15 Gears anteriores

| Gear | Antes | Ahora / motivo |
| --- | --- | --- |
| Escudo Aegis | Crafteo (medalla 5) | Incursión Maestro: duplica las subidas defensivas, ventaja muy alta. |
| Nihil Engine | Crafteo (medalla 6) | Incursión Maestro: limpieza de trampas estratégica. |
| Guantes Cohete | Crafteo (medalla 4) | Incursión Experto: prioridad de golpes físicos de puño. |
| Piedra de Afilar | Crafteo (medalla 4) | Incursión Experto: escalada de Ataque por corte. |
| Túnica Rúnica | Crafteo (medalla 6) | Poké Ball de Ruta 123: gran inmunidad al desgaste ambiental. |

Los Gears ya equipados o guardados siguen siendo válidos; no se reordenan IDs ni se destruyen existencias previas.

### Recompensas de incursiones

- **Experto:** el tercer premio de botín alterna de forma determinista entre Guantes Cohete y Piedra de Afilar.
- **Maestro:** el tercer premio alterna entre Núcleo Hydra, Escudo Aegis y Nihil Engine.
- El resultado se fija con día + ID del entrenador. La ficha de elección y el premio efectivo usan una sola función, por lo que siempre coinciden; capturar al jefe sigue excluyendo el botín.
- Se mantienen las cantidades de los otros dos premios y el resto de recompensas de cada dificultad.

## Siguientes tandas de arte aún sin lógica

Se integran 20 sprites aprobados de las tandas cuarta, quinta y MASKARA para que el código futuro use los archivos originales. **No están registrados como Gears jugables** hasta conectar sus efectos, revisar interacciones como protección/prioridad global y validar combate:

- Aguja Cronos, Asiento Ejector, Bastión Gemelo, El Milagro, Inyección Berserker.
- Estaca de Ébano, Estaca de Plata, Hacha de Guerra, Red de Caza, Velo Amaterasu.
- Kabuto, Máscara Faraónica, Sutton Hoo, Cara de Bronce, Plague Mask.
- Casco Kipchak, Máscara de Agamenón, Casco Corintio, Yelmo Saboyardo, Casco Mirmillón.

La especificación de efectos aprobados vive en `docs/GEARS_MASKARA_ARSENAL.md`; no se sustituye por efectos falsos para apresurar la compilación.

### Decisiones confirmadas por Camilo (16-11-2026)

Estas tres reglas sustituyen los límites provisionales de la propuesta de los 20 Gears pendientes:

1. **El Milagro — sin límite de activaciones:** siempre que un golpe habría sido letal y el portador **sobreviva gracias a otro efecto**, restaura el **20 % del daño calculado antes de aplicar esa supervivencia**. No salva del KO ni se activa solo por estar bajo un umbral de PS. Puede volver a funcionar si sobrevive a otro golpe letal, incluso durante el mismo combate. Las restricciones anticuración (como Muramasa) siguen aplicándose a la recuperación.
2. **Aguja de Cronos — cada 3 turnos:** durante su turno de activación anula toda prioridad positiva o negativa del campo, tanto en individuales como en dobles; afecta a movimientos, habilidades, objetos y Gears. Las acciones se ordenan por Velocidad normal, respetando la inversión de Espacio Raro si está activa. No genera acciones adicionales ni restaura PP/stamina. El contador avanza por **turno completo**, no por ataque encadenado, y cambiar de Pokémon no debe reiniciarlo.
3. **Inyección Berserker — +50 % / −40 %:** mientras esté activa, **Ataque y Ataque Especial efectivos suben 50 %**, y **Defensa y Defensa Especial efectivos bajan 40 %**. No cambia las etapas reales de estadísticas. Se conserva como propuesta el ciclo de **2 turnos activos / 3 de recarga**, pendiente de confirmar al revisar las interacciones. En cálculos defensivos, el mínimo numérico válido es 1 para evitar desbordamientos.

Los otros diecisiete efectos conservan los valores presentados en la propuesta de revisión hasta que Camilo los corrija o apruebe. **No se consideran implementados por documentar estas decisiones.**

## QA a completar

- Verificar compilación de Emerald/FireRed/LeafGreen, pruebas `test/unholy_recent_gears.c` y validación de textos.
- Confirmar en emulador el efecto de sonido, los cambios de fase, dobles con dos Hoplitas y restauración visual de PS en Hydra.
- Probar la elección de captura frente a botín, progreso de medallas, Poké Balls de las rutas y guardados anteriores.
