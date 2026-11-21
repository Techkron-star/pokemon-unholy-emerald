# Unholy Extended memory — experimental prototype

This is an opt-in experiment, not a replacement for the normal GBA build.
The regular ROM must continue to boot in mGBA, Pizza Boy, and real GBA hardware.

## Protocol v1

- An instrumented mGBA core provides 4 MiB of host-backed volatile RAM at
  `0x01000000..0x013FFFFF` (previously unmapped on the GBA).
- A read-only identification page occupies the first 4 KiB:
  - `0x01000000`: magic `0x58454855` (`UHEX`, little-endian)
  - `0x01000004`: protocol version `1`
  - `0x01000008`: usable capacity `0x003FF000`
- The game uses `0x01001000..0x013FFFFF` only after all three identification
  words match. There is an EWRAM fallback for the PC browser's temporary
  folder-key and Pokémon-reference lists.
- This region is **not persistent save storage**. Existing `.sav` data and
  SaveBlock5 are unchanged.

## Desktop test of the mGBA patch

1. Clone `https://github.com/mgba-emu/mgba` (the prototype was made against
   upstream `4a390e470a1c90e3b91721a247e5a5e836c31b8e`).
2. Apply the `0001-feat-gba-prototype-4-MiB-Unholy-extended-memory-wind.patch`
   file in this directory with `git am /path/to/that-file.patch`.
3. Build a Libretro core:

   ```sh
   cmake -S . -B build-unholy -DBUILD_QT=OFF -DBUILD_SDL=OFF -DBUILD_LIBRETRO=ON
   cmake --build build-unholy --target mgba_libretro -j 4
   ```

The core builds as `mgba_libretro.so` on Linux. The accompanying
`build-unholy-extended-core.yml` GitHub Actions workflow also builds an
Android ARM64 Libretro `.so` using the NDK. This is **not** an Android APK:
it requires a compatible frontend (e.g. RetroArch). Pizza Boy does not
implement this protocol.

## Verification performed

- The modified mGBA core and Linux headless emulator compiled successfully.
- An mGBA in-process smoke test loaded a compiled Unholy Emerald ROM and
  verified protocol identification, read-only header, 8/16/32-bit accesses,
  region boundaries, separation from standard EWRAM, save-state roundtrip,
  and zeroing on reset.
- A GitHub Actions Android ARM64 core build passed (run `44954574560`).
- The original GBA map and 256 KiB EWRAM remain untouched.

## Known gaps — do not treat this as a release

- The custom core appends 4 MiB to its GBA save-state snapshot. Its save states
  are **not compatible with stock mGBA/Pizza Boy states**; the binary `.sav`
  battery-backed game saves are unaffected. The state roundtrip has a native
  smoke test but not yet a live Android runtime test.
- GBA DMA, all emulator memory-view interfaces, and Android on-device
  behavior need end-to-end testing.
- This prototype does not magically increase CPU performance, sprite hardware
  limits, or the size of game cartridge ROM. Future features must opt into the
  extra region explicitly.
- Do not merge into the standard release until real boot/PC tests pass.

Upstream mGBA is under the Mozilla Public License 2.0; the patch keeps the
original copyright headers and includes corresponding modified source.
