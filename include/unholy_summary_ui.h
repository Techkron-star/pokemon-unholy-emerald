#ifndef GUARD_UNHOLY_SUMMARY_UI_H
#define GUARD_UNHOLY_SUMMARY_UI_H

bool32 UnholySummary_TryOpen(u8 mode, void *mons, u8 index, u8 last, void (*callback)(void));
void ShowPokemonSummaryScreenLegacy(u8 mode, void *mons, u8 index, u8 last, void (*callback)(void));
void ShowPokemonSummaryScreenLegacyPage(u8 mode, void *mons, u8 index, u8 last, void (*callback)(void), u8 page);

#endif
