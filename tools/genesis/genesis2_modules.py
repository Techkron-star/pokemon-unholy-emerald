#!/usr/bin/env python3
"""Extract a reviewed native module catalog, never infer complete art from noise.

General's 2x2 standalone tree and 4x4 grass clearing were reviewed from the
actual metatile atlas and native source layouts. No rocks, buildings, water,
cliffs, cut trees or sprite events are guessed. Each module is primary-only:
its references stay in General's 512 graphics tiles and six palette banks.
The catalogue can therefore be shared across General/secondary pairs without
importing secondary metatile IDs. Unknown assets fail closed for review.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import zlib

VERSION = 1
MAX_CELLS = 36
TREE, CLEARING = 1, 2
ID, COLLISION, ELEVATION = 0x03FF, 0x0C00, 0xF000
PRIMARY = 'gTileset_General'

# Reviewed asset identity. A replacement graphic at the same numeric ID must
# be explicitly reviewed, rather than inheriting the tree semantic by accident.
APPROVED_ASSETS = {
    'tiles.png': '365ba5482e7564763e614a0009900d1c317790dcd9f7a06c4ab74c363a3ba2f5',
    'metatiles.bin': '17974b0ba1bb9d105d5ba255c69a3b80d8484a26ea0b8ff22c6c6567c6c22910',
    'metatile_attributes.bin': '06c9548a6c70d133b6d47638090f5d36f3fec92aace3d90deca758a4404cfa9b',
    'palettes/02.pal': '6a2609b1c79e8489ed11182eb075edf0e25d80b5b3b7912c0de20dc5da594eb9',
}

# The tree is a complete 32x32 graphic: upper outer edges D6/D7 and lower
# outer edges E6/E7. D4/D5 and E4/E5 are forest continuation pieces and are
# intentionally excluded even though many native maps contain their 2x2 pair.
RECIPES = (
    {'name': 'general_standalone_tree', 'kind': TREE,
     'layout': 'LAYOUT_LAVARIDGE_TOWN', 'map': 'LavaridgeTown', 'x': 12, 'y': 5,
     'width': 2, 'height': 2,
     'cells': [0x35D6, 0x35D7, 0x35E6, 0x35E7],
     'halo_sha256': 'f7b41628a95ba302951ba78371bc4cab52cf97ff6ba453514d0494a9c83cf056'},
    {'name': 'general_grass_clearing', 'kind': CLEARING,
     'layout': 'LAYOUT_ROUTE102', 'map': 'Route102', 'x': 36, 'y': 7,
     'width': 4, 'height': 4, 'cells': [0x3001] * 16,
     'halo_sha256': '70fc7a7103e07422ab263418bb6cc5cd12ea6b4b3024ef4da1c716b6514600bb'},
)


class CatalogError(ValueError):
    pass


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=True).encode('ascii')


def checksum(value):
    return zlib.crc32(canonical(value))


def read_bytes(path):
    try:
        return path.read_bytes()
    except OSError as e:
        raise CatalogError(f'missing/unreadable native asset: {path}') from e


def read_json(path):
    try:
        return json.loads(read_bytes(path))
    except (json.JSONDecodeError, UnicodeDecodeError) as e:
        raise CatalogError(f'invalid native metadata: {path}') from e


def words(data, name):
    if len(data) % 2:
        raise CatalogError(f'odd word length: {name}')
    return struct.unpack('<' + 'H' * (len(data)//2), data)


def validate_module(module, metatiles, attributes):
    """Validate representation, exact reviewed footprint and primary-only art."""
    try:
        w, h, kind, cells = module['width'], module['height'], module['kind'], module['cells']
    except (KeyError, TypeError) as e:
        raise CatalogError('module is missing its footprint') from e
    if (not isinstance(cells, (list, tuple)) or not isinstance(kind, int) or isinstance(kind, bool)
            or not isinstance(w, int) or not isinstance(h, int) or isinstance(w, bool) or isinstance(h, bool)
            or w < 1 or h < 1 or w*h > MAX_CELLS or len(cells) != w*h):
        raise CatalogError('module footprint exceeds its capacity or is incomplete')
    recipe = next((r for r in RECIPES if r['kind'] == kind and r['width'] == w
                   and r['height'] == h and list(cells) == r['cells']), None)
    if recipe is None:
        raise CatalogError('module is not an approved complete graphic')
    for cell in cells:
        if not isinstance(cell, int) or not 0 <= cell <= 0xFFFF:
            raise CatalogError('module cell is not a native u16')
        mid = cell & ID
        if mid >= 512 or mid >= len(attributes) or (mid+1)*8 > len(metatiles):
            raise CatalogError('module references unavailable/secondary metatiles')
        if cell & ELEVATION != 0x3000 or attributes[mid] != 0:
            raise CatalogError('module requires ordinary native elevation-3 behavior')
        expected_collision = 0x0400 if kind == TREE else 0
        if cell & COLLISION != expected_collision:
            raise CatalogError('module collision differs from reviewed native art')
        for word in metatiles[mid*8:(mid+1)*8]:
            if (word & 1023) >= 512 or (word >> 12) >= 6:
                raise CatalogError('module depends on secondary graphics or palettes')
    return True


def _extract_recipe(root, layouts, recipe, metatiles, attributes):
    layout = layouts.get(recipe['layout'])
    if (not layout or layout.get('layout_version') != 'emerald'
            or layout.get('primary_tileset') != PRIMARY):
        raise CatalogError(f"reviewed source layout changed: {recipe['layout']}")
    w, h, x, y, mw, mh = (layout['width'], layout['height'], recipe['x'], recipe['y'],
                           recipe['width'], recipe['height'])
    if not (isinstance(w, int) and isinstance(h, int) and w > 0 and h > 0
            and 1 <= x and 1 <= y and x+mw < w and y+mh < h):
        raise CatalogError('module source footprint/halo lies outside layout')
    path = root/layout['blockdata_filepath']
    raw = read_bytes(path)
    grid = words(raw, str(path))
    if len(grid) != w*h:
        raise CatalogError('module source dimensions differ from block data')
    cells = [grid[(y+dy)*w+x+dx] for dy in range(mh) for dx in range(mw)]
    module = dict(width=mw, height=mh, kind=recipe['kind'], cells=cells)
    validate_module(module, metatiles, attributes)
    if cells != recipe['cells']:
        raise CatalogError('reviewed source module changed')
    halo = [grid[yy*w+xx] for yy in range(y-1, y+mh+1) for xx in range(x-1, x+mw+1)]
    halo_raw = struct.pack('<'+'H'*len(halo), *halo)
    if hashlib.sha256(halo_raw).hexdigest() != recipe['halo_sha256']:
        raise CatalogError('reviewed source neighborhood changed; review module boundaries')
    meta = read_json(root/'data/maps'/recipe['map']/'map.json')
    if meta.get('layout') != recipe['layout']:
        raise CatalogError('source map is no longer bound to reviewed layout')
    for field in ('object_events', 'warp_events', 'coord_events', 'bg_events'):
        for e in meta.get(field) or []:
            if x <= e['x'] < x+mw and y <= e['y'] < y+mh:
                raise CatalogError('module footprint contains an event/warp/script')
    module['source'] = {
        'name': recipe['name'], 'layout': recipe['layout'], 'map': recipe['map'],
        'x': x, 'y': y, 'blockdata_path': layout['blockdata_filepath'],
        'blockdata_sha256': hashlib.sha256(raw).hexdigest(),
        'halo_radius': 1, 'halo_width': mw+2, 'halo_height': mh+2, 'halo_cells': halo,
        'halo_sha256': recipe['halo_sha256'], 'primary_only': True,
    }
    module['hash'] = checksum(module)
    return module


def extract_catalog(root, layout):
    """Return a serializable catalog for a verified compatible native layout.

    Unsupported tilesets/domains return an empty catalog. Corrupt, missing or
    changed reviewed assets raise CatalogError, so no guessed art is emitted.
    """
    root = Path(root)
    if layout.get('primary_tileset') != PRIMARY or layout.get('layout_version') != 'emerald':
        return {'version': VERSION, 'primary': layout.get('primary_tileset'),
                'modules': [], 'hash': 0, 'reason': 'unsupported native primary/domain'}
    base = root/'data/tilesets/primary/general'
    asset_data = {}
    for name, approved in APPROVED_ASSETS.items():
        data = read_bytes(base/name)
        digest = hashlib.sha256(data).hexdigest()
        if digest != approved:
            raise CatalogError(f'General asset changed; explicit module review required: {name}')
        asset_data[name] = data
    metatiles = words(asset_data['metatiles.bin'], 'General metatiles')
    attributes = words(asset_data['metatile_attributes.bin'], 'General attributes')
    if len(metatiles) != 512*8 or len(attributes) != 512:
        raise CatalogError('General primary capacity differs from reviewed native contract')
    layouts = {l['id']: l for l in read_json(root/'data/layouts/layouts.json')['layouts']}
    modules = [_extract_recipe(root, layouts, r, metatiles, attributes) for r in RECIPES]
    result = {'version': VERSION, 'primary': PRIMARY, 'compatibility': 'primary-only',
              'asset_sha256': dict(APPROVED_ASSETS), 'modules': modules}
    result['hash'] = checksum(result)
    return result


def emit_catalog(lines, catalog, symbol):
    """Append C99 tables to lines and return the G2_Catalog object's name."""
    if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', symbol):
        raise CatalogError('invalid C catalog symbol')
    modules = catalog['modules']
    if len(modules) > 0xFFFF:
        raise CatalogError('catalog exceeds native count capacity')
    array = symbol + 'Modules'
    if modules:
        lines.append(f'static const struct G2_Module {array}[] = {{')
        for module in modules:
            cells = ','.join(f'0x{v:04X}' for v in module['cells'])
            lines.append('    {' + f'0x{module["hash"]:08X}u,{{{cells}}},'
                         + f'{module["width"]},{module["height"]},{module["kind"]}' + '},')
        lines.append('};')
    lines.append(f'static const struct G2_Catalog {symbol} = '
                 + '{' + f'{array if modules else "NULL"},0x{catalog["hash"]:08X}u,{len(modules)}' + '};')
    return symbol


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path('.'))
    p.add_argument('--output', type=Path)
    p.add_argument('--manifest', type=Path)
    a = p.parse_args()
    layout = {'primary_tileset': PRIMARY, 'layout_version': 'emerald'}
    catalog = extract_catalog(a.root, layout)
    if a.output:
        lines = ['/* Generated reviewed native modules; do not edit. */', '#include "genesis2/modules.h"']
        emit_catalog(lines, catalog, 'sG2GeneralCatalog')
        a.output.parent.mkdir(parents=True, exist_ok=True)
        a.output.write_text('\n'.join(lines)+'\n')
    if a.manifest:
        a.manifest.parent.mkdir(parents=True, exist_ok=True)
        a.manifest.write_text(json.dumps(catalog, indent=2)+'\n')
    print(f'Genesis2 modules: {len(catalog["modules"])} reviewed modules, hash {catalog["hash"]:08x}')
