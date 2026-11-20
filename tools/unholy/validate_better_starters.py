#!/usr/bin/env python3
"""Validate Better Starter pools against canonical Pokémon species data.

The 18 options must all be true starting-form members of the selected type,
within a modest initial-stat budget. Also guard the opening script, save-vars
and automatic fallback before Birch's original three-Poké-Ball selection.
"""

import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/unholy_starters.c").read_text(encoding="utf-8")
SPECIES = (ROOT / "include/constants/species.h").read_text(encoding="utf-8")


def initializer(name):
    match = re.search(rf"static const u16 {name}\[[^;]+?=\s*\{{(.*?)\n\}};", SOURCE, re.S)
    if not match:
        raise ValueError(f"Could not locate {name}")
    return match.group(1)


types = re.findall(r"TYPE_\w+", initializer("sStarterTypes"))
pools = [re.findall(r"SPECIES_\w+", group) for group in re.findall(r"\{([^{}]+)\}", initializer("sStarterPools"))]
expected_types = ["TYPE_NORMAL", "TYPE_FIRE", "TYPE_WATER", "TYPE_ELECTRIC", "TYPE_GRASS", "TYPE_ICE", "TYPE_FIGHTING", "TYPE_POISON", "TYPE_GROUND", "TYPE_FLYING", "TYPE_PSYCHIC", "TYPE_BUG", "TYPE_ROCK", "TYPE_GHOST", "TYPE_DRAGON", "TYPE_DARK", "TYPE_STEEL", "TYPE_FAIRY"]
if types != expected_types or len(pools) != 18:
    raise ValueError(f"Expected all 18 unique Pokémon types, got {len(types)} types and {len(pools)} pools")

defined_species = set(re.findall(r"\b(SPECIES_\w+)\s*=", SPECIES))
aliases = dict(re.findall(r"\b(SPECIES_\w+)\s*=\s*(SPECIES_\w+)", SPECIES))
info = {}
evolved_species = set()
for path in (ROOT / "src/data/pokemon/species_info").glob("gen_*_families.h"):
    text = path.read_text(encoding="utf-8")
    # Evolutions are declared alongside species. Exclude their targets from
    # first-stage pools even if their early stats happen to meet our budget.
    for evolution in re.findall(r"\.evolutions\s*=\s*EVOLUTION\((.*?)\),", text, re.S):
        evolved_species.update(re.findall(r"\{EVO_\w+,\s*[^,{}]+,\s*(SPECIES_\w+)", evolution))
    entries = list(re.finditer(r"\[(SPECIES_\w+)\]\s*=\s*\{", text))
    for i, entry in enumerate(entries):
        chunk = text[entry.end(): entries[i + 1].start() if i + 1 < len(entries) else len(text)]
        species_end = re.search(r"\n    \},", chunk)
        if species_end:
            chunk = chunk[:species_end.end()]
        match = re.search(r"\.types\s*=\s*(?:MON_TYPES\(([^)]+)\)|(\w+))", chunk)
        if match:
            mon_types = set()
            type_list = match.group(1) or match.group(2)
            for type_token in type_list.split(","):
                type_token = type_token.strip()
                if type_token.startswith("TYPE_"):
                    mon_types.add(type_token)
                else:
                    definitions = re.findall(rf"^\s*#define\s+{re.escape(type_token)}\s+([^\n]+)", text, re.M)
                    for definition in definitions:
                        mon_types.update(re.findall(r"TYPE_\w+", definition))
            stat_lines = re.findall(r"\.(baseHP|baseAttack|baseDefense|baseSpeed|baseSpAttack|baseSpDefense)\s*=\s*([^,\n]+)", chunk)
            # Some generations gate a stat (e.g. Rattata's Sp. Def) with a
            # version ternary. Use the maximum possible numeric branch.
            stats = {}
            for stat, expression in stat_lines:
                values = [int(x) for x in re.findall(r"(?<![A-Za-z_])\d+", expression)]
                if not values:
                    macro = expression.strip()
                    definition = re.search(rf"^#define\s+{re.escape(macro)}\s+([^\n]+)", text, re.M)
                    if definition:
                        values = [int(x) for x in re.findall(r"(?<![A-Za-z_])\d+", definition.group(1))]
                if values:
                    stats[stat] = max(values)
            info[entry.group(1)] = (mon_types, stats)

all_candidates = set()
for selection, (chosen_type, pool) in enumerate(zip(types, pools)):
    if len(pool) != 8 or len(set(pool)) != 8:
        raise ValueError(f"Type {chosen_type} has {len(pool)} options, expected eight unique candidates")
    for mon in pool:
        if mon not in defined_species:
            raise ValueError(f"Unknown candidate {mon}")
        info_name = mon
        while info_name not in info and info_name in aliases:
            info_name = aliases[info_name]
        if info_name not in info:
            raise ValueError(f"No source Pokémon type data for {mon}")
        actual_types, stats = info[info_name]
        if chosen_type not in actual_types:
            raise ValueError(f"{mon} in {chosen_type} menu, but actual type is {sorted(actual_types)}")
        if mon in evolved_species:
            raise ValueError(f"{mon} already evolves from another Pokémon: only unevolved forms permitted")
        if len(stats) != 6 or sum(int(x) for x in stats.values()) > 400:
            raise ValueError(f"{mon} exceeds level-five stat budget or has missing stats: {stats}")
        all_candidates.add(mon)

vars_header = (ROOT / "include/constants/vars.h").read_text(encoding="utf-8")
for name, owner in (("TYPE", "105"), ("OFFER_0", "106"), ("OFFER_1", "107"), ("OFFER_2", "108")):
    if f"#define VAR_UNHOLY_STARTER_{name}" not in vars_header or f"VAR_ROUTE{owner}_STATE" not in vars_header:
        raise ValueError(f"Persistent starter variable {name} missing")

script = (ROOT / "data/scripts/players_house.inc").read_text(encoding="utf-8")
route = (ROOT / "data/maps/Route101/scripts.inc").read_text(encoding="utf-8")
menu = (ROOT / "src/field_specials.c").read_text(encoding="utf-8")
menu_block = re.search(r"\[SCROLL_MULTI_UNHOLY_STARTER_TYPES\]\s*=\s*\{(.*?)\n\s*\}", menu, re.S)
if "call UnholyStarters_EventScript_SelectFavoriteType" not in script:
    raise ValueError("Mother does not ask the favorite type during the compulsory introduction")
if "special UnholyStarterOptionsReady" not in route or "call UnholyStarters_EventScript_SelectFavoriteType" not in route:
    raise ValueError("Birch's bag must recover from bypassed introductions and old saves")
if not menu_block:
    raise ValueError("The scrollable type menu is missing")
menu_labels = re.findall(r'COMPOUND_STRING\("([^"]+)"\)', menu_block.group(1))
if menu_labels != ["NORMAL", "FUEGO", "AGUA", "ELÉCTRICO", "PLANTA", "HIELO", "LUCHA", "VENENO", "TIERRA", "VOLADOR", "PSÍQUICO", "BICHO", "ROCA", "FANTASMA", "DRAGÓN", "SINIESTRO", "ACERO", "HADA"]:
    raise ValueError(f"The menu must display 18 types in exactly the same order as the pools: {menu_labels}")
if "return candidate;" not in (ROOT / "src/starter_choose.c").read_text(encoding="utf-8"):
    raise ValueError("The three Poké Balls do not read the generated starter species")

print(f"Better Starter: 18 verified types, 144 pool entries, {len(all_candidates)} distinct first-stage Pokémon; save, mother, rescue and menu hooks present")
