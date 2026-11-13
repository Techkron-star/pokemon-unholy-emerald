# Mutation badges · artwork batch 2 (6/6)

Completes the nine-mutation set started in batch 1 (Hyper, Titan, Overloaded). Each badge is a small symbol, not a letter.

| Badge (`enum PokemonBadge`) | Symbol |
|---|---|
| `POKEMON_BADGE_ALPHA` | Glowing red predator eye with a slit pupil. |
| `POKEMON_BADGE_SUPER` | Gold star. |
| `POKEMON_BADGE_ENRAGED` | Three red-orange claw slashes. |
| `POKEMON_BADGE_ANCESTRAL` | Spiral ammonite fossil in amber. |
| `POKEMON_BADGE_ASCENDED` | Blue gem under a gold halo. |
| `POKEMON_BADGE_OMEGA` | Eclipse: dark void with a magenta corona and a glint. |

Files: `alpha`, `super`, `enraged`, `ancestral`, `ascended`, `omega` (`.png` here, `.pal` in `palettes/`). Original pixels are in `source_grid_batch2.json`.

## Asset contract (same as batch 1)

- 16×16 indexed 4bpp PNG, exactly 16 palette slots, index 0 transparent.
- `palettes/*.pal` is a JASC-PAL that matches the PNG's palette. Colour channels are multiples of 8 so the GBA 5-bit conversion is exact.
- Art is centred and legible on dark and light backgrounds. No text or letters.

## Scope

ART ONLY. Nothing registers or draws these sprites yet; the summary/battle UI still has to load them using the existing mutation flags. Existing `graphics/battle_interface/alpha_indicator.png` and `omega_indicator.png` (8×16 letters) and any other Omega/Ancestral visuals are untouched.
