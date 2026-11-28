"""Genesis immutable puzzle islands and fail-closed native map contracts.

Coordinates are native metatiles, not procedural module indices. The puzzle
footprint must cover every legal boulder position, switches, currents,
one-way tiles, scripted mutations and their working space. A radius around
an object is NOT sufficient evidence. Preserve a one-metatile halo as well.
"""
import argparse
import json
from pathlib import Path

# These families warrant an explicit review even if map.json has no objects.
REVIEW_PREFIXES = ("MagmaHideout_", "SeafloorCavern_", "SkyPillar_",
                   "VictoryRoad_", "MirageTower_", "SealedChamber_",
                   "TrickHouse_", "AquaHideout_", "GraniteCave_",
                   "IceCave_", "ShoalCave_", "NewMauville_")
REVIEW_NAMES = {"DesertRuins", "IslandCave", "AncientTomb", "JaggedPass",
                "Underwater_SealedChamber", "Underwater_SeafloorCavern"}
PUZZLE_TOKENS = ("STRENGTHBOULDER", "PUSHABLE_BOULDER", "BREAKABLE_ROCK",
                 "ROCKSMASH", "BRAILLE", "SWITCH", "FLOORHOLE")


def review_reasons(name, meta):
    reasons = []
    if name in REVIEW_NAMES or name.startswith(REVIEW_PREFIXES):
        reasons.append("known puzzle/story family; tile-only mechanics possible")
    for field in ("object_events", "coord_events", "bg_events"):
        for event in meta.get(field, []):
            data = json.dumps(event).upper()
            if any(token in data for token in PUZZLE_TOKENS):
                reasons.append(f"{field}: {event.get('x')},{event.get('y')}")
    return reasons


def protected_cells(width, height, rectangles, halo=1):
    """Half-open, reviewed footprints plus boundary buffer."""
    if width <= 0 or height <= 0 or halo < 1:
        raise ValueError("invalid protection dimensions")
    locked = set()
    for x0,y0,x1,y1 in rectangles:
        if not (0<=x0<x1<=width and 0<=y0<y1<=height):
            raise ValueError("invalid puzzle footprint")
        for y in range(max(0,y0-halo),min(height,y1+halo)):
            for x in range(max(0,x0-halo),min(width,x1+halo)):
                locked.add((x,y))
    return locked


def contract(name, meta, width, height, reviewed=None):
    """Unreviewed puzzle maps fail closed, not guessed from event radius."""
    if width <= 0 or height <= 0:
        raise ValueError("invalid map size")
    reasons = review_reasons(name,meta)
    native = meta.get("map_type") in {"MAP_TYPE_TOWN","MAP_TYPE_CITY"}
    if native or (reasons and reviewed is None):
        return {"mode":"native","reasons":reasons,"locked":set()}
    if reviewed is None:
        return {"mode":"procedural","reasons":reasons,"locked":set()}
    if not reviewed.get("footprints") or not reviewed.get("states"):
        raise ValueError("reviewed puzzle requires footprint and progression states")
    locked = protected_cells(width,height,reviewed["footprints"])
    # Portals are native boundary tiles through which the surrounding map
    # can approach the island. They never grant a new movement capability.
    portals = {tuple(p) for p in reviewed.get("portals", [])}
    if not portals or not portals <= locked:
        raise ValueError("reviewed puzzle requires native portals")
    for x,y in portals:
        if not any(0<=a<width and 0<=b<height and (a,b) not in locked
                   for a,b in ((x-1,y),(x+1,y),(x,y-1),(x,y+1))):
            raise ValueError("portal must touch the unlocked map")
    return {"mode":"partial","reasons":reasons,"locked":locked,
            "portals":portals,"states":tuple(reviewed["states"])}


def module_mask(locked, width, height, module_size):
    """Any module intersecting a protected metatile stays native."""
    if module_size < 1:
        raise ValueError("invalid module size")
    if any(not (0<=x<width and 0<=y<height) for x,y in locked):
        raise ValueError("invalid protected cell")
    return {(x//module_size,y//module_size) for x,y in locked}


def mark_environment(plan, locked_modules):
    if plan["mode"] == "native":
        return plan
    if any(not (0<=x<plan["width"] and 0<=y<plan["height"])
           for x,y in locked_modules):
        raise ValueError("protected module outside plan")
    for cell in plan["cells"]:
        if (cell["x"],cell["y"]) in locked_modules:
            cell.update({"native":True,"background":None,"corridor":None,
                         "landmark":None})
    plan["protected_modules"] = [list(p) for p in sorted(locked_modules)]
    return plan


def validate_render(original, candidate, width, height, policy,
                    original_events, candidate_events, native_reach=None,
                    generated_reach=None):
    """Final acceptance gate. Reachability must come from an independent
    movement/state solver on each actual layout, not the procedural graph.
    Each state maps named entry/exit and story anchor pairs to booleans.
    Exact equality preserves requirements and rejects alternate bypasses.
    """
    if len(original)!=width*height or len(candidate)!=width*height:
        raise ValueError("layout size mismatch")
    if original_events != candidate_events:
        raise ValueError("authored event or script changed")
    if policy["mode"] == "native":
        if original != candidate:
            raise ValueError("native map changed")
        return
    for x,y in policy["locked"]:
        i=y*width+x
        if original[i]!=candidate[i]:
            raise ValueError(f"protected puzzle cell changed: {x},{y}")
    if policy["mode"] == "partial":
        if not native_reach or not generated_reach:
            raise ValueError("missing independent puzzle reachability proof")
        for state in policy["states"]:
            a,b=native_reach.get(state),generated_reach.get(state)
            if not a or a!=b:
                raise ValueError(f"puzzle reachability mismatch: {state}")


def audit(root):
    result=[]
    for path in sorted((root/"data/maps").glob("*/map.json")):
        meta=json.loads(path.read_text())
        name=path.parent.name
        reasons=review_reasons(name,meta)
        if reasons:
            result.append({"map":name,"policy":"native_pending_review",
                           "reasons":reasons,"warps":meta.get("warp_events",[])})
    if not result:
        raise ValueError("no puzzle maps found; wrong repository root?")
    return result


def selftest():
    meta={"map_type":"MAP_TYPE_UNDERGROUND",
          "object_events":[{"x":3,"y":3,"script":"EventScript_StrengthBoulder"}]}
    assert contract("MagmaHideout_1F",meta,12,12)["mode"]=="native"
    assert contract("SkyPillar_2F",{},12,12)["mode"]=="native"
    assert contract("Route101",{"map_type":"MAP_TYPE_ROUTE"},12,12)["mode"]=="procedural"
    review={"footprints":[[3,3,7,7]],"portals":[[2,4],[7,4]],
            "states":["before_strength","after_strength"]}
    p=contract("MagmaHideout_1F",meta,12,12,review)
    assert p["mode"]=="partial" and len(p["locked"])==36
    assert module_mask(p["locked"],12,12,3)=={(x,y) for x in range(3) for y in range(3)}
    a=[0]*144
    b=a.copy()
    b[0]=1  # surrounding corridor may change
    reach={"before_strength":{"entry->exit":False},
           "after_strength":{"entry->exit":True}}
    validate_render(a,b,12,12,p,meta,meta,reach,reach)
    for bad,events,proof in (
        (b[:38]+[1]+b[39:],meta,reach),
        (b,{},reach),
        (b,meta,{"before_strength":{"entry->exit":True},
                 "after_strength":{"entry->exit":True}}),
        (b,meta,None),
    ):
        try:
            validate_render(a,bad,12,12,p,meta,events,reach,proof)
        except ValueError:
            pass
        else:
            raise AssertionError("invalid puzzle change accepted")
    print("Genesis puzzle islands: immutable footprint, halo, events and gate-state contracts")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--audit",type=Path)
    args=parser.parse_args()
    if args.audit:
        print(json.dumps(audit(args.audit),indent=2))
    else:
        selftest()
