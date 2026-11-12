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

static EWRAM_DATA struct RaidState sRaidState = {0};

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

static void CB2_InitRaidBattle(void)
{
    sRaidState.raid.currentRivalIndex = 0;
    sRaidState.raid.raidRoundsCleared = 0;
    sRaidState.raid.isActive = TRUE;
    sRaidState.raid.rewardType = 0;
    sRaidState.raid.captureRate = 0;

    u8 difficulty = RAID_DIFFICULTY_EASY;
    RaidBattle_GenerateRaid(difficulty);

    SetMainCallback2(CB2_InitRaidBattle);
}

void OpenRaidBattleFromHFCWatch(void)
{
    SetMainCallback2(CB2_InitRaidBattle);
}

void CloseRaidBattle(void)
{
    SetMainCallback2(CB2_InitHfcWatch);
}

void RaidBattle_HandleInput(void)
{
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

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        CloseRaidBattle();
        PlaySE(SE_SELECT);
    }
}

void RaidBattle_UpdateRotation(void)
{
}

bool8 RaidBattle_IsDifficultyUnlocked(u8 difficulty)
{
    return TRUE;
}

void RaidBattle_GenerateRaid(u8 difficulty)
{
    u8 i;
    sRaidState.raid.difficulty = difficulty;

    for (i = 0; i < RAID_RIVAL_COUNT; i++)
    {
        u8 speciesIndex = (sRaidState.raid.daySeed + i * 7) % RAID_POKEMON_COUNT;
        sRaidState.raid.rivals[i].species = sRaidPokemonCatalog[speciesIndex];
        sRaidState.raid.rivals[i].level = 40 + (difficulty * 10);
        sRaidState.raid.rivals[i].hasMegaEvolution = (difficulty >= RAID_DIFFICULTY_EXPERT);
    }

    u8 bossSpeciesIndex = (sRaidState.raid.daySeed + 37) % RAID_POKEMON_COUNT;
    sRaidState.raid.boss.species = sRaidPokemonCatalog[bossSpeciesIndex];
    sRaidState.raid.boss.level = 60 + (difficulty * 15);
    sRaidState.raid.boss.mutationMask = 0;

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

    sRaidState.raid.rewardType = (sRaidState.raid.daySeed % 2);
    sRaidState.raid.captureRate = (sRaidState.raid.daySeed % 100) + 1;

    sRaidState.raid.currentRivalIndex = 0;
    sRaidState.raid.raidRoundsCleared = 0;
    sRaidState.raid.isActive = TRUE;
}

void RaidBattle_ApplyMutationToBoss(void)
{
}

u16 RaidBattle_CalculateCaptureRate(void)
{
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

bool8 RaidBattle_CanUseMegaEvolution(void)
{
    return TRUE;
}