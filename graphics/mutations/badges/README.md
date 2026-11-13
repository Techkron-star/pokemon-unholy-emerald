# Mutation indicators · artwork batch 1 (3/3)

Approved tiny mutation symbols for Pokémon summary / battle interfaces:

- **Hyper:** green double upward chevrons.
- **Titan:** centered stone fortress with small golden core.
- **Overloaded:** a **single cyan lightning bolt with purple outline**, no backing circle, glowing disc, decorations, text, or label.

## GBA asset contract

- Each image is a **16×16 indexed 4bpp PNG** with exactly 16 RGB palette slots and transparent index 0.
- Each `palettes/*.pal` is a JASC-PAL matching the PNG's indexed RGB table.
- Artwork is centered in its 16×16 canvas and legible on dark and light backgrounds.
- No names, nameplates, borders around the canvas, or mutation text are baked into the image.
- These are **only assets**. Do not assume they already appear in the game's screens: the sprites still need to be registered and drawn by the summary/battle UI alongside names, using the mutation enum/flags that already exist.
- The existing Omega/Ancestral visuals and mutation game rules are **untouched**.

Local QA included PNG signature, IHDR 16×16 indexed 4bpp, correct CRC for all chunks, transparency, and byte-for-byte PNG PLTE vs JASC-PAL matching. Git blob SHAs were checked during upload.
