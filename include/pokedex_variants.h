#ifndef GUARD_POKEDEX_VARIANTS_H
#define GUARD_POKEDEX_VARIANTS_H

#define POKEDEX_VARIANT_COUNT 9
#define POKEDEX_VARIANT_STATS 7 // Six statistics followed by stamina.

struct BoxPokemon;

struct PokedexVariantRange
{
    s16 minimum, maximum;
};

struct PokedexVariantBucket
{
    u16 count;
    struct PokedexVariantRange bonuses[POKEDEX_VARIANT_STATS];
};

// Current owned collection, not a permanent history of encountered variants.
struct PokedexVariantCollection
{
    u16 species;
    u16 total, normal, combined, party, boxes, unknown;
    struct PokedexVariantBucket variants[POKEDEX_VARIANT_COUNT];
};

struct PokedexVariantRecord
{
    u16 species, mutations;
    s16 bonuses[POKEDEX_VARIANT_STATS];
    bool8 egg, invalid, inParty;
};

void PokedexVariants_Init(struct PokedexVariantCollection *collection, u16 species);
void PokedexVariants_Add(struct PokedexVariantCollection *collection, const struct PokedexVariantRecord *record);
u16 PokedexVariants_BadgeAt(u8 index);
// Reads encrypted data from a local copy; invalid checksums cannot alter a source.
void PokedexVariants_AddBoxMon(struct PokedexVariantCollection *collection, const struct BoxPokemon *source, bool8 inParty);
void PokedexVariants_ReadCollection(struct PokedexVariantCollection *collection, u16 species);
u16 PokedexVariants_ScanTotal(void);
// Spend budget only on occupied slots; empty boxes do not add artificial delays.
bool32 PokedexVariants_ScanStep(struct PokedexVariantCollection *collection, u16 *cursor, u16 budget);

// Modal for the regular HGSS Info page. Suspends and restores its owning task.
bool32 PokedexVariants_Open(u16 species, u8 ownerTaskId);

#endif
