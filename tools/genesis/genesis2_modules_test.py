#!/usr/bin/env python3
"""Executable checks for reviewed module provenance, capacities and fail-closed extraction."""
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

from genesis2_modules import (APPROVED_ASSETS, CatalogError, CLEARING, TREE,
                              emit_catalog, extract_catalog, validate_module, words)

ROOT = Path(__file__).resolve().parents[2]
LAYOUT = {'primary_tileset': 'gTileset_General', 'secondary_tileset': 'gTileset_Petalburg',
          'layout_version': 'emerald'}


class ModulesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog = extract_catalog(ROOT, LAYOUT)
        base = ROOT/'data/tilesets/primary/general'
        cls.metatiles = words((base/'metatiles.bin').read_bytes(), 'metatiles')
        cls.attributes = words((base/'metatile_attributes.bin').read_bytes(), 'attributes')

    def fixture(self):
        td = tempfile.TemporaryDirectory()
        self.addCleanup(td.cleanup)
        root = Path(td.name)
        paths = ['data/layouts/layouts.json', 'data/layouts/LavaridgeTown/map.bin',
                 'data/layouts/Route102/map.bin', 'data/maps/LavaridgeTown/map.json',
                 'data/maps/Route102/map.json']
        paths.extend('data/tilesets/primary/general/'+name for name in APPROVED_ASSETS)
        for path in paths:
            dst = root/path
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT/path, dst)
        return root

    def test_reproducible_and_primary_only(self):
        self.assertEqual(self.catalog, extract_catalog(ROOT, LAYOUT))
        other = dict(LAYOUT, secondary_tileset='gTileset_Rustboro')
        self.assertEqual(self.catalog, extract_catalog(ROOT, other))
        self.assertEqual([m['kind'] for m in self.catalog['modules']], [TREE, CLEARING])
        for module in self.catalog['modules']:
            self.assertTrue(validate_module(module, self.metatiles, self.attributes))
            for cell in module['cells']:
                self.assertLess(cell&1023, 512)
                for ref in self.metatiles[(cell&1023)*8:((cell&1023)+1)*8]:
                    self.assertLess(ref&1023, 512)
                    self.assertLess(ref>>12, 6)

    def test_source_is_native_exact(self):
        layouts = {l['id']:l for l in json.loads((ROOT/'data/layouts/layouts.json').read_text())['layouts']}
        for module in self.catalog['modules']:
            source = module['source']
            layout = layouts[source['layout']]
            raw = (ROOT/source['blockdata_path']).read_bytes()
            grid = words(raw, 'source')
            x, y, w, h = source['x'], source['y'], module['width'], module['height']
            actual = [grid[(y+dy)*layout['width']+x+dx] for dy in range(h) for dx in range(w)]
            self.assertEqual(actual, module['cells'])
            self.assertEqual(len(source['halo_cells']), (w+2)*(h+2))
            self.assertTrue(source['primary_only'])
        self.assertEqual(self.catalog['modules'][0]['cells'], [0x35D6,0x35D7,0x35E6,0x35E7])

    def test_unsupported_layout_does_not_read_assets(self):
        for layout in [dict(LAYOUT,primary_tileset='gTileset_Cave'),
                       dict(LAYOUT,layout_version='firered')]:
            catalog = extract_catalog(Path('/unavailable-module-source'), layout)
            self.assertEqual(catalog['modules'], [])
            self.assertEqual(catalog['hash'], 0)

    def test_footprint_and_full_structure_validation(self):
        base = self.catalog['modules'][0]
        cases = [dict(base,width=0),dict(base,height=0),dict(base,width=37,height=1),
                 dict(base,width=True),dict(base,cells=None),dict(base,kind=True),
                 dict(base,cells=base['cells'][:-1]),dict(base,cells=base['cells']+[0]),
                 dict(base,cells=[0x35D4,0x35D5,0x35E4,0x35E5])]
        for case in cases:
            with self.subTest(case=case):
                with self.assertRaises(CatalogError):
                    validate_module(case,self.metatiles,self.attributes)

    def test_behavior_and_secondary_graphics_rejected(self):
        base = self.catalog['modules'][0]
        attrs = list(self.attributes)
        attrs[0x1D6] = 0x10
        with self.assertRaises(CatalogError):
            validate_module(base,self.metatiles,attrs)
        for ref in [0x23FF,0x6008]:
            metatiles = list(self.metatiles)
            metatiles[0x1D6*8] = ref
            with self.assertRaises(CatalogError):
                validate_module(base,metatiles,self.attributes)

    def test_missing_asset_is_rejected(self):
        root = self.fixture()
        (root/'data/tilesets/primary/general/tiles.png').unlink()
        with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_changed_graphic_at_same_id_is_rejected(self):
        root = self.fixture()
        path = root/'data/tilesets/primary/general/tiles.png'
        data = bytearray(path.read_bytes());data[-1]^=1;path.write_bytes(data)
        with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_changed_native_footprint_is_rejected(self):
        root = self.fixture()
        path = root/'data/layouts/LavaridgeTown/map.bin'
        layouts = json.loads((root/'data/layouts/layouts.json').read_text())['layouts']
        w = next(l['width'] for l in layouts if l['id']=='LAYOUT_LAVARIDGE_TOWN')
        data = bytearray(path.read_bytes())
        struct.pack_into('<H',data,(5*w+12)*2,0x3001);path.write_bytes(data)
        with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_changed_source_halo_is_rejected(self):
        root = self.fixture()
        path = root/'data/layouts/LavaridgeTown/map.bin'
        layouts = json.loads((root/'data/layouts/layouts.json').read_text())['layouts']
        w = next(l['width'] for l in layouts if l['id']=='LAYOUT_LAVARIDGE_TOWN')
        data = bytearray(path.read_bytes())
        struct.pack_into('<H',data,(4*w+11)*2,0x3001);path.write_bytes(data)
        with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_event_in_source_is_rejected(self):
        for field in ['object_events','warp_events','coord_events','bg_events']:
            with self.subTest(field=field):
                root = self.fixture()
                path = root/'data/maps/LavaridgeTown/map.json'
                meta = json.loads(path.read_text())
                meta.setdefault(field,[]).append({'x':12,'y':5})
                path.write_text(json.dumps(meta))
                with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_source_bounds_are_rejected(self):
        root = self.fixture()
        path = root/'data/layouts/layouts.json'
        data = json.loads(path.read_text())
        next(l for l in data['layouts'] if l['id']=='LAYOUT_LAVARIDGE_TOWN')['width']=13
        path.write_text(json.dumps(data))
        with self.assertRaises(CatalogError): extract_catalog(root,LAYOUT)

    def test_emitted_c99_and_empty_catalog(self):
        with tempfile.TemporaryDirectory() as td:
            source = Path(td)/'test.c'
            lines = ['#include "genesis2/modules.h"']
            self.assertEqual(emit_catalog(lines,self.catalog,'reviewedCatalog'),'reviewedCatalog')
            emit_catalog(lines,{'modules':[],'hash':0},'emptyCatalog')
            lines.append('int main(void) { return reviewedCatalog.count != 2 || emptyCatalog.modules != NULL; }')
            source.write_text('\n'.join(lines))
            binary = Path(td)/'test'
            subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Wconversion','-Werror',
                            '-I'+str(ROOT/'include'),str(source),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)
        with self.assertRaises(CatalogError): emit_catalog([],self.catalog,'bad;symbol')


if __name__ == '__main__':
    unittest.main()
