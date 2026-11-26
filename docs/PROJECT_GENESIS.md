# Project Genesis — world discoveries

## P1: registry and Zapdos
- Registry evaluates original story flags/vars plus persistent completion flag; the map and script call the same predicate.
- No new saveblock fields, no network events or RTC dependency. Uses permanent flags 0x91E/0x91F on this integration branch (do not reuse them elsewhere).
- Zapdos: sixth badge AND `VAR_NEW_MAUVILLE_STATE == 2`. Native overworld graphic, provisional position (34,6), level 50. Native legendary battle. Capturing clears the discovery permanently; defeating or fleeing leaves it pending and permits a retry on the next map entry. Generator already completed in existing saves counts.
- Regional marker shows !!! at Mauville, not the species or exact location. Separate purple sprite sheet, deduplicated by section, capped at 8 concurrent markers for P1. Existing bounty marker preserved; offset on overlap. Further pagination or aggregate indicators needed when there are more than 8 pending sections.
- Encounter level, location and retry policy are initial balance proposals.

## Regional sequel rule
- A static encounter with an official regional form may have a second registry entry at the same location, requiring its original Genesis capture flag AND every badge. This is independent of the shared National Dex entry; trading or defeating the original does not count.
- Galarian Zapdos: level 65 (provisional), same position as original on next map entry, native species follower graphics, separate completion and visibility flags (0x020/0x021).
- The map marker becomes available again (or stays pending when all badges are already earned). Capturing the regional form completes its event; defeat/flee allows retry. No simultaneous originals and regional forms.
- Future Articuno/Moltres encounters must register their Galarian forms using this same rule, not recolor the original or replace its capture flag.
- If a species has no official regional form, do not invent one under this rule.

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

## P2 — remaining legendary birds
- Moltres: Mt Chimney, fourth badge and `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`, level 45. Uses Maxie's former tile after the story; check collision and trainer visibility in emulator.
- Articuno: Shoal Cave low-tide ice room, seventh badge, level 55. Existing tide access controls entry; marker remains visible at high tide. Provisional tile (12,9), collision QA required.
- Each original capture unlocks its Galarian variant on re-entry once all eight badges are present; level 65 provisional. Flags 0x022–0x029 reserved. No capture by trade or Dex flag. Same retry policy as Zapdos.
- Combat work is separate. Confirmed designs: Demon Surge (Fighting/Dark damaging moves, Electric mixed strike BP25/45 if primary crit, independent crit, paralysis); Dark Phoenix (one revival at half max HP, then 50% healing ceiling); Ice Sovereign (heals 20% of actual Ice damage dealt, 40% against Frostbite; not freeze); EverIce (permanent hail while active, heals 1/10 max HP each turn). Blitz Storm and Bright Phoenix named; mechanics not yet confirmed. Proposed Galar typings Fighting/Electric, Dark/Fire, Psychic/Ice.

## P3 — native ticket islands
- Aurora Ticket: scientist at Mossdeep Space Center gives or recognizes the original ticket after the climate crisis is resolved (Archie/Maxie leave Sootopolis) and all eight badges are held. Existing enabled travel remains valid. The always-visible sailor in Lilycove Harbor offers an optional research charter before the League; normal SS Tidal progression is untouched.
- After the League, the harbor sailor grants the Old Sea Map and Eon Ticket. The Mystic Ticket additionally requires the three original Genesis bird captures. These acquisition choices are provisional; Eon Ticket does not require resolving the original roamer.
- All four ticket destinations remain accessible once enabled and shown, independently of ticket possession; new grants never force immediate travel. Existing ticket flags are respected.
- Deoxys: one interaction with the triangle reveals Deoxys at (15,10), then the player approaches to battle. Level 60 provisional. Native capture and reveal flags reused; defeating/fleeing allows retry on re-entry. Existing captured event remains complete. Off-map island is represented by the Lilycove transport hub, not a fabricated map coordinate.
- No new island species or terrain changes beyond Deoxys placement yet. Native Mew, Lugia, Ho-Oh and Eon encounters unchanged. Island ecosystems and distribution audit are P4.
- QA pending: pre-League ferry round trip, post-League ferry menu and existing tickets, bag-full behavior, save/reload, rock interaction and Deoxys collision, original roamer selection, native island encounters, compile and execute tests.
