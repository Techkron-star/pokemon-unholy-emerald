#include "global.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "test/test.h"

TEST("All 151 Kanto species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_BULBASAUR; i <= NATIONAL_DEX_MEW; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    // Untranslated species still show their original description.
    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_BULBASAUR; i <= NATIONAL_DEX_MEW; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 135 Hoenn species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_TREECKO; i <= NATIONAL_DEX_DEOXYS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_TREECKO; i <= NATIONAL_DEX_DEOXYS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 100 Johto species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_CHIKORITA; i <= NATIONAL_DEX_CELEBI; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    // Species not yet translated still use the original description.
    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_CHIKORITA; i <= NATIONAL_DEX_CELEBI; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 107 Sinnoh species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_TURTWIG; i <= NATIONAL_DEX_ARCEUS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_TURTWIG; i <= NATIONAL_DEX_ARCEUS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 156 Unova species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_VICTINI; i <= NATIONAL_DEX_GENESECT; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_VICTINI; i <= NATIONAL_DEX_GENESECT; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 72 Kalos species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_CHESPIN; i <= NATIONAL_DEX_VOLCANION; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_CHESPIN; i <= NATIONAL_DEX_VOLCANION; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 88 Alola species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_ROWLET; i <= NATIONAL_DEX_MELMETAL; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    // Untranslated alternative forms keep their original English text.
    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_ROWLET; i <= NATIONAL_DEX_MELMETAL; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 96 Galar and Hisui species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_GROOKEY; i <= NATIONAL_DEX_ENAMORUS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_GROOKEY; i <= NATIONAL_DEX_ENAMORUS; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}

TEST("All 120 Paldea species have Spanish Pokedex entries that fit the screen")
{
    bool8 originalLanguage = gSaveBlock2Ptr->optionsLanguageSpanish;

    gSaveBlock2Ptr->optionsLanguageSpanish = TRUE;
    for (u32 i = NATIONAL_DEX_SPRIGATITO; i <= NATIONAL_DEX_PECHARUNT; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        const u8 *description = GetSpeciesPokedexDescription(species);

        EXPECT_NE(species, SPECIES_NONE);
        EXPECT_NE(description, NULL);
        EXPECT_NE(description, gSpeciesInfo[species].description);
        EXPECT_NE(StringLength(description), 0);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), DISPLAY_WIDTH);
    }

    // Only the base forms are translated in this localization pass.
    EXPECT_EQ(GetSpeciesPokedexDescription(SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM),
              gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description == NULL
              ? gSpeciesInfo[SPECIES_NONE].description
              : gSpeciesInfo[SPECIES_ALCREMIE_STRAWBERRY_RUBY_CREAM].description);

    gSaveBlock2Ptr->optionsLanguageSpanish = FALSE;
    for (u32 i = NATIONAL_DEX_SPRIGATITO; i <= NATIONAL_DEX_PECHARUNT; i++)
    {
        enum Species species = NationalPokedexNumToSpecies(i);
        EXPECT_EQ(GetSpeciesPokedexDescription(species), gSpeciesInfo[species].description);
    }
    gSaveBlock2Ptr->optionsLanguageSpanish = originalLanguage;
}
