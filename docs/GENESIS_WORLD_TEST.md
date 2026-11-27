# Genesis World Test — disposable experimental ROM

This branch deliberately activates Genesis Engine at normal map initialization. It is a stress test, not the NG+ world generator. Start a new game or use a copy of a save; never overwrite your main save. Do not merge this branch into production.

## Scope
- Deterministically regenerates terrain in 32x32 chunks on eligible land routes, ocean-route islands, and ordinary underground maps. Entire eligible maps are traversed in chunks, even if larger than 64x64. The same layout and seed produce the same overlay.
- Preserves original cities/towns, buildings, map dimensions and authored map connections, water, scripted warp/interaction tiles and special behaviors. Route 101 is kept intact before Birch rescue. Interiors, special map types and noncompatible tilesets are excluded. Caves with coordinate triggers are left alone. Other maps may contain no eligible cells and remain unchanged.
- Reuses only verified General grass and rock tiles or neutral, already authored cave tiles. Original elevation is kept. Candidate blockers must have a completely walkable same-elevation eight-neighbor ring, preserving the preexisting local cardinal connectivity of plain floor. Borders are untouched (7 cells), as are event neighborhoods. This is NOT proof that arbitrary script movement or object-aware paths are safe.
- New obstacles and altered grass distribution are visible; wild species, NPC placements, story, save schema and global route topology are unchanged. No actual new routes or persistent NG+ generation yet.
- Engine failures leave a chunk unchanged. Debugger can inspect gGenesisWorldTestStats (seed, generated/failed chunks, changed cells and added obstacles).
- Existing save map-view data may override some cells on reload. Test a fresh save, and report camera seam, warp, collision, encounter, performance or save/load issues. Do not use save states from another ROM.

## Build
Existing feat/** ROM workflow compiles and publishes the experimental pokeemerald.gba artifact if successful. The separate core PR is its base. This test branch enables GENESIS_WORLD_TEST_ENABLED only here. Local overlay topology test:
```
gcc -std=c99 -Wall -Wextra -Werror -DGENESIS_HOST -Iinclude src/genesis/genesis_world_test_overlay.c tools/genesis/genesis_world_test_overlay_test.c -o /tmp/genesis-overlay
/tmp/genesis-overlay
```
GBA compilation, ROM-size and EWRAM budgets and emulator gameplay must be confirmed separately. 
