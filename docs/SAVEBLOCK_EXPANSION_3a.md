# Guía de ampliación del SaveBlock — Pokémon Unholy Emerald (parte 3a de 3)

---

## 10. Checklist de validación (antes y después)

### Antes de empezar (línea base)

- [ ] El proyecto compila sin errores: make (o build del repo) → OK.
- [ ] El juego arranca en emulador (mGBA/No$GBA).
- [ ] Se puede crear una partida nueva, guardar y cargar.
- [ ] Has anotado los tamaños actuales: SaveBlock1 = 15568 B (real), SaveBlock2 = 3884 B (real).
- [ ] Has creado la rama feat/expand-saveblock y hecho un commit de base limpia.
- [ ] Has borrado saves .sav antiguos (no serán compatibles).

### Durante la modificación

- [ ] include/config/save.h: flags FREE_* ajustados para liberar el espacio necesario.
- [ ] include/global.h: nuevos campos añadidos al final de struct SaveBlock1.
- [ ] src/save.c: sSaveSlotLayout[] revisado (offsets/tamaños por sector).
- [ ] src/save.c: STATIC_ASSERTs pasan (compilación no falla por exceso de tamaño).
- [ ] src/save.c: UpdateSaveAddresses() sigue apuntando a los punteros correctos.
- [ ] test/save.c: T_SAVEBLOCK1_SIZE actualizado de 15568 a 15672.
- [ ] Nuevos campos inicializados con valores por defecto en nueva partida.
- [ ] No quedan accesos por offset numérico hardcodeado a campos desplazados (revisar .s y .c).

### Después de la modificación (antes de commitear)

- [ ] Compilación limpia: make clean && make → OK, sin warnings nuevos.
- [ ] Las STATIC_ASSERT de SaveBlock pasan.
- [ ] Emulador: crear partida nueva → guardar → soft-reset → cargar → los nuevos campos tienen los valores esperados.
- [ ] Emulador: modificar un nuevo campo en juego (ej. comprar en Horizon, equipar un Gear) → guardar → cargar → el cambio persiste.
- [ ] Los sistemas existentes que usan SaveBlock (entrenador, Pokédex, equipo, opciones) siguen funcionando.
- [ ] diseño.md actualizado para registrar la excepción al principio #1.
- [ ] Guía copiada a docs/SAVEBLOCK_EXPANSION.md (o este archivo).
- [ ] docs/SUMMARY.md actualizado si la documentación se genera con mdbook.

### Antes de abrir un PR (si aplica)

- [ ] Diff revisado: git status, git diff --check, git diff --stat.
- [ ] Mensaje de commit incluye aviso de ruptura de saves viejos.
- [ ] No se han tocado ramas de otros desarrolladores.
- [ ] No se hace merge automático a master sin revisión.

---

## 11. Dónde guardar esta documentación

Se recomienda guardar esta guía en:

docs/SAVEBLOCK_EXPANSION.md

Motivos:
- El repositorio ya tiene una carpeta docs/ con libro (mdbook: book.toml, SUMMARY.md).
- Mantiene la documentación técnica cerca del código, separada del diseño de alto nivel (diseño.md en la raíz).
- Es fácil de encontrar para contribuyentes.

Pasos para integrarla al libro mdbook (opcional):

1. Copia este archivo a docs/SAVEBLOCK_EXPANSION.md.
2. Edita docs/SUMMARY.md y añade una entrada:

# SaveBlock

* [Ampliación del SaveBlock](SAVEBLOCK_EXPANSION.md)

3. Asegúrate de que docs/book.toml tenga configurado el idioma/español si deseas consistencia.
4. Si usas docs/fix_links.py, ejecútalo para verificar enlaces internos.

Alternativa: Si prefieres no crear un archivo nuevo, puedes añadir un anexo al final de diseño.md. Pero dado que diseño.md ya es largo y esta guía es puramente técnica/procedural, docs/SAVEBLOCK_EXPANSION.md es la opción más limpia.

