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

## P4 — island ecology, first pass
- Birth Island: visible Elgyem (36), Beldum (36), and Beheeyem (48, after native Deoxys capture flag).
- Faraway Island entrance: visible Oranguru (48), Passimian (48), Komala (42), Toedscool (43).
- Navel Rock B1F: visible Dreepy (44), Duraludon (48), Drampa (55).
- Southern Island: interact with existing sign, opt into investigating psychic presence; random Munna (42), Espurr (44), Hatenna (46), male Indeedee (50). One encounter per map visit. No new tile layout needed.
- These 14 species are absent from the ordinary wild encounter tables on the P3 base; this does not assert absence from gifts, breeding, raids, shops, or other scripted encounters. Levels provisional. Regional forms/other forms not automatically added here.
- Visible fauna is repeatable on re-entry, using temporary map-local flags rather than permanent per-individual completion. First successful capture per island clears its discovery marker (permanent flags 0x02A–0x02D); access never closes. Off-map discovery markers use Lilycove transport hub. No species names revealed on map.
- Static collision/elevation and connectivity from entry verified for the ten new visible positions using raw layout blocks; actual walking, camera, sprites, battles, re-entry, collision around legendary events and sign interaction require emulator QA.
- No random grass/water tables changed and no new terrain. Later expansions may add distinct biomes, new maps, artifacts and further families after a full acquisition audit.

- Beheeyem has a separate post-Deoxys discovery marker until first capture (0x02E). Deoxys elevation corrected to the actual floor elevation 3 at its existing tile.

## P5 — National Dex census and first environmental placements
- Census snapshot: 1,025 National Dex entries. 299 have ordinary wild table references, 37 have verified selected island/scripted sources, 233 share a family with a documented source (evolution direction and method not guaranteed), and 456 lack sources in these audited tables. These are triage categories, NOT an unobtainability verdict. 8 entries lacked explicit natDexNum mapping in the first pass and use their Dex name as fallback; forms are not counted as separate National Dex numbers. Coverage still needs shops, raids, eggs, static gifts, config, enabled maps, and runtime gates.
- Confirmed native gifts/encounters include Beldum (Steven; thus not actually exclusive to Birth Island), Lileep/Anorith, Hoenn legendaries/Regis and Johto starters. Keep existing acquisition paths intact.
- New Mauville: after generator shutdown, interact with the existing generator to find Tadbulb (32) and Varoom (34); prioritize uncaught discoveries, then random repeatable encounters. Route 124: optional Cramorant (40) encounter by the Hunter's House sign after sixth badge. One encounter per visit; captures clear their individual regional markers; no new grass tables or map edits. Levels provisional.
- Audit script is a lexical inventory tool, not a proof of availability; manual review required for dynamic sources and unreachable maps. Environmental scripts and flags need compile/emulator validation.

Run `python3 tools/genesis_census.py --output /tmp/genesis-census.csv` from a full checkout for an expanded lexical scan of map scripts and candidate acquisition code; this script has not been executed in the connector environment. It intentionally uses separate evidence categories from the curated snapshot.

## P6 — badge-gated habitat expansion
- 131 encounter entries, 127 distinct National Dex species, across 63 maps. Cumulative badge count (1–8), independent 20% pool chance on ordinary land/surf generation, equally weighted eligible species; native table is used otherwise. No changes to map encounter trigger frequency, fishing tables, outbreak/roamer priority or save layout. Existing species remain possible. Repel, level-based ability rejection and frequency mutation creation remain on the common path. Type-attraction abilities do not influence the bonus pool; Pokélocator and other native-table-only tools do not yet list these additions.
- Mirage Tower floors 1–4 have Silicobra, Rellor, Flittle and Scraggy; Desert Underpass is a permanent fallback after eight badges because the tower collapses. No unavailable Cave of Origin Ruby/Sapphire maps used.
- Caves, Mt. Pyre, Safari, coastal land and surfing habitats receive thematic species. Existing levels and badge requirements are balancing choices subject to testing. Includes no starter, fossil, pseudo, mythical, legendary, Ultra Beast or Paradox species.
- Corrected Feebas census: native Route 119 fishing already exists (see gWildFeebas in wild_encounter.c); preserve it. Snapshot now: {"no_source_in_audited_tables":211,"ordinary_wild":299,"family_present_review_evolution":349,"badge_gated_wild":127,"genesis_or_island":22,"scripted_source":17}. These remain triage counts, not verified total obtainable. Evolution methods and forms, gifts/shops/raids still require audit.
- Pending QA: compile, run tests, emulator check all map access gates (including Safari expansion), probability/repel/ability balance, integration with Pokélocator and other encounter readers, and compare all pending conventional families against existing acquisition systems. Data file src/data/unholy_genesis_wild.inc is authoritative.

- Castform verified as a native Weather Institute 2F gift; no additional wild habitat added. Only the Porygon family remains without a conventional source in this snapshot; reserve for a computer-based encounter or verified existing shop. Pokélocator integration is deferred, not claimed complete.
