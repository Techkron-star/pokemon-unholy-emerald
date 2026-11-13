#include "global.h"
#include "raid_battle.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "data.h"
#include "menu.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "text.h"
#include "window.h"

// Estado del sistema de incursiones
static EWRAM_DATA struct RaidState sRaidState = {0};

// Catálogo de 20 especies para incursiones
static const u16 sRaidPokemonCatalog[RAID_POKEMON_COUNT] = {
    SPECIES_PIKACHU,
    SPECIES_CHARIZARD,
    SPECIES_BLASTOISE,
    SPECIES_VENUSAUR,
    SPECIES_ALAKAZAM,
    SPECIES_GENGAR,
    SPECIES_DRAGONITE,
    SPECIES_TYRANITAR,
    SPECIES_SLAKING,
    SPECIES_SALAMENCE,
    SPECIES_METAGROSS,
    SPECIES_LATIOS,
    SPECIES_LATIAS,
    SPECIES_KYOGRE,
    SPECIES_GROUDON,
    SPECIES_RAYQUAZA,
    SPECIES_JIRACHI,
    SPECIES_DEOXYS,
    SPECIES_LUGIA,
    SPECIES_HO_OH,
};

// Inicializar la pantalla de Incursiones
static void CB2_InitRaidBattle(void)
{
    // Inicializar estado
    sRaidState.raid.currentRivalIndex = 0;
    sRaidState.raid.raidRoundsCleared = 0;
    sRaidState.raid.isActive = TRUE;
    sRaidState.raid.rewardType = 0;
    sRaidState.raid.captureRate = 0;

    // Determinar dificultad basada en medallas obtenidas
    // TODO: Leer medallas obtenidas del jugador
    u8 difficulty = RAID_DIFFICULTY_EASY; // Por defecto fácil

    // Generar incursión
    RaidBattle_GenerateRaid(difficulty);

    SetMainCallback2(CB2_InitRaidBattle);
}

// Abrir Incursiones desde HFC Watch
void OpenRaidBattleFromHFCWatch(void)
{
    SetMainCallback2(CB2_InitRaidBattle);
}

// Cerrar Incursiones de forma segura
void CloseRaidBattle(void)
{
    // Reconstruir el campo y el menú HFC Watch
    SetMainCallback2(CB2_InitHfcWatch);
}

// Manejo de entrada de usuario en Incursiones
void RaidBattle_HandleInput(void)
{
    // Navegación entre rivales (izquierda/derecha)
    if (JOY_NEW(DPAD_LEFT))
    {
        if (sRaidState.raid.currentRivalIndex > 0)
            sRaidState.raid.currentRivalIndex--;
        else
            sRaidState.raid.currentRivalIndex = RAID_RIVAL_COUNT - 1;
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (sRaidState.raid.currentRivalIndex < RAID_RIVAL_COUNT - 1)
            sRaidState.raid.currentRivalIndex++;
        else
            sRaidState.raid.currentRivalIndex = 0;
        PlaySE(SE_SELECT);
    }

    // A para iniciar combate contra rival actual
    if (JOY_NEW(A_BUTTON))
    {
        // TODO: Iniciar batalla contra el rival actual
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        CloseRaidBattle();
        PlaySE(SE_SELECT);
    }
}

// Actualizar rotación diaria de incursiones
void RaidBattle_UpdateRotation(void)
{
    // TODO: Obtener fecha actual del reloj del juego
    // u8 currentDay = GetCurrentDayIndex();
    // if (currentDay != sRaidState.lastDayIndex)
    // {
    //     sRaidState.lastDayIndex = currentDay;
    //     RaidBattle_GenerateRaid(sRaidState.raid.difficulty);
    // }
}

// Verificar si una dificultad está desbloqueada
bool8 RaidBattle_IsDifficultyUnlocked(u8 difficulty)
{
    // 5ª medalla = Easy, 6ª = Medium, 7ª = Hard, 8ª = Expert
    // TODO: Verificar medallas obtenidas del jugador
    return TRUE; // Placeholder
}

// Generar una nueva incursión
void RaidBattle_GenerateRaid(u8 difficulty)
{
    u8 i;
    sRaidState.raid.difficulty = difficulty;

    // Seleccionar 5 rivales aleatorios del catálogo
    for (i = 0; i < RAID_RIVAL_COUNT; i++)
    {
        u8 speciesIndex = (sRaidState.raid.daySeed + i * 7) % RAID_POKEMON_COUNT;
        sRaidState.raid.rivals[i].species = sRaidPokemonCatalog[speciesIndex];
        sRaidState.raid.rivals[i].level = 40 + (difficulty * 10); // Escala según dificultad
        sRaidState.raid.rivals[i].hasMegaEvolution = (difficulty >= RAID_DIFFICULTY_EXPERT);
    }

    // Generar jefe con mutación aleatoria
    u8 bossSpeciesIndex = (sRaidState.raid.daySeed + 37) % RAID_POKEMON_COUNT;
    sRaidState.raid.boss.species = sRaidPokemonCatalog[bossSpeciesIndex];
    sRaidState.raid.boss.level = 60 + (difficulty * 15);
    sRaidState.raid.boss.mutationMask = 0;

    // Asignar mutación aleatoria al jefe
    u8 mutationRoll = (sRaidState.raid.daySeed * 17) % 9;
    switch (mutationRoll)
    {
        case 0: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_ALPHA; break;
        case 1: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_HYPER; break;
        case 2: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_SUPER; break;
        case 3: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_OVERLOADED; break;
        case 4: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_ENRAGED; break;
        case 5: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_ANCESTRAL; break;
        case 6: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_TITAN; break;
        case 7: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_ASCENDIDO; break;
        case 8: sRaidState.raid.boss.mutationMask |= MUTATION_MASK_OMEGA; break;
    }

    // Determinar tipo de recompensa
    sRaidState.raid.rewardType = (sRaidState.raid.daySeed % 2);
    sRaidState.raid.captureRate = (sRaidState.raid.daySeed % 100) + 1; // 1-100%

    sRaidState.raid.currentRivalIndex = 0;
    sRaidState.raid.raidRoundsCleared = 0;
    sRaidState.raid.isActive = TRUE;
}

// Aplicar mutación al jefe
void RaidBattle_ApplyMutationToBoss(void)
{
    // TODO: Aplicar efectos de mutación al jefe en batalla
    // Según la máscara de mutación, aplicar bonificaciones correspondientes
}

// Calcular probabilidad de captura
u16 RaidBattle_CalculateCaptureRate(void)
{
    // Probabilidad base basada en dificultad
    u16 baseRate = 50;
    u8 difficulty = sRaidState.raid.difficulty;

    switch (difficulty)
    {
        case RAID_DIFFICULTY_EASY:   baseRate = 60; break;
        case RAID_DIFFICULTY_MEDIUM: baseRate = 45; break;
        case RAID_DIFFICULTY_HARD:   baseRate = 35; break;
        case RAID_DIFFICULTY_EXPERT: baseRate = 25; break;
    }

    return baseRate;
}

// Verificar si se puede usar megaevolución
bool8 RaidBattle_CanUseMegaEvolution(void)
{
    // Máximo una megaevolución rival por combate
    // El jugador puede megaevolucionar una vez
    // Las dificultades avanzadas permiten que el rival también megaevolucione
    return TRUE;
}