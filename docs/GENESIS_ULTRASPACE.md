# Genesis XIII — Ultra Wormhole overworld resource

## Implemented in this branch
- `OBJ_EVENT_GFX_ULTRA_WORMHOLE`: provisional 32x32 static overworld graphic, 512 bytes of native tiled 4bpp graphics and one 16-entry BGR555 palette (index 0 transparent). No shadow or tracks. Palette tag 0x1135 and native 32x32 OAM/subsprite layout. Stored as native C arrays to avoid introducing a new graphic conversion rule.
- New graphic appended before `NUM_OBJ_EVENT_GFX`; existing fixed object IDs are preserved. Audit dynamic graphics IDs derived from that count when integrating and testing older saves.
- No portal placed on a route yet; no animation or map transfer behavior claimed. Validate graphics loading, palette conflicts, collision and interaction using an authored demo map before release.

## Planned contract (not implemented)
- Portal controller owns a stable portal ID, source map/local coordinates, an entry seed, biome/difficulty profile and an expedition version. It must not own procedural terrain algorithms.
- The generator receives an immutable expedition seed and profile, builds and validates the destination; only then may the controller commit the transition. Invalid generation leaves the player safely at the source.
- Use a single active expedition initially. Reserve a deterministic safe return point on the source map before entry. No overwriting normal last-heal/escape state without an explicit design. Validate source and destination, including warp/position/elevation and map-local events. No new persistent save fields in this PR.
- Marker `!!!`, repeatable appearances, post-League gates, Ultra Beast encounters and rewards remain later integration work. Decide abandonment and save/resume rules before exposing expeditions.
- INFINITY Genesis Engine and its graph/terrain APIs are developed separately; do not create a second procedural engine here.

## QA
- Build, inspect 32x32 graphic on GBA/emulator, check palette and object slots, sprite layering and interactions. Test old save loading and dynamic graphics IDs. No build or emulator execution performed in this branch.
