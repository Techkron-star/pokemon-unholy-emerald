#ifndef GUARD_UNHOLY_STARTERS_H
#define GUARD_UNHOLY_STARTERS_H

#define UNHOLY_STARTER_TYPE_COUNT 18

// Returns SPECIES_NONE when no preferences have been generated yet.
u16 UnholyGetStarterOption(u16 slot);
void UnholySetFavoriteStarterType(void);
void UnholyStarterOptionsReady(void);

#endif // GUARD_UNHOLY_STARTERS_H
