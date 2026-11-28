"""Continuous, deterministic world climate and biome blending (host only).

Use a common world origin for connected maps, in module coordinates. Climate
is sampled by world position, never by map ID. Local biome preference fades
into a common climate blend in a six-module border strip. Native layouts and
actual movement/tileset seam checks remain the responsibility of the adapter.
"""
from functools import lru_cache

# Temperature, moisture, elevation, continentalness, geothermal activity.
# All axes normalized to 0..1. This is an artistic model, not real climate.
PROTOTYPES = {
    "plains": (.55,.42,.40,.65,.25),
    "forest": (.48,.68,.48,.72,.25),
    "jungle": (.87,.86,.36,.60,.32),
    "desert": (.90,.10,.48,.85,.35),
    "tundra": (.10,.35,.58,.73,.20),
    "wetland": (.64,.88,.14,.40,.20),
    "mountain": (.30,.40,.90,.76,.35),
    "archipelago": (.62,.66,.20,.08,.25),
    "savanna": (.79,.35,.38,.68,.23),
    "badlands": (.75,.17,.72,.82,.48),
    "volcanic": (.83,.25,.75,.65,.95),
    "coast": (.60,.55,.22,.23,.20),
    "open_sea": (.48,.70,.10,.04,.20),
    "reef": (.86,.72,.18,.13,.20),
    "glacial_sea": (.09,.65,.15,.08,.15),
    "crystal_cave": (.42,.40,.65,.65,.48),
    "limestone_cave": (.48,.55,.47,.70,.26),
    "fungal_cave": (.63,.85,.38,.70,.30),
    "ice_cave": (.10,.55,.72,.70,.12),
    "volcanic_cave": (.90,.25,.70,.75,.96),
    "subterranean_lake": (.48,.90,.15,.65,.20),
}


def smooth(t):
    t = max(0., min(1., t))
    return t*t*(3.-2.*t)


@lru_cache(maxsize=65536)
def lattice(seed, x, y, axis):
    n = (seed ^ (x*0x85ebca6b) ^ (y*0xc2b2ae35) ^
         (axis*0x9e3779b9)) & 0xffffffff
    n = ((n ^ (n >> 16))*0x7feb352d) & 0xffffffff
    n = ((n ^ (n >> 15))*0x846ca68b) & 0xffffffff
    return (n ^ (n >> 16))/0xffffffff


def noise(seed, x, y, axis, period):
    gx, dx = divmod(x, period)
    gy, dy = divmod(y, period)
    a, b = smooth(dx/period), smooth(dy/period)
    top = lattice(seed, gx, gy, axis)*(1-a)+lattice(seed, gx+1, gy, axis)*a
    bottom = lattice(seed, gx, gy+1, axis)*(1-a)+lattice(seed, gx+1, gy+1, axis)*a
    return top*(1-b)+bottom*b


def climate(seed, x, y):
    axes = [0.65*noise(seed,x,y,i,64)+0.25*noise(seed,x,y,i+5,24)
            +0.10*noise(seed,x,y,i+10,8) for i in range(5)]
    # Height cools the climate; continentalness and height remain independent.
    axes[0] = max(0., min(1., axes[0]+(.5-axes[2])*.35))
    return tuple(axes)


def normalized(weights):
    total = sum(weights.values())
    if total <= 0:
        raise ValueError("empty biome blend")
    return {k: v/total for k,v in weights.items() if v > 0}


def climate_weights(sample, choices):
    # Positive weights avoid an abrupt nearest-biome boundary. Sharpening
    # favors the nearest climates without discarding the neighbors.
    result = {}
    for name in choices:
        d = sum((a-b)**2 for a,b in zip(sample, PROTOTYPES[name]))
        result[name] = 1/(.045+d)**3
    return normalized(result)


def patch_weights(x, y, centers, width=4):
    # Blend in a fixed-width distance band around weighted Voronoi boundaries.
    distances = [abs(x-a)+abs(y-b)-bias for a,b,_,bias in centers]
    nearest = min(distances)
    result = {}
    for distance, (_,_,name,_) in zip(distances, centers):
        weight = 1-smooth((distance-nearest)/width)
        result[name] = max(result.get(name,0.),weight)
    return normalized(result)


def blend(seed, x, y, w, h, centers, choices, origin=None, seam_width=6):
    wx,wy = (origin[0]+x,origin[1]+y) if origin is not None else (x,y)
    sample = climate(seed,wx,wy)
    local = patch_weights(x,y,centers)
    if origin is not None:
        distance = min(x,y,w-1-x,h-1-y)
        seam = 1-smooth(distance/seam_width)
        global_weights = climate_weights(sample,choices)
        local = normalized({name: (1-seam)*local.get(name,0.)
                            +seam*global_weights.get(name,0.) for name in choices})
    return sample, local


def quantize(weights):
    """Integer blend weights sum to exactly 255; deterministic tie breaking."""
    exact = {k:v*255 for k,v in weights.items()}
    result = {k:int(v) for k,v in exact.items()}
    order = sorted(result,key=lambda k:(-(exact[k]-result[k]),k))
    for k in order[:255-sum(result.values())]:
        result[k] += 1
    return {k:v for k,v in sorted(result.items(),key=lambda item:(-item[1],item[0]))
            if v}


def selftest():
    for seed in range(32):
        for x in range(-64,65):
            a,b = climate(seed,x,11),climate(seed,x+1,11)
            assert all(0<=v<=1 for v in a)
            assert max(abs(u-v) for u,v in zip(a,b))<.15
        choices = ("desert","savanna","forest","tundra")
        left = [(0,0,"desert",0)]
        right = [(0,0,"tundra",0)]
        for y in range(16):
            # Explicitly overlapping seam coordinates (adapter contract).
            a,wa = blend(seed,15,y,16,16,left,choices,origin=(0,0))
            b,wb = blend(seed,0,y,16,16,right,choices,origin=(15,0))
            assert a==b and quantize(wa)==quantize(wb)
            assert sum(quantize(wa).values())==255
    centers = [(0,0,"desert",0),(10,0,"savanna",0)]
    assert patch_weights(5,0,centers)=={"desert":.5,"savanna":.5}
    assert patch_weights(0,0,centers)=={"desert":1.}
    print("Genesis climate: continuous fields, ecotones and shared seam contracts")


if __name__ == "__main__":
    selftest()
