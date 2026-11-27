# Genesis World Free — TEST ONLY

This branch is the aggressive alternative to conservative World Test build 2. It replaces the full interior terrain of ordinary routes, compatible underground maps and ocean routes at map initialization, including originally blocked tiles, not merely grass or isolated obstacle overlays. City/town and building geometry is untouched. The Birch rescue map stays authored until that event finishes. Unknown tilesets, tiny maps, underwater and special maps are excluded; caves require usable authored floor/rock IDs.

The existing map IDs, dimensions, connections, scripts, trainers, item events, encounter tables and story flags remain. A fixed experimental master seed derives a layout seed and chunk seeds. Each eligible map is partitioned into actual 8–32 tile chunks, including routes larger than 64 tiles. Shared two-cell center lanes are opened and the complete logical chunk is connected before rendering. Ocean masks denote connected water and generate new islands in the inverse mask. All eight logical profiles may be selected, but their native visuals reuse General tiles or locally verified cave tiles rather than eight unique tilesets. Protected border/event patches and the landing tile can interrupt the generated lanes, so real in-game reachability, elevations, scripts, Surf, warps and reloads are NOT guaranteed. The mode intentionally can introduce sequence breaks and should be tested only on a disposable save. It does not create new global route IDs or alter the save schema.

Stats: gGenesisWorldTestStats includes selected biome, generated/failed/emergency chunks, replaced/changed/opened cells and new blockers. Generation failure leaves the original chunk.

Host verification (run from repository root):
```
gcc -std=c99 -Wall -Wextra -Werror -DGENESIS_HOST -Iinclude src/genesis/genesis_*.c tools/genesis/genesis_free_test_host.c -o /tmp/genesis-free
/tmp/genesis-free
```
The 1,024 generated chunks, 233 axis partition cases, 373,264 cell checks and 74,194 seam cell checks passed locally in GCC, including ASan/UBSan. This tests the logical chunks and renderer, not actual game event paths. CI compiles the separate feat branch and uploads the experimental ROM. Never merge this branch into production.
