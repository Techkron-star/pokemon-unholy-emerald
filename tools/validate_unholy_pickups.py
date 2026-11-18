#!/usr/bin/env python3
"""Check Unholy route Gear pickups before generating a ROM."""

import json
import struct
from collections import deque
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
LAYOUTS = {
    layout["id"]: layout
    for layout in json.loads((ROOT / "data/layouts/layouts.json").read_text())["layouts"]
}

# Map, Gear item, pickup flag, position, and a known reachable starting point.
PICKUPS = [
    ("Route110", "ITEM_GEAR_LENTE_CERTEZA", "FLAG_UNHOLY_ROUTE110_LENTE_CERTEZA", (34, 65), (30, 69)),
    ("Route111", "ITEM_GEAR_PIEDRA_AFILAR", "FLAG_UNHOLY_ROUTE111_PIEDRA_AFILAR", (16, 58), (12, 54)),
    ("Route115", "ITEM_GEAR_GUANTES_COHETE", "FLAG_UNHOLY_ROUTE115_GUANTES_COHETE", (22, 11), (18, 7)),
    ("Route117", "ITEM_GEAR_BATERIA_RESERVA", "FLAG_UNHOLY_ROUTE117_BATERIA_RESERVA", (9, 17), (16, 18)),
    ("Route119", "ITEM_GEAR_BOTON_PANICO", "FLAG_UNHOLY_ROUTE119_PANIC", (12, 121), None),
    ("Route120", "ITEM_GEAR_CASCO_HOPLITA", "FLAG_UNHOLY_ROUTE120_HOPLITA", (20, 55), None),
    ("Route121", "ITEM_GEAR_BOTAS_UMBRAL", "FLAG_UNHOLY_ROUTE121_BOTAS_UMBRAL", (46, 13), (38, 13)),
    ("Route123", "ITEM_GEAR_TUNICA_RUNICA", "FLAG_UNHOLY_ROUTE123_TUNICA", (57, 16), None),
    ("MtPyre_4F", "ITEM_GEAR_VELO_CRIPTICO", "FLAG_UNHOLY_MT_PYRE_VELO_CRIPTICO", (8, 12), (3, 11)),
    ("ShoalCave_LowTideIceRoom", "ITEM_GEAR_PERMAFROST_FRAGMENT", "FLAG_UNHOLY_SHOAL_CAVE_PERMAFROST", (8, 4), (12, 8)),
    ("Underwater_Route127", "ITEM_GEAR_ARIETE_NAUTILO", "FLAG_UNHOLY_UNDERWATER127_ARIETE_NAUTILO", (59, 12), (57, 5)),
    ("VictoryRoad_B1F", "ITEM_GEAR_NIHIL_ENGINE", "FLAG_UNHOLY_VICTORY_ROAD_NIHIL_ENGINE", (35, 10), (32, 3)),
]


def validate_pickup(entry, items, flags):
    map_name, item, flag, position, start = entry
    errors = []
    map_data = json.loads((ROOT / f"data/maps/{map_name}/map.json").read_text())
    layout = LAYOUTS[map_data["layout"]]
    width, height = layout["width"], layout["height"]
    data = (ROOT / layout["blockdata_filepath"]).read_bytes()
    tiles = struct.unpack(f"<{width * height}H", data)

    def walkable(pos, elevation):
        x, y = pos
        if not (0 <= x < width and 0 <= y < height):
            return False
        tile = tiles[y * width + x]
        return (tile >> 10) & 3 == 0 and tile >> 12 == elevation

    def adjacent(pos):
        x, y = pos
        return [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]

    objects = map_data.get("object_events", [])
    found = [obj for obj in objects if obj.get("flag") == flag]
    if len(found) != 1:
        return [f"{map_name}: {flag} occurs {len(found)} times, expected once"]

    obj = found[0]
    if obj["graphics_id"] != "OBJ_EVENT_GFX_ITEM_BALL":
        errors.append("wrong overworld graphic")
    if obj["script"] != "Common_EventScript_FindItem":
        errors.append("missing pickup interaction script")
    if obj["trainer_sight_or_berry_tree_id"] != item or item not in items:
        errors.append("missing or mismatched Gear item")
    if flag not in flags:
        errors.append("pickup flag is undefined")
    if (obj["x"], obj["y"]) != position:
        errors.append("unexpected pickup coordinates")
    if len(objects) > 64:
        errors.append("map exceeds saved event-template capacity")
    elevation = obj["elevation"]
    if not walkable(position, elevation):
        errors.append("pickup tile is impassable or at wrong elevation")
    if not any(walkable(neighbor, elevation) for neighbor in adjacent(position)):
        errors.append("no adjacent walkable tile from which to press A")

    occupied = (
        [(other["x"], other["y"]) for other in objects if other is not obj]
        + [(event["x"], event["y"]) for key in ("warp_events", "coord_events", "bg_events")
           for event in map_data.get(key, [])]
    )
    if position in occupied:
        errors.append("pickup overlaps another event")

    if start is not None:
        # Underwater warp coordinates can have a different elevation. Starting
        # from their adjacent seabed tiles still checks the actual path.
        reachable = [start] if walkable(start, elevation) else [
            neighbor for neighbor in adjacent(start) if walkable(neighbor, elevation)
        ]
        queue = deque(reachable)
        seen = set(reachable)
        while queue and position not in seen:
            for neighbor in adjacent(queue.popleft()):
                if neighbor not in seen and walkable(neighbor, elevation):
                    seen.add(neighbor)
                    queue.append(neighbor)
        if position not in seen:
            errors.append("pickup is disconnected from its access area")

    return [f"{map_name} {position}: {error}" for error in errors]


def main():
    items = (ROOT / "include/constants/items.h").read_text()
    flags = (ROOT / "include/constants/flags.h").read_text()
    errors = []
    seen = set()
    for entry in PICKUPS:
        if entry[2] in seen:
            errors.append(f"duplicate pickup flag: {entry[2]}")
        seen.add(entry[2])
        errors.extend(validate_pickup(entry, items, flags))

    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        raise SystemExit(1)
    print(f"OK: {len(PICKUPS)} unique Gear pickups have valid scripts, rewards, tiles, and flags")


if __name__ == "__main__":
    main()
