# Unholy Emerald — localización del juego

## Idiomas

- Español neutro (`Español`), predeterminado al crear una partida nueva.
- Inglés (`English`), disponible en **Ajustes → Idioma**.
- Las partidas anteriores conservan el valor cero de la nueva opción (inglés), salvo que el jugador lo cambie.
- La opción se almacena en un bit previamente reservado de `SaveBlock2`, sin desplazar campos ni agrandar la estructura. El test de compatibilidad exige `sizeof(SaveBlock2) == 3884`.
- **No modificar `GAME_LANGUAGE` ni el campo `language` de un Pokémon:** esos campos codifican su procedencia y tienen usos distintos a la interfaz.

## Reglas editoriales

- Usar español neutro, terminología Pokémon coherente y **artefacto** en vez de `gear` en todo texto dirigido al jugador.
- Las descripciones deben explicar el efecto jugable: detonante, objetivo, intensidad, duración, límite y coste cuando corresponda. Evitar frases genéricas o técnicas internas.
- Comprobar en la pantalla GBA (240 × 160) todos los textos, saltos de línea, símbolos, tildes y contadores. Evitar recortes y elementos superpuestos.
- Al alternar el idioma, redibujar las pantallas abiertas con sus cadenas del idioma seleccionado. Si la pantalla no lo permite, recargarla al salir y volver a entrar.
- Centralizar pares de cadenas con ID en catálogos; no alterar nombres internos ni identificadores de eventos.

## Progreso y cola de integración

1. [x] Preferencia de idioma compatible con la estructura de guardado.
2. [x] Ajustes bilingües con selección inmediata en pantalla.
3. [x] Etiquetas bilingües del menú de inicio.
4. [ ] Pantallas base: mochila, party, resumen, reloj HFC, Horizon Shop, crafting, equipo, PokéLocator y guardado.
   - [x] Resumen nativo: vista de mutaciones con efectos implementados, navegación por páginas y bonos netos individuales de PS, Ataque, Defensa, Velocidad, Ataque Especial, Defensa Especial y Stamina. Los ceros se distinguen de penalizaciones y aumentos; requiere QA visual en GBA.
5. [x] 320 IDs de habilidades: nombres y descripciones traducidos, con fallback a inglés, pantalla de resumen y ventanas de combate conectadas. Falta comprobación visual en emulador.
6. [ ] Textos de combate y estados, movimientos, objetos, artefactos y recetas.
   - [x] 934 movimientos: 847 de generaciones I a IX, 35 Z y 52 Dinamax/Gigamax. Nombres y descripciones en español, conectados a combate, resumen y aprendizaje. Los nombres largos se abrevian a un máximo de 16 caracteres.
   - [ ] Validación visual de movimientos (nombres, tildes, descripciones y desbordamiento) en emulador.
   - [x] Los 47 artefactos equipables cuentan con nombres y descripciones en español e inglés. Mochila, crafting, equipos y ambas vistas de resumen comparten el mismo catálogo, con nombres de hasta 20 caracteres y descripciones de dos líneas.
   - [ ] Validación visual de artefactos en emulador y revisión editorial de la duración de los efectos.
7. [ ] Guiones de historia, eventos, NPC, tutoriales y mensajes de sistema.
8. [ ] Auditoría de todas las apariciones visibles de `gear`, `gears`, restos de inglés y abreviaciones incomprensibles.
9. [ ] Prueba en hardware/emulador: nuevo juego, carga de partida previa, cambio en caliente, guardado, exportación, desbordamientos y acentos.
10. [ ] Descripciones de especies en la Pokédex. Traducción independiente de `SpeciesInfo`, con fallback inglés para especies aún pendientes.
    - [x] Kanto #001-#151: traducciones de los párrafos originales, cuatro líneas por especie y pruebas automatizadas de ancho.
    - [x] Johto #152-#251: 100 descripciones traducidas, cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Hoenn #252-#386: 135 descripciones traducidas, cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Sinnoh #387-#493: 107 descripciones traducidas, cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Teselia #494-#649: 156 descripciones traducidas, cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Kalos #650-#721: 72 descripciones traducidas, cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Alola #722-#809: 88 descripciones traducidas (incluidos Meltan y Melmetal), cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Galar #810-#905: 96 descripciones traducidas (incluidos los siete Pokémon nuevos de Hisui), cuatro líneas por especie y pruebas de cobertura y ancho.
    - [x] Paldea #906-#1025: 120 descripciones traducidas (incluidas las especies de las expansiones), cuatro líneas por especie y pruebas de cobertura y ancho.
    - [ ] Formas alternativas, revisión editorial y validación visual real en 240 × 160.

La infraestructura inicial no significa que los apartados aún pendientes estén traducidos. Mientras se migran a los catálogos de texto, algunas pantallas seguirán mostrando su idioma original.
