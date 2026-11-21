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

The core builds as `mgba_libretro.so` on Linux. This is **not** an Android APK.
Running it on Android will require an NDK-built Libretro core and a compatible
frontend (e.g. RetroArch); Pizza Boy does not implement this protocol.

## Verification performed

- The modified mGBA core and Linux headless emulator compiled successfully.
- An mGBA in-process smoke test loaded a compiled Unholy Emerald ROM and
  verified protocol identification, read-only header, 8/16/32-bit accesses,
  region boundaries, separation from standard EWRAM, and zeroing on reset.
- The original GBA map and 256 KiB EWRAM remain untouched.

## Known gaps — do not treat this as a release

- Extended RAM is **not serialized into save states** yet. Avoid using load
  states with experimental caches until this is implemented and tested.
- GBA DMA, all emulator memory-view interfaces, and Android/ARM64 builds need
  end-to-end testing.
- This prototype does not magically increase CPU performance, sprite hardware
  limits, or the size of game cartridge ROM. Future features must opt into the
  extra region explicitly.
- Do not merge into the standard release until real boot/PC tests pass.

Upstream mGBA is under the Mozilla Public License 2.0; the patch keeps the
original copyright headers and includes corresponding modified source.
