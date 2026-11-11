# Guía de ampliación del SaveBlock — Pokémon Unholy Emerald (parte 3b de 3)

---

## 12. Notas de compatibilidad Emerald vs FRLG

El repositorio compila tanto para Pokémon Emerald como para FireRed / LeafGreen (IS_FRLG se usa en varios archivos, p. ej. src/data.c, include/credits_frlg.h, include/hall_of_fame_frlg.h).

Consideraciones al ampliar el SaveBlock:

1. Mismo tamaño en ambos juegos: Si el SaveBlock compartido (struct SaveBlock1 / struct SaveBlock2) se usa tanto en Emerald como en FRLG, cualquier campo añadido debe tener el mismo tamaño en ambas plataformas. No uses tipos que varíen de tamaño según el juego.

2. Flags FREE_* compartidos: include/config/save.h se compila para ambos juegos. Si desactivas un sistema que solo existe en Emerald (p. ej., FREE_TRAINER_HILL o FREE_TRAINER_TOWER), verifica que no rompa la compilación de FRLG si ese código no existe allí. Envía en #if !IS_FRLG o similar si es necesario.

3. Direcciones de RAM: Los punteros gSaveBlock1Ptr, gSaveBlock2Ptr pueden apuntar a direcciones EWRAM distintas en Emerald vs FRLG. No hardcodees direcciones absolutas; usa siempre los punteros globales.

4. offsetof y layout: El layout del struct es igual si el compilador y las opciones de compilación son iguales. Como el Makefile del proyecto es compartido, el layout debe ser consistente entre Emerald y FRLG.

5. Prueba en ambos: Antes de dar por buena la ampliación, compila y prueba al menos en Emerald (plataforma principal del hack). Si distribuyes también builds de FRLG, valida que el save funcione allí también.

6. diseño.md Principio de compatibilidad: El punto 1.2 de diseño.md dice: "El código debe compilar en Pokémon Emerald, FireRed y LeafGreen. Las banderas específicas de FRLG deben declararse para evitar errores de compilación (ej. EXP Share).". Mantén este principio al añadir campos al SaveBlock.

---

## 13. Apéndice: relación con diseño.md

El archivo diseño.md (raíz del repositorio) establece como Principio 1.1:

No ampliar SaveBlock. Los sistemas nuevos deben reutilizar variables de guardado existentes o usar espacio sin usar (ej. 0x40B8, 0x40BB, 0x40F7–0x40FE). No cambiar el tamaño de SaveBlock1 ni SaveBlock2.

Si apruebas la ampliación del SaveBlock descrita en esta guía, estás haciendo una excepción deliberada a ese principio. Por tanto, debes actualizar diseño.md:

1. Añade una sección o nota registrando que se amplió el SaveBlock, la fecha, el motivo (mutaciones, stamina, Horizon, Gears) y los campos nuevos.
2. Cambia el principio #1 para reflejar la nueva política, o añade una excepción documentada:

### 1.1. Espacio de guardado (SaveBlock)

Regla general: No ampliar SaveBlock; reutilizar variables existentes o espacio sin usar (0x40B8, 0x40BB, 0x40F7–0x40FE, etc.).

Excepción documentada: El SaveBlock se amplió en [fecha] para persistir datos de mutaciones, stamina, mercado Horizon y Gears. Véase docs/SAVEBLOCK_EXPANSION.md. Esta es una excepción deliberada al principio general; cualquier nueva ampliación requiere justificación y actualización de este documento.

3. Actualiza la tabla de Variables reservadas de la sección 3.7 si alguna de esas variables (0x40B8, 0x40BB, 0x40F7–0x40FE) deja de usarse porque sus datos ahora viven en el SaveBlock.

---

## Resumen rápido para copiar y pegar (comandos)

1. Rama nueva: git checkout -b feat/expand-saveblock
2. Editar configuración y structs:
   - include/config/save.h   -> ajustar FREE_*
   - include/global.h        -> añadir campos al final de SaveBlock1
   - src/save.c              -> revisar sSaveSlotLayout[] y STATIC_ASSERTs
   - test/save.c             -> actualizar T_SAVEBLOCK1_SIZE de 15568 a 15672
3. Compilar: make clean && make
4. Probar en emulador: nueva partida, guardar, reset, cargar, verificar persistencia
5. Documentar:
   - Actualizar diseño.md (excepción al principio #1)
   - Copiar esta guía a docs/SAVEBLOCK_EXPANSION.md
   - Actualizar docs/SUMMARY.md (si usa mdbook)
6. Commit con aviso de ruptura de saves:
   git commit -m "feat: ampliar SaveBlock con mutaciones, stamina, Horizon y Gears

Rompe compatibilidad con saves previos a este cambio.
Borra .sav antiguos antes de probar."

---

Fin de la guía. Para dudas, consulta include/config/save.h, include/global.h, src/save.c y diseño.md en el repositorio.