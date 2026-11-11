#!/bin/sh
# Standalone native smoke test for the *real* extension sources.
# No ARM packages, emulator, ROM build, network access, or flash writes.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM

cat > "$TMP/global.h" <<'EOF'
#ifndef UNHOLY_HOST_GLOBAL_H
#define UNHOLY_HOST_GLOBAL_H
#include <stdint.h>
#include <string.h>
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint32_t bool32;
#define TRUE 1
#define FALSE 0
#include "unholy_save.h"
struct SaveBlock3 { struct SaveBlock4 unholySave4; };
extern struct SaveBlock3 *gSaveBlock3Ptr;
#endif
EOF

cat > "$TMP/pokemon_storage_system.h" <<'EOF'
#ifndef UNHOLY_HOST_STORAGE_H
#define UNHOLY_HOST_STORAGE_H
#include "unholy_save.h"
struct PokemonStorage { struct SaveBlock5 unholySave5; };
extern struct PokemonStorage *gPokemonStoragePtr;
#endif
EOF

${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I"$TMP" -I"$ROOT/include" \
  "$ROOT/src/unholy_save.c" "$ROOT/src/unholy_mon_ext.c" "$ROOT/src/stamina_ring.c" \
  "$ROOT/tools/unholy/host_save_schema.c" -o "$TMP/unholy-save-host"
"$TMP/unholy-save-host"
