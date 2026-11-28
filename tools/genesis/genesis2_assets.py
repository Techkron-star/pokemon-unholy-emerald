#!/usr/bin/env python3
"""Extract versioned native land-route inputs; unknown puzzle contracts fail closed."""
import argparse
import json
import re
import struct
import zlib
import hashlib
from collections import deque
from pathlib import Path
from genesis_bake import protected, load_words, GRASS, TALL, SAND, ID, COLLISION
from genesis_puzzles import review_reasons
from genesis2_modules import extract_catalog, emit_catalog

TOPOLOGY_REVIEW = {
    'Route102': {
        'data/maps/Route102/scripts.inc': '2b8b8fd50cc24f9021e66bd270dbba0400769627fd71eb0cd3c841f2e1b90b2c',
        # Wally's tutorial uses Route102 text but runs in Petalburg's city map.
        'data/maps/PetalburgCity/scripts.inc': '4cd8f6263fb17d7577a8b56fe9abab7eb615002534c33a047c73bb140f75902f',
    },
}


def topology_mask(root, name, layout, words, editable, primary):
    w, h = layout['width'], layout['height']
    mask = bytearray(len(editable))
    review = TOPOLOGY_REVIEW.get(name)
    if not review or any(hashlib.sha256((root/p).read_bytes()).hexdigest()!=sha for p,sha in review.items()):
        return mask
    # Special movement is kept native, including jump landings two cells away.
    # Only the reviewed General/Petalburg pair is enabled in this milestone.
    if layout['secondary_tileset'] != 'gTileset_Petalburg':
        return mask
    secondary = load_words(root/'data/tilesets/secondary/petalburg/metatile_attributes.bin')
    attrs = tuple(primary)+tuple(secondary)
    hazards = {k for k,v in enumerate(words) if v&ID>=len(attrs) or attrs[v&ID]&255 not in (0,2)}
    meta = json.loads((root/'data/maps'/name/'map.json').read_text())
    for k,v in enumerate(words):
        if not editable[k//8]&(1<<(k%8)) or v&0xF000!=0x3000:
            continue
        x,y=k%w,k//w
        # Trainer approach stops one step before the player. Protect every
        # possible endpoint in addition to each actor's normal wander range.
        if any(e.get('trainer_type','TRAINER_TYPE_NONE')!='TRAINER_TYPE_NONE'
               and abs(x-e['x'])<=max(2,int(e['trainer_sight_or_berry_tree_id'])-1)+int(e.get('movement_range_x',0))
               and abs(y-e['y'])<=max(2,int(e['trainer_sight_or_berry_tree_id'])-1)+int(e.get('movement_range_y',0))
               for e in meta.get('object_events',[])):
            continue
        if any(0<=x+dx<w and 0<=y+dy<h and (y+dy)*w+x+dx in hazards
               for dy in range(-2,3) for dx in range(-2,3) if abs(dx)+abs(dy)<=2):
            continue
        mask[k//8] |= 1<<(k%8)
    return mask


def extract(root):
    layouts = json.loads((root / 'data/layouts/layouts.json').read_text())['layouts']
    by_layout = {l['id']: (i + 1, l) for i, l in enumerate(layouts)}
    groups = json.loads((root / 'data/maps/map_groups.json').read_text())
    ids = {name: (g << 8) | n for g, group in enumerate(groups['group_order'])
           for n, name in enumerate(groups[group])}
    maps = {}
    for p in sorted((root / 'data/maps').glob('*/map.json')):
        m = json.loads(p.read_text())
        _, l = by_layout[m['layout']]
        if l.get('layout_version') == 'emerald' and m['map_type'] in {
                'MAP_TYPE_ROUTE', 'MAP_TYPE_TOWN', 'MAP_TYPE_CITY'}:
            maps[m['id']] = (p.parent.name, m, l)
    # Solve real offsets, including native towns between routes. A conflicting
    # component cannot claim global climate continuity: isolate its maps.
    coordinates, domains, conflicts = {}, {}, []
    for start in sorted(maps):
        if start in coordinates:
            continue
        local = {start: (0, 0)}
        queue = deque([start])
        bad = []
        while queue:
            mid = queue.popleft()
            _, m, l = maps[mid]
            x, y = local[mid]
            for c in m.get('connections') or []:
                other = c['map']
                if other not in maps or c['direction'] not in {'left','right','up','down'}:
                    continue
                nl = maps[other][2]
                off = c['offset']
                expect = {'right': (x + l['width'], y + off),
                          'left': (x - nl['width'], y + off),
                          'down': (x + off, y + l['height']),
                          'up': (x + off, y - nl['height'])}[c['direction']]
                if other in local:
                    if local[other] != expect:
                        bad.append([mid, other])
                else:
                    local[other] = expect
                    queue.append(other)
        conflicts.extend(bad)
        for mid, point in local.items():
            coordinates[mid] = point if not bad else (0, 0)
            domains[mid] = zlib.crc32((mid if bad else start).encode())
    attributes = load_words(root / 'data/tilesets/primary/general/metatile_attributes.bin')
    if tuple(attributes[i] for i in (GRASS, TALL, SAND)) != (0, 2, 0):
        raise ValueError('native movement/catalog contract changed')
    output = []
    for mid, (name, meta, l) in sorted(maps.items()):
        if (not re.fullmatch(r'Route1[0-3][0-9]', name)
                or meta['map_type'] != 'MAP_TYPE_ROUTE'
                or l['primary_tileset'] != 'gTileset_General'
                or review_reasons(name, meta)):
            continue
        w, h = l['width'], l['height']
        if (w + 15) * (h + 14) > 10240:
            continue
        words = load_words(root / l['blockdata_filepath'])
        if len(words) != w * h:
            raise ValueError(f'bad dimensions: {name}')
        mask = bytearray((w*h + 7)//8)
        # Keep the complete outer ring even without a connection. The seven
        # native connection rows/columns stay byte exact on both sides.
        for y in range(1, h-1):
            for x in range(1, w-1):
                k = y*w+x
                if (words[k] & ID in (GRASS, TALL) and not words[k] & COLLISION
                        and not protected(meta,x,y,w,h)):
                    mask[k//8] |= 1 << (k%8)
        if not any(mask):
            continue
        raw = struct.pack('<'+'H'*len(words), *words)
        topology = topology_mask(root,name,l,words,mask,attributes)
        catalog = extract_catalog(root,l) if any(topology) else None
        # Asset identity also includes tileset attributes and effective rules.
        assets = raw + mask + json.dumps(meta,sort_keys=True).encode()
        assets += (root / 'data/tilesets/primary/general/metatile_attributes.bin').read_bytes()
        assets += topology
        if catalog:
            assets += struct.pack('<I',catalog['hash'])
            for source in TOPOLOGY_REVIEW[name]: assets += (root/source).read_bytes()
        output.append(dict(name=name, mapId=ids[name], layoutId=by_layout[meta['layout']][0],
                           width=w, height=h, worldX=coordinates[mid][0], worldY=coordinates[mid][1],
                           domain=domains[mid], assetHash=zlib.crc32(assets),
                           native=list(words), editable=list(mask), topology=list(topology), catalog=catalog))
    if not output:
        raise ValueError('no eligible maps')
    return output, conflicts


def write(root, output, manifest):
    maps, conflicts = extract(root)
    lines = ['/* Generated by genesis2_assets.py. Native C99/ARM inputs. */']
    catalogs = {}
    for m in maps:
        if m['catalog'] and m['catalog']['hash'] not in catalogs:
            symbol = f'sG2Catalog{len(catalogs)}'
            emit_catalog(lines,m['catalog'],symbol)
            catalogs[m['catalog']['hash']] = symbol
    for i, m in enumerate(maps):
        arrays = [('Native','uint16_t',m['native']), ('Editable','uint8_t',m['editable'])]
        if m['catalog']: arrays.append(('Topology','uint8_t',m['topology']))
        for kind, ctype, values in arrays:
            lines.append(f'static const {ctype} sG2{kind}{i}[] = {{')
            for n in range(0, len(values), 16):
                lines.append('    '+','.join(hex(v) for v in values[n:n+16])+',')
            lines.append('};')
    lines.append('static const struct G2_Map sG2Maps[] = {')
    for i, m in enumerate(maps):
        values = [m[k] for k in ['mapId','layoutId','width','height','worldX','worldY','domain','assetHash']]
        lines.append('    {'+','.join(str(v) for v in values)
                     +f',sG2Native{i},sG2Editable{i},'
                     +(f'sG2Topology{i},&'+catalogs[m['catalog']['hash']] if m['catalog'] else 'NULL,NULL')
                     +'}, /* '+m['name']+' */')
    lines.append('};')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text('\n'.join(lines)+'\n')
    if manifest:
        manifest.parent.mkdir(parents=True, exist_ok=True)
        manifest.write_text(json.dumps(dict(generatorVersion=0x020100,catalogVersion=2,
                                            georeferenceConflicts=conflicts,maps=maps)))
    print(f'Genesis2: {len(maps)} land routes, {sum(m["width"]*m["height"]*2 for m in maps)} cache bytes, '
          f'{len(conflicts)} georeference conflicts')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path('.'))
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--manifest', type=Path)
    a = p.parse_args()
    write(a.root,a.output,a.manifest)
