#!/usr/bin/env python3
"""Host-only Genesis route module planner; no ROM writes.

One cell denotes a multi-metatile module, not one GBA metatile. An explicit
zone is an ungated bidirectional movement component. Cross-zone connections
exist only through supplied directed gates. Native map extraction/rendering
must establish those semantics before using a plan in the game.
"""
import argparse
import heapq
import json
import random
from collections import deque

DIR = ((0, -1, 1, 2), (0, 1, 2, 1), (-1, 0, 4, 8), (1, 0, 8, 4))
VERSION = 1
# Module-planning presets only: native visual catalog is a later phase.
PROFILES = {"balanced": (4, 1, 6), "forest": (2, 2, 5),
            "plains": (6, 1, 1), "trails": (8, 1, 3),
            "clearings": (5, 1, 4), "mountain": (3, 3, 6),
            "coastal": (3, 1, 4), "swamp": (2, 2, 6)}


def neighbors(p):
    x, y = p
    for dx, dy, bit, opposite in DIR:
        yield (x + dx, y + dy), bit, opposite


def route_to_tree(start, tree, allowed, rng, cost_min=1, cost_max=6):
    """Random positive edge costs; a shortest path in the weighted grid."""
    if start in tree:
        return [start]
    costs = {start: 0}
    prev = {}
    queue = [(0, start)]
    while queue:
        cost, p = heapq.heappop(queue)
        if cost != costs[p]:
            continue
        if p in tree:
            path = [p]
            while p != start:
                p = prev[p]
                path.append(p)
            return path[::-1]
        for q, _, _ in neighbors(p):
            if q not in allowed:
                continue
            new = cost + rng.randint(cost_min, cost_max)
            if new < costs.get(q, float("inf")):
                costs[q] = new
                prev[q] = p
                heapq.heappush(queue, (new, q))
    raise ValueError("unreachable required anchor")


def add_edge(ports, p, q):
    for dest, bit, opposite in neighbors(p):
        if dest == q:
            ports[p] = ports.get(p, 0) | bit
            ports[q] = ports.get(q, 0) | opposite
            return
    raise ValueError("noncardinal connection")


def component(seed, allowed, anchors, branches, cost_min=1, cost_max=6):
    rng = random.Random(seed)
    ports = {anchors[0]: 0}
    for anchor in anchors[1:]:
        path = route_to_tree(anchor, ports, allowed, rng, cost_min, cost_max)
        for p, q in zip(path, path[1:]):
            add_edge(ports, p, q)
    # Optional dead ends, deliberately not bridges across progression gates.
    # A branch can terminate at a reward, landmark or clearing.
    for _ in range(branches):
        frontier = [(p, q) for p in sorted(ports)
                    for q, _, _ in neighbors(p)
                    if q in allowed and q not in ports]
        if not frontier:
            break
        p, q = rng.choice(frontier)
        add_edge(ports, p, q)
        p = q
        for _ in range(rng.randrange(4)):
            choices = [q for q, _, _ in neighbors(p)
                       if q in allowed and q not in ports]
            if not choices:
                break
            q = rng.choice(choices)
            add_edge(ports, p, q)
            p = q
    # A few local loops create alternate routes without changing components.
    loops = [(p, q) for p in sorted(ports) for q, bit, _ in neighbors(p)
             if q in ports and p < q and not ports[p] & bit]
    rng.shuffle(loops)
    for p, q in loops[:branches // 2]:
        add_edge(ports, p, q)
    return ports


def reachable(start, adjacency, capabilities):
    seen = {start}
    queue = deque([start])
    while queue:
        p = queue.popleft()
        for q, required in adjacency[p]:
            if required & capabilities == required and q not in seen:
                seen.add(q)
                queue.append(q)
    return seen


def validate(zones, anchors, gates, ports):
    """Check reciprocity and exact abstract progression for all gate states."""
    owner = {p: z for z, cells in zones.items() for p in cells}
    if not set(ports) <= owner.keys():
        raise ValueError("module outside allowed zone")
    adjacency = {p: [] for p in ports}
    baseline = {z: [] for z in zones}
    for p, mask in ports.items():
        if mask & ~15:
            raise ValueError("invalid port mask")
        for q, bit, opposite in neighbors(p):
            if mask & bit:
                if not ports.get(q, 0) & opposite or owner[p] != owner[q]:
                    raise ValueError("unpaired or cross-zone port")
                adjacency[p].append((q, 0))
    for p, q, required in gates:
        if p not in ports or q not in ports:
            raise ValueError("missing gate endpoint")
        adjacency[p].append((q, required))
        baseline[owner[p]].append((owner[q], required))
    for z, group in anchors.items():
        if not set(group) <= ports.keys():
            raise ValueError("missing anchor")
        internal = {p: [(q, req) for q, req in adjacency[p]
                        if owner[q] == z and req == 0]
                    for p in ports if owner[p] == z}
        if reachable(group[0], internal, 0) != internal.keys():
            raise ValueError("disconnected module within zone")
    all_anchors = [(z, p) for z, group in anchors.items() for p in group]
    # Capability bits are deliberately limited to eight for exhaustive checks.
    # Enumerate subsets of only the bits actually used.
    used = 0
    for _, _, required in gates:
        used |= required
    state = used
    while True:
        for z, p in all_anchors:
            expected = reachable(z, baseline, state)
            actual = reachable(p, adjacency, state)
            if {q for zz, q in all_anchors if zz in expected} != (
                    actual & {q for _, q in all_anchors}):
                raise ValueError("progression reachability mismatch")
        if state == 0:
            break
        state = (state - 1) & used


def generate(seed, zones, anchors, gates=(), branches=None, profile="balanced"):
    if profile not in PROFILES:
        raise ValueError("unknown terrain profile")
    default_branches, cost_min, cost_max = PROFILES[profile]
    if branches is None:
        branches = default_branches
    if not zones or set(zones) != set(anchors) or not 0 <= branches <= 64:
        raise ValueError("invalid route definition")
    if not 0 <= seed <= 0xFFFFFFFF:
        raise ValueError("seed must be unsigned 32-bit")
    zones = {z: set(cells) for z, cells in zones.items()}
    anchors = {z: list(group) for z, group in anchors.items()}
    owner = {}
    for z, cells in zones.items():
        if not anchors[z] or len(set(anchors[z])) != len(anchors[z]):
            raise ValueError("missing or duplicate anchors")
        if not set(anchors[z]) <= cells:
            raise ValueError("anchor outside its zone")
        for p in cells:
            if p in owner:
                raise ValueError("overlapping zones")
            owner[p] = z
    gates = tuple(gates)
    for p, q, required in gates:
        if (p not in owner or q not in owner or owner[p] == owner[q]
                or abs(p[0] - q[0]) + abs(p[1] - q[1]) != 1
                or not 0 <= required <= 255):
            raise ValueError("invalid directed gate")
        for endpoint in (p, q):
            group = anchors[owner[endpoint]]
            if endpoint not in group:
                group.append(endpoint)
    ports = {}
    for i, z in enumerate(sorted(zones)):
        ports.update(component((seed << 16) ^ (VERSION << 8) ^ i,
                               zones[z], anchors[z], branches, cost_min, cost_max))
    validate(zones, anchors, gates, ports)
    # Exact socket masks are mandatory. Variant is a catalog lookup key,
    # not a license to substitute an incompatible module or change sockets.
    motifs = random.Random((seed << 8) ^ 0xDEC0 ^ VERSION)
    modules = []
    for p, mask in sorted(ports.items()):
        gate_mask = 0
        for dest, bit, _ in neighbors(p):
            if any((a == p and b == dest) or (b == p and a == dest)
                   for a, b, _ in gates):
                gate_mask |= bit
        modules.append({"x": p[0], "y": p[1], "zone": owner[p],
                        "ports": mask, "gate_ports": gate_mask,
                        "variant": motifs.randrange(4)})
    return {"version": VERSION, "seed": seed, "profile": profile, "modules": modules,
            "gates": [{"from": p, "to": q, "requires": req}
                      for p, q, req in gates]}


def demo(seed, profile="balanced"):
    # A synthetic land route with a river/cliff barrier, one fixed gate,
    # and two entrances. A separate native adapter must extract real zones.
    left = {(x, y) for x in range(7) for y in range(9)}
    right = {(x, y) for x in range(7, 14) for y in range(9)}
    left -= {(3, 3), (3, 4), (4, 3)}
    return generate(seed, {"west": left, "east": right},
                    {"west": [(0, 4)], "east": [(13, 4), (11, 8)]},
                    [((6, 4), (7, 4), 1)], profile=profile)


def selftest():
    fingerprints = set()
    for seed in range(256):
        plan = demo(seed)
        assert plan == demo(seed)
        fingerprints.add(tuple((m["x"], m["y"], m["ports"])
                               for m in plan["modules"]))
        assert all(0 <= m["variant"] < 4 for m in plan["modules"])
    assert len(fingerprints) >= 240, "insufficient topology variation"
    for profile in PROFILES:
        for seed in range(32):
            plan = demo(seed, profile)
            assert plan == demo(seed, profile)
            assert plan["profile"] == profile
    for zones, anchors, gates in (
        ({"a": {(0, 0), (2, 0)}}, {"a": [(0, 0), (2, 0)]}, ()),
        ({"a": {(0, 0)}, "b": {(0, 0)}},
         {"a": [(0, 0)], "b": [(0, 0)]}, ()),
        ({"a": {(0, 0)}, "b": {(2, 0)}},
         {"a": [(0, 0)], "b": [(2, 0)]}, (((0, 0), (2, 0), 1),)),
    ):
        try:
            generate(0, zones, anchors, gates)
        except ValueError:
            pass
        else:
            raise AssertionError("invalid route accepted")
    # One-way gates remain directional and closed without the capability.
    graph = {0: [(1, 1)], 1: []}
    assert reachable(0, graph, 0) == {0}
    assert reachable(0, graph, 1) == {0, 1}
    assert reachable(1, graph, 1) == {1}
    # The validator rejects a renderer/planner that bypasses a gate.
    try:
        validate({"a": {(0, 0)}, "b": {(1, 0)}},
                 {"a": [(0, 0)], "b": [(1, 0)]}, (),
                 {(0, 0): 8, (1, 0): 4})
    except ValueError:
        pass
    else:
        raise AssertionError("ungated cross-zone connection accepted")
    print("Genesis modules: 256 seeds, deterministic topology, gate and port contracts")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seed", type=lambda s: int(s, 0), default=1)
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--profile", choices=PROFILES, default="balanced")
    args = parser.parse_args()
    if args.selftest:
        selftest()
    else:
        print(json.dumps(demo(args.seed, args.profile), indent=2))
