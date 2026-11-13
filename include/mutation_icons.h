#ifndef GUARD_MUTATION_ICONS_H
#define GUARD_MUTATION_ICONS_H

struct Sprite;

// Existing mutation bits, enumerated without changing the Pokémon or RNG.
u16 MutationIcon_BadgeAt(u16 badges, u32 ordinal);
u8 MutationIcon_Create(u16 badge, s16 x, s16 y, u8 priority);
void MutationIcon_Destroy(u8 spriteId);
void MutationIcon_ResetBattle(void);
void MutationIcon_ClearBattle(void);
bool32 MutationIcon_UpdateBattle(u8 battler, const struct Sprite *box, u16 badges, s16 x, s16 y);
bool32 MutationIcon_BattleVisible(u8 battler);
const u8 *MutationIcon_BattleLabel(u16 badges);
bool32 MutationIcon_IsPaletteTag(u16 tag);
bool32 MutationIcon_ReleaseBattlePalette(void);

#endif
