#ifndef GENESIS_MOCK_FIELD_H
#define GENESIS_MOCK_FIELD_H
#include "global.h"
struct Tileset {u8 dummy;};
struct MapLayout {s32 width,height;const u16 *border,*map;const struct Tileset *primaryTileset,*secondaryTileset;u8 isFrlg,borderWidth,borderHeight,padding;};
struct BackupMapLayout {s32 width,height;u16 *map;};
struct MapHeader {const struct MapLayout *mapLayout;};
extern struct MapHeader gMapHeader;
extern struct BackupMapLayout gBackupMapLayout;
#endif
