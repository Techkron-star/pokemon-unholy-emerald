#!/usr/bin/env python3
"""Validate contract NPC placement against the actual collision maps."""
import json
import re
import struct
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LAYOUTS = {x['id']: x for x in json.loads((ROOT / 'data/layouts/layouts.json').read_text())['layouts']}


def grid(map_data):
    layout = LAYOUTS[map_data['layout']]
    raw = (ROOT / layout['blockdata_filepath']).read_bytes()
    return layout['width'], layout['height'], struct.unpack('<' + 'H' * (len(raw) // 2), raw)


def connected(width, height, blocks, start, goal):
    seen = {start}
    queue = deque([start])
    while queue:
        x, y = queue.popleft()
        if abs(x - goal[0]) + abs(y - goal[1]) == 1:
            return True
        for nx, ny in [(x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)]:
            if 0 <= nx < width and 0 <= ny < height and (nx, ny) not in seen and (nx, ny) != goal:
                if blocks[ny * width + nx] & 0xC00 == 0:
                    seen.add((nx, ny))
                    queue.append((nx, ny))
    return False


def validate():
    sellers = 0
    for path in sorted((ROOT / 'data/maps').glob('*/map.json')):
        map_data = json.loads(path.read_text())
        if map_data['region'] != 'REGION_HOENN' or map_data['map_type'] not in ('MAP_TYPE_CITY', 'MAP_TYPE_TOWN', 'MAP_TYPE_ROUTE'):
            continue
        shops = [w for w in map_data.get('warp_events', []) if (w['dest_map'].endswith('_MART') and 'UNUSED' not in w['dest_map']) or w['dest_map'] == 'MAP_LILYCOVE_CITY_DEPARTMENT_STORE_1F']
        contracts = [e for e in map_data.get('object_events', []) if e['script'] == 'EventScript_UnholyBountySeller']
        assert len(contracts) == len(shops), path
        if not shops:
            continue
        width, height, blocks = grid(map_data)
        occupied = {(e['x'], e['y']) for e in map_data['object_events'] if e not in contracts}
        occupied.update((e['x'], e['y']) for e in map_data.get('warp_events', []))
        for shop, seller in zip(shops, contracts):
            point = seller['x'], seller['y']
            assert point not in occupied, (path, 'overlap')
            assert blocks[point[1] * width + point[0]] & 0xC00 == 0, (path, 'collision')
            assert point != (shop['x'], shop['y'] + 1), (path, 'door approach blocked')
            assert abs(point[0] - shop['x']) <= 2 and 0 < point[1] - shop['y'] <= 2, (path, 'too far from shop')
            assert connected(width, height, blocks, (shop['x'], shop['y'] + 1), point), (path, 'inaccessible')
            sellers += 1
    source = (ROOT / 'src/unholy_bounty.c').read_text()
    targets = re.findall(r'\{MAP_GROUP\(MAP_(ROUTE\d+)\),.*?, (\d+), (\d+)\}', source)
    for route, sx, sy in targets:
        name = route.title().replace('Route', 'Route')
        map_data = json.loads((ROOT / f'data/maps/{name}/map.json').read_text())
        width, height, blocks = grid(map_data)
        x, y = int(sx), int(sy)
        occupied = {(e['x'], e['y']) for key in ('object_events', 'warp_events', 'coord_events', 'bg_events') for e in map_data.get(key, []) if 'x' in e}
        assert (x, y) not in occupied, (route, 'target overlap')
        assert blocks[y * width + x] & 0xC00 == 0, (route, 'target collision')
        approaches = sum(blocks[ny * width + nx] & 0xC00 == 0 for nx, ny in [(x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)])
        assert approaches >= 3, (route, 'target has insufficient approaches')
    assert sellers == 13 and len(targets) == 5, (sellers, targets)
    print(f'PASS: {sellers} shop sellers and {len(targets)} route targets; no door or object overlaps.')


if __name__ == '__main__':
    validate()
