# Genesis Terrain Renderer — experimental pass 1

Separate test-only branch atop World Free. Retains the same world seed for A/B comparisons. Geometry remains the previous Genesis chunk generator; this change addresses representation.

- Before generation, scan the original active layout. Classify each cell as ground, wall, water or other using native collision and behavior. Learn up to 256 eight-neighbor patterns each for walls, water and floor transitions. Restrict learned entries to the original layout's own tile IDs, compatible behavior/elevation and relevant tileset bank. Deterministic bounded majority candidates, not hand-authored exhaustive atlas.
- Store generated semantic classes and render after all chunks, so the neighbor lookup sees both sides of a chunk boundary. Generated cells only; protected objects, warps, story patches and seams are not rewritten. Wall rules prefer exact patterns and then the same cardinal neighborhood; unknown patterns use a solid-interior or original fallback. Water/floor transitions use exact matches only. Collision/elevation bits never change in this final pass. This is an initial example-based autotiler, not a guaranteed complete visual grammar: incomplete source patterns can still yield imperfect corners and shores, and multi-metatile walls/animated transitions require explicit atlas work.
- Saved-map-view restoration skips cells marked generated on the active layout, avoiding stale save-view tiles replacing fresh geometry. This is deliberately experimental and can discard temporary visual changes on generated cells; never use the main save. Save states bypass normal map initialization; enter a map again after switching ROMs.
- Additional static EWRAM: 10,240 byte semantic grid + 3,072 byte rule tables and six bytes of layout metadata, plus alignment/diagnostics. Confirm link map and ROM runtime. No per-cell heap allocation.
- Debug counters: learnedPatterns, autotiledCells (second pass), replacedCells (first pass), generatedChunks, etc. For Crystal Cave use the same fixed world seed as the free build and compare on fresh map entry. City and special map limitations from World Free remain. No new routes or true cross-map generation.
- Host test from repository root:
```
gcc -std=c99 -Wall -Wextra -Werror -DGENESIS_HOST -Iinclude src/genesis/genesis_terrain.c tools/genesis/genesis_terrain_test.c -o /tmp/genesis-terrain && /tmp/genesis-terrain
```
Passed 132,608 assertions plus edge cases on GCC and ASan/UBSan: all 256 neighborhood masks, exact and cardinal fallback selection, all 65,536 packed-cell values and invalid IDs. This verifies the pure selector, not native visuals. ROM build and emulator QA pending. Never merge into production.
