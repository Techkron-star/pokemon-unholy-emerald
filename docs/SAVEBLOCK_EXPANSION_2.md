# Guía de ampliación del SaveBlock — Pokémon Unholy Emerald (parte 2 de 3)

---

## 6. Cómo actualizar los offsets de campos que se desplazan

### Cuando los offsets NO cambian (añadir al final)

Si añadiste campos al final del struct, los offsets de todos los campos existentes se mantienen igual. El código que accede a gSaveBlock1->campoViejo sigue funcionando sin cambios.

### Cuando los offsets SÍ cambian (insertar en medio o reordenar)

Si insertaste un campo en medio del struct, todos los campos posteriores se desplazan y cualquier acceso por offset numérico hardcodeado se romperá.

**Cómo detectar desplazamientos:**

1. Busca offsetof(struct SaveBlockX, campo) — estos se recalculan solos en compilación, son seguros.
2. Busca offsets hardcodeados en asm (.s) o en C como punteros casts: (u16 *)(gSaveBlock1 + 0xNN).
3. Busca #define que calculen offsets manualmente.

**Cómo corregir:**

- Reemplaza accesos hardcodeados por acceso directo al struct: gSaveBlock1Ptr->nuevoCampo.
- Si necesitas el offset en tiempo de ejecución, usa offsetof(struct SaveBlock1, campo).
- En asm, usa etiquetas símbolos generadas por el compilador en lugar de offsets numéricos, o actualiza los valores numéricos con la nueva diferencia en bytes.

**Ejemplo de corrección:**

Antes (roto tras desplazamiento):

// asm: lee el byte en offset 0x50 de SaveBlock1 (ya no es el campo correcto)
ldrb r0, [r1, #0x50]

Después (correcto):

// C: acceso por nombre de campo
u8 valor = gSaveBlock1Ptr->miCampo;

O en asm, pasar el offset calculado por offsetof desde C.

---

## 7. Scripts y ensamblador que dependen del SaveBlock

### Buscar dependencias

Busca en todo el repositorio referencias a gSaveBlock1, gSaveBlock2, gPokemonStorage, SaveBlock1, SaveBlock2:

# Linux/macOS
grep -rn "SaveBlock" --include="*.c" --include="*.h" --include="*.s" --include="*.inc" .

# Windows (PowerShell)
Select-String -Path . -Pattern "SaveBlock" -Include "*.c","*.h","*.s","*.inc"

Presta especial atención a:
- src/save.c — UpdateSaveAddresses(), GetSaveBlocksPointersBaseOffset(), sSaveSlotLayout[].
- asm/ — cualquier rutina asm que lea direcciones del save.
- include/constants/*.inc — constantes de offset para asm.

### Cómo ajustar asm

Si una rutina asm lee un campo del SaveBlock por offset hardcodeado, tienes dos opciones:

**Opción A (recomendada):** Crear una función wrapper en C que devuelva el valor, y llamarla desde asm.

// include/save.h
u8 GetSaveblockMutationTier(void);

// src/save.c
u8 GetSaveblockMutationTier(void)
{
    return gSaveBlock1Ptr->mutationTier;
}

; asm/alguna_rutina.s
bl GetSaveblockMutationTier

**Opción B:** Calcular el offset con offsetof y pasarlo como inmediato.

// Desde C, al llamar a la función asm:
u16 offset = offsetof(struct SaveBlock1, mutationTier);
RutinaAsm(gSaveBlock1Ptr, offset);

; asm/alguna_rutina.s
; r0 = puntero a SaveBlock1, r1 = offset
ldrb r0, [r0, r1]

### Scripts de juego (eventos)

Los scripts de eventos del juego (.event / comando call / setflag) normalmente no tocan el SaveBlock directamente; operan sobre flags y variables del juego que sí viven en el SaveBlock. Si tus nuevos campos se exponen como flags/variables de script, asegúrate de que sus offsets en el SaveBlock coincidan con lo que el motor de scripts espera. Revisa include/event_data.h y src/event_data.c si añades nuevas variables de juego persistentes.

---

## 8. Qué se rompe con saves viejos (y por qué no importa)

### Qué se rompe

1. Tamaño diferente: El SaveBlock ampliado tiene más bytes que antes. El sistema de sectores lee/escribe según sSaveSlotLayout[], que ahora cubre un rango distinto.
2. Offsets desplazados: Incluso si el tamaño fuera el mismo, insertar campos mueve los datos. Un save viejo interpretado con el nuevo struct leería campos en posiciones erróneas (p. ej., leería "stamina" donde antes había "nombre del entrenador").
3. Checksums inválidos: Cada sector tiene un checksum calculado sobre sus datos. Los saves viejos tienen checksums calculados sobre el layout antiguo; el nuevo código los rechazará como corruptos.
4. STATIC_ASSERT y layout: El código nuevo asume el nuevo tamaño; si intentas cargar un save viejo, los datos no encajan en los sectores esperados.

### Por qué no importa en este caso

- Eres el único desarrollador y no te importan las partidas guardadas viejas.
- Estás en una rama propia; no hay usuarios finales con saves existentes que preservar.
- El repositorio no distribuye saves de prueba que deban seguir funcionando.

**Acción recomendada:** Al hacer el commit de la ampliación, añade un aviso claro en el mensaje de commit y en diseño.md:

* Rompe compatibilidad con saves previos a la ampliación del SaveBlock.
* Borra cualquier archivo .sav antiguo antes de probar.

Si en el futuro quisieras compatibilidad, tendrías que escribir un código de migración ("save migration") que lea el layout antiguo y lo traduzca al nuevo, pero eso está fuera del alcance de este proyecto.

---

## 9. Ejemplos de código (C y asm)

### 9.1. Antes y después de un struct de SaveBlock

**Antes** (include/save.h, extracto ilustrativo):

struct SaveBlock1
{
    /* 0x000 */ struct SaveBlock1MapPosition mapPosition;
    /* 0x010 */ u8 playerName[PLAYER_NAME_LENGTH];
    /* 0x01C */ u16 playerTrainerId;
    /* 0x01E */ u16 playTimeHours;
    /* 0x020 */ u16 playTimeMinutes;
    /* 0x022 */ u16 playTimeSeconds;
    /* 0x024 */ u16 playTimeFrames;
    /* 0x026 */ u8 options;
    /* ... más campos ... */
}; // tamaño: 2516 bytes (con FREE_* en FALSE)

**Después** (añadido al final):

struct SaveBlock1
{
    /* 0x000 */ struct SaveBlock1MapPosition mapPosition;
    /* 0x010 */ u8 playerName[PLAYER_NAME_LENGTH];
    /* 0x01C */ u16 playerTrainerId;
    /* 0x01E */ u16 playTimeHours;
    /* 0x020 */ u16 playTimeMinutes;
    /* 0x022 */ u16 playTimeSeconds;
    /* 0x024 */ u16 playTimeFrames;
    /* 0x026 */ u8 options;
    /* ... más campos existentes ... */

    /* --- NUEVOS CAMPOS (mutaciones, stamina, Horizon, Gears) --- */
    /* 0x??? */ u16 mutationTier;
    /* 0x??? */ u32 mutationMask;
    /* 0x??? */ u8  mutationBadges;
    /* 0x??? */ u16 staminaCurrent;
    /* 0x??? */ u16 staminaMax;
    /* 0x??? */ u32 staminaCooldown;
    /* 0x??? */ u32 horizonSeedDiscs;
    /* 0x??? */ u32 horizonSeedItems;
    /* 0x??? */ u16 horizonPokemonOffer1;
    /* 0x??? */ u16 horizonPokemonOffer2;
    /* 0x??? */ u8  equippedGearSlot;
    /* 0x??? */ u16 gearCharges[6];
}; // tamaño: > 2516 bytes (ajustar según flags FREE_*)

> Los comentarios /* 0x??? */ son indicativos; los offsets reales los calcula el compilador. No los pongas a mano.

### 9.2. Cómo acceder a los nuevos campos (C)

#include "save.h"

// Ejemplo: leer el tier de mutación guardado
u8 GetPlayerMutationTier(void)
{
    return gSaveBlock1Ptr->mutationTier;
}

// Ejemplo: establecer stamina máxima
void SetPlayerStaminaMax(u16 max)
{
    gSaveBlock1Ptr->staminaMax = max;
    gSaveBlock1Ptr->staminaCurrent = max;
}

// Ejemplo: avanzar la semilla del mercado Horizon (DISCS)
void AdvanceHorizonDiscsSeed(void)
{
    gSaveBlock1Ptr->horizonSeedDiscs += 0x9E3779B9;  // constante de semilla, ejemplo
}

### 9.3. Cómo acceder desde ensamblador (ARM/Thumb GBA)

@ asm/mutation_utils.s
@ Lee el mutationTier desde SaveBlock1 y lo devuelve en r0
GetSaveblockMutationTier:
    ldr r1, =gSaveBlock1Ptr
    ldr r1, [r1]              @ r1 = puntero al struct SaveBlock1
    ldrh r0, [r1, #OFFSET_MUTATION_TIER]  @ carga u16 en el offset del campo
    bx lr

Donde OFFSET_MUTATION_TIER se obtiene en C con offsetof(struct SaveBlock1, mutationTier) y se pasa al asm, o se define como constante tras verificar el offset con el mapa del símbolo (pokemon-unholy-emerald.map o similar tras compilar).

Mejor práctica: evita hardcodear offsets en asm. Usa una función C wrapper (ver sección 7, Opción A).

### 9.4. Actualizar referencias en código existente

Supón que existe esta función antigua:

// Código existente antes de la ampliación
void SomeOldFunction(void)
{
    u8 opt = gSaveBlock1Ptr->options;
    // ...
}

Tras la ampliación (campos añadidos al final), options sigue en el mismo offset, así que no hace falta tocar esta función. Solo añade nuevo código que use los nuevos campos:

// Nuevo código tras la ampliación
void UpdateStamina(void)
{
    if (gSaveBlock1Ptr->staminaCurrent < gSaveBlock1Ptr->staminaMax)
    {
        if (gSaveBlock1Ptr->staminaCooldown == 0)
            gSaveBlock1Ptr->staminaCurrent++;
    }
}

Si por el contrario insertaste un campo antes de options, entonces options se desplazó y cualquier acceso directo sigue funcionando (porque es acceso por nombre de campo en C), pero cualquier acceso por offset numérico hardcodeado se rompe. Reemplaza esos accesos por nombre de campo.

### 9.5. Inicializar nuevos campos en una partida nueva

Busca la función que prepara un SaveBlock1 limpio (en este repo suele estar en src/save.c o src/new_game.c) y añade:

gSaveBlock1Ptr->mutationTier = 0;
gSaveBlock1Ptr->mutationMask = 0;
gSaveBlock1Ptr->mutationBadges = 0;
gSaveBlock1Ptr->staminaCurrent = gSaveBlock1Ptr->staminaMax;
gSaveBlock1Ptr->staminaCooldown = 0;
gSaveBlock1Ptr->horizonSeedDiscs = 0;
gSaveBlock1Ptr->horizonSeedItems = 0;
gSaveBlock1Ptr->horizonPokemonOffer1 = 0;
gSaveBlock1Ptr->horizonPokemonOffer2 = 0;
gSaveBlock1Ptr->equippedGearSlot = 0xFF;  // 0xFF = ningún gear equipado
for (int i = 0; i < 6; i++)
    gSaveBlock1Ptr->gearCharges[i] = 0;