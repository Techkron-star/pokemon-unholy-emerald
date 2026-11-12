#ifndef GUARD_STAMINA_RING_H
#define GUARD_STAMINA_RING_H

// 32x32 4bpp (sixteen 8x8 OBJ tiles, 512 bytes). The caller owns this buffer.
// No OBJ VRAM access or battle globals: render and test independently.
#define STAMINA_RING_TILE_BYTES 512
// The visible ring is 9x9, centred in a 16x16 OBJ (four tiles).
#define STAMINA_RING_COMPACT_TILE_BYTES 128
#define STAMINA_RING_COMPACT_RADIUS 4
u32 StaminaRing_FilledSegments(u32 current, u32 max);
void StaminaRing_Render(u8 *tiles, u32 current, u32 max, bool32 exhausted);
void StaminaRing_RenderCompact(u8 *tiles, u32 current, u32 max, bool32 exhausted);

#endif // GUARD_STAMINA_RING_H
