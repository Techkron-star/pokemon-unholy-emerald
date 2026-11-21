#!/usr/bin/env python3
"""Build GBA-compatible battle and bag icons from approved 32px artifact art.

Requires ImageMagick's `magick` and Pillow for PNG palette inspection.
The 32x32 originals are preserved; no ROM data depends on ImageMagick.
"""

from pathlib import Path
import subprocess

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
APPROVED = ROOT / "graphics/unholy_concepts/approved_32"
GENERATED = ROOT / "graphics/unholy_concepts/generated_32"

APPROVED_IN_GAME = (
    "bateria_reserva",
    "cartel_busqueda",
    "crisol_espiritual",
    "guantes_cohete",
    "cetro_siniestro",
    "corazon_titan",
    "impulso_cosmico",
    "escudo_aegis",
    "amuleto_inquebrantable",
    "casco_hoplita",
    "tunica_runica",
    "anillo_feerico",
    "ariete_nautilo",
    "armadura_meteorica",
)

GENERATED_IN_GAME = (
    "estandarte_guerra",
    "torre_vigilancia",
    "faro",
    "rompeolas",
    "red_silos",
    "tablero_trifasico",
    "crt",
    "deshumidificador",
    "bomba_ariete",
    "glicerina",
    "abono",
)


def render_icon(source: Path, dest: Path, size: int) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    command = [
        "magick", str(source), "-channel", "A", "-threshold", "50%",
        "+channel", "-filter", "point", "-resize",
        f"{size}x{size}", "-depth", "4",
    ]
    if size == 16:
        command += [
            "(", "+clone", "-channel", "RGB", "-evaluate",
            "Multiply", "0.55", "+channel", ")", "-append",
            "+dither", "-colors", "16", "-depth", "4",
        ]
    command += ["PNG:" + str(dest)]
    subprocess.run(command, check=True)


def write_palette(icon: Path, dest: Path) -> None:
    with Image.open(icon) as art:
        if art.mode != "P" or art.info.get("transparency") != 0:
            raise ValueError(f"{icon}: expected indexed PNG with transparent index 0")
        colors = art.getpalette()
        if colors is None or len(colors) // 3 > 16:
            raise ValueError(f"{icon}: expected up to 16 palette entries")
        rgb = [colors[i:i + 3] for i in range(0, len(colors), 3)]
        rgb = (rgb + [[0, 0, 0]] * 16)[:16]
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(
        "JASC-PAL\n0100\n16\n" +
        "".join(f"{r} {g} {b}\n" for r, g, b in rgb), encoding="ascii"
    )


def install(name: str, source: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(source)
    gear_dir = ROOT / "graphics/gears" / name
    gear_dir.mkdir(parents=True, exist_ok=True)
    (gear_dir / "icon_32x32_source.png").write_bytes(source.read_bytes())
    hud = gear_dir / "icon.png"
    render_icon(source, hud, 16)
    write_palette(hud, gear_dir / "icon.pal")
    bag = ROOT / "graphics/items/icons" / (name + ".png")
    render_icon(source, bag, 24)
    write_palette(bag, ROOT / "graphics/items/icon_palettes" / (name + ".pal"))
    print(f"OK: {name}: battle 16x32, bag 24x24, source 32x32")


def main() -> None:
    for name in APPROVED_IN_GAME:
        install(name, APPROVED / (name + ".png"))
    for name in GENERATED_IN_GAME:
        install(name, GENERATED / (name + ".png"))
    install("escudo_paladin", GENERATED / "escudo_paladin.png")


if __name__ == "__main__":
    main()
