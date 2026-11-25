#!/usr/bin/env python3
"""Build deterministic spatial palette roles from indexed battle sprites.

Only Python stdlib is needed. No changes to PNGs, save data or game RNG.
A palette signature that has conflicting roles is omitted, not guessed.
"""
import argparse
import collections
import pathlib
import struct
import zlib

ROOT = pathlib.Path("graphics/pokemon")
PNG_SIG = b"\x89PNG\r\n\x1a\n"
MASK32 = 0xFFFFFFFF


def rgb555(rgb):
    r, g, b = rgb
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def read_pal(path):
    lines = path.read_text(encoding="ascii").split()
    if lines[:3] != ["JASC-PAL", "0100", "16"] or len(lines) != 51:
        raise ValueError("not a 16-entry JASC palette")
    values = list(map(int, lines[3:]))
    if any(v < 0 or v > 255 for v in values):
        raise ValueError("palette channel out of bounds")
    return tuple(rgb555(values[i:i+3]) for i in range(0, 48, 3))


def paeth(a, b, c):
    p = a + b - c
    da, db, dc = abs(p-a), abs(p-b), abs(p-c)
    return a if da <= db and da <= dc else b if db <= dc else c


def read_png(path):
    data = path.read_bytes()
    if not data.startswith(PNG_SIG):
        raise ValueError("invalid PNG")
    pos, chunks, palette, header = 8, [], None, None
    while pos + 12 <= len(data):
        size, tag = struct.unpack_from(">I4s", data, pos)
        end = pos + 8 + size
        if end + 4 > len(data):
            raise ValueError("truncated PNG")
        payload = data[pos+8:end]
        if tag == b"IHDR":
            header = struct.unpack(">IIBBBBB", payload)
        elif tag == b"PLTE":
            palette = tuple(rgb555(payload[i:i+3]) for i in range(0, len(payload), 3))
        elif tag == b"IDAT":
            chunks.append(payload)
        elif tag == b"IEND":
            break
        pos = end + 4
    if header is None or palette is None:
        raise ValueError("missing indexed palette")
    w, h, depth, kind, comp, filt, interlace = header
    if kind != 3 or depth not in (1, 2, 4, 8) or comp or filt or interlace:
        raise ValueError("unsupported PNG layout")
    if not (0 < w <= 512 and 0 < h <= 1024 and len(palette) <= 256):
        raise ValueError("unsupported sprite dimensions")
    stride = (w * depth + 7) // 8
    raw = zlib.decompress(b"".join(chunks))
    if len(raw) != h * (stride + 1):
        raise ValueError("invalid PNG row length")
    prev, rows = bytearray(stride), []
    for y in range(h):
        offset = y * (stride + 1)
        mode = raw[offset]
        row = bytearray(raw[offset+1:offset+1+stride])
        if mode > 4:
            raise ValueError("invalid PNG row filter")
        for x in range(stride):
            a = row[x-1] if x else 0
            b = prev[x]
            c = prev[x-1] if x else 0
            pred = (0, a, b, (a+b)//2, paeth(a,b,c))[mode]
            row[x] = (row[x] + pred) & 255
        rows.append([((row[x*depth//8] >> (8-depth-(x*depth)%8))
                      & ((1 << depth)-1)) for x in range(w)])
        prev = row
    if any(p >= len(palette) or p >= 16 for row in rows for p in row):
        raise ValueError("sprite uses indices beyond GBA 4bpp palette")
    return palette, rows


def measure(images, normal):
    counts = [0]*16
    edges = [0]*16
    links = [[0]*16 for _ in range(16)]
    for path in images:
        try:
            pal, rows = read_png(path)
        except (OSError, ValueError, struct.error, zlib.error):
            continue
        # Never guess how a different PNG palette was reindexed by gbagfx.
        if len(pal) < 16 or pal[1:16] != normal[1:16]:
            continue
        h, w = len(rows), len(rows[0])
        for y, row in enumerate(rows):
            for x, p in enumerate(row):
                if not p:
                    continue
                counts[p] += 1
                for dx, dy in ((1,0),(-1,0),(0,1),(0,-1)):
                    xx, yy = x+dx, y+dy
                    # Separate tiled 64px animation frames.
                    if not (0 <= xx < w and 0 <= yy < h) or xx//64 != x//64 or yy//64 != y//64:
                        edges[p] += 1
                        continue
                    q = rows[yy][xx]
                    if not q:
                        edges[p] += 1
                    elif p != q:
                        links[p][q] += 1
    return counts, edges, links


def features(color):
    r, g, b = color & 31, (color >> 5) & 31, (color >> 10) & 31
    hi, lo = max(r,g,b), min(r,g,b)
    ch = hi-lo
    if not ch:
        hue = 0
    elif hi == r:
        hue = (256*(g-b)//ch) % 1536
    elif hi == g:
        hue = (512+256*(b-r)//ch) % 1536
    else:
        hue = (1024+256*(r-g)//ch) % 1536
    return hue, ch, hi, r+g+b


def analyze(normal, counts, edges, links):
    """Cluster shading ramps using hue, neutral/chromatic split and contacts.

    Largest cluster is body; remaining clusters are secondary/accent.
    Tiny/unused or likely outline indices are not recolored.
    """
    feat = [features(c) for c in normal]
    eligible = [i for i in range(1,16) if counts[i] and 6 < feat[i][3] < 87
                and not (feat[i][2] <= 6 and edges[i]*3 >= counts[i])]
    if not eligible:
        return 0

    def distance(a,b):
        ha, ca, va, _ = feat[a]
        hb, cb, vb, _ = feat[b]
        contact = links[a][b] + links[b][a]
        bonus = min(70, 120*contact//max(1,min(counts[a],counts[b])))
        neutral_a, neutral_b = ca <= 4, cb <= 4
        if neutral_a != neutral_b:
            return 310 + 3*abs(va-vb) - bonus//3
        if neutral_a:
            return 5*abs(va-vb) - bonus
        hue = abs(ha-hb)
        hue = min(hue,1536-hue)
        return hue + 3*abs(ca-cb) + 2*abs(va-vb) - bonus

    clusters = [(i,) for i in eligible]

    def group_distance(a,b):
        # Average linkage avoids chaining unrelated hues through one shade.
        return sum(distance(i,j) for i in a for j in b)//(len(a)*len(b))

    while len(clusters) > 1:
        score, a, b = min((group_distance(clusters[i],clusters[j]),i,j)
                          for i in range(len(clusters)) for j in range(i+1,len(clusters)))
        if score > 135 and len(clusters) <= 3:
            break
        clusters[a] = tuple(sorted(clusters[a]+clusters[b]))
        del clusters[b]
    clusters.sort(key=lambda group: (-sum(counts[i] for i in group),group))
    mask = 0
    for role, group in enumerate(clusters, 1):
        for i in group:
            mask |= role << (2*i)
    return mask


def signature(pal):
    a, b = 2166136261, 0x9E3779B9
    for value in pal[1:16]:
        v = value & 0x7FFF
        a = ((a ^ v)*16777619) & MASK32
        b = ((b ^ v)*2246822519) & MASK32
    return a,b


def generate(root, output):
    records = {}
    conflicts = set()
    scanned, skipped = 0, 0
    for normal_path in sorted(root.rglob("normal*.pal")):
        if normal_path.name not in ("normal.pal", "normal_gba.pal"):
            continue
        scanned += 1
        suffix = "_gba" if normal_path.stem.endswith("_gba") else ""
        folder = normal_path.parent
        images = [folder/(name+suffix+".png") for name in
                  ("front","anim_front","back","frontf","anim_frontf","backf")]
        images = [p for p in images if p.is_file()]
        try:
            normal = read_pal(normal_path)
            counts, edges, links = measure(images,normal)
            mask = analyze(normal,counts,edges,links)
        except (OSError, ValueError):
            skipped += 1
            continue
        if not mask:
            skipped += 1
            continue
        for path in (normal_path, folder/("shiny"+suffix+".pal")):
            try:
                pal = read_pal(path)
            except (OSError, ValueError):
                continue
            key = signature(pal)
            if key in records and records[key] != (pal[1:],mask):
                conflicts.add(key)
            else:
                records[key] = (pal[1:],mask)
    rows = [(a,b,mask) for (a,b),(_,mask) in sorted(records.items())
            if (a,b) not in conflicts]
    if not rows:
        raise SystemExit("chromatic generator found no compatible battle palettes")
    text = ["/* Generated by tools/misc/unholy_chromatic_regions.py; do not edit. */",
            "static const struct GeneratedChromaticRole sGeneratedChromaticRoles[] = {"]
    text.extend("    { 0x%08Xu, 0x%08Xu, 0x%08Xu }," % row for row in rows)
    text.extend(["};",""])
    output.parent.mkdir(parents=True,exist_ok=True)
    tmp = output.with_suffix(output.suffix+".tmp")
    tmp.write_text("\n".join(text),encoding="ascii")
    tmp.replace(output)
    print("chromatic regions: %d signatures, %d palette families, %d skipped, %d conflicts"
          % (len(rows),scanned,skipped,len(conflicts)))


def self_test():
    # Body (warm ramp) versus a small neutral metal ramp and an accent.
    pal = (0,0, 27|(15<<5)|(3<<10), 19|(10<<5)|(2<<10),
           20|(20<<5)|(20<<10), 12|(12<<5)|(12<<10),
           3|(20<<5)|(26<<10))+(0,)*9
    counts=[0,20,100,80,15,10,8]+[0]*9
    edges=[0]*16
    links=[[0]*16 for _ in range(16)]
    links[2][3]=links[3][2]=20
    links[4][5]=links[5][4]=8
    mask=analyze(pal,counts,edges,links)
    role=lambda i: (mask>>(2*i))&3
    assert role(2)==role(3)==1
    assert role(4)==role(5)!=0 and role(4)!=1
    assert role(6) not in (0,1,role(4))
    assert role(0)==role(1)==0
    assert signature(pal)==signature((123,)+pal[1:])
    assert signature(pal)!=signature(pal[:2]+(pal[2]^1,)+pal[3:])


if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=pathlib.Path,default=ROOT)
    parser.add_argument("--output",type=pathlib.Path,required=True)
    args=parser.parse_args()
    self_test()
    generate(args.root,args.output)
