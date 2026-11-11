#!/usr/bin/env python3
"""Strict 24x24 indexed 4bpp PNG/JASC palette checks; no third-party packages."""
from pathlib import Path
import struct
import sys
import zlib

ITEMS = ("corazon_titan", "ancla_realidad", "impulso_cosmico",
         "cetro_siniestro", "super_exp_share")

def validate(root, name):
    base = "graphics/items" if name == "super_exp_share" else "graphics/unholy_concepts"
    icon = root / base / "icons" / (name + ".png")
    pal = root / base / ("icon_palettes" if name == "super_exp_share" else "palettes") / (name + ".pal")
    png = icon.read_bytes()
    assert png[:8] == b"\x89PNG\r\n\x1a\n", icon
    offset, chunks = 8, {}
    while offset < len(png):
        length = struct.unpack_from(">I", png, offset)[0]
        kind = png[offset+4:offset+8]
        payload = png[offset+8:offset+8+length]
        chunks.setdefault(kind, []).append(payload)
        offset += length + 12
        if kind == b"IEND":
            break
    meta = struct.unpack(">IIBBBBB", chunks[b"IHDR"][0])
    assert meta == (24, 24, 4, 3, 0, 0, 0), (name, meta)
    assert len(chunks[b"PLTE"][0]) == 48, name
    alpha = chunks[b"tRNS"][0]
    assert alpha[0] == 0 and all(v != 0 for v in alpha[1:]), name
    lines = pal.read_text().splitlines()
    assert lines[:3] == ["JASC-PAL", "0100", "16"], name
    expected = bytes(int(t) for row in lines[3:19] for t in row.split())
    assert chunks[b"PLTE"][0] == expected, name
    raw = zlib.decompress(b"".join(chunks[b"IDAT"]))
    assert len(raw) == 24 * 13, name
    print("OK", name, "24x24, indexed 4bpp, matching 16-color palette")

def main():
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent
    for name in ITEMS:
        validate(root, name)

if __name__ == "__main__":
    main()
