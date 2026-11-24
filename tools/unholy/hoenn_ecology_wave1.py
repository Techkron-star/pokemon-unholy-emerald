#!/usr/bin/env python3
"""Curate the first Hoenn encounter expansion without losing native species.

Run with --write to update wild_encounters.json; otherwise validate the applied
manifest. Only the species occupying an existing weighted slot changes. The
encounter rates, fishing rod groups and level ranges remain untouched.
"""

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"

# Map -> encounter method -> {weighted slot: replacement species suffix}.
# Slots use their existing 20/20/10/... or rod-specific encounter chances.
PATCHES = {
    "MAP_ROUTE101": {
        "land_mons": {2: "SENTRET", 3: "BIDOOF", 6: "STARLY", 7: "ROOKIDEE", 10: "SCATTERBUG"},
    },
    "MAP_ROUTE102": {
        "land_mons": {2: "LILLIPUP", 5: "BOUNSWEET", 8: "SEWADDLE", 10: "NYMBLE"},
        "water_mons": {1: "WOOPER", 2: "DEWPIDER", 3: "SURSKIT"},
        "fishing_mons": {4: "POLIWAG", 7: "BARBOACH", 8: "CHEWTLE", 9: "TYMPOLE"},
    },
    "MAP_ROUTE103": {
        "land_mons": {2: "SHINX", 3: "NICKIT", 7: "LILLIPUP", 9: "PIDOVE", 10: "DUCKLETT"},
        "water_mons": {2: "BUIZEL", 4: "FRILLISH"},
        "fishing_mons": {7: "SHELLDER", 8: "REMORAID", 9: "CLAUNCHER"},
    },
    "MAP_ROUTE104": {
        "land_mons": {5: "FLETCHLING", 4: "BOUNSWEET", 7: "SEWADDLE", 10: "COMBEE"},
        "water_mons": {1: "BUIZEL", 2: "DUCKLETT", 4: "FRILLISH"},
        "fishing_mons": {1: "KRABBY", 3: "TENTACOOL", 4: "SHELLDER", 6: "HORSEA", 7: "REMORAID", 8: "CLAUNCHER", 9: "CORSOLA"},
    },
    "MAP_PETALBURG_WOODS": {
        "land_mons": {3: "SEWADDLE", 6: "SCATTERBUG", 10: "PINECO", 11: "PHANTUMP"},
    },
    "MAP_ROUTE116": {
        "land_mons": {4: "DIGLETT", 7: "ROGGENROLA", 9: "TIMBURR", 11: "DWEBBLE"},
    },
}

# The original Emerald encounter identities are hard constraints, not merely
# the currently present species (which already include previous wave edits).
ORIGINALS = {
    "MAP_ROUTE101": {"land_mons": {"WURMPLE", "POOCHYENA", "ZIGZAGOON"}},
    "MAP_ROUTE102": {
        "land_mons": {"POOCHYENA", "WURMPLE", "LOTAD", "ZIGZAGOON", "RALTS", "SEEDOT"},
        "water_mons": {"MARILL", "GOLDEEN"},
        "fishing_mons": {"MAGIKARP", "GOLDEEN", "CORPHISH"},
    },
    "MAP_ROUTE103": {
        "land_mons": {"POOCHYENA", "WINGULL", "ZIGZAGOON"},
        "water_mons": {"TENTACOOL", "WINGULL", "PELIPPER"},
        "fishing_mons": {"MAGIKARP", "TENTACOOL", "WAILMER", "SHARPEDO"},
    },
    "MAP_ROUTE104": {
        "land_mons": {"POOCHYENA", "WURMPLE", "MARILL", "TAILLOW", "WINGULL"},
        "water_mons": {"WINGULL", "PELIPPER"},
        "fishing_mons": {"MAGIKARP"},
    },
    "MAP_PETALBURG_WOODS": {"land_mons": {"POOCHYENA", "WURMPLE", "SHROOMISH", "SILCOON", "CASCOON", "TAILLOW", "SLAKOTH"}},
    "MAP_ROUTE116": {"land_mons": {"POOCHYENA", "WHISMUR", "NINCADA", "ABRA", "TAILLOW", "SKITTY"}},
}


def run(write):
    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    maps = {
        entry["map"]: entry
        for entry in data["wild_encounter_groups"][0]["encounters"]
    }
    species_source = (ROOT / "include/constants/species.h").read_text(encoding="utf-8")
    species_ids = set(re.findall(r"\b(SPECIES_\w+)\s*=", species_source))

    placements = 0
    methods = 0
    for map_name, method_patches in PATCHES.items():
        if map_name not in maps:
            raise ValueError(f"Encounter map missing: {map_name}")
        encounter = maps[map_name]
        for method, slots in method_patches.items():
            methods += 1
            mons = encounter[method]["mons"]
            before_levels = [(mon["min_level"], mon["max_level"]) for mon in mons]
            for index, species in slots.items():
                name = "SPECIES_" + species
                if name not in species_ids:
                    raise ValueError(f"{map_name} {method}[{index}]: unknown {name}")
                if index >= len(mons):
                    raise ValueError(f"{map_name} {method}[{index}]: invalid slot")
                if write:
                    mons[index]["species"] = name
                elif mons[index]["species"] != name:
                    raise ValueError(f"{map_name} {method}[{index}] != {name}; run --write")
                placements += 1

            if [(mon["min_level"], mon["max_level"]) for mon in mons] != before_levels:
                raise ValueError(f"Level ranges changed for {map_name}/{method}")
            present = {mon["species"].removeprefix("SPECIES_") for mon in mons}
            missing = ORIGINALS[map_name][method] - present
            if missing:
                raise ValueError(f"Native species lost in {map_name}/{method}: {sorted(missing)}")

    if write:
        ENCOUNTERS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Hoenn ecology wave 1: {len(PATCHES)} maps, {methods} encounter tables, {placements} curated slots; original species preserved")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="Apply this deterministic curation to wild_encounters.json")
    run(parser.parse_args().write)
