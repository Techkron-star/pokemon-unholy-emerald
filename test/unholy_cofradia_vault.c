#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "money.h"
#include "item.h"
#include "constants/items.h"
#include "constants/species.h"
#include "unholy_save.h"
#include "unholy_cofradia.h"
#include "test/test.h"

TEST("Cofradia vault preserves complete party Pokemon and keeps two living allies")
{
    struct SaveBlock4 old4 = gSaveBlock3Ptr->unholySave4;
    struct SaveBlock5 old5 = gPokemonStoragePtr->unholySave5;
    struct Pokemon oldParty[PARTY_SIZE];
    struct Pokemon *party = gParties[B_TRAINER_PLAYER];
    memcpy(oldParty, party, sizeof(oldParty));
    u32 oldCash = GetMoney(&gSaveBlock1Ptr->money);
    struct CofradiaVaultView view;
    UnholySave4_Reset();
    UnholySave5_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 500000);
    ZeroPartyMons(party);
    CreateRandomMon(&party[0], SPECIES_CHARIZARD, 50);
    CreateRandomMon(&party[1], SPECIES_PIKACHU, 50);
    CreateRandomMon(&party[2], SPECIES_BLASTOISE, 50);
    CalculatePlayerPartyCount();
    u16 badges = POKEMON_BADGE_OMEGA | POKEMON_BADGE_ALPHA;
    SetMonData(&party[0], MON_DATA_SPECIAL_BADGES, &badges);
    struct Pokemon original = party[0];
    EXPECT(!CofradiaVaultHoldMon(COFRADIA_VAULT_PARTY, 0));
    EXPECT_EQ(CofradiaBeginCredit(0, 500000, 500000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(FALSE));
    party[0].box.isFavorite = TRUE;
    EXPECT(!CofradiaVaultHoldMon(COFRADIA_VAULT_PARTY, 0));
    party[0].box.isFavorite = FALSE;
    EXPECT(CofradiaVaultHoldMon(COFRADIA_VAULT_PARTY, 0));
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_PIKACHU);
    EXPECT(!CofradiaVaultHoldMon(COFRADIA_VAULT_PARTY, 0));
    EXPECT(CofradiaVaultGetView(&view));
    EXPECT_EQ(view.pokemon, 1);
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u32 value;
    EXPECT(CofradiaVaultDescribeMon(0, nickname, &value));
    EXPECT_EQ(value, view.totalValue);
    EXPECT(!CofradiaVaultDescribeMon(COFRADIA_VAULT_MON_CAPACITY, nickname, &value));
    EXPECT(!CofradiaVaultReturnMon(0));
    EXPECT_EQ(CofradiaRepay(500000), COFRADIA_OK);
    EXPECT_EQ(CofradiaBeginCredit(0, 5000, 5000), COFRADIA_DEBT_PENDING);
    EXPECT(CofradiaVaultReturnMon(0));
    EXPECT_EQ(memcmp(&party[2], &original, sizeof(original)), 0);
    EXPECT(!CofradiaVaultReturnMon(0));
    EXPECT(CofradiaVaultGetView(&view));
    EXPECT_EQ(view.pokemon, 0);
    EXPECT_EQ(view.totalValue, 0);
    EXPECT(UnholySave5_Validate(&gPokemonStoragePtr->unholySave5));
    memcpy(party, oldParty, sizeof(oldParty));
    CalculatePlayerPartyCount();
    gSaveBlock3Ptr->unholySave4 = old4;
    gPokemonStoragePtr->unholySave5 = old5;
    SetMoney(&gSaveBlock1Ptr->money, oldCash);
}

TEST("Cofradia vault preserves original PC bytes and returns mixed bag collateral")
{
    struct SaveBlock4 old4 = gSaveBlock3Ptr->unholySave4;
    struct SaveBlock5 old5 = gPokemonStoragePtr->unholySave5;
    struct BoxPokemon *slot = GetBoxedMonPtr(0, 0);
    struct BoxPokemon oldBox = *slot;
    u32 oldCash = GetMoney(&gSaveBlock1Ptr->money);
    struct Pokemon mon;
    struct CofradiaVaultView view;
    UnholySave4_Reset();
    UnholySave5_Reset();
    SetMoney(&gSaveBlock1Ptr->money, 500000);
    CreateRandomMon(&mon, SPECIES_CHARIZARD, 40);
    *slot = mon.box;
    struct BoxPokemon original = *slot;
    u16 before = CountTotalItemQuantityInBag(ITEM_POTION);
    EXPECT(AddBagItem(ITEM_POTION, 2));
    EXPECT_EQ(CofradiaAppraiseItem(ITEM_MACH_BIKE), 0);
    EXPECT(CofradiaAppraiseItem(ITEM_ROTOM_CATALOG) > 0);
    EXPECT(CofradiaAppraiseItem(ITEM_GEAR_VISOR_COSMICO) > 0);
    EXPECT_EQ(CofradiaBeginCredit(0, 500000, 500000), COFRADIA_OK);
    EXPECT(CofradiaArm());
    EXPECT(CofradiaResolve(FALSE));
    EXPECT(CofradiaVaultHoldMon(0, 0));
    EXPECT_EQ(GetBoxMonData(slot, MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT(CofradiaVaultHoldItem(ITEM_POTION, 2));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), before);
    EXPECT(!CofradiaVaultHoldItem(ITEM_MACH_BIKE, 1));
    EXPECT(CofradiaVaultGetView(&view));
    EXPECT_EQ(view.pokemon, 1);
    EXPECT_EQ(view.itemStacks, 1);
    enum Item item;
    u16 quantity;
    u32 value;
    EXPECT(CofradiaVaultDescribeItem(0, &item, &quantity, &value));
    EXPECT_EQ(item, ITEM_POTION);
    EXPECT_EQ(quantity, 2);
    EXPECT_EQ(value, CofradiaAppraiseItem(ITEM_POTION) * 2);
    EXPECT(!CofradiaVaultDescribeItem(COFRADIA_VAULT_ITEM_CAPACITY, &item, &quantity, &value));
    EXPECT(!CofradiaVaultReturnItem(0));
    EXPECT_EQ(CofradiaRepay(500000), COFRADIA_OK);
    EXPECT(CofradiaVaultReturnMon(0));
    EXPECT_EQ(memcmp(slot, &original, sizeof(original)), 0);
    EXPECT(CofradiaVaultReturnItem(0));
    EXPECT(!CofradiaVaultReturnItem(0));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POTION), before + 2);
    EXPECT(RemoveBagItem(ITEM_POTION, 2));
    EXPECT(CofradiaVaultGetView(&view));
    EXPECT_EQ(view.totalValue, 0);
    EXPECT(UnholySave5_Validate(&gPokemonStoragePtr->unholySave5));
    *slot = oldBox;
    gSaveBlock3Ptr->unholySave4 = old4;
    gPokemonStoragePtr->unholySave5 = old5;
    SetMoney(&gSaveBlock1Ptr->money, oldCash);
}
