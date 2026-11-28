#!/usr/bin/env python3
"""Compile a headless native harness and execute actual ROM routines on GBAX.

Example: python3 tools/genesis/genesis2_arm_test.py --core ../mgba/build-libretro/mgba_libretro.so
Requires a matching compiled ELF/ROM, host cc, nm and the core's CMake build tree.
This is synthetic entry/publication verification, not an end-to-end playthrough.
"""
import argparse
from pathlib import Path
import shlex
import subprocess
import tempfile
import os

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--core', type=Path, required=True)
p.add_argument('--elf', type=Path, default=Path('pokeemerald.elf'))
p.add_argument('--rom', type=Path, default=Path('pokeemerald.gba'))
p.add_argument('--nm', default='arm-none-eabi-nm')
a = p.parse_args()
build = a.core.resolve().parent
source = Path((build/'CMakeCache.txt').read_text().split('CMAKE_HOME_DIRECTORY:INTERNAL=')[1].splitlines()[0])
flags = (build/'CMakeFiles/mgba_libretro.dir/flags.make').read_text()
defs = shlex.split(next(l for l in flags.splitlines() if l.startswith('C_DEFINES =')).partition('=')[2])
symbols = {s[2]:s[0] for l in subprocess.check_output([a.nm,str(a.elf)],text=True).splitlines()
           if len(s:=l.split())==3}
names = ['gGenesis2Stats','G2_StartNewGame','G2_RuntimeTick','G2_ApplyCurrentMap',
         'G2_CommitNewGame','G2_LoadWorld','gSaveBlock1Ptr','gSaveBlock2Ptr','gSaveBlock3Ptr',
         'gSaveblock1','gSaveblock2','gSaveblock3','gMapHeader','gBackupMapLayout',
         'sBackupMapData','gMapGroups','gRngValue','G2_PrepareSavedView',
         'G2_ShouldRestoreSavedCell','G2_ReconcileCurrentModules']
with tempfile.TemporaryDirectory() as td:
    binary = Path(td)/'genesis2-arm-test'
    subprocess.run(['cc','-O2','-std=c11',*defs,'-iquote','include',
                    '-I'+str(source/'include'),'-I'+str(build/'include'),
                    'src/genesis2/world.c','src/genesis2/topology.c','tools/genesis/genesis2_arm_test.c',
                    '-L'+str(build),'-Wl,-rpath,'+str(build),'-l:'+a.core.name,
                    '-o',str(binary)],check=True)
    subprocess.run([str(binary),str(a.rom),*[symbols[n] for n in names]],check=True)
    subprocess.run([str(binary),str(a.rom),*[symbols[n] for n in names]],check=True,
                   env={**os.environ,'G2_ARM_SEED':'12'})
    native = bytearray(a.rom.read_bytes())
    native[0xA0] = ord('N')
    native[0xBD] = (-sum(native[0xA0:0xBD])-0x19)&255
    unmarked = Path(td)/'native.gba'
    unmarked.write_bytes(native)
    subprocess.run([str(binary),str(unmarked),*[symbols[n] for n in names]],check=True)
