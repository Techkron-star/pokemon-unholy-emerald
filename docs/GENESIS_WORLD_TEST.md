# Genesis World Test — disposable experimental ROM

This branch deliberately activates Genesis Engine at normal map initialization. It is a stress test, not the NG+ world generator. Start a new game or use a copy of a save; the v2 margin change makes terrain differ from the first test build; never overwrite your main save. Do not merge this branch into production.

## Scope
- Deterministically regenerates terrain in 32x32 chunks on eligible land routes, ocean-route islands, and ordinary underground maps. Entire eligible maps are traversed in chunks, even if larger than 64x64. The same layout and seed produce the same overlay except for an explicit landing-position protection (an obstacle is skipped if it would occupy the player's arrival tile).
- Preserves original cities/towns, buildings, map dimensions and authored map connections, water, scripted warp/interaction tiles and special behaviors. Route 101 is kept intact before Birch rescue. Interiors, special map types and noncompatible tilesets are excluded. Caves with coordinate triggers are left alone. Other maps may contain no eligible cells and remain unchanged.
- Reuses only verified General grass and rock tiles or neutral, already authored cave tiles. Original elevation is kept. Candidate blockers must have a completely walkable same-elevation eight-neighbor ring, preserving the preexisting local cardinal connectivity of plain floor. Connected borders retain a seven-cell margin; unconnected edges retain one cell. Event neighborhoods remain protected. This is NOT proof that arbitrary script movement or object-aware paths are safe.
- New obstacles and altered grass distribution are visible; wild species, NPC placements, story, save schema and global route topology are unchanged. No actual new routes or persistent NG+ generation yet.
- Engine failures leave a chunk unchanged. Debugger can inspect gGenesisWorldTestStats (seed, generated/failed/emergency chunks, candidate/protected/changed cells, added obstacles, landing repairs, connected sides and result code).
- Existing save map-view data may override some cells on reload. Test a fresh save, and report camera seam, warp, collision, encounter, performance or save/load issues. Do not use save states from another ROM.

## Build
Existing feat/** ROM workflow compiles and publishes the experimental pokeemerald.gba artifact if successful. The separate core PR is its base. This test branch enables GENESIS_WORLD_TEST_ENABLED only here. Local overlay topology test:
```
gcc -std=c99 -Wall -Wextra -Werror -DGENESIS_HOST -Iinclude src/genesis/genesis_world_test_overlay.c tools/genesis/genesis_world_test_overlay_test.c -o /tmp/genesis-overlay
/tmp/genesis-overlay
```
GBA compilation, ROM-size and EWRAM budgets and emulator gameplay must be confirmed separately. 

## Test iteration 2
Directional margin regression test: a 50x20 east/west-connected route has 216 geometrically eligible cells with the original margin and 648 with directional margins, before event and tile filtering. Preserve the existing seven-cell camera seam wherever a connection exists. The host tests cover all four directions, tiny maps, invalid coordinates and original local topology assertions. This is geometric capacity, not the number of actual cells modified on Route 102. The ROM workflow and emulator QA remain the native verification gate.
