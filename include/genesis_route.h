#ifndef GUARD_GENESIS_ROUTE_H
#define GUARD_GENESIS_ROUTE_H
#include "genesis_core.h"

/* Template-first land route grammar. Immutable authored layout, exact
 * neutral ground/grass pair, deterministic regional vegetation patches.
 * Never changes collision/elevation, water, ledges, walls or other IDs.
 * Out-of-bounds / invalid input returns original when addressable, else 0.
 * Caller protects events, map seams, arrival and scripted cells. */
u16 GenesisRoute_PlanCell(const u16 *authored, u16 w, u16 h, u16 x, u16 y,
                          u16 floor, u16 grass, u32 seed);
#endif
