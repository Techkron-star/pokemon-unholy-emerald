# Genesis Engine 1.1 — Stabilization

Corrected implementation based on the three DeepSeek design/code deliveries for Pokémon Unholy Emerald / INFINITY. Nine procedural subsystems plus a separate Emerald adapter. This version is not seed-compatible with 1.0. No save migration is needed because 1.0 was not integrated.

## Build and run (host)

```
make test             # 10,000 seeds for each of 8 profiles + 1,026 extra cases
make asan             # same suite with ASan and UBSan
make fast             # shorter suite
make native-test      # native branch against minimal mock headers (not a GBA build)
make edge-test        # forced fallback, stream isolation, overlap checks
make demo SEED=0x12345678 BIOME=0
```

On Linux, GCC or Clang; no external libraries. `-DGENESIS_HOST` and `-D__GENESIS_HOST__` are accepted. The demo's profile IDs are 0 meadow, 1 forest, 2 desert, 3 cave, 4 glacier, 5 volcano, 6 ruins, 7 alien. The ASCII map contains `#` blocked rock, `~` blocked water, `.` ground, `"` vegetation, `E` entry, `X` exit, `e/n/o/p/v/B` population metadata. The graph is printed as an adjacency list.

## API / storage

```c
#include "genesis_api.h"
GENESIS_DECLARE_BUFFERS(bufs, 32, 32);  /* file scope */
static GenesisWorld world;             /* use EWRAM_DATA on GBA */
GenesisConfig cfg = {0};
/* inside a function: */
cfg.master_seed = 0x12345678u;
cfg.w = cfg.h = 32;
cfg.biome = GENESIS_BIOME_MEADOW;
cfg.entrances = cfg.exits = 1;
cfg.spawn_cfg = (GenesisSpawnConfig){8, 2, 3, 1, 1, 1};
cfg.generate_graph = GTRUE;
cfg.graph_nodes = 20;
if (!GenesisGenerate(&world, &cfg, &bufs)) { /* abort entry */ }
```

Buffer capacity is in cells, not bytes. Supply distinct, aligned buffers of at least capacity elements; overlapping buffers and invalid dimensions/counts are rejected by the top-level API. Dimensions 8..64; population ≤64, ends ≤4 each, optional graph 5..30 nodes. Direct lower-level APIs require initialized, adequately sized buffers. World owns a copy of the profile and config values; `world.cfg.profile_override` is cleared (no retained pointer). Status OK = ordinary generation; EMERGENCY = structured deterministic fallback; FAILED = no valid result, never commit. Population metadata does not spawn real game entities.

9 bytes/cell: mask, scratch, parent (u8); labels, cells, queue (u16). 32×32 = 9,216 bytes, 64×64 = 36,864. On host, `sizeof(GenesisWorld)=1192`, `GenesisBSP=890`, `GenesisGraph=214`, `GenesisMap=96`; target ABI and total free EWRAM must be measured in ROM. The GBA declaration macro puts buffers in EWRAM; BSP workspace is static EWRAM on native build. Generate on the main thread, not in interrupts. No heap or unbounded recursion. Do not allocate the large buffers on stack. Current FBM uses 64-bit operations; actual CPU budget on ARM7TDMI is still unmeasured.

## Emerald adapter

The host defaults have logical placeholder tiles and cannot be committed to a real map. For General tileset meadow/forest, the native helper `Genesis_GetGeneralProfile(id, &profile)` checks the active primary tileset and supplies verified General metatile labels. For other biomes, provide an explicit compatible profile; do not reuse placeholder IDs. Assign `cfg.profile_override = &profile` before generation. The adapter checks exact active layout dimensions, padded width/height, total capacity, and status. It writes only `[7,7]` through `[7+w-1,7+h-1]`, with native padded stride; it does not edit ROM. `Genesis_CommitToMap(&world)` returns success/failure, and is false on host. Call only on a dedicated procedural map after initialization and before drawing. The mock validates this path but does not validate rendering, tileset transitions, scripts, player placement, wild encounters, object limits or save/resume. Those require actual ROM compilation/emulator testing and a map/warp controller. Do not call this on an authored story map. `Genesis_CommitToLayout` is separately testable on host.

The graph guarantees one exit edge through the boss node and graph connectivity, not combat victory; the future portal controller must enforce the boss-defeated flag. A population position is reachable on terrain before actual blocking NPCs/objects; the integration layer must validate interaction and paths after spawning game objects. Separate RNG seeds (including a dedicated graph stream) isolate terrain/graph from population; categories compete for free cells and can therefore affect later categories' placements. Decoration is not yet implemented. Determinism requires the version, complete configuration and profile, not the seed alone.

## Corrections

- Host macro consistency, fixed-point noise scaling, normalized FBM and clamped arithmetic.
- CA exterior is blocked, BSP stores 63 nodes for up to 32 leaves; room margins avoid overly open maps.
- 16-bit region IDs and BFS queue, bounded orthogonal repair, deterministic distinct endpoints with connecting corridors.
- Structured fallback, density range 20–80%, retry cap, failure propagation; no silently missing population.
- Dedicated graph RNG, exact requested node counts, bounded depth, reciprocal edges, no exit bypass.
- Exact population counts, reserved endpoints, multiple/zero bosses, safe configuration and buffer checks.
- Native layout commit with offsets, real General profile, host and mock validation.

## Integration boundary

No new route, warp, event, object, encounter, reward, save field or NG+ feature is enabled by this version. The kernel is ready to be called by those controllers. Eight logical terrain profiles exist; only meadow/forest have a concrete native General tileset mapping. A real GBA build, timing and emulator QA are mandatory before merging into a playable ROM.
