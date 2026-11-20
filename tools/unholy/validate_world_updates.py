#!/usr/bin/env python3
"""Check world-update persistence, progressive spawns and map safety.

The on-map A-button flow still requires emulator QA; this validator prevents
save-flag regressions and impossible object placements at authoring time.
"""

import json
import re
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LAYOUTS = {row["id"]: row for row in json.loads((ROOT / "data/layouts/layouts.json").read_text())["layouts"]}

# Map, existing-event count, event's stable ordinal, visibility flag, expected
# position, reward (or None for NPC), and permanent claimed flag (if any).
EVENTS = [
    ("Route102", 9, 9, "FLAG_UNHOLY_WORLD_ROUTE102_BALL_HIDE", (22, 9), "ITEM_GREAT_BALL", "FLAG_UNHOLY_WORLD_ROUTE102_BALL_CLAIMED"),
    ("Route102", 9, 10, "FLAG_UNHOLY_WORLD_ROUTE102_NPC_HIDE", (30, 12), None, None),
    ("Route104", 34, 34, "FLAG_UNHOLY_WORLD_ROUTE104_BALL_HIDE", (27, 7), "ITEM_ULTRA_BALL", "FLAG_UNHOLY_WORLD_ROUTE104_BALL_CLAIMED"),
    ("Route110", 37, 37, "FLAG_UNHOLY_WORLD_ROUTE110_NPC_HIDE", (15, 8), None, None),
]


def fail(message):
    raise ValueError(message)


flags = (ROOT / "include/constants/flags.h").read_text(encoding="utf-8")
vars_header = (ROOT / "include/constants/vars.h").read_text(encoding="utf-8")
world_source = (ROOT / "src/unholy_world_update.c").read_text(encoding="utf-8")
overworld = (ROOT / "src/overworld.c").read_text(encoding="utf-8")

if not re.search(r"#define VAR_UNHOLY_WORLD_VERSION\s+VAR_ROUTE109_STATE", vars_header):
    fail("World version must occupy previously unused persistent Route109 variable")
if not re.search(r"#define UNHOLY_WORLD_VERSION_LATEST\s+1\b", world_source):
    fail("World update V1 needs a stable version number")
if "VarSet(VAR_UNHOLY_WORLD_VERSION, UNHOLY_WORLD_VERSION_LATEST)" not in world_source:
    fail("Legacy saves do not receive a world version")
if overworld.count("LoadCurrentMapData();\n    UnholyWorldUpdate_Reconcile();") != 2:
    fail("Reconciliation must run before map object spawning on both warp and camera transitions")
if "!FlagGet(claimedFlag)" not in world_source or re.search(r"FlagClear\(\s*claimedFlag\s*\)", world_source):
    fail("Collected world-update rewards must never be re-enabled")

flag_aliases = {}
for line in flags.splitlines():
    match = re.match(r"#define (FLAG_UNHOLY_WORLD_\w+)\s+(FLAG_UNUSED_0x\w+)", line)
    if match:
        flag_aliases[match.group(1)] = match.group(2)
if len(flag_aliases) != 6 or len(set(flag_aliases.values())) != 6:
    fail("World Update's six permanent flags must be uniquely allocated")

event_map_cache = {}
for map_name, original_count, ordinal, hide_flag, position, item, claimed in EVENTS:
    if hide_flag not in flag_aliases or (claimed and claimed not in flag_aliases):
        fail(f"Missing permanent World Update flag for {map_name} {position}")

    if map_name not in event_map_cache:
        map_data = json.loads((ROOT / f"data/maps/{map_name}/map.json").read_text())
        layout = LAYOUTS[map_data["layout"]]
        width, height = layout["width"], layout["height"]
        blocks = struct.unpack(f"<{width * height}H", (ROOT / layout["blockdata_filepath"]).read_bytes())
        event_map_cache[map_name] = (map_data, width, height, blocks)
    map_data, width, height, blocks = event_map_cache[map_name]
    events = map_data["object_events"]
    if len(events) > 64 or ordinal < original_count or ordinal >= len(events):
        fail(f"{map_name}: new objects must be appended after all original local IDs")

    obj = events[ordinal]
    x, y = position
    if (obj["x"], obj["y"]) != position or obj["flag"] != hide_flag:
        fail(f"{map_name}: world event moved or changed its stable hide flag")
    if item and (obj["graphics_id"] != "OBJ_EVENT_GFX_ITEM_BALL" or obj["trainer_sight_or_berry_tree_id"] != item):
        fail(f"{map_name}: wrong reward or overworld sprite for {position}")
    if not item and obj["graphics_id"] == "OBJ_EVENT_GFX_ITEM_BALL":
        fail(f"{map_name}: NPC expected at {position}")

    if not (0 <= x < width and 0 <= y < height):
        fail(f"{map_name}: event outside map")
    tile = blocks[y * width + x]
    if (tile >> 10) & 3 or ((tile >> 12) & 0xF) not in (0, obj["elevation"]):
        fail(f"{map_name}: blocked tile or wrong elevation at {position}")
    adjacent = [(x + dx, y + dy) for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0))]
    if not any(0 <= ax < width and 0 <= ay < height
               and ((blocks[ay * width + ax] >> 10) & 3) == 0
               and ((blocks[ay * width + ax] >> 12) & 0xF) in (0, obj["elevation"])
               for ax, ay in adjacent):
        fail(f"{map_name}: no walkable neighboring tile for {position}")
    for other in events:
        if other is not obj and (other["x"], other["y"]) == position:
            fail(f"{map_name}: overlapping events at {position}")
    for key in ("warp_events", "bg_events", "coord_events"):
        if any((event["x"], event["y"]) == position for event in map_data.get(key, [])):
            fail(f"{map_name}: world event overlaps {key} at {position}")

    script_source = (ROOT / f"data/maps/{map_name}/scripts.inc").read_text(encoding="utf-8")
    script_name = obj["script"]
    block_match = re.search(rf"^{re.escape(script_name)}::\s*\n(.*?)(?=^\w+::|^\w+:|\Z)", script_source, re.M | re.S)
    if not block_match:
        fail(f"{map_name}: missing NPC or item event script {script_name}")
    if item:
        body = block_match.group(1)
        for expected in (f"giveitem {item}", "goto_if_eq VAR_RESULT, FALSE", f"setflag {claimed}", "removeobject VAR_LAST_TALKED"):
            if expected not in body:
                fail(f"{map_name}: {script_name} has no safe reward handling: {expected}")
        if f"ReconcileClaimableWorldEvent({hide_flag}" not in world_source:
            fail(f"{map_name}: claimed reward is not reconciled before map spawn")
    elif f"ReconcileWorldEvent({hide_flag}" not in world_source:
        fail(f"{map_name}: NPC visibility is not updated by progression")

print("World Update V1: save version, 6 permanent flags, 4 progressive events on 3 maps, 2 non-respawning rewards; warp/camera reload and safe positions verified")
