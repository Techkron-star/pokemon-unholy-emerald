#include "global.h"
#include "pokemon.h"
#include "test/test.h"

TEST("All 37 Unholy abilities are distributed to at least one species or form")
{
    static const enum Ability abilities[] = {
        ABILITY_DRAGONBORN, ABILITY_REAPER, ABILITY_DUAL_WIELD, ABILITY_MARATONISTA,
        ABILITY_REACTOR, ABILITY_MAGMA_ARMOR, ABILITY_WORLD_TREE, ABILITY_ABISAL,
        ABILITY_SUPER_BATTERY, ABILITY_BELICO, ABILITY_SLAYER, ABILITY_KNIGHT,
        ABILITY_WIZARD, ABILITY_MALVADO, ABILITY_GREEDY, ABILITY_GLUTTONY,
        ABILITY_ENVY, ABILITY_COLERICO, ABILITY_MANTHRA, ABILITY_FALLEN_ANGEL,
        ABILITY_SAMURAI, ABILITY_GELIDO, ABILITY_BANSHEE, ABILITY_ONI,
        ABILITY_DRAKE, ABILITY_MONJE, ABILITY_STEEL_SCALES, ABILITY_DAWN_LIGHT,
        ABILITY_GREAT_ECLIPSE, ABILITY_GODHAND, ABILITY_TIME_REALM,
        ABILITY_SPACE_REALM, ABILITY_ANTI_REALM, ABILITY_GOD_REALM,
        ABILITY_HYPERBARIC, ABILITY_ANTITHESIS, ABILITY_BLACK_HOLE,
    };
    u32 i, species;
    bool32 found;

    EXPECT_EQ(ARRAY_COUNT(abilities), 37);
    for (i = 0; i < ARRAY_COUNT(abilities); i++)
    {
        found = FALSE;
        for (species = 1; species < NUM_SPECIES; species++)
        {
            if (GetSpeciesAbility(species, 3) == abilities[i])
            {
                found = TRUE;
                break;
            }
        }
        EXPECT(found);
    }
}

TEST("Unholy Reaper is a second hidden ability and does not replace Sceptile's original abilities")
{
    struct Pokemon mon;
    u32 secondHiddenSlot = 3;

    EXPECT_EQ(GetSpeciesAbility(SPECIES_SCEPTILE, 0), ABILITY_OVERGROW);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_SCEPTILE, 2), ABILITY_UNBURDEN);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_SCEPTILE, 3), ABILITY_REAPER);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_LURANTIS, 3), ABILITY_REAPER);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_INTELEON, 3), ABILITY_REAPER);

    CreateMon(&mon, SPECIES_SCEPTILE, 50, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &secondHiddenSlot);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 3);
    EXPECT_EQ(GetMonAbility(&mon), ABILITY_REAPER);
}

TEST("Unholy realm abilities only occupy the designated legendary forms")
{
    EXPECT_EQ(GetSpeciesAbility(SPECIES_ARCEUS_NORMAL, 3), ABILITY_GODHAND);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_ARCEUS_FAIRY, 3), ABILITY_GODHAND);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_DIALGA, 3), ABILITY_TIME_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_DIALGA_ORIGIN, 3), ABILITY_TIME_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_PALKIA, 3), ABILITY_SPACE_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_PALKIA_ORIGIN, 3), ABILITY_SPACE_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_GIRATINA_ORIGIN, 3), ABILITY_ANTI_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_GIRATINA, 3), ABILITY_ANTITHESIS);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_NECROZMA_ULTRA, 3), ABILITY_GOD_REALM);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_GUZZLORD, 3), ABILITY_BLACK_HOLE);
    EXPECT_NE(GetSpeciesAbility(SPECIES_PIKACHU, 3), ABILITY_GODHAND);
}

TEST("Unholy second hidden ability persists through non-mega alternate forms")
{
    EXPECT_EQ(GetSpeciesAbility(SPECIES_TOXTRICITY_LOW_KEY, 3), ABILITY_SUPER_BATTERY);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_TOXTRICITY_AMPED, 3), ABILITY_SUPER_BATTERY);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_PALAFIN_HERO, 3), ABILITY_ABISAL);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_PALAFIN_ZERO, 3), ABILITY_ABISAL);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_EISCUE_NOICE, 3), ABILITY_GELIDO);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_EISCUE_ICE, 3), ABILITY_GELIDO);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_ZAMAZENTA_CROWNED, 3), ABILITY_KNIGHT);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_ZAMAZENTA_HERO, 3), ABILITY_KNIGHT);
}
