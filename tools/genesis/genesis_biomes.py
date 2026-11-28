#!/usr/bin/env python3
"""Genesis hierarchical biome assignment, host-side, independent of traversal.

A geographic biome patch is NOT a movement/progression zone. Every cell has
one environmental family; existing route sockets, gates, authored anchors and
seam contracts must remain intact. This module requests motifs, not GBA tile IDs.
"""
import argparse
import json
import random
from genesis_climate import blend, quantize, climate, climate_weights

VERSION = 1
# Weights are experimental, not tied to original Hoenn climate.
CATALOG = {
    "land": {
        "plains": (12, "meadow", "dirt_track", "rock_outcrop"),
        "forest": (12, "dense_woods", "forest_track", "clearing"),
        "jungle": (9, "rainforest", "jungle_track", "vine_grove"),
        "desert": (10, "dunes", "sand_track", "oasis"),
        "tundra": (8, "snowfield", "snow_track", "ice_outcrop"),
        "wetland": (9, "marsh", "boardwalk", "reed_pool"),
        "mountain": (8, "cliff", "mountain_track", "plateau"),
        "archipelago": (7, "sea", "causeway", "islet"),
        "savanna": (7, "dry_grass", "dirt_track", "acacia_grove"),
        "badlands": (5, "eroded_rock", "rock_track", "mesa"),
        "volcanic": (4, "ash_field", "basalt_track", "lava_basin"),
        "coast": (8, "shore", "coastal_track", "sandbar"),
    },
    "water": {
        "archipelago": (35, "sea", "water_channel", "islet"),
        "open_sea": (20, "deep_sea", "water_channel", "reef"),
        "reef": (20, "coral", "water_channel", "lagoon"),
        "wetland": (15, "marsh", "water_channel", "reed_pool"),
        "glacial_sea": (10, "iceberg", "water_channel", "ice_floe"),
    },
    "cave": {
        "crystal_cave": (25, "crystal_wall", "cave_track", "crystal_grove"),
        "limestone_cave": (25, "stone_wall", "cave_track", "stalagmites"),
        "fungal_cave": (15, "fungal_wall", "cave_track", "fungal_grove"),
        "ice_cave": (15, "ice_wall", "cave_track", "ice_crystals"),
        "volcanic_cave": (12, "basalt_wall", "cave_track", "lava_basin"),
        "subterranean_lake": (8, "underground_water", "dry_causeway", "pool"),
    },
}
# Neighboring patches usually share climate or form natural ecotones.
RELATED = {
    "plains": ("forest", "savanna", "wetland"),
    "forest": ("plains", "jungle", "mountain"),
    "jungle": ("forest", "wetland", "coast"),
    "desert": ("badlands", "savanna", "mountain"),
    "tundra": ("mountain", "plains"),
    "wetland": ("jungle", "forest", "coast", "archipelago"),
    "mountain": ("forest", "tundra", "badlands"),
    "archipelago": ("coast", "wetland", "reef", "open_sea"),
    "savanna": ("plains", "desert", "badlands"),
    "badlands": ("desert", "mountain", "volcanic"),
    "volcanic": ("badlands", "mountain"),
    "coast": ("archipelago", "wetland", "plains"),
    "open_sea": ("reef", "archipelago", "glacial_sea"),
    "reef": ("open_sea", "archipelago"),
    "glacial_sea": ("open_sea",),
    "crystal_cave": ("limestone_cave", "ice_cave"),
    "limestone_cave": ("crystal_cave", "fungal_cave", "subterranean_lake"),
    "fungal_cave": ("limestone_cave", "subterranean_lake"),
    "ice_cave": ("crystal_cave", "limestone_cave"),
    "volcanic_cave": ("limestone_cave", "crystal_cave"),
    "subterranean_lake": ("limestone_cave", "fungal_cave"),
}
# Settlements and interiors are native by default. Special maps can opt out.
NATIVE = {"city", "town", "interior", "scripted", "native"}


def rng_for(seed, map_id):
    # Stable across Python hash randomization, traversal order and map visits.
    if not 0 <= seed <= 0xFFFFFFFF or not 0 <= map_id <= 0xFFFFFFFF:
        raise ValueError("seed and map_id must be unsigned 32-bit")
    return random.Random((seed << 40) ^ (map_id << 8) ^ VERSION)


def assign(seed, map_id, width, height, domain="land", eligible=None,
           homogeneous=None, dominant=None, origin=None):
    """Assign each module-sized coordinate an environmental patch and motif.

    eligible is the renderer's supported biome set. Empty -> native fallback.
    homogeneous=None gives a 40% chance of an entire map in one biome.
    Mixed maps reserve one large dominant region and 1–4 secondary patches.
    """
    if not 1 <= width <= 256 or not 1 <= height <= 256:
        raise ValueError("invalid module dimensions")
    rng = rng_for(seed, map_id)
    if domain in NATIVE:
        return {"version": VERSION, "mode": "native", "map_id": map_id}
    if domain not in CATALOG:
        raise ValueError("unknown movement domain")
    catalog = CATALOG[domain]
    choices = [name for name in catalog if eligible is None or name in eligible]
    if not choices:
        return {"version": VERSION, "mode": "native", "map_id": map_id}
    if dominant is None:
        weights = [catalog[n][0] for n in choices]
        if origin is not None:
            regional = climate_weights(climate(seed, origin[0]+width//2,
                                               origin[1]+height//2), choices)
            # Retain a chance of unusual maps while favoring shared regional climate.
            weights = [w*(.05+regional[n]) for n,w in zip(choices,weights)]
        dominant = rng.choices(choices, weights)[0]
    elif dominant not in choices:
        raise ValueError("unsupported dominant biome")
    related = [n for n in RELATED.get(dominant, ()) if n in choices]
    single = rng.randrange(100) < 40 if homogeneous is None else homogeneous
    # Distinct seeds are spaced apart to avoid checkerboard-like biomes.
    centers = [(width // 2, height // 2, dominant, max(width, height) // 3)]
    if not single and related and width * height >= 16:
        for _ in range(rng.randint(1, 4)):
            for _attempt in range(24):
                x, y = rng.randrange(width), rng.randrange(height)
                spacing = max(2, min(width, height) // 3)
                if all(abs(x-a)+abs(y-b) >= spacing for a,b,_,_ in centers):
                    centers.append((x, y, rng.choice(related), 0))
                    break
    cells = []
    for y in range(height):
        for x in range(width):
            index = min(range(len(centers)), key=lambda i: (
                abs(x-centers[i][0]) + abs(y-centers[i][1])-centers[i][3], i))
            name = centers[index][2]
            sample, weights = blend(seed, x, y, width, height, centers, choices, origin)
            mixture = quantize(weights)
            visual = next(iter(mixture))
            cells.append({"x": x, "y": y, "biome": name, "patch": index,
                          "visual_biome": visual, "blend": mixture,
                          "ecotone": len(mixture) > 1,
                          "climate": [round(v, 4) for v in sample],
                          "background": catalog[visual][1],
                          "corridor": catalog[visual][2],
                          "landmark": catalog[visual][3]})
    # A generated secondary region might be swallowed by the dominant bias.
    active = sorted({cell["patch"] for cell in cells})
    return {"version": VERSION, "mode": "single" if len(active)==1 else "mosaic",
            "map_id": map_id, "domain": domain, "dominant": dominant,
            "width": width, "height": height, "origin": origin, "cells": cells}


def selftest():
    observed = set()
    for domain, catalog in CATALOG.items():
        seen = set()
        for seed in range(512):
            plan = assign(seed, 101, 12, 10, domain)
            assert plan == assign(seed, 101, 12, 10, domain)
            assert len(plan["cells"]) == 120
            assert plan["dominant"] in catalog
            seen.add(plan["dominant"])
            observed.add(plan["mode"])
            for cell in plan["cells"]:
                assert cell["biome"] in catalog
            # Voronoi patch cells must be cardinally connected.
            for p in {c["patch"] for c in plan["cells"]}:
                cells = {(c["x"], c["y"]) for c in plan["cells"] if c["patch"]==p}
                visited = {next(iter(cells))}
                queue = list(visited)
                for x,y in queue:
                    for q in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                        if q in cells and q not in visited:
                            visited.add(q)
                            queue.append(q)
                assert visited == cells
        assert seen == set(catalog)
    assert observed == {"single", "mosaic"}
    for name in NATIVE:
        assert assign(0, 0, 20, 20, name)["mode"] == "native"
    assert assign(0, 0, 20, 20, eligible=set())["mode"] == "native"
    desert = assign(1, 101, 20, 20, dominant="desert", homogeneous=True)
    assert {c["biome"] for c in desert["cells"]} == {"desert"}
    assert all(c["blend"] == {"desert":255} for c in desert["cells"])
    for seed in range(16):
        a = assign(seed, 101, 16, 16, origin=(0,0))
        b = assign(seed, 102, 16, 16, origin=(15,0))
        for y in range(16):
            left, right = a["cells"][y*16+15], b["cells"][y*16]
            assert left["blend"] == right["blend"]
            assert left["climate"] == right["climate"]
            assert sum(left["blend"].values()) == 255
    print("Genesis biomes: 1536 map seeds; coherent patches and native fallback")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seed", type=lambda s: int(s, 0), default=1)
    parser.add_argument("--map-id", type=int, default=101)
    parser.add_argument("--width", type=int, default=12)
    parser.add_argument("--height", type=int, default=10)
    parser.add_argument("--domain", choices=sorted(set(CATALOG)|NATIVE), default="land")
    parser.add_argument("--dominant")
    parser.add_argument("--origin", nargs=2, type=int, metavar=("X", "Y"))
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()
    else:
        print(json.dumps(assign(args.seed, args.map_id, args.width, args.height,
                                args.domain, dominant=args.dominant, origin=args.origin), indent=2))
