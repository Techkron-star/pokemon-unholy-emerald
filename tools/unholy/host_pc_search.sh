#!/bin/sh
# Exercise the actual read-only search core without a ROM or emulator.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
cat > "$TMP/global.h" <<'EOF'
#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t bool8;
typedef uint32_t bool32;
#define TRUE 1
#define FALSE 0
EOF
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -iquote "$TMP" -iquote "$ROOT/include" \
  "$ROOT/src/pc_search.c" "$ROOT/tools/unholy/host_pc_search.c" -o "$TMP/pc-search"
"$TMP/pc-search"
