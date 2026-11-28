#ifndef GUARD_GENESIS2_MODULES_H
#define GUARD_GENESIS2_MODULES_H
#include <stdint.h>
#include <stddef.h>

#define G2_MODULE_MAX_CELLS 36u
#define G2_MODULE_CATALOG_VERSION UINT32_C(1)

enum G2_ModuleKind {
    G2_MODULE_TREE = 1,
    G2_MODULE_CLEARING = 2
};

/* Every footprint is copied atomically, including native collision/elevation.
 * These reviewed modules use only the General primary tiles/palettes. They
 * require an ordinary grass ground footprint at native elevation 3; callers
 * own a non-overlapping halo and progression/movement validation. */
struct G2_Module {
    uint32_t hash;
    uint16_t cells[G2_MODULE_MAX_CELLS];
    uint8_t width, height, kind;
};
struct G2_Catalog {
    const struct G2_Module *modules;
    uint32_t hash;
    uint16_t count;
};
#endif
