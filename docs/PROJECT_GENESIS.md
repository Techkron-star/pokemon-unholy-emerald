# Project Genesis — world discoveries

## P1: registry and Zapdos
- Registry evaluates original story flags/vars plus persistent completion flag; the map and script call the same predicate.
- No new saveblock fields, no network events or RTC dependency. Uses permanent flags 0x91E/0x91F on this integration branch (do not reuse them elsewhere).
- Zapdos: sixth badge AND `VAR_NEW_MAUVILLE_STATE == 2`. Native overworld graphic, provisional position (34,6), level 50. Native legendary battle. Capturing clears the discovery permanently; defeating or fleeing leaves it pending and permits a retry on the next map entry. Generator already completed in existing saves counts.
- Regional marker shows !!! at Mauville, not the species or exact location. Separate purple sprite sheet, deduplicated by section, capped at 8 concurrent markers for P1. Existing bounty marker preserved; offset on overlap. Further pagination or aggregate indicators needed when there are more than 8 pending sections.
- Encounter level, location and retry policy are initial balance proposals.

## Acceptance checks (not executed yet)
- Build ROM and run test/unholy_genesis.c.
- Test with neither prerequisite, each prerequisite independently, both in either order, existing save, capture and re-entry/save/reload, defeat and flee.
- Check sprite at generator, collision and accessibility; confirm map marker in full/zoomed map and simultaneous bounty, resource cleanup, and no marker after capture.
- Verify that a saved game inside New Mauville before this update correctly loads the new map object.
- Verify permanent flag assignments when integrating other branches.

## Next phases
1. Articuno/Moltres and other encounters gated by native progress.
2. Original Aurora/Mystic/Eon tickets and Old Sea Map: in-game acquisition, permanent travel, Deoxys after climate crisis and eight badges, simplified triangle.
3. Unique island encounters, availability audit across all enabled species, fossil market, overworld interactions.
4. Ultra Space using authored maps and variable connections; time distortions and other special ecosystems.
