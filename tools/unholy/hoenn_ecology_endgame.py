#!/usr/bin/env python3
"""Curate the remaining major Hoenn dungeons by floor and habitat.

Each replacement checks a frozen vanilla slot identity. All original Pokémon
remain available with their original method and fishing-rod tier. Encounter
weights and level bands are not modified. Run --write to apply, otherwise
validate the applied manifest.
"""

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ENCOUNTERS = ROOT / "src/data/wild_encounters.json"


def slots(*names):
    return " ".join(names)


PYRE_LOWER = slots(*(["SHUPPET"] * 12))
PYRE_UPPER = slots(*(["SHUPPET"] * 8 + ["DUSKULL"] * 4))
SHOAL_COMMON = slots("ZUBAT", "SPHEAL", "ZUBAT", "SPHEAL", "ZUBAT", "SPHEAL", "ZUBAT", "SPHEAL", "GOLBAT", "SPHEAL", "GOLBAT", "SPHEAL")
SEAFLOOR_COMMON = slots(*(["ZUBAT"] * 8 + ["GOLBAT"] * 4))
SKY_COMMON = slots("SABLEYE", "GOLBAT", "GOLBAT", "SABLEYE", "CLAYDOL", "BANETTE", "BANETTE", "CLAYDOL", "CLAYDOL", "CLAYDOL", "CLAYDOL", "CLAYDOL")

# Static reference slots are based on the original Emerald encounter tables.
LAND = {
    "MAP_MT_PYRE_EXTERIOR": slots("SHUPPET", "SHUPPET", "SHUPPET", "SHUPPET", "VULPIX", "VULPIX", "VULPIX", "VULPIX", "WINGULL", "WINGULL", "WINGULL", "WINGULL"),
    "MAP_MT_PYRE_SUMMIT": slots("SHUPPET", "SHUPPET", "SHUPPET", "SHUPPET", "SHUPPET", "SHUPPET", "SHUPPET", "DUSKULL", "DUSKULL", "DUSKULL", "CHIMECHO", "CHIMECHO"),
    "MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM": slots("ZUBAT", "SPHEAL", "ZUBAT", "SPHEAL", "ZUBAT", "SPHEAL", "SNORUNT", "SPHEAL", "GOLBAT", "SNORUNT", "GOLBAT", "SNORUNT"),
    "MAP_VICTORY_ROAD_1F": slots("GOLBAT", "HARIYAMA", "LAIRON", "LOUDRED", "ZUBAT", "MAKUHITA", "GOLBAT", "HARIYAMA", "ARON", "WHISMUR", "ARON", "WHISMUR"),
    "MAP_VICTORY_ROAD_B1F": slots("GOLBAT", "HARIYAMA", "LAIRON", "LAIRON", "GOLBAT", "HARIYAMA", "GOLBAT", "HARIYAMA", "LAIRON", "MAWILE", "LAIRON", "MAWILE"),
    "MAP_VICTORY_ROAD_B2F": slots("GOLBAT", "SABLEYE", "LAIRON", "LAIRON", "GOLBAT", "SABLEYE", "GOLBAT", "SABLEYE", "LAIRON", "MAWILE", "LAIRON", "MAWILE"),
    "MAP_CAVE_OF_ORIGIN_1F": slots("ZUBAT", "ZUBAT", "ZUBAT", "SABLEYE", "SABLEYE", "SABLEYE", "ZUBAT", "ZUBAT", "GOLBAT", "GOLBAT", "GOLBAT", "GOLBAT"),
    "MAP_SKY_PILLAR_5F": slots("SABLEYE", "GOLBAT", "GOLBAT", "SABLEYE", "CLAYDOL", "BANETTE", "BANETTE", "CLAYDOL", "CLAYDOL", "ALTARIA", "ALTARIA", "ALTARIA"),
}
for floor in (1, 2, 3):
    LAND[f"MAP_MT_PYRE_{floor}F"] = PYRE_LOWER
for floor in (4, 5, 6):
    LAND[f"MAP_MT_PYRE_{floor}F"] = PYRE_UPPER
for zone in ("ENTRANCE_ROOM", "STAIRS_ROOM", "LOWER_ROOM", "INNER_ROOM"):
    LAND[f"MAP_SHOAL_CAVE_LOW_TIDE_{zone}"] = SHOAL_COMMON
for room in range(1, 9):
    LAND[f"MAP_SEAFLOOR_CAVERN_ROOM{room}"] = SEAFLOOR_COMMON
LAND["MAP_CAVE_OF_ORIGIN_ENTRANCE"] = SEAFLOOR_COMMON
for floor in (1, 3):
    LAND[f"MAP_SKY_PILLAR_{floor}F"] = SKY_COMMON

ROCK_VICTORY = "GRAVELER GEODUDE GRAVELER GRAVELER GRAVELER"
SURF_SHOAL = "TENTACOOL ZUBAT SPHEAL SPHEAL SPHEAL"
SURF_SEAFLOOR = "TENTACOOL ZUBAT ZUBAT GOLBAT GOLBAT"
SURF_VICTORY = "GOLBAT GOLBAT GOLBAT GOLBAT GOLBAT"
FISH_SEA = "MAGIKARP TENTACOOL MAGIKARP TENTACOOL WAILMER WAILMER WAILMER WAILMER WAILMER WAILMER"
FISH_VICTORY = "MAGIKARP GOLDEEN MAGIKARP GOLDEEN BARBOACH BARBOACH BARBOACH WHISCASH WHISCASH WHISCASH"

# 1% floor/rod slots hold the genuinely exceptional finds. No legendary or
# mythical encounters are inserted into random wild tables.
PATCHES = {
    "MAP_MT_PYRE_1F": {"land_mons": {3: "DUSKULL", 5: "GASTLY", 7: "YAMASK", 9: "LITWICK", 11: "PHANTUMP"}},
    "MAP_MT_PYRE_2F": {"land_mons": {3: "DUSKULL", 5: "GASTLY", 7: "SINISTEA", 9: "LITWICK", 11: "MISDREAVUS"}},
    "MAP_MT_PYRE_3F": {"land_mons": {3: "DUSKULL", 5: "GASTLY", 7: "YAMASK", 9: "LITWICK", 11: "MIMIKYU"}},
    "MAP_MT_PYRE_4F": {"land_mons": {3: "GASTLY", 5: "LITWICK", 7: "YAMASK", 9: "MISDREAVUS", 11: "GOLETT"}},
    "MAP_MT_PYRE_5F": {"land_mons": {3: "GASTLY", 5: "LITWICK", 7: "SINISTEA", 9: "GOLETT", 11: "MIMIKYU"}},
    "MAP_MT_PYRE_6F": {"land_mons": {3: "YAMASK", 5: "LITWICK", 7: "GOLETT", 9: "MISDREAVUS", 11: "SPIRITOMB"}},
    "MAP_MT_PYRE_EXTERIOR": {"land_mons": {2: "PHANTUMP", 5: "DUSKULL", 9: "MURKROW", 11: "MISDREAVUS"}},
    "MAP_MT_PYRE_SUMMIT": {"land_mons": {3: "LITWICK", 5: "GOLETT", 6: "GASTLY", 8: "MISDREAVUS", 9: "PHANTUMP", 11: "MIMIKYU"}},
    "MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM": {
        "land_mons": {2: "SHELLDER", 4: "SEEL", 6: "WOOBAT", 9: "BERGMITE", 11: "SNOM"},
        "water_mons": {3: "SHELLDER", 4: "BERGMITE"},
        "fishing_mons": {6: "SHELLDER", 7: "CHINCHOU", 8: "REMORAID", 9: "CLOBBOPUS"},
    },
    "MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM": {"land_mons": {2: "SEEL", 4: "SHELLDER", 6: "WOOBAT", 9: "BERGMITE", 11: "SNOM"}},
    "MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM": {"land_mons": {2: "SEEL", 4: "BERGMITE", 6: "WOOBAT", 9: "SHELLDER", 11: "CRYOGONAL"}},
    "MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM": {
        "land_mons": {2: "SEEL", 4: "SHELLDER", 6: "BERGMITE", 9: "WOOBAT", 11: "SNOM"},
        "water_mons": {3: "SHELLDER", 4: "BERGMITE"},
        "fishing_mons": {6: "CHINCHOU", 7: "SHELLDER", 8: "REMORAID", 9: "LAPRAS"},
    },
    "MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM": {"land_mons": {2: "BERGMITE", 3: "SEEL", 4: "SNOM", 7: "CRYOGONAL", 11: "AMAURA"}},
    "MAP_VICTORY_ROAD_1F": {"land_mons": {6: "DRILBUR", 7: "ROCKRUFF", 10: "RIOLU", 11: "AXEW"}},
    "MAP_VICTORY_ROAD_B1F": {
        "land_mons": {4: "DURANT", 5: "DRILBUR", 6: "NOIBAT", 7: "RIOLU", 8: "AXEW", 10: "GIBLE", 11: "CARBINK"},
        "rock_smash_mons": {4: "CARBINK"},
    },
    "MAP_VICTORY_ROAD_B2F": {
        "land_mons": {4: "NOIBAT", 5: "RIOLU", 6: "DURANT", 7: "DRILBUR", 8: "AXEW", 10: "DEINO", 11: "GIBLE"},
        "water_mons": {1: "WOOBAT", 2: "TYNAMO", 3: "POLIWAG", 4: "DRATINI"},
        "fishing_mons": {6: "TYNAMO", 8: "BASCULIN", 9: "DRATINI"},
    },
    "MAP_CAVE_OF_ORIGIN_ENTRANCE": {"land_mons": {2: "WOOBAT", 3: "CARBINK", 4: "GLIMMET", 6: "ROGGENROLA", 7: "NOIBAT", 9: "SABLEYE", 11: "MINIOR"}},
    "MAP_CAVE_OF_ORIGIN_1F": {"land_mons": {2: "WOOBAT", 4: "CARBINK", 6: "GLIMMET", 7: "NOIBAT", 9: "MINIOR", 11: "GIBLE"}},
    "MAP_SKY_PILLAR_1F": {"land_mons": {2: "NOIBAT", 3: "ROCKRUFF", 6: "LUNATONE", 7: "MINIOR", 8: "CARBINK", 9: "SWABLU", 10: "ALTARIA", 11: "AXEW"}},
    "MAP_SKY_PILLAR_3F": {"land_mons": {2: "NOIBAT", 3: "LUNATONE", 6: "DRATINI", 7: "MINIOR", 8: "CARBINK", 9: "AXEW", 10: "DEINO", 11: "GIBLE"}},
    "MAP_SKY_PILLAR_5F": {"land_mons": {2: "NOIBAT", 3: "LUNATONE", 6: "DRATINI", 7: "MINIOR", 8: "CARBINK", 10: "AXEW", 11: "DEINO"}},
    "MAP_SEAFLOOR_CAVERN_ENTRANCE": {
        "water_mons": {2: "FRILLISH", 4: "MAREANIE"},
        "fishing_mons": {6: "CHINCHOU", 7: "SKRELP", 8: "MAREANIE", 9: "DHELMISE"},
    },
}

SEAFLOOR_LAND = {
    1: {2: "WOOBAT", 3: "BINACLE", 4: "DWEBBLE", 6: "WIMPOD", 7: "CARBINK", 9: "SABLEYE", 11: "GLIMMET"},
    2: {2: "WOOBAT", 3: "BINACLE", 4: "ROCKRUFF", 6: "SABLEYE", 7: "GLIMMET", 9: "CARBINK", 11: "NOIBAT"},
    3: {2: "WOOBAT", 3: "DWEBBLE", 4: "DRILBUR", 6: "CARBINK", 7: "GLIMMET", 9: "SABLEYE", 11: "NOIBAT"},
    4: {2: "WOOBAT", 3: "BINACLE", 4: "DWEBBLE", 6: "CARBINK", 7: "ROCKRUFF", 9: "SABLEYE", 11: "GLIMMET"},
    5: {2: "WOOBAT", 3: "DRILBUR", 4: "FERROSEED", 6: "CARBINK", 7: "SABLEYE", 9: "GLIMMET", 11: "MINIOR"},
    6: {2: "WOOBAT", 3: "BINACLE", 4: "DWEBBLE", 6: "FERROSEED", 7: "CARBINK", 9: "SABLEYE", 11: "GLIMMET"},
    7: {2: "WOOBAT", 3: "DWEBBLE", 4: "FERROSEED", 6: "CARBINK", 7: "SABLEYE", 9: "GLIMMET", 11: "MINIOR"},
    8: {2: "WOOBAT", 3: "DRILBUR", 4: "FERROSEED", 6: "CARBINK", 7: "SABLEYE", 9: "GLIMMET", 11: "DHELMISE"},
}
for room, land_slots in SEAFLOOR_LAND.items():
    PATCHES[f"MAP_SEAFLOOR_CAVERN_ROOM{room}"] = {"land_mons": land_slots}
for room, water, fish in (
    (6, {2: "FRILLISH", 4: "MAREANIE"}, {6: "SKRELP", 7: "CLAUNCHER", 8: "CHINCHOU", 9: "RELICANTH"}),
    (7, {2: "FRILLISH", 4: "MAREANIE"}, {6: "CHINCHOU", 7: "CLAUNCHER", 8: "MAREANIE", 9: "DHELMISE"}),
):
    PATCHES[f"MAP_SEAFLOOR_CAVERN_ROOM{room}"].update({"water_mons": water, "fishing_mons": fish})


def original_slots(map_name, method):
    if method == "land_mons":
        return LAND[map_name].split()
    if method == "rock_smash_mons" and map_name == "MAP_VICTORY_ROAD_B1F":
        return ROCK_VICTORY.split()
    if method == "water_mons":
        if map_name.startswith("MAP_SHOAL_CAVE_"):
            return SURF_SHOAL.split()
        if map_name == "MAP_VICTORY_ROAD_B2F":
            return SURF_VICTORY.split()
        if map_name.startswith("MAP_SEAFLOOR_CAVERN_"):
            return SURF_SEAFLOOR.split()
    if method == "fishing_mons":
        if map_name == "MAP_VICTORY_ROAD_B2F":
            return FISH_VICTORY.split()
        if map_name.startswith(("MAP_SEAFLOOR_CAVERN_", "MAP_SHOAL_CAVE_")):
            return FISH_SEA.split()
    raise ValueError(f"Unexpected encounter method: {map_name}/{method}")


def run(write):
    if set(PATCHES) != set(LAND) | {"MAP_SEAFLOOR_CAVERN_ENTRANCE"}:
        raise ValueError("A dungeon map is missing from the manifests")
    data = json.loads(ENCOUNTERS.read_text(encoding="utf-8"))
    maps = {entry["map"]: entry for entry in data["wild_encounter_groups"][0]["encounters"]}
    ids = set(re.findall(r"\b(SPECIES_\w+)\s*=", (ROOT / "include/constants/species.h").read_text(encoding="utf-8")))

    placements = 0
    methods = 0
    for map_name, method_patches in PATCHES.items():
        if map_name not in maps:
            raise ValueError(f"Encounter map does not exist: {map_name}")
        for method, replacements in method_patches.items():
            methods += 1
            mons = maps[map_name][method]["mons"]
            original = original_slots(map_name, method)
            if len(mons) != len(original) or any(index < 0 or index >= len(mons) for index in replacements):
                raise ValueError(f"Invalid number of encounter slots: {map_name}/{method}")
            for index, (mon, native) in enumerate(zip(mons, original)):
                expected = "SPECIES_" + replacements.get(index, native)
                if expected not in ids:
                    raise ValueError(f"Unknown species {expected} in {map_name}/{method}[{index}]")
                if mon["species"] not in {"SPECIES_" + native, expected}:
                    raise ValueError(f"Slot drift: {map_name}/{method}[{index}]={mon['species']}")
                if write:
                    mon["species"] = expected
                elif mon["species"] != expected:
                    raise ValueError(f"Unapplied encounter {map_name}/{method}[{index}]; run --write")

            rods = ((0, 2), (2, 5), (5, 10)) if method == "fishing_mons" else ((0, len(mons)),)
            for start, end in rods:
                originally = set(original[start:end])
                now = {mon["species"].removeprefix("SPECIES_") for mon in mons[start:end]}
                if originally - now:
                    raise ValueError(f"Lost native encounter {map_name}/{method} {start}:{end}: {sorted(originally - now)}")
            placements += len(replacements)

    # Do not modify the deprecated Ruby/Sapphire-only Origin floor tables or
    # introduce wild legendary Pokémon that replace their static story events.
    for map_name in PATCHES:
        for method in PATCHES[map_name]:
            for mon in maps[map_name][method]["mons"]:
                if mon["species"] in {"SPECIES_KYOGRE", "SPECIES_GROUDON", "SPECIES_RAYQUAZA"}:
                    raise ValueError(f"Boss legendary added to random encounters: {map_name}")

    if write:
        ENCOUNTERS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Hoenn endgame ecology: {len(PATCHES)} maps, {methods} tables, {placements} curated slots; native methods and rod tiers preserved")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="Apply the curated endgame encounter expansion")
    run(parser.parse_args().write)
