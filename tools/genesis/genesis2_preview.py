#!/usr/bin/env python3
"""Render actual native metatiles/palettes from C output; no Python terrain clone."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
from PIL import Image, ImageDraw


def palette(path):
    rows=path.read_text().splitlines()
    assert rows[:3]==['JASC-PAL','0100','16']
    # Native 5-bit palette, expanded for PNG display.
    return [tuple((int(v)>>3)*255//31 for v in line.split()) for line in rows[3:19]]


def atlas(root, layout):
    primary=root/'data/tilesets/primary/general'
    secondary=root/'data/tilesets/secondary'/layout['secondary_tileset'].removeprefix('gTileset_').lower()
    # Resolve source folder from the actual tileset definition for non-Petalburg
    # maps in a future expansion. Current review target is General/Petalburg.
    assert layout['secondary_tileset']=='gTileset_Petalburg'
    palettes=[palette((primary if n<6 else secondary)/'palettes'/f'{n:02}.pal') for n in range(16)]
    tiles=[]
    for folder in [primary,secondary]:
        im=Image.open(folder/'tiles.png')
        assert im.mode=='P'
        cells=[im.crop((x,y,x+8,y+8)) for y in range(0,im.height,8) for x in range(0,im.width,8)]
        tiles.extend(cells)
    assert Image.open(primary/'tiles.png').width*Image.open(primary/'tiles.png').height//64==512
    words=[]
    for folder in [primary,secondary]:
        b=(folder/'metatiles.bin').read_bytes()
        words.extend(struct.unpack('<'+'H'*(len(b)//2),b))
    attrs=[]
    for folder in [primary,secondary]:
        b=(folder/'metatile_attributes.bin').read_bytes()
        attrs.extend(struct.unpack('<'+'H'*(len(b)//2),b))
    output=[]
    for mid in range(len(words)//8):
        canvas=Image.new('RGB',(16,16),palettes[0][0])
        for layer in range(2):
            for part in range(4):
                word=words[mid*8+layer*4+part]
                tid=word&1023
                if tid>=len(tiles):
                    # Unused metatiles can reference animation-reserved tiles.
                    continue
                tile=tiles[tid]
                if word&1024:tile=tile.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
                if word&2048:tile=tile.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
                bank=palettes[word>>12]
                for y in range(8):
                    for x in range(8):
                        index=tile.getpixel((x,y))
                        if layer==0 or index:
                            canvas.putpixel(((part%2)*8+x,(part//2)*8+y),bank[index])
        output.append(canvas)
    return output


def render(words,w,h,tiles):
    result=Image.new('RGB',(w*16,h*16))
    for k,value in enumerate(words):
        assert value&1023<len(tiles)
        result.paste(tiles[value&1023],((k%w)*16,(k//w)*16))
    return result


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,default=Path('build/genesis2-previews'))
    p.add_argument('--seeds',type=int,default=100)
    a=p.parse_args()
    assert 1<=a.seeds<=1000
    root=Path('.')
    data=json.loads((root/'build/genesis2-assets.json').read_text())
    m=next(m for m in data['maps'] if m['name']=='Route102')
    layouts=json.loads((root/'data/layouts/layouts.json').read_text())['layouts']
    layout=layouts[m['layoutId']-1]
    tiles=atlas(root,layout)
    a.output.mkdir(parents=True,exist_ok=True)
    exporter=a.output/'export'
    subprocess.run(['cc','-O2','-std=c99','-Iinclude','src/genesis2/world.c',
                    'tools/genesis/genesis2_export.c','-o',str(exporter)],check=True)
    native=render(m['native'],m['width'],m['height'],tiles)
    native.save(a.output/'original.png')
    sheet=Image.new('RGB',(2000,100*((a.seeds+9)//10)), '#141921')
    draw=ImageDraw.Draw(sheet)
    changed=set()
    for seed in range(a.seeds):
        raw=a.output/f'seed-{seed:03}.bin'
        subprocess.run([str(exporter),str(seed),str(m['mapId']),str(raw)],check=True)
        words=struct.unpack('<'+'H'*(len(raw.read_bytes())//2),raw.read_bytes())
        changed.add(words)
        image=render(words,m['width'],m['height'],tiles)
        image.save(a.output/f'seed-{seed:03}.png')
        thumb=image.resize((200,80),Image.Resampling.NEAREST)
        x,y=(seed%10)*200,(seed//10)*100
        sheet.paste(thumb,(x,y))
        draw.text((x+5,y+82),f'Route102 / seed {seed}',fill='#eeeeee')
    assert len(changed)>1 or a.seeds==1
    sheet.save(a.output/'contact-sheet.png')
    print(f'{a.seeds} authentic native-asset previews; {len(changed)} distinct maps')
