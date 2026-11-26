#!/usr/bin/env python3
"""Generate a conservative National Dex acquisition inventory (stdlib only).

Run from repository root:
  python3 tools/genesis_census.py --output /tmp/genesis-census.csv

A lexical reference is evidence for review, NOT proof a species is obtainable.
No source means no source found by this scanner, not unobtainable.
"""
import argparse
import csv
import json
import re
from collections import defaultdict, Counter
from pathlib import Path

SPECIES = re.compile(r"\bSPECIES_([A-Z0-9_]+)\b")
DEX_ENTRY = re.compile(r"^\s*NATIONAL_DEX_([A-Z0-9_]+),\s*$", re.M)
FAMILY = re.compile(r"^#if\s+P_FAMILY_([A-Z0-9_]+)")
FAMILY_END = re.compile(r"^#endif\s*//\s*P_FAMILY_")
ENTRY = re.compile(r"^\s*\[SPECIES_([A-Z0-9_]+)\]\s*=")
DEX_FIELD = re.compile(r"\.natDexNum\s*=\s*NATIONAL_DEX_([A-Z0-9_]+)")
SCRIPT_SOURCE = re.compile(r"\b(?:givemon|givepokemon|setwildbattle|seteventmon)\s+SPECIES_([A-Z0-9_]+)")
C_CALL = re.compile(r"\b(?:CreateMon|CreateBoxMon|ScriptGiveMon|GiveMonToPlayer|TryAddRoamer|CreateWildMon|CreateEventLegalMon)\s*\([^;{}]*?\bSPECIES_([A-Z0-9_]+)", re.S)
ACQUISITION_FILE = re.compile(r"(raid|shop|mall|fossil|gift|roam|trade|encounter|starter|egg|bounty|outbreak|swarm|cofradia)", re.I)
EXCLUDE = re.compile(r"(trainer|learnset|species_info|pokedex|sprite|graphics|animation|battle_ai)", re.I)


def audit(root, output):
    dex = [n for n in DEX_ENTRY.findall((root / "include/constants/pokedex.h").read_text()) if n != "NONE"]
    aliases, families = {}, {}
    info = root / "src/data/pokemon/species_info"
    files = sorted(info.glob("gen_*_families.h"))
    if not files:
        raise RuntimeError("No species family headers found")
    for file in files:
        family = species = None
        for line in file.read_text().splitlines():
            match = FAMILY.match(line)
            if match:
                family, species = match.group(1), None
            if FAMILY_END.match(line):
                family = species = None
            match = ENTRY.match(line)
            if match:
                species = match.group(1)
            match = DEX_FIELD.search(line)
            if species and match:
                name = match.group(1)
                aliases[species] = name
                families[name] = family or name
    dex_set = set(dex)
    wild, scripted, candidates = (defaultdict(set) for _ in range(3))
    unresolved = set()

    def add(target, token, source):
        name = aliases.get(token, token)
        if name in dex_set:
            target[name].add(source)
        else:
            unresolved.add(token)

    groups = json.loads((root / "src/data/wild_encounters.json").read_text())["wild_encounter_groups"]
    group = next(g for g in groups if g["label"] == "gWildMonHeaders")
    for encounter in group["encounters"]:
        for token in SPECIES.findall(json.dumps(encounter)):
            add(wild, token, encounter["map"])
    for file in sorted((root / "data").rglob("*.inc")):
        for token in SCRIPT_SOURCE.findall(file.read_text(errors="replace")):
            add(scripted, token, str(file.relative_to(root)))
    for directory in ("src", "data"):
        for file in sorted((root / directory).rglob("*")):
            if not file.is_file() or file.suffix not in {".c", ".h", ".inc", ".json"}:
                continue
            relative = str(file.relative_to(root))
            if EXCLUDE.search(relative) or relative == "src/data/wild_encounters.json":
                continue
            if file.suffix in {".c", ".h"}:
                content = file.read_text(errors="replace")
                for token in C_CALL.findall(content):
                    add(candidates, token, relative + ":call")
            elif not ACQUISITION_FILE.search(relative):
                continue
            else:
                content = file.read_text(errors="replace")
            if ACQUISITION_FILE.search(relative):
                for token in SPECIES.findall(content):
                    add(candidates, token, relative + ":reference")

    family_sources = defaultdict(set)
    for name in set(wild) | set(scripted):
        family_sources[families.get(name, name)].add(name)
    counts = Counter()
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(["national_dex", "species", "family", "status", "ordinary_wild_maps",
                         "scripted_sources", "other_references_review", "family_members_with_source"])
        for number, name in enumerate(dex, 1):
            family = families.get(name, name)
            members = family_sources[family]
            if wild[name]:
                status = "ordinary_wild_reference"
            elif scripted[name]:
                status = "scripted_reference"
            elif candidates[name]:
                status = "other_reference_review"
            elif members:
                status = "family_present_review_evolution"
            else:
                status = "no_source_in_audited_tables"
            counts[status] += 1
            writer.writerow([number, name, family, status, ";".join(sorted(wild[name])),
                             ";".join(sorted(scripted[name])), ";".join(sorted(candidates[name])),
                             ";".join(sorted(members))])
    print(f"National Dex entries: {len(dex)}")
    for status, count in sorted(counts.items()):
        print(f"{status}: {count}")
    print("Family mapping fallback:", ", ".join(sorted(dex_set - families.keys())) or "none")
    print("Unresolved species tokens:", ", ".join(sorted(unresolved)) or "none")
    print("Review all references and evolution methods; no category proves unobtainability.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--output", type=Path, default=Path("genesis_census.csv"))
    args = parser.parse_args()
    audit(args.root, args.output)
