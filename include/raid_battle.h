#ifndef GUARD_RAID_BATTLE_H
#define GUARD_RAID_BATTLE_H

#include "global.h"

// Número de rivales en el catálogo de incursiones
#define RAID_POKEMON_COUNT 20
#define RAID_RIVAL_COUNT 5
#define RAID_MAX_GEARS 6

// Dificultades de incursión (según medallas obtenidas)
enum RaidDifficulty
{
    RAID_DIFFICULTY_EASY,    // 5ª medalla
    RAID_DIFFICULTY_MEDIUM,  // 6ª medalla
    RAID_DIFFICULTY_HARD,    // 7ª medalla
    RAID_DIFFICULTY_EXPERT,  // 8ª medalla
    RAID_DIFFICULTY_COUNT,
};

// Estado de una incursión
struct RaidRival
{
    u16 species;           // Especie del Pokémon rival
    u8 level;              // Nivel del rival
    u8 moves[4];           // Movimientos del rival
    u8 heldItem;           // Ítem equipado (0 = ninguno)
    bool8 hasMegaEvolution; // Si puede megaevolucionar
};

struct RaidBoss
{
    u16 species;           // Especie del jefe
    u8 level;              // Nivel del jefe
    u8 moves[4];           // Movimientos del jefe
    u32 mutationMask;      // Mutación aleatoria del jefe
    u8 auraType;           // Tipo de aura del jefe
};

struct RaidRaidData
{
    struct RaidRival rivals[RAID_RIVAL_COUNT]; // 5 rivales
    struct RaidBoss boss;                      // Jefe final
    u8 currentRivalIndex;                      // Rival actual (0-4)
    u8 difficulty;                             // Dificultad actual
    u8 daySeed;                                // Semilla de rotación diaria
    u8 raidRoundsCleared;                      // Rondas completadas (0-3)
    bool8 isActive;                            // Si hay incursión activa
    u8 rewardType;                             // 0 = objetos, 1 = captura
    u16 captureRate;                           // Probabilidad de captura (0-100)
};

// Estado global de incursiones
struct RaidState
{
    struct RaidRaidData raid;
    u8 lastDayIndex;         // Último día procesado (para rotación)
    u8 raidsCompleted;       // Total de incursiones completadas
    u8 raidsWon;             // Total de incursiones ganadas
    u16 gearsUsed;           // Gears usados en incursiones
    u16 megaEvolutionsUsed;  // Megaevoluciones usadas
};

// Prototipos de funciones
void CB2_InitRaidBattle(void);
void OpenRaidBattleFromHFCWatch(void);
void CloseRaidBattle(void);
void RaidBattle_HandleInput(void);
void RaidBattle_UpdateRotation(void);
bool8 RaidBattle_IsDifficultyUnlocked(u8 difficulty);
void RaidBattle_GenerateRaid(u8 difficulty);
void RaidBattle_ApplyMutationToBoss(void);
u16 RaidBattle_CalculateCaptureRate(void);
bool8 RaidBattle_CanUseMegaEvolution(void);

#endif // GUARD_RAID_BATTLE_H