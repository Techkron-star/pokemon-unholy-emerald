#include "global.h"
#include "pokemon.h"
#include "unholy_mon_identity.h"
#include "test/test.h"

TEST("Unholy migration fingerprint survives Pokémon copy and mutation")
{
    struct Pokemon mon, duplicate;
    struct UnholyMonLegacyKey original, copied;
    struct UnholyMonIdentityScan scan;

    CreateMonWithIVs(&mon, SPECIES_SCEPTILE, 42, 0x44332211,
                     OTID_STRUCT_PRESET(0x88776655), MAX_PER_STAT_IVS);
    EXPECT_EQ(UnholyMonIdentity_ReadMon(&mon, &original), TRUE);

    // Moving/copying a Pokémon to another slot preserves its legacy key.
    CopyMon(&duplicate, &mon, sizeof(mon));
    EXPECT_EQ(UnholyMonIdentity_ReadMon(&duplicate, &copied), TRUE);
    EXPECT_EQ(UnholyMonIdentity_KeyEquals(&original, &copied), TRUE);
    EXPECT_EQ(original.personality, 0x44332211);
    EXPECT_EQ(original.otId, 0x88776655);

    Pokemon_SetBadge(&duplicate, POKEMON_BADGE_OMEGA);
    EXPECT_EQ(UnholyMonIdentity_ReadMon(&duplicate, &copied), TRUE);
    EXPECT_EQ(UnholyMonIdentity_KeyEquals(&original, &copied), TRUE);

    // It is UNsafe to attach two identical-key Pokémon to one extension.
    UnholyMonIdentity_ScanInit(&scan, &original);
    EXPECT_EQ(UnholyMonIdentity_ScanStatus(&scan), UNHOLY_MON_ID_MISSING);
    UnholyMonIdentity_ScanObserve(&scan, &original);
    EXPECT_EQ(UnholyMonIdentity_ScanStatus(&scan), UNHOLY_MON_ID_UNIQUE);
    UnholyMonIdentity_ScanObserve(&scan, &copied);
    EXPECT_EQ(UnholyMonIdentity_ScanStatus(&scan), UNHOLY_MON_ID_AMBIGUOUS);

    // A third clone cannot make the ambiguity disappear through overflow.
    UnholyMonIdentity_ScanObserve(&scan, &copied);
    EXPECT_EQ(UnholyMonIdentity_ScanStatus(&scan), UNHOLY_MON_ID_AMBIGUOUS);
}

TEST("Unholy identity scanner fails closed with no initialized key")
{
    struct UnholyMonIdentityScan scan;
    struct UnholyMonLegacyKey candidate = {0x33333333, 0x55555555};

    UnholyMonIdentity_ScanInit(&scan, NULL);
    UnholyMonIdentity_ScanObserve(&scan, &candidate);
    EXPECT_EQ(UnholyMonIdentity_ScanStatus(&scan), UNHOLY_MON_ID_INVALID);
    EXPECT_EQ(UnholyMonIdentity_KeyEquals(&candidate, NULL), FALSE);
    EXPECT_EQ(UnholyMonIdentity_ReadMon(NULL, &candidate), FALSE);
}
