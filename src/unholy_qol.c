#include "global.h"
#include "event_data.h"
#include "field_move.h"
#include "menu.h"
#include "script_menu.h"
#include "unholy_qol.h"
#include "hfc_watch.h"
#include "item.h"
#include "pokemon.h"
#include "config/item.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/vars.h"

#define QOL_FLAGS_INITIALIZED (1 << 15)
#define QOL_DEFAULT_FLAGS (1 << UQOL_AUTO_RUN)

// Names are kept short enough for the GBA overworld menu.
static const u8 sText_QolRepelOn[] = _("Repel auto: ON");
static const u8 sText_QolRepelOff[] = _("Repel auto: OFF");
static const u8 sText_QolRunOn[] = _("Autorun: ON");
static const u8 sText_QolRunOff[] = _("Autorun: OFF");
static const u8 sText_QolCutOn[] = _("Auto Cut: ON");
static const u8 sText_QolCutOff[] = _("Auto Cut: OFF");
static const u8 sText_QolStrengthOn[] = _("Auto Strength: ON");
static const u8 sText_QolStrengthOff[] = _("Auto Strength: OFF");
static const u8 sText_QolSmashOn[] = _("Auto Smash: ON");
static const u8 sText_QolSmashOff[] = _("Auto Smash: OFF");
static const u8 sText_QolSurfOn[] = _("Auto Surf: ON");
static const u8 sText_QolSurfOff[] = _("Auto Surf: OFF");
static const u8 sText_QolTextOn[] = _("Auto Text: ON");
static const u8 sText_QolTextOff[] = _("Auto Text: OFF");
static const u8 sText_QolWorldUpdate[] = _("Actualizar mundo");
static const u8 sText_QolInfinityOn[] = _("Infinity Repel: ON");
static const u8 sText_QolInfinityOff[] = _("Infinity Repel: OFF");
static const u8 sText_QolFlashOn[] = _("Flasher: ON");
static const u8 sText_QolFlashOff[] = _("Flasher: OFF");
static const u8 sText_QolHealOn[] = _("Auto Heal: ON");
static const u8 sText_QolHealOff[] = _("Auto Heal: OFF");

static const u8 *const sQolLabels[UQOL_OPTION_COUNT][2] =
{
    [UQOL_AUTO_REPEL]      = {sText_QolRepelOff, sText_QolRepelOn},
    [UQOL_AUTO_RUN]        = {sText_QolRunOff, sText_QolRunOn},
    [UQOL_AUTO_CUT]        = {sText_QolCutOff, sText_QolCutOn},
    [UQOL_AUTO_STRENGTH]   = {sText_QolStrengthOff, sText_QolStrengthOn},
    [UQOL_AUTO_ROCK_SMASH] = {sText_QolSmashOff, sText_QolSmashOn},
    [UQOL_AUTO_SURF]       = {sText_QolSurfOff, sText_QolSurfOn},
    [UQOL_AUTO_DIALOGUE]   = {sText_QolTextOff, sText_QolTextOn},
    [UQOL_INFINITE_REPEL] = {sText_QolInfinityOff, sText_QolInfinityOn},
    [UQOL_AUTO_FLASH]     = {sText_QolFlashOff, sText_QolFlashOn},
    [UQOL_AUTO_HEAL]      = {sText_QolHealOff, sText_QolHealOn},
};

static EWRAM_DATA u8 sQolMenuCursor = 0;

static u16 GetQolFlags(void)
{
    u16 flags = VarGet(VAR_UNHOLY_QOL_FLAGS);

    // Preserve the previous fixed-on Autorun behavior for existing saves.
    return (flags & QOL_FLAGS_INITIALIZED) ? flags : QOL_DEFAULT_FLAGS;
}

bool32 UnholyQolIsEnabled(enum UnholyQolOption option)
{
    if (option >= UQOL_OPTION_COUNT)
        return FALSE;
    if (option == UQOL_AUTO_REPEL)
        return VarGet(VAR_REPEL_AUTO_RENEW) == 2;

    // Never grant a tool after a downgrade or before its normal field unlock.
    if (option == UQOL_INFINITE_REPEL && HfcWatch_GetTier() < HFC_WATCH_TIER_HF)
        return FALSE;
    if (option == UQOL_AUTO_FLASH
     && (HfcWatch_GetTier() < HFC_WATCH_TIER_SUPER
      || !FlagGet(IS_FRLG ? FLAG_BADGE01_GET : FLAG_BADGE02_GET)
      || !CheckBagHasItem(ITEM_HM_FLASH, 1)))
        return FALSE;

    if (option == UQOL_AUTO_HEAL && HfcWatch_GetTier() < HFC_WATCH_TIER_POKE)
        return FALSE;

    return (GetQolFlags() & (1 << option)) != 0;
}

// Automatic field moves are learned by the trainer, not tied to one party
// member. Keep progression intact: the relevant HM must be in the Bag and
// its badge/field unlock must be earned. Other manual field moves still need
// a Pokemon that knows them.
bool32 UnholyQolAutoFieldMoveAvailable(enum FieldMove fieldMove)
{
    enum UnholyQolOption option;
    enum Item hm;

    switch (fieldMove)
    {
    case FIELD_MOVE_CUT:        option = UQOL_AUTO_CUT; break;
    case FIELD_MOVE_ROCK_SMASH: option = UQOL_AUTO_ROCK_SMASH; break;
    case FIELD_MOVE_STRENGTH:   option = UQOL_AUTO_STRENGTH; break;
    case FIELD_MOVE_SURF:       option = UQOL_AUTO_SURF; break;
    default:                    return FALSE;
    }

    if (!UnholyQolIsEnabled(option) || !IsFieldMoveUnlocked(fieldMove))
        return FALSE;

    hm = GetTMHMItemIdFromMoveId(FieldMove_GetMoveId(fieldMove));
    return hm != ITEM_NONE && CheckBagHasItem(hm, 1);
}

void UnholyQolToggleOption(enum UnholyQolOption option)
{
    if (option >= UQOL_OPTION_COUNT)
        return;

    if (option == UQOL_AUTO_REPEL)
    {
        // 1 means disabled/previously answered; 2 enables automatic renewal.
        VarSet(VAR_REPEL_AUTO_RENEW, UnholyQolIsEnabled(option) ? 1 : 2);
    }
    else
    {
        VarSet(VAR_UNHOLY_QOL_FLAGS, (GetQolFlags() ^ (1 << option)) | QOL_FLAGS_INITIALIZED);
    }
}

void UnholyQolDrawMenu(struct ScriptContext *ctx)
{
    struct MenuAction menuItems[UQOL_GENERAL_OPTION_COUNT + 1] = {0};
    u32 i;

    for (i = 0; i < UQOL_GENERAL_OPTION_COUNT; i++)
        menuItems[i].text = sQolLabels[i][UnholyQolIsEnabled(i)];
    menuItems[UQOL_GENERAL_OPTION_COUNT].text = sText_QolWorldUpdate;

    DrawMultichoiceMenuInternal(1, 1, 0, FALSE, sQolMenuCursor, menuItems, UQOL_GENERAL_OPTION_COUNT + 1);
}

void UnholyQolToggleSelection(struct ScriptContext *ctx)
{
    u16 choice = gSpecialVar_Result;

    if (choice >= UQOL_GENERAL_OPTION_COUNT)
        return;
    sQolMenuCursor = choice;
    UnholyQolToggleOption(choice);
}

u32 UnholyQolGetScriptSetting(void)
{
    return UnholyQolIsEnabled(gSpecialVar_0x8000);
}

// No free healing and no revives: use ordinary medicine after successful,
// non-facility battles. Prefer the smallest potion that finishes the job;
// otherwise use the strongest stocked one, then re-evaluate the remaining HP.
static enum Item AutoHealChoosePotion(u16 missingHp)
{
    static const struct
    {
        enum Item item;
        u16 healing;
    } sPotions[] =
    {
        {ITEM_POTION, 20},
        {ITEM_SUPER_POTION, I_HEALTH_RECOVERY >= GEN_7 ? 60 : 50},
        {ITEM_HYPER_POTION, I_HEALTH_RECOVERY >= GEN_7 ? 120 : 200},
        {ITEM_MAX_POTION, 0xFFFF},
    };
    enum Item selected = ITEM_NONE;
    u16 selectedHeal = 0;

    for (u8 i = 0; i < ARRAY_COUNT(sPotions); i++)
    {
        u16 healing = sPotions[i].healing;
        if (!CheckBagHasItem(sPotions[i].item, 1))
            continue;
        // Spare Max Potions for larger wounds when cheaper medicine exists.
        if (sPotions[i].item == ITEM_MAX_POTION && missingHp <= 200
         && (CheckBagHasItem(ITEM_POTION, 1)
          || CheckBagHasItem(ITEM_SUPER_POTION, 1)
          || CheckBagHasItem(ITEM_HYPER_POTION, 1)))
            continue;
        if (selected == ITEM_NONE
         || (healing >= missingHp && selectedHeal < missingHp)
         || (healing >= missingHp && selectedHeal >= missingHp && healing < selectedHeal)
         || (healing < missingHp && selectedHeal < missingHp && healing > selectedHeal))
        {
            selected = sPotions[i].item;
            selectedHeal = healing;
        }
    }
    return selected;
}

void UnholyQolAutoHealAfterBattle(void)
{
    if (!UnholyQolIsEnabled(UQOL_AUTO_HEAL))
        return;

    for (u8 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u16 currentHp, maxHp;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        currentHp = GetMonData(mon, MON_DATA_HP);
        maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        if (currentHp == 0 || currentHp >= maxHp)
            continue;

        while (currentHp < maxHp)
        {
            enum Item potion = AutoHealChoosePotion(maxHp - currentHp);
            u16 heal;
            if (potion == ITEM_NONE || !RemoveBagItem(potion, 1))
                break;
            heal = potion == ITEM_MAX_POTION ? maxHp :
                potion == ITEM_HYPER_POTION ? (I_HEALTH_RECOVERY >= GEN_7 ? 120 : 200) :
                potion == ITEM_SUPER_POTION ? (I_HEALTH_RECOVERY >= GEN_7 ? 60 : 50) : 20;
            currentHp = min(maxHp, currentHp + heal);
            SetMonData(mon, MON_DATA_HP, &currentHp);
        }
    }
}
