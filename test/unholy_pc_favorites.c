#include "global.h"
#include "pokemon.h"
#include "unholy_mon_identity.h"
#include "constants/gears.h"
#include "test/test.h"

TEST("PC favorite uses only the spare header bit and preserves existing mon data")
{
    struct Pokemon mon, original;
    struct UnholyMonLegacyKey before, after;
    u8 value = 0xFF;
    u8 markings = 0xF;
    u8 gear = GEAR_PERMAFROST_FRAGMENT;
    u16 item = ITEM_LEFTOVERS;
    u32 status = STATUS1_POISON;
    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 42, 0x44332211,
                     OTID_STRUCT_PRESET(0x88776655), 20);
    SetMonData(&mon, MON_DATA_MARKINGS, &markings);
    SetMonData(&mon, MON_DATA_STATUS, &status);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(&mon, MON_DATA_GEAR, &gear);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_ALPHA);
    Pokemon_SetBadge(&mon, POKEMON_BADGE_TITAN);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_FAVORITE), FALSE);
    EXPECT(UnholyMonIdentity_ReadMon(&mon, &before));
    original = mon;

    SetMonData(&mon, MON_DATA_IS_FAVORITE, &value);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_FAVORITE), TRUE);
    EXPECT_EQ(GetBoxMonData(&mon.box, MON_DATA_IS_FAVORITE), TRUE);
    EXPECT(UnholyMonIdentity_ReadMon(&mon, &after));
    EXPECT(UnholyMonIdentity_KeyEquals(&before, &after));
    for (u32 i = 0; i < sizeof(mon); i++)
        EXPECT_EQ(((u8 *)&mon)[i], ((u8 *)&original)[i] | (i == 0x13 ? 0x80 : 0));

    value = FALSE;
    SetBoxMonData(&mon.box, MON_DATA_IS_FAVORITE, &value);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_IS_FAVORITE), FALSE);
    EXPECT_EQ(memcmp(&mon, &original, sizeof(mon)), 0);
}

TEST("PC favorite survives box and party conversion without spreading to new mons")
{
    struct Pokemon source, withdrawn, fresh;
    struct BoxPokemon deposited, moved;
    u8 favorite = TRUE;
    CreateMonWithIVs(&source, SPECIES_EEVEE, 20, 0x12345678,
                     OTID_STRUCT_PRESET(0x11223344), 15);
    SetMonData(&source, MON_DATA_IS_FAVORITE, &favorite);
    deposited = source.box;
    moved = deposited;
    memset(&deposited, 0, sizeof(deposited));
    EXPECT_EQ(GetBoxMonData(&moved, MON_DATA_IS_FAVORITE), TRUE);
    EXPECT_EQ(GetBoxMonData(&deposited, MON_DATA_IS_FAVORITE), FALSE);
    BoxMonToMon(&moved, &withdrawn);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_IS_FAVORITE), TRUE);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_SPECIES), SPECIES_EEVEE);
    EXPECT_EQ(GetMonData(&withdrawn, MON_DATA_CHECKSUM), GetMonData(&source, MON_DATA_CHECKSUM));
    CreateMonWithIVs(&fresh, SPECIES_EEVEE, 20, 0x12345678,
                     OTID_STRUCT_PRESET(0x11223344), 15);
    EXPECT_EQ(GetMonData(&fresh, MON_DATA_IS_FAVORITE), FALSE);
    favorite = FALSE;
    SetMonData(&withdrawn, MON_DATA_IS_FAVORITE, &favorite);
    EXPECT_EQ(GetBoxMonData(&moved, MON_DATA_IS_FAVORITE), TRUE);
}
