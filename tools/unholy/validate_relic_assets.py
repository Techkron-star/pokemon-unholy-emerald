#!/usr/bin/env python3
"""Validate Unholy Emerald item art before starting the expensive ROM build.

Checks the three registered icons and staged concept art. Requires no packages.
Valid art does not imply that an item has gameplay handlers or shop entries.
"""
from pathlib import Path
import struct
import sys
import zlib

ROOT = Path(__file__).resolve().parents[2]
REGISTERED = ("infinity_ball", "omega_ball", "super_exp_share")
SIGNATURE = bytes((137, 80, 78, 71, 13, 10, 26, 10))


def check_png(path):
    data = path.read_bytes()
    if not data.startswith(SIGNATURE):
        raise ValueError("invalid PNG signature")
    offset = 8
    seen = set()
    plte = None
    transparency = None
    compressed = bytearray()
    while offset < len(data):
        if offset + 12 > len(data):
            raise ValueError("truncated chunk header")
        length = struct.unpack_from(">I", data, offset)[0]
        kind = data[offset + 4:offset + 8]
        end = offset + 12 + length
        if end > len(data):
            raise ValueError("truncated PNG chunk")
        payload = data[offset + 8:offset + 8 + length]
        stored_crc = struct.unpack_from(">I", data, offset + 8 + length)[0]
        if zlib.crc32(kind + payload) & 0xFFFFFFFF != stored_crc:
            raise ValueError(f"{kind!r}: bad CRC")
        if kind == b"IHDR":
            if "IHDR" in seen or length != 13:
                raise ValueError("invalid IHDR")
            header = struct.unpack(">IIBBBBB", payload)
            if header != (24, 24, 8, 3, 0, 0, 0):
                raise ValueError(f"expected 24x24 indexed PNG, got {header}")
        if kind == b"PLTE":
            if plte is not None or length != 48:
                raise ValueError("expected 16 RGB palette entries")
            plte = payload
        if kind == b"tRNS":
            if transparency is not None:
                raise ValueError("duplicate transparency chunk")
            transparency = payload
        if kind == b"IDAT":
            compressed.extend(payload)
        seen.add(kind.decode("ascii", "replace"))
        offset = end
        if kind == b"IEND":
            if length or offset != len(data):
                raise ValueError("invalid IEND or trailing bytes")
            break
    if not all(k in seen for k in ("IHDR", "PLTE", "tRNS", "IDAT", "IEND")):
        raise ValueError("missing PNG chunks")
    if transparency != bytes((0,)):
        raise ValueError("palette index 0 must be transparent")
    if len(zlib.decompress(compressed)) != 24 * 25:
        raise ValueError("unexpected decompressed pixel data size")
    return plte


def check_palette(path, rgb):
    lines = path.read_text(encoding="ascii").splitlines()
    if lines[:3] != ["JASC-PAL", "0100", "16"] or len(lines) != 19:
        raise ValueError("expected a 16-entry JASC-PAL")
    values = []
    for line in lines[3:]:
        color = [int(x) for x in line.split()]
        if len(color) != 3 or any(not 0 <= x <= 255 for x in color):
            raise ValueError("invalid JASC RGB row")
        values.extend(color)
    if bytes(values) != rgb:
        raise ValueError("JASC palette differs from PNG")


def main():
    icons = ROOT / "graphics/items/icons"
    palettes = ROOT / "graphics/items/icon_palettes"
    concepts = ROOT / "graphics/unholy_concepts/icons"
    concept_palettes = ROOT / "graphics/unholy_concepts/palettes"
    targets = [(icons / f"{name}.png", palettes / f"{name}.pal") for name in REGISTERED]
    targets += [(p, concept_palettes / (p.stem + ".pal")) for p in sorted(concepts.glob("*.png"))]
    if len(targets) <= len(REGISTERED):
        print("FAIL: no staged concept icons were found", file=sys.stderr)
        return 1
    failed = 0
    for png, pal in targets:
        try:
            check_palette(pal, check_png(png))
        except (OSError, ValueError, zlib.error, struct.error) as exc:
            print(f"FAIL {png.relative_to(ROOT)}: {exc}", file=sys.stderr)
            failed += 1
    print(f"Unholy sprite preflight: {len(targets)-failed}/{len(targets)} valid.")
    return int(failed != 0)


if __name__ == "__main__":
    raise SystemExit(main())
