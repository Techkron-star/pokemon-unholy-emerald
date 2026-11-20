#!/usr/bin/env python3
"""Expand encounters on every remaining Hoenn route (105–134, except 116).

The manifest is intentionally explicit: each replacement is validated against
the original Emerald slot, and every native species must remain obtainable by
its original method (and, for fishing, by its original rod tier).

Run with --write to apply. With no arguments, validate the committed JSON.
"""

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"

# The 12 land slots are ordered by their original encounter weights. These
# frozen profiles also prevent accidentally replacing an irreplaceable native.
LAND_BASELINES = {
    110: "POOCHYENA ELECTRIKE GULPIN ELECTRIKE MINUN ODDISH MINUN GULPIN WINGULL WINGULL PLUSLE PLUSLE",
    111: "SANDSHREW TRAPINCH SANDSHREW TRAPINCH BALTOY BALTOY SANDSHREW TRAPINCH BALTOY CACNEA CACNEA CACNEA",
    112: "NUMEL NUMEL MARILL NUMEL NUMEL MARILL NUMEL MARILL NUMEL NUMEL NUMEL NUMEL",
    113: "SPINDA SPINDA SLUGMA SPINDA SPINDA SLUGMA SPINDA SLUGMA SPINDA SKARMORY SPINDA SKARMORY",
    114: "SWABLU LOTAD SWABLU SWABLU LOTAD LOMBRE LOMBRE LOMBRE SEVIPER SEVIPER SEVIPER NUZLEAF",
    115: "SWABLU TAILLOW SWABLU TAILLOW TAILLOW SWELLOW JIGGLYPUFF JIGGLYPUFF WINGULL WINGULL WINGULL WINGULL",
    117: "POOCHYENA ODDISH POOCHYENA ODDISH MARILL ODDISH ILLUMISE ILLUMISE ILLUMISE ILLUMISE VOLBEAT SEEDOT",
    118: "ZIGZAGOON ELECTRIKE ZIGZAGOON ELECTRIKE LINOONE MANECTRIC WINGULL WINGULL WINGULL WINGULL WINGULL KECLEON",
    119: "ZIGZAGOON LINOONE ZIGZAGOON ODDISH LINOONE ODDISH ODDISH ODDISH TROPIUS TROPIUS TROPIUS KECLEON",
    120: "POOCHYENA MIGHTYENA MIGHTYENA ODDISH MARILL ODDISH ODDISH MARILL ABSOL ABSOL KECLEON SEEDOT",
    121: "POOCHYENA SHUPPET MIGHTYENA SHUPPET MIGHTYENA ODDISH ODDISH GLOOM WINGULL WINGULL WINGULL KECLEON",
    123: "POOCHYENA SHUPPET MIGHTYENA SHUPPET MIGHTYENA ODDISH ODDISH GLOOM WINGULL WINGULL WINGULL KECLEON",
}

# Surf rates: 60%, 30%, 5%, 4%, 1%. Rod rates: Old (70/30),
# Good (60/20/20), Super (40/40/15/4/1).
SURF_COAST = "TENTACOOL WINGULL WINGULL PELIPPER PELIPPER"
SURF_RIVER = "MARILL MARILL MARILL MARILL GOLDEEN"
SURF_WAILORD = "TENTACOOL WINGULL WINGULL PELIPPER WAILORD"
FISH_COAST = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL WAILMER WAILMER WAILMER WAILMER WAILMER WAILMER"
FISH_SHARPEDO = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL WAILMER SHARPEDO WAILMER WAILMER WAILMER WAILMER"
FISH_CURRENTS = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL WAILMER SHARPEDO WAILMER HORSEA WAILMER WAILMER"
FISH_CORAL = "MAGIKARP TENTACOOL MAGIKARP LUVDISC WAILMER LUVDISC WAILMER CORSOLA WAILMER WAILMER"
FISH_CARVANHA = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL CARVANHA SHARPEDO CARVANHA CARVANHA CARVANHA CARVANHA"
FISH_RAIN = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL CARVANHA CARVANHA CARVANHA CARVANHA CARVANHA CARVANHA"
FISH_RIVER = "MAGIKARP GOLDEEN MAGIKARP GOLDEEN BARBOACH BARBOACH BARBOACH BARBOACH BARBOACH BARBOACH"
FISH_GARDEN = "MAGIKARP GOLDEEN MAGIKARP GOLDEEN CORPHISH CORPHISH CORPHISH CORPHISH CORPHISH CORPHISH"

# Route -> method -> {existing weighted slot: new species}. Species new to an
# area are concentrated in repeated slots; no new legendary or mythical spawns.
PATCHES = {
    105: {"water_mons": {2: "FINNEON", 4: "MANTYKE"},
          "fishing_mons": {6: "KRABBY", 7: "SHELLDER", 8: "HORSEA", 9: "CORSOLA"}},
    106: {"water_mons": {2: "DUCKLETT", 4: "FINNEON"},
          "fishing_mons": {6: "KRABBY", 7: "SHELLDER", 8: "REMORAID", 9: "CLAMPERL"}},
    107: {"water_mons": {2: "MANTYKE", 4: "FINNEON"},
          "fishing_mons": {6: "HORSEA", 7: "REMORAID", 8: "CHINCHOU", 9: "CORSOLA"}},
    108: {"water_mons": {2: "FRILLISH", 4: "FINNEON"},
          "fishing_mons": {6: "SHELLDER", 7: "CHINCHOU", 8: "SKRELP", 9: "DHELMISE"}},
    109: {"water_mons": {2: "BUIZEL", 4: "FINNEON"},
          "fishing_mons": {6: "KRABBY", 7: "CLAUNCHER", 8: "SHELLDER", 9: "CORSOLA"}},
    110: {"land_mons": {3: "SHINX", 7: "BUIZEL", 9: "PIDOVE", 11: "YAMPER"},
          "water_mons": {2: "BUIZEL", 4: "FINNEON"},
          "fishing_mons": {6: "SHELLDER", 7: "HORSEA", 8: "REMORAID", 9: "CHINCHOU"}},
    111: {"land_mons": {2: "SANDILE", 3: "HELIOPTILE", 5: "HIPPOPOTAS", 8: "DWEBBLE", 11: "SIGILYPH"},
          "water_mons": {2: "WOOPER", 3: "BUIZEL"},
          "fishing_mons": {6: "POLIWAG", 7: "TYMPOLE", 8: "CHEWTLE", 9: "BASCULIN"}},
    112: {"land_mons": {3: "ROLYCOLY", 4: "HOUNDOUR", 6: "MACHOP", 8: "SLUGMA", 11: "SALANDIT"}},
    113: {"land_mons": {3: "KOFFING", 4: "ROLYCOLY", 6: "NUMEL", 8: "HOUNDOUR", 11: "LARVESTA"}},
    114: {"land_mons": {2: "ROCKRUFF", 3: "ROGGENROLA", 4: "SEWADDLE", 8: "TIMBURR", 10: "DWEBBLE"},
          "water_mons": {2: "WOOPER", 3: "POLIWAG"},
          "fishing_mons": {6: "TYMPOLE", 7: "CHEWTLE", 8: "BASCULIN", 9: "TYNAMO"}},
    115: {"land_mons": {3: "ROCKRUFF", 4: "ROGGENROLA", 7: "DWEBBLE", 9: "SHELLOS", 11: "ABSOL"},
          "water_mons": {2: "FINNEON", 4: "MANTYKE"},
          "fishing_mons": {6: "KRABBY", 7: "SHELLDER", 8: "REMORAID", 9: "CORSOLA"}},
    117: {"land_mons": {2: "COTTONEE", 3: "PETILIL", 7: "BOUNSWEET", 8: "CUTIEFLY", 9: "COMBEE"},
          "water_mons": {2: "WOOPER", 3: "DEWPIDER"},
          "fishing_mons": {6: "POLIWAG", 7: "TYMPOLE", 8: "CHEWTLE", 9: "BASCULIN"}},
    118: {"land_mons": {2: "BUIZEL", 3: "SHINX", 7: "POLIWAG", 9: "DUCKLETT", 10: "SHELLOS"},
          "water_mons": {2: "BUIZEL", 4: "FRILLISH"},
          "fishing_mons": {7: "HORSEA", 8: "REMORAID", 9: "CHINCHOU"}},
    119: {"land_mons": {2: "SEWADDLE", 4: "FOONGUS", 6: "DEWPIDER", 7: "APPLIN", 9: "MORELULL", 10: "HERACROSS"},
          "water_mons": {2: "DEWPIDER", 4: "FRILLISH"},
          "fishing_mons": {6: "BARBOACH", 7: "POLIWAG", 8: "CHEWTLE", 9: "TYNAMO"}},
    120: {"land_mons": {2: "FOONGUS", 5: "CUTIEFLY", 6: "SEWADDLE", 7: "MORELULL", 9: "PHANTUMP"},
          "water_mons": {2: "WOOPER", 3: "DEWPIDER"},
          "fishing_mons": {6: "POLIWAG", 7: "TYMPOLE", 8: "BASCULIN", 9: "TYNAMO"}},
    121: {"land_mons": {3: "CUTIEFLY", 4: "COTTONEE", 6: "HERACROSS", 9: "BOUNSWEET", 10: "PHANTUMP"},
          "water_mons": {2: "FINNEON", 4: "FRILLISH"},
          "fishing_mons": {6: "HORSEA", 7: "REMORAID", 8: "SHELLDER", 9: "CORSOLA"}},
    122: {"water_mons": {2: "FRILLISH", 4: "MANTYKE"},
          "fishing_mons": {7: "FRILLISH", 8: "SKRELP", 9: "DHELMISE"}},
    123: {"land_mons": {3: "SMOLIV", 4: "BOUNSWEET", 6: "COMBEE", 9: "PETILIL", 10: "HERACROSS"},
          "water_mons": {2: "BUIZEL", 4: "DUCKLETT"},
          "fishing_mons": {6: "KRABBY", 7: "SHELLOS", 8: "CLAUNCHER", 9: "CORSOLA"}},
    124: {"water_mons": {2: "FINNEON", 4: "MANTYKE"},
          "fishing_mons": {7: "CHINCHOU", 8: "REMORAID", 9: "CLAUNCHER"}},
    125: {"water_mons": {2: "SEEL", 4: "MANTYKE"},
          "fishing_mons": {7: "SHELLDER", 8: "CHINCHOU", 9: "CLAUNCHER"}},
    126: {"water_mons": {2: "FINNEON", 4: "FRILLISH"},
          "fishing_mons": {7: "CHINCHOU", 8: "CLAUNCHER", 9: "CORSOLA"}},
    127: {"water_mons": {2: "MANTYKE", 4: "FRILLISH"},
          "fishing_mons": {7: "CHINCHOU", 8: "SKRELP", 9: "RELICANTH"}},
    128: {"water_mons": {2: "FINNEON", 4: "MANTYKE"},
          "fishing_mons": {8: "CLAUNCHER", 9: "MAREANIE"}},
    129: {"water_mons": {2: "MANTYKE"},
          "fishing_mons": {7: "CHINCHOU", 8: "REMORAID", 9: "RELICANTH"}},
    130: {"water_mons": {2: "FINNEON", 4: "FRILLISH"},
          "fishing_mons": {7: "HORSEA", 8: "REMORAID", 9: "MAREANIE"}},
    131: {"water_mons": {2: "FINNEON", 4: "MANTYKE"},
          "fishing_mons": {7: "REMORAID", 8: "CHINCHOU", 9: "CORSOLA"}},
    132: {"water_mons": {2: "MANTYKE", 4: "FINNEON"},
          "fishing_mons": {8: "CHINCHOU", 9: "CLAUNCHER"}},
    133: {"water_mons": {2: "FRILLISH", 4: "MANTYKE"},
          "fishing_mons": {8: "SKRELP", 9: "MAREANIE"}},
    134: {"water_mons": {2: "FINNEON", 4: "FRILLISH"},
          "fishing_mons": {8: "CHINCHOU", 9: "RELICANTH"}},
}


def original_slots(route, method):
    if method == "land_mons":
        return LAND_BASELINES[route].split()
    if method == "water_mons":
        if route in {111, 114, 117, 120}:
            return SURF_RIVER.split()
        return (SURF_WAILORD if route == 129 else SURF_COAST).split()
    if method == "fishing_mons":
        if route in {111, 114, 120}:
            profile = FISH_RIVER
        elif route == 117:
            profile = FISH_GARDEN
        elif route == 118:
            profile = FISH_CARVANHA
        elif route == 119:
            profile = FISH_RAIN
        elif route == 128:
            profile = FISH_CORAL
        elif route in {132, 133, 134}:
            profile = FISH_CURRENTS
        elif route in {122, 124, 125, 126, 127, 129, 130, 131}:
            profile = FISH_SHARPEDO
        else:
            profile = FISH_COAST
        return profile.split()
    raise ValueError(f"Unsupported encounter method: {method}")


def run(write):
    if set(PATCHES) != (set(range(105, 135)) - {116}):
        raise ValueError("Missing or unexpected route in the curated manifest")
    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    maps = {entry["map"]: entry for entry in data["wild_encounter_groups"][0]["encounters"]}
    species_source = (ROOT / "include/constants/species.h").read_text(encoding="utf-8")
    valid_species = set(re.findall(r"\b(SPECIES_\w+)\s*=", species_source))

    # The complete Mirage Island Wynaut grass table is intentionally untouched.
    island = maps["MAP_ROUTE130"]["land_mons"]["mons"]
    if len(island) != 12 or any(mon["species"] != "SPECIES_WYNAUT" for mon in island):
        raise ValueError("Mirage Island native Wynaut encounters must stay intact")

    placements = 0
    methods = 0
    for number, method_patches in sorted(PATCHES.items()):
        route = f"MAP_ROUTE{number}"
        if route not in maps:
            raise ValueError(f"Encounter map missing: {route}")
        for method, replacements in method_patches.items():
            methods += 1
            mons = maps[route][method]["mons"]
            baseline = original_slots(number, method)
            if len(mons) != len(baseline):
                raise ValueError(f"Unexpected number of slots: {route}/{method}")
            for index, (mon, original) in enumerate(zip(mons, baseline)):
                new = replacements.get(index, original)
                species = "SPECIES_" + new
                if species not in valid_species:
                    raise ValueError(f"Unknown species: {route}/{method}[{index}] {species}")
                before = mon["species"]
                if before not in {"SPECIES_" + original, species}:
                    raise ValueError(f"Unexpected native slot: {route}/{method}[{index}] {before}")
                if write:
                    mon["species"] = species
                elif before != species:
                    raise ValueError(f"Unapplied slot: {route}/{method}[{index}] expected {species}; run --write")
            if any(index < 0 or index >= len(mons) for index in replacements):
                raise ValueError(f"Out-of-range slot: {route}/{method}")

            # Guard the original method and fishing rod availability, too.
            rod_groups = ((0, 2), (2, 5), (5, 10)) if method == "fishing_mons" else ((0, len(mons)),)
            for start, end in rod_groups:
                native = set(baseline[start:end])
                present = {mon["species"].removeprefix("SPECIES_") for mon in mons[start:end]}
                missing = native - present
                if missing:
                    raise ValueError(f"Lost native encounter in {route}/{method} [{start}:{end}]: {sorted(missing)}")
            placements += len(replacements)

    if write:
        ENCOUNTERS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Hoenn ecology remaining: {len(PATCHES)} routes, {methods} tables, {placements} curated slots; all native species and rod tiers preserved")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="Apply curation to wild_encounters.json")
    run(parser.parse_args().write)
