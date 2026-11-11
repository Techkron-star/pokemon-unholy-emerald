#ifndef GUARD_UNHOLY_MON_SUMMARY_H
#define GUARD_UNHOLY_MON_SUMMARY_H

// Read-only snapshot shared by future Summary / Stats / Stamina / Mutations UI.
// These values are derived from the live Pokémon; they do NOT create any new
// save fields, mutate an encrypted BoxPokemon, or consume battle state.
struct UnholyMonSummary
{
    u32 species;
    u32 level;
    u32 stats[NUM_STATS]; // HP, Attack, Defense, Speed, Sp. Atk, Sp. Def
    u32 ivs[NUM_STATS];   // 0-31; original storage and mechanics unchanged
    u32 evs[NUM_STATS];   // 0-252; Omega/Ascended may total 1512
    s32 mutationBonusPercent[NUM_STATS];
    u32 staminaMax;
    u32 mutationBadges;
    u32 gearId;
};

// The caller owns 'out'; no allocation, redraw or global battle change.
// An uninitialized/empty Pokémon is rejected without touching 'out'.
bool32 UnholyMonSummary_Build(struct Pokemon *mon, struct UnholyMonSummary *out);

#endif // GUARD_UNHOLY_MON_SUMMARY_H
