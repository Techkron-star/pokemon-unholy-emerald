#!/usr/bin/env python3
"""First cave biome expansion: Granite, Rusturf, volcanic and Meteor areas.

Each patched slot retains its original level and weight. Frozen vanilla slot
profiles guard native species, including their original fishing rod category.
Use --write to apply; without arguments the committed data is checked.
"""

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"

LAND = {
    "MAP_RUSTURF_TUNNEL": "WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR WHISMUR",
    "MAP_GRANITE_CAVE_1F": "ZUBAT MAKUHITA MAKUHITA ZUBAT MAKUHITA ABRA MAKUHITA MAKUHITA GEODUDE GEODUDE GEODUDE GEODUDE",
    "MAP_GRANITE_CAVE_B1F": "ZUBAT ARON ARON ARON ZUBAT ABRA MAKUHITA MAKUHITA SABLEYE SABLEYE SABLEYE SABLEYE",
    "MAP_GRANITE_CAVE_B2F": "ZUBAT ARON ARON ZUBAT ARON ABRA SABLEYE SABLEYE SABLEYE SABLEYE SABLEYE SABLEYE",
    "MAP_GRANITE_CAVE_STEVENS_ROOM": "ZUBAT MAKUHITA MAKUHITA ZUBAT MAKUHITA ABRA MAKUHITA MAKUHITA ARON ARON ARON ARON",
    "MAP_FIERY_PATH": "NUMEL KOFFING NUMEL MACHOP TORKOAL SLUGMA KOFFING MACHOP TORKOAL TORKOAL GRIMER GRIMER",
    "MAP_JAGGED_PASS": "NUMEL NUMEL MACHOP NUMEL SPOINK MACHOP SPOINK MACHOP NUMEL SPOINK NUMEL SPOINK",
    "MAP_METEOR_FALLS_1F_1R": "ZUBAT ZUBAT ZUBAT ZUBAT ZUBAT SOLROCK SOLROCK SOLROCK ZUBAT ZUBAT ZUBAT ZUBAT",
    "MAP_METEOR_FALLS_1F_2R": "GOLBAT GOLBAT GOLBAT SOLROCK SOLROCK SOLROCK GOLBAT SOLROCK GOLBAT GOLBAT GOLBAT GOLBAT",
    "MAP_METEOR_FALLS_B1F_1R": "GOLBAT GOLBAT GOLBAT SOLROCK SOLROCK SOLROCK GOLBAT SOLROCK GOLBAT GOLBAT GOLBAT GOLBAT",
    "MAP_METEOR_FALLS_B1F_2R": "GOLBAT GOLBAT BAGON SOLROCK BAGON SOLROCK BAGON SOLROCK GOLBAT GOLBAT GOLBAT GOLBAT",
    "MAP_METEOR_FALLS_STEVENS_CAVE": "GOLBAT GOLBAT GOLBAT SOLROCK SOLROCK SOLROCK GOLBAT SOLROCK GOLBAT GOLBAT GOLBAT GOLBAT",
}

ROCK_SMASH = "GEODUDE NOSEPASS GEODUDE GEODUDE GEODUDE"
SURF_FIRST_ROOM = "ZUBAT ZUBAT SOLROCK SOLROCK SOLROCK"
SURF_DEEP_ROOMS = "GOLBAT GOLBAT SOLROCK SOLROCK SOLROCK"
FISH_FIRST_ROOM = "MAGIKARP GOLDEEN MAGIKARP GOLDEEN BARBOACH BARBOACH BARBOACH BARBOACH BARBOACH BARBOACH"
FISH_DEEP_ROOMS = "MAGIKARP GOLDEEN MAGIKARP GOLDEEN BARBOACH BARBOACH BARBOACH WHISCASH WHISCASH WHISCASH"

# Rates for land: 20/20/10/10/10/10/5/5/4/4/1/1.
# A rare Rockruff inhabits Rusturf, and Bagon remains exclusive to the same
# deep Meteor Falls chamber as in vanilla Emerald.
PATCHES = {
    "MAP_RUSTURF_TUNNEL": {"land_mons": {3: "ROGGENROLA", 4: "DIGLETT", 6: "GEODUDE", 7: "DWEBBLE", 9: "ROCKRUFF", 11: "NOIBAT"}},
    "MAP_GRANITE_CAVE_1F": {"land_mons": {4: "DRILBUR", 6: "DWEBBLE", 9: "ROCKRUFF", 11: "CARBINK"}},
    "MAP_GRANITE_CAVE_B1F": {"land_mons": {2: "DRILBUR", 3: "ROCKRUFF", 7: "DWEBBLE", 10: "NOIBAT", 11: "CARBINK"}},
    "MAP_GRANITE_CAVE_B2F": {
        "land_mons": {2: "DRILBUR", 4: "DWEBBLE", 9: "CARBINK", 10: "ROCKRUFF", 11: "NOIBAT"},
        "rock_smash_mons": {4: "CARBINK"},
    },
    "MAP_GRANITE_CAVE_STEVENS_ROOM": {"land_mons": {4: "DRILBUR", 6: "DWEBBLE", 9: "ROCKRUFF", 11: "CARBINK"}},
    "MAP_FIERY_PATH": {"land_mons": {2: "SALANDIT", 6: "ROLYCOLY", 7: "HOUNDOUR", 9: "CHARCADET", 11: "LARVESTA"}},
    "MAP_JAGGED_PASS": {"land_mons": {3: "ROCKRUFF", 5: "ROLYCOLY", 6: "HOUNDOUR", 8: "DWEBBLE", 10: "GLIMMET", 11: "SALANDIT"}},
    "MAP_METEOR_FALLS_1F_1R": {
        "land_mons": {2: "ROCKRUFF", 3: "ROGGENROLA", 4: "NOIBAT", 8: "CARBINK", 9: "GLIMMET", 10: "MINIOR", 11: "LUNATONE"},
        "water_mons": {1: "WOOBAT", 4: "TYNAMO"},
        "fishing_mons": {6: "CHEWTLE", 7: "TYNAMO", 8: "TYMPOLE", 9: "BASCULIN"},
    },
    "MAP_METEOR_FALLS_1F_2R": {
        "land_mons": {2: "ROCKRUFF", 4: "GLIMMET", 6: "NOIBAT", 8: "CARBINK", 9: "LUNATONE", 10: "MINIOR", 11: "AXEW"},
        "water_mons": {1: "WOOBAT", 4: "TYNAMO"},
        "fishing_mons": {6: "TYNAMO", 8: "TYMPOLE", 9: "BASCULIN"},
    },
    "MAP_METEOR_FALLS_B1F_1R": {
        "land_mons": {2: "ROCKRUFF", 4: "GLIMMET", 6: "NOIBAT", 8: "CARBINK", 9: "LUNATONE", 10: "MINIOR", 11: "AXEW"},
        "water_mons": {1: "WOOBAT", 4: "TYNAMO"},
        "fishing_mons": {6: "TYNAMO", 8: "TYMPOLE", 9: "BASCULIN"},
    },
    "MAP_METEOR_FALLS_B1F_2R": {
        "land_mons": {5: "LUNATONE", 8: "NOIBAT", 9: "CARBINK", 10: "MINIOR", 11: "GIBLE"},
        "water_mons": {1: "WOOBAT", 4: "TYNAMO"},
        "fishing_mons": {6: "TYNAMO", 8: "TYMPOLE", 9: "DRATINI"},
    },
    "MAP_METEOR_FALLS_STEVENS_CAVE": {"land_mons": {2: "ROCKRUFF", 4: "GLIMMET", 6: "NOIBAT", 8: "CARBINK", 9: "LUNATONE", 10: "MINIOR", 11: "AXEW"}},
}


def original_slots(map_name, method):
    if method == "land_mons":
        return LAND[map_name].split()
    if method == "rock_smash_mons":
        return ROCK_SMASH.split()
    if method == "water_mons":
        return (SURF_FIRST_ROOM if map_name == "MAP_METEOR_FALLS_1F_1R" else SURF_DEEP_ROOMS).split()
    if method == "fishing_mons":
        return (FISH_FIRST_ROOM if map_name == "MAP_METEOR_FALLS_1F_1R" else FISH_DEEP_ROOMS).split()
    raise ValueError(f"Unexpected cave method: {map_name}/{method}")


def run(write):
    if set(PATCHES) != set(LAND):
        raise ValueError("Some cave map lacks a manifest or baseline")
    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    maps = {entry["map"]: entry for entry in data["wild_encounter_groups"][0]["encounters"]}
    ids = set(re.findall(r"\b(SPECIES_\w+)\s*=", (ROOT / "include/constants/species.h").read_text(encoding="utf-8")))

    placements = 0
    methods = 0
    for map_name, method_patches in PATCHES.items():
        if map_name not in maps:
            raise ValueError(f"Missing cave: {map_name}")
        for method, replacements in method_patches.items():
            methods += 1
            mons = maps[map_name][method]["mons"]
            originals = original_slots(map_name, method)
            if len(mons) != len(originals) or any(index >= len(mons) or index < 0 for index in replacements):
                raise ValueError(f"Unexpected encounter slots: {map_name}/{method}")
            for index, (mon, original) in enumerate(zip(mons, originals)):
                expected = "SPECIES_" + replacements.get(index, original)
                if expected not in ids:
                    raise ValueError(f"Unknown species: {map_name}/{method}[{index}] {expected}")
                if mon["species"] not in {"SPECIES_" + original, expected}:
                    raise ValueError(f"Unexpected original: {map_name}/{method}[{index}]")
                if write:
                    mon["species"] = expected
                elif mon["species"] != expected:
                    raise ValueError(f"Unapplied slot: {map_name}/{method}[{index}] expected {expected}; run --write")

            ranges = ((0, 2), (2, 5), (5, 10)) if method == "fishing_mons" else ((0, len(mons)),)
            for start, end in ranges:
                native = set(originals[start:end])
                current = {mon["species"].removeprefix("SPECIES_") for mon in mons[start:end]}
                if native - current:
                    raise ValueError(f"Native lost: {map_name}/{method} [{start}:{end}]: {sorted(native - current)}")
            placements += len(replacements)

    # Bagon remains unique to the deepest chamber and is never moved to a
    # broadly accessible part of Meteor Falls.
    for map_name in LAND:
        present = {mon["species"] for mon in maps[map_name]["land_mons"]["mons"]}
        if ("SPECIES_BAGON" in present) != (map_name == "MAP_METEOR_FALLS_B1F_2R"):
            raise ValueError(f"Meteor Falls Bagon location changed: {map_name}")

    if write:
        ENCOUNTERS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Hoenn cave ecology: {len(PATCHES)} maps, {methods} tables, {placements} new placements; native species and Bagon exclusivity preserved")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="Apply the curated cave encounter expansion")
    run(parser.parse_args().write)
