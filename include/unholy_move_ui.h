#ifndef GUARD_UNHOLY_MOVE_UI_H
#define GUARD_UNHOLY_MOVE_UI_H

struct Pokemon;
void UnholyMoveUi_Open(struct Pokemon *mon, u8 partySlot, void (*onReturn)(void));

#endif
