#!/usr/bin/env python3
"""Curated encounters for Hoenn's power plant and secondary interiors.

Includes New Mauville's generator map without changing its switches, three
scripted Voltorb fights or Wattson's generator-shutdown quest. Original
species remain present and levels/slot weights are never rewritten.

Run --write to apply to wild_encounters.json; without it, validate the result.
"""

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"
GENERATOR_SCRIPTS = ROOT / "data/maps/NewMauville_Inside/scripts.inc"

# Frozen pre-expansion slot profiles; never infer natives from edited data.
MAUVILLE_ENTRANCE = "VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE"
MAUVILLE_GENERATOR = "VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE VOLTORB MAGNEMITE ELECTRODE MAGNETON"
MIRAGE = "SANDSHREW TRAPINCH SANDSHREW TRAPINCH SANDSHREW TRAPINCH SANDSHREW TRAPINCH SANDSHREW TRAPINCH SANDSHREW TRAPINCH"
DESERT_UNDERPASS = "DITTO WHISMUR DITTO LOUDRED DITTO WHISMUR LOUDRED DITTO WHISMUR DITTO LOUDRED DITTO"
ARTISAN = " ".join(["SMEARGLE"] * 12)
MAGMA = "GEODUDE TORKOAL GEODUDE TORKOAL GEODUDE GEODUDE GEODUDE GRAVELER GRAVELER GRAVELER GRAVELER GRAVELER"
UNDERWATER = "CLAMPERL CHINCHOU CLAMPERL RELICANTH RELICANTH"

ORIGINALS = {
    "MAP_NEW_MAUVILLE_ENTRANCE": {"land_mons": MAUVILLE_ENTRANCE},
    "MAP_NEW_MAUVILLE_INSIDE": {"land_mons": MAUVILLE_GENERATOR},
    "MAP_DESERT_UNDERPASS": {"land_mons": DESERT_UNDERPASS},
    "MAP_ARTISAN_CAVE_1F": {"land_mons": ARTISAN},
    "MAP_ARTISAN_CAVE_B1F": {"land_mons": ARTISAN},
    "MAP_UNDERWATER_ROUTE124": {"water_mons": UNDERWATER},
    "MAP_UNDERWATER_ROUTE126": {"water_mons": UNDERWATER},
}
for floor in range(1, 5):
    ORIGINALS[f"MAP_MIRAGE_TOWER_{floor}F"] = {"land_mons": MIRAGE}
for suffix in ("1F", "2F_1R", "2F_2R", "2F_3R", "3F_1R", "3F_2R", "3F_3R", "4F"):
    ORIGINALS[f"MAP_MAGMA_HIDEOUT_{suffix}"] = {"land_mons": MAGMA}

# Slot weights are land 20/20/10/10/10/10/5/5/4/4/1/1;
# underwater Surf uses 60/30/5/4/1. Rotom sits at the generator's 1% slot.
# Magneton moves from the original 1% slot to an existing 4% Magnemite slot,
# keeping Voltorb, Magnemite, Electrode and Magneton in the same room.
PATCHES = {
    "MAP_NEW_MAUVILLE_ENTRANCE": {"land_mons": {2: "ELEKID", 3: "JOLTIK", 6: "TYNAMO", 8: "KLINK", 10: "TOXEL", 11: "CHARJABUG"}},
    "MAP_NEW_MAUVILLE_INSIDE": {"land_mons": {2: "JOLTIK", 3: "KLINK", 6: "ELEKID", 7: "TYNAMO", 8: "TOXEL", 9: "MAGNETON", 11: "ROTOM"}},
    "MAP_MIRAGE_TOWER_1F": {"land_mons": {2: "SANDILE", 3: "BALTOY", 6: "DWEBBLE", 8: "YAMASK", 10: "SIGILYPH", 11: "UNOWN"}},
    "MAP_MIRAGE_TOWER_2F": {"land_mons": {2: "SANDILE", 3: "BALTOY", 6: "DWEBBLE", 8: "HELIOPTILE", 10: "SIGILYPH", 11: "UNOWN"}},
    "MAP_MIRAGE_TOWER_3F": {"land_mons": {2: "BALTOY", 3: "GOLETT", 6: "YAMASK", 8: "DWEBBLE", 10: "SIGILYPH", 11: "UNOWN"}},
    "MAP_MIRAGE_TOWER_4F": {"land_mons": {2: "BALTOY", 3: "GOLETT", 6: "YAMASK", 8: "CARBINK", 10: "SIGILYPH", 11: "TYRUNT"}},
    "MAP_DESERT_UNDERPASS": {"land_mons": {2: "DRILBUR", 4: "ROGGENROLA", 5: "DIGLETT", 6: "DWEBBLE", 8: "NOIBAT", 11: "ZORUA"}},
    "MAP_ARTISAN_CAVE_1F": {"land_mons": {2: "SHROODLE", 3: "GRAFAIAI", 6: "UNOWN", 8: "KLEFKI", 10: "SUDOWOODO", 11: "DITTO"}},
    "MAP_ARTISAN_CAVE_B1F": {"land_mons": {2: "SHROODLE", 3: "GRAFAIAI", 6: "UNOWN", 8: "KLEFKI", 10: "SUDOWOODO", 11: "ZORUA"}},
    "MAP_UNDERWATER_ROUTE124": {"water_mons": {2: "SKRELP", 4: "MAREANIE"}},
    "MAP_UNDERWATER_ROUTE126": {"water_mons": {2: "CLAUNCHER", 4: "TYNAMO"}},
}

MAGMA_PATCHES = {
    "1F": {2: "ROLYCOLY", 3: "SLUGMA", 4: "NUMEL", 6: "HOUNDOUR", 9: "SALANDIT", 11: "CHARCADET"},
    "2F_1R": {2: "ROLYCOLY", 3: "SLUGMA", 4: "NUMEL", 6: "HOUNDOUR", 9: "KOFFING", 11: "CHARCADET"},
    "2F_2R": {2: "ROLYCOLY", 3: "HOUNDOUR", 4: "SLUGMA", 6: "SALANDIT", 9: "KOFFING", 11: "LARVESTA"},
    "2F_3R": {2: "ROLYCOLY", 3: "SLUGMA", 4: "SALANDIT", 6: "HOUNDOUR", 9: "CHARCADET", 11: "LARVESTA"},
    "3F_1R": {2: "KOFFING", 3: "SLUGMA", 4: "SALANDIT", 6: "HOUNDOUR", 9: "ROLYCOLY", 11: "LARVESTA"},
    "3F_2R": {2: "ROLYCOLY", 3: "SALANDIT", 4: "SLUGMA", 6: "KOFFING", 9: "CHARCADET", 11: "LARVESTA"},
    "3F_3R": {2: "ROLYCOLY", 3: "SLUGMA", 4: "HOUNDOUR", 6: "SALANDIT", 9: "CHARCADET", 11: "LARVESTA"},
    "4F": {2: "SLUGMA", 3: "SALANDIT", 4: "HOUNDOUR", 6: "CHARCADET", 9: "KOFFING", 11: "LARVESTA"},
}
for suffix, slots in MAGMA_PATCHES.items():
    PATCHES[f"MAP_MAGMA_HIDEOUT_{suffix}"] = {"land_mons": slots}


def run(write):
    if set(PATCHES) != set(ORIGINALS):
        raise ValueError("Missing secondary map baseline or patch")
    # The existing generator remains a quest interaction, not a new encounter
    # object that could block Wattson's power-plant progression.
    scripts = GENERATOR_SCRIPTS.read_text(encoding="utf-8")
    for name in ("NewMauville_Inside_EventScript_Generator::", "NewMauville_Inside_EventScript_RedButton::", "NewMauville_Inside_EventScript_Voltorb1::", "NewMauville_Inside_EventScript_Voltorb2::", "NewMauville_Inside_EventScript_Voltorb3::"):
        if name not in scripts:
            raise ValueError(f"New Mauville generator quest event missing: {name}")

    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    maps = {entry["map"]: entry for entry in data["wild_encounter_groups"][0]["encounters"]}
    ids = set(re.findall(r"\b(SPECIES_\w+)\s*=", (ROOT / "include/constants/species.h").read_text(encoding="utf-8")))
    placements = 0
    methods = 0
    for map_name, methods_patches in PATCHES.items():
        if map_name not in maps:
            raise ValueError(f"Secondary map missing: {map_name}")
        for method, replacements in methods_patches.items():
            methods += 1
            baseline = ORIGINALS[map_name][method].split()
            mons = maps[map_name][method]["mons"]
            if len(baseline) != len(mons) or any(i < 0 or i >= len(mons) for i in replacements):
                raise ValueError(f"Invalid encounter slot: {map_name}/{method}")
            for index, (mon, native) in enumerate(zip(mons, baseline)):
                expected = "SPECIES_" + replacements.get(index, native)
                if expected not in ids:
                    raise ValueError(f"Unknown species: {map_name}/{method}[{index}] {expected}")
                if mon["species"] not in {"SPECIES_" + native, expected}:
                    raise ValueError(f"Native encounter slot drift: {map_name}/{method}[{index}]")
                if write:
                    mon["species"] = expected
                elif mon["species"] != expected:
                    raise ValueError(f"Unapplied slot: {map_name}/{method}[{index}], run --write")
            natives = set(baseline)
            present = {mon["species"].removeprefix("SPECIES_") for mon in mons}
            if natives - present:
                raise ValueError(f"Original Pokémon lost: {map_name}/{method}: {sorted(natives - present)}")
            placements += len(replacements)

    # This is specifically the power generator's *inside* map; Rotom's last
    # grass slot has its original 1% weight and all four native lines remain.
    inside = maps["MAP_NEW_MAUVILLE_INSIDE"]["land_mons"]["mons"]
    if inside[11]["species"] != "SPECIES_ROTOM" or inside[9]["species"] != "SPECIES_MAGNETON":
        raise ValueError("New Mauville generator Rotom/Magneton arrangement changed")

    if write:
        ENCOUNTERS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Hoenn side-area ecology: {len(PATCHES)} maps, {methods} encounter tables, {placements} placements; generator quest and native encounters preserved")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="Apply the curated secondary-map encounter changes")
    run(parser.parse_args().write)
