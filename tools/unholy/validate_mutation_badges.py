#!/usr/bin/env python3
"""Check all nine imported badges without external Python packages."""
from pathlib import Path
from validate_relic_assets import check_png, check_palette

ROOT = Path(__file__).resolve().parents[2]
NAMES = ("alpha", "hyper", "super", "overloaded", "enraged", "ancestral", "titan", "ascended", "omega")

def main():
    folder = ROOT / "graphics/mutations/badges"
    for name in NAMES:
        check_palette(folder / "palettes" / f"{name}.pal", check_png(folder / f"{name}.png", size=16))
    print("Mutation badges: 9/9 valid (16x16, 4bpp, CRC, transparency, matching palettes)")

if __name__ == "__main__":
    main()
