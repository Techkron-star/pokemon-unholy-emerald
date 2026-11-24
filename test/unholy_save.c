#include "global.h"
#include "pokemon_storage_system.h"
#include "unholy_save.h"
#include "test/test.h"

// Logical SaveBlock4 lives in the sidecar and needs its own CRC32 check.
// SaveBlock5 also receives an envelope CRC as defense in depth.
TEST("Unholy logical SaveBlock4 detects tampering and unsupported versions")
{
    struct SaveBlock4 *block;
    u8 oldByte;

    UnholySave4_Reset();
    block = UnholySave4_Get();
    EXPECT_EQ(block != NULL, TRUE);
    EXPECT_EQ(UnholySave4_Validate(block), TRUE);

    block->payload[0] = 0x5A;
    EXPECT_EQ(UnholySave4_Validate(block), FALSE);
    EXPECT_EQ(UnholySave4_Get() == NULL, TRUE);
    EXPECT_EQ(UnholySave4_Commit(), TRUE);
    EXPECT_EQ(UnholySave4_Validate(block), TRUE);

    oldByte = block->payload[0];
    block->payload[0] ^= 0x40;
    EXPECT_EQ(UnholySave4_Validate(block), FALSE);
    block->payload[0] = oldByte;
    EXPECT_EQ(UnholySave4_Validate(block), TRUE);

    block->version++;
    EXPECT_EQ(UnholySave4_Get() == NULL, TRUE);
    EXPECT_EQ(UnholySave4_Commit(), FALSE);
    block->version = UNHOLY_SAVE4_VERSION;
    EXPECT_EQ(UnholySave4_Validate(block), TRUE);
}

TEST("Unholy logical SaveBlock5 checks data integrity without moving box offsets")
{
    struct SaveBlock5 *block;
    u8 oldByte;

    UnholySave5_Reset();
    block = UnholySave5_Get();
    EXPECT_EQ(block != NULL, TRUE);
    EXPECT_EQ(UnholySave5_Validate(block), TRUE);

    block->payload[UNHOLY_SAVE5_PAYLOAD_BYTES - 1] = 0xA5;
    EXPECT_EQ(UnholySave5_Validate(block), FALSE);
    EXPECT_EQ(UnholySave5_Commit(), TRUE);
    EXPECT_EQ(UnholySave5_Validate(block), TRUE);

    oldByte = block->payload[UNHOLY_SAVE5_PAYLOAD_BYTES - 1];
    block->payload[UNHOLY_SAVE5_PAYLOAD_BYTES - 1] ^= 0x01;
    EXPECT_EQ(UnholySave5_Get() == NULL, TRUE);
    block->payload[UNHOLY_SAVE5_PAYLOAD_BYTES - 1] = oldByte;
    EXPECT_EQ(UnholySave5_Validate(block), TRUE);
}
