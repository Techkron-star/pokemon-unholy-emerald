# Guía de ampliación del SaveBlock — Pokémon Unholy Emerald

> **Repositorio:** https://github.com/Techkron-star/pokemon-unholy-emerald (rama `master`)
> **Autor:** único desarrollador (tú)
> **Idioma:** español
> **Estado:** referencia técnica para futura ampliación y contribuciones

---

## Tabla de contenidos

1. [Introducción y alcance](#1-introducción-y-alcance)
2. [Estado actual del SaveBlock](#2-estado-actual-del-saveblock)
3. [Qué se va a ampliar y por qué](#3-qué-se-va-a-ampliar-y-por-qué)
4. [Archivos que se deben modificar (rutas exactas)](#4-archivos-que-se-deben-modificar-rutas-exactas)
5. [Procedimiento paso a paso para ampliar el SaveBlock](#5-procedimiento-paso-a-paso-para-ampliar-el-saveblock)

---

## 1. Introducción y alcance

Esta guía describe cómo ampliar de forma segura el **SaveBlock** (el bloque de datos de partida guardada) en el repositorio `pokemon-unholy-emerald`.

El SaveBlock es la estructura de memoria RAM que se persiste en la flash interna de la GBA cada vez que el jugador guarda la partida. Cualquier dato que deba sobrevivir a un apagado/soft-reset (posiciones, objetos, Pokémon del equipo, flags, etc.) vive aquí.

**Contexto del proyecto:**
- Eres el único desarrollador.
- **No te importan las partidas guardadas viejas** (no hay compatibilidad hacia atrás con saves previos a la ampliación).
- Trabajas en una rama propia (rama `master` de tu fork).
- El objetivo es dejar todo documentado para futura referencia y para otros contribuyentes.

> **Advertencia importante:** Ampliar el SaveBlock **cambia el tamaño y los offsets** de toda la estructura de guardado. Los saves generados antes del cambio serán ilegibles/corruptos después del cambio. Dado que no te importan los saves viejos, esto es aceptable, pero debe documentarse y comunicarse a cualquier contribuyente.

---

## 2. Estado actual del SaveBlock

### SaveBlock1

Archivo de configuración: `include/config/save.h`

SaveBlock1 total: 2516 bytes (cuando todos los FREE_* = FALSE)

SaveBlock1 almacena datos de la partida en curso que se escriben en los sectores 1-4 de cada slot de guardado (SECTOR_ID_SAVEBLOCK1_START a SECTOR_ID_SAVEBLOCK1_END).

Contiene, entre otros: estado del jugador, posición en el mapa, equipo de Pokémon, Pokédex, flags de eventos, match call, datos de entrenador, datos de mysteries, etc.

Los flags FREE_* permiten liberar espacio dentro de SaveBlock1 desactivando sistemas no usados:

| Flag | Qué libera | Bytes |
|------|-----------|-------|
| FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK1 | Flags Pokédex seen no usados | 52 |
| FREE_TRAINER_HILL | Datos de Trainer Hill | 28 |
| FREE_TRAINER_TOWER | Datos de Trainer Tower | x |
| FREE_MYSTERY_EVENT_BUFFERS | ramScript | 1104 |
| FREE_MATCH_CALL | Match call / rematch / VS Seeker | 104 |
| FREE_UNION_ROOM_CHAT | Chat de sala de unión | 212 |
| FREE_ENIGMA_BERRY | Enigma Berry (E-Reader) | 52 |
| FREE_LINK_BATTLE_RECORDS | Récords de batalla link | 88 |
| FREE_MYSTERY_GIFT | Mystery Gift | 876 |

### SaveBlock2

SaveBlock2 total: 1274 bytes (cuando todos los FREE_* = FALSE)

SaveBlock2 almacena datos más estáticos (ID del entrenador, nombre, opciones, horas jugadas, etc.) y se escribe en el sector 0 de cada slot (SECTOR_ID_SAVEBLOCK2).

Flags FREE_* para SaveBlock2:

| Flag | Qué libera | Bytes |
|------|-----------|-------|
| FREE_BATTLE_TOWER_E_READER | Battle Tower E-Reader | 188 |
| FREE_POKEMON_JUMP | Pokémon Jump | 16 |
| FREE_RECORD_MIXING_HALL_RECORDS | Récords de mixing de sala | 1032 |
| FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK2 | Flags Pokédex seen no usados | 108 |

### Totales actuales

SaveBlock1 total: 2516 bytes
SaveBlock2 total: 1274 bytes
Grand Total: 3790

### Sistema de sectores de guardado

Archivos: include/save.h y src/save.c

Este fork no usa el sistema de guardado vanilla de pokeemerald, sino un sistema basado en sectores de flash personalizado:

#define SECTOR_DATA_SIZE   3968   // bytes de datos reales por sector
#define SAVE_BLOCK_3_CHUNK_SIZE 116
#define SECTOR_FOOTER_SIZE 12
#define SECTOR_SIZE        (SECTOR_DATA_SIZE + SAVE_BLOCK_3_CHUNK_SIZE + SECTOR_FOOTER_SIZE)  // 0x1000 = 4096

#define NUM_SAVE_SLOTS     2
#define NUM_SECTORS_PER_SLOT 14

#define SECTOR_ID_SAVEBLOCK2          0
#define SECTOR_ID_SAVEBLOCK1_START    1
#define SECTOR_ID_SAVEBLOCK1_END      4
#define SECTOR_ID_PKMN_STORAGE_START  5
#define SECTOR_ID_PKMN_STORAGE_END   13

Distribución por slot de guardado:

| Sectores | Contenido | Struct |
|----------|-----------|--------|
| 0 | SaveBlock2 | struct SaveBlock2 |
| 1-4 | SaveBlock1 (4 sectores) | struct SaveBlock1 |
| 5-13 | Almacenamiento de Pokémon (PC) | struct PokemonStorage |
| 28-29 | Hall of Fame | — |
| 30 | Trainer Hill | — |
| 31 | Batalla grabada | — |

En src/save.c, la tabla sSaveSlotLayout[] asigna offsets y tamaños a cada sector usando la macro SAVEBLOCK_CHUNK. Las STATIC_ASSERT verifican que cada struct quepa en el espacio asignado:

STATIC_ASSERT(sizeof(struct SaveBlock2) <= SECTOR_DATA_SIZE, SaveBlock2FreeSpace);
STATIC_ASSERT(sizeof(struct SaveBlock1) <= SECTOR_DATA_SIZE * (SECTOR_ID_SAVEBLOCK1_END - SECTOR_ID_SAVEBLOCK1_START + 1), SaveBlock1FreeSpace);

UpdateSaveAddresses() inicializa gRamSaveSectorLocations[] apuntando a gSaveBlock1Ptr, gSaveBlock2Ptr y gPokemonStoragePtr con los offsets correctos.

---

## 3. Qué se va a ampliar y por qué

Según el contexto del proyecto, se quiere persistir datos de sistemas nuevos que actualmente no usan espacio del SaveBlock (por decisión del principio #1 de diseño.md, se usan variables sueltas en RAM sin guardar: 0x40B8, 0x40BB, 0x40F7–0x40FE, etc.).

Si se decide ampliar el SaveBlock, los candidatos a mover al SaveBlock son:

| Sistema | Datos a persistir | Motivo |
|---------|------------------|--------|
| Mutaciones | Tier de mutación por Pokémon, badges obtenidos, máscara de mutación activa | Debe sobrevivir a guardado/carga y guardar/cargar |
| Stamina | Barra de stamina de Pokémon activo, cooldowns, regeneración | Estado de batalla/campo persistente |
| Mercado Horizon | Semillas de stock, ofertas de Pokémon/ítems/MTs, estado de pestañas | El diseño.md ya sugiere usar 0x40B8, 0x40BB, 0x40F7–0x40FE, pero moverlo al SaveBlock lo hace más robusto |
| Gears | Gear equipado por Pokémon, cargas restantes, estado de fabricación | Debe persistir entre sesiones |

**¿Por qué ampliar en lugar de seguir usando variables sueltas?**
- Las variables sueltas en RAM (ej. 0x40B8) no se guardan automáticamente; hay que guardarlas y restaurarlas manualmente en cada save/load.
- Moverlas al SaveBlock las hace persister automáticamente con el sistema de sectores existente.
- Centraliza todos los datos guardados en una sola estructura con checksum y control de sectores dañados.

> **Nota de diseño:** diseño.md establece como principio #1: "No ampliar SaveBlock. Los sistemas nuevos deben reutilizar variables de guardado existentes o usar espacio sin usar." Ampliar el SaveBlock es por tanto una excepción deliberada a ese principio. Si apruebas este cambio, debes actualizar diseño.md (ver sección 13).

---

## 4. Archivos que se deben modificar (rutas exactas)

Estas son las rutas exactas en el repositorio Techkron-star/pokemon-unholy-emerald (rama master):

### 4.1. Archivos obligatorios (siempre se modifican)

| Ruta | Qué se modifica |
|------|-----------------|
| include/config/save.h | Flags FREE_* (para hacer espacio) y comentarios de tamaño total |
| include/save.h | Definición de struct SaveBlock1 y struct SaveBlock2 (añadir nuevos campos), posiblemente struct SaveBlock3 si se usa |
| src/save.c | sSaveSlotLayout[] (offsets/tamaños por sector), STATIC_ASSERTs, UpdateSaveAddresses() si cambian los punteros |

> Nota: En este fork, struct SaveBlock1 y struct SaveBlock2 están definidos en include/save.h (junto con el resto del sistema de sectores). Verifica su ubicación exacta antes de editar; si en tu rama están en otro header (p. ej. include/save_block.h o dentro de include/global.h), ajusta las rutas.

### 4.2. Archivos condicionales (según qué se amplíe)

| Ruta | Cuándo se modifica |
|------|-------------------|
| src/data.c | Si declaras gSaveBlock1, gSaveBlock2, gPokemonStorage allí (extern/definición de variables globales) |
| src/save.c | Cualquier inicialización de nuevos campos en ClearSaveData() o funciones de new game |
| include/pokemon_storage_system.h | Si amplías el almacenamiento de PC (struct PokemonStorage) |
| include/constants/*.h | Si añades nuevas constantes/IDs relacionadas con los nuevos campos |
| asm/*.s | Cualquier código asm que lea/escriba offsets directos del SaveBlock |
| src/load_save.c | Si hay lógica de carga/guardado adicional ligada al SaveBlock |
| diseño.md | Para registrar la excepción al principio #1 y las nuevas variables de save |
| docs/SAVEBLOCK_EXPANSION.md | Esta guía (copia de este documento) |

### 4.3. Archivos de referencia (no se modifican, solo se consultan)

| Ruta | Propósito |
|------|-----------|
| include/save.h | Sistema de sectores, SaveSector, SECTOR_ID_*, SECTOR_DATA_SIZE |
| src/save.c | Lógica de escritura/lectura de sectores, WriteSaveSectorOrSlot, TryLoadSaveSlot |
| include/config/save.h | Flags FREE_* para liberar espacio |
| diseño.md | Principios de diseño actuales (incluye "No ampliar SaveBlock") |
| docs/SUMMARY.md | Índice de la documentación (mdbook) — actualizar si añades la guía al libro |

---

## 5. Procedimiento paso a paso para ampliar el SaveBlock

Sigue este orden. No saltes pasos.

### Paso 0: Preparación y respaldo

1. Crea una rama dedicada, p. ej. feat/expand-saveblock.
2. Haz un commit limpio de la base actual (git commit -m "base previa a ampliación SaveBlock").
3. Asegúrate de que el proyecto compila y arranca en emulador antes de tocar nada (make o el sistema de build del repo).
4. Descarta cualquier save .sav antiguo; no será compatible.

### Paso 1: Liberar espacio en include/config/save.h

Decide qué sistemas desactivar para ganar bytes. Ejemplo: si no usas Trainer Hill, Mystery Gift, Match Call, etc., pon sus flags en TRUE:

#define FREE_TRAINER_HILL                   TRUE    // 28 bytes
#define FREE_MYSTERY_GIFT                   TRUE    // 876 bytes
#define FREE_MATCH_CALL                     TRUE    // 104 bytes
#define FREE_UNION_ROOM_CHAT                TRUE    // 212 bytes
// ... otros según necesites

Esto libera bytes dentro de SaveBlock1/SaveBlock2 sin cambiar su tamaño total. Si necesitas más espacio del que se puede liberar con flags, pasa al Paso 2.

### Paso 2: Añadir campos a los structs en include/save.h

Edita struct SaveBlock1 y/o struct SaveBlock2 para añadir los nuevos campos.

Ejemplo (adaptar a los nombres reales de tu struct):

struct SaveBlock1
{
    // ... campos existentes ...

    // --- Campos nuevos: Mutaciones ---
    u16 mutationTier;          // Tier de mutación del Pokémon activo / líder
    u32 mutationMask;          // Máscara de mutaciones activas
    u8  mutationBadges;        // Badges de mutación obtenidos (bitfield)

    // --- Campos nuevos: Stamina ---
    u16 staminaCurrent;        // Stamina actual
    u16 staminaMax;            // Stamina máxima
    u32 staminaCooldown;       // Temporizador de regeneración

    // --- Campos nuevos: Mercado Horizon ---
    u32 horizonSeedDiscs;      // Semilla de stock de MTs (DISCS)
    u32 horizonSeedItems;      // Semilla de stock de ítems (ITEMS)
    u16 horizonPokemonOffer1;  // Oferta Pokémon 1
    u16 horizonPokemonOffer2;  // Oferta Pokémon 2

    // --- Campos nuevos: Gears ---
    u8  equippedGearSlot;      // Slot de Gear equipado (0-5, 0xFF = ninguno)
    u16 gearCharges[6];        // Cargas restantes por Gear
};

**Regla de oro:** Añade los nuevos campos al final del struct (antes del cierre };), nunca en medio, a menos que estés dispuesto a reajustar todos los offsets posteriores manualmente. Añadir al final minimiza el desplazamiento de campos existentes.

### Paso 3: Ajustar sSaveSlotLayout[] en src/save.c

La tabla sSaveSlotLayout[] define qué parte de cada struct va en qué sector. Si creciste SaveBlock1 más allá de lo que cabía en 4 sectores, o si reordenaste campos, actualiza los offsets/tamaños.

struct
{
    u16 offset;
    u16 size;
} static const sSaveSlotLayout[NUM_SECTORS_PER_SLOT] =
{
    SAVEBLOCK_CHUNK(struct SaveBlock2, 0), // SECTOR_ID_SAVEBLOCK2

    SAVEBLOCK_CHUNK(struct SaveBlock1, 0), // SECTOR_ID_SAVEBLOCK1_START
    SAVEBLOCK_CHUNK(struct SaveBlock1, 1),
    SAVEBLOCK_CHUNK(struct SaveBlock1, 2),
    SAVEBLOCK_CHUNK(struct SaveBlock1, 3), // SECTOR_ID_SAVEBLOCK1_END

    SAVEBLOCK_CHUNK(struct PokemonStorage, 0), // SECTOR_ID_PKMN_STORAGE_START
    // ...
};

La macro SAVEBLOCK_CHUNK(structure, chunkNum) calcula automáticamente offset y tamaño basándose en sizeof(structure). Si SaveBlock1 ahora ocupa menos de 4 sectores, puedes reducir SECTOR_ID_SAVEBLOCK1_END; si ocupa más, necesitas más sectores (ajusta NUM_SECTORS_PER_SLOT y la distribución).

### Paso 4: Actualizar las STATIC_ASSERT en src/save.c

STATIC_ASSERT(sizeof(struct SaveBlock2) <= SECTOR_DATA_SIZE, SaveBlock2FreeSpace);
STATIC_ASSERT(sizeof(struct SaveBlock1) <= SECTOR_DATA_SIZE * (SECTOR_ID_SAVEBLOCK1_END - SECTOR_ID_SAVEBLOCK1_START + 1), SaveBlock1FreeSpace);
STATIC_ASSERT(sizeof(struct PokemonStorage) <= SECTOR_DATA_SIZE * (SECTOR_ID_PKMN_STORAGE_END - SECTOR_ID_PKMN_STORAGE_START + 1), PokemonStorageFreeSpace);

Si creciste un struct, estas aserciones fallarán en compilación si no hay espacio. O bien:
- Libera más espacio con flags FREE_* en include/config/save.h, o
- Asigna más sectores a ese struct en sSaveSlotLayout[].

### Paso 5: Inicializar los nuevos campos

Busca dónde se inicializa el SaveBlock para una partida nueva (ClearSaveData(), NewGame, o similar) y asigna valores por defecto a los nuevos campos para evitar basura de RAM:

gSaveBlock1Ptr->mutationTier = 0;
gSaveBlock1Ptr->mutationMask = 0;
gSaveBlock1Ptr->staminaCurrent = gSaveBlock1Ptr->staminaMax;

### Paso 6: Actualizar referencias en código existente

Cualquier código que acceda a campos de SaveBlock por offset numérico o por offsetof debe revisarse. Si añadiste campos al final, los offsets de campos existentes no cambian (porque son structs, no arrays plano), pero si usas offsetof(struct SaveBlock2, playerTrainerId[0]) (como hace GetSaveBlocksPointersBaseOffset en src/save.c), sigue funcionando porque offsetof se calcula en compilación.

### Paso 7: Revisar dependencias de ensamblador

Busca archivos .s que referencien gSaveBlock1, gSaveBlock2, o offsets del SaveBlock. Si algún asm lee un campo por offset hardcodeado, actualízalo. Ver sección 7.

### Paso 8: Compilar y validar

1. make clean && make (o el comando de build del repo).
2. Verifica que las STATIC_ASSERT pasen.
3. Arranca en emulador (mGBA/No$GBA), crea una partida nueva, guarda, resetea, carga.
4. Comprueba que los nuevos campos persisten correctamente.

### Paso 9: Documentar

- Actualiza diseño.md para registrar la excepción al principio #1.
- Copia esta guía a docs/SAVEBLOCK_EXPANSION.md (o actualízala).
- Actualiza docs/SUMMARY.md si usas mdbook.

