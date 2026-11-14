#include "global.h"
#include "unholy_evolution.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "region_map.h"
#include "string_util.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/rtc.h"
#include "constants/regions.h"
#include "constants/weather.h"

static const struct Evolution *Entry(enum Species species, u16 index)
{
    const struct Evolution *rows = GetSpeciesEvolutions(SanitizeSpeciesId(species));
    if (rows == NULL) return NULL;
    for (u16 i = 0; i <= index; i++)
        if (rows[i].method == EVOLUTIONS_END) return NULL;
    return &rows[index];
}

static bool32 Visible(const struct Evolution *row)
{
    return row->method != EVO_NONE && row->method != EVO_SPLIT_FROM_EVO
        && SanitizeSpeciesId(row->targetSpecies) != SPECIES_NONE;
}

static bool32 FirstTarget(const struct Evolution *rows, u16 index)
{
    if (!Visible(&rows[index])) return FALSE;
    for (u16 i = 0; i < index; i++)
        if (Visible(&rows[i]) && rows[i].targetSpecies == rows[index].targetSpecies) return FALSE;
    return TRUE;
}

u16 UnholyEvolution_Count(enum Species species)
{
    const struct Evolution *rows = GetSpeciesEvolutions(SanitizeSpeciesId(species));
    u16 count = 0;
    if (rows != NULL)
        for (u16 i = 0; rows[i].method != EVOLUTIONS_END; i++) count += FirstTarget(rows, i);
    return count;
}

static void Block(struct UnholyEvolutionOption *out, u8 status, u16 condition)
{
    if (out->status == UNHOLY_EVO_READY)
    {
        out->status = status;
        out->missingCondition = condition;
    }
}

static void Cost(struct UnholyEvolutionOption *out, u16 item, u16 count, bool8 held)
{
    if (item == ITEM_NONE || item >= ITEMS_COUNT || count == 0)
    { Block(out, UNHOLY_EVO_INVALID, 0); return; }
    for (u8 i = 0; i < out->costCount; i++)
        if (out->costs[i].item == item && out->costs[i].held == held)
        {
            if ((u32)out->costs[i].count + count > 0xFFFF) Block(out, UNHOLY_EVO_INVALID, 0);
            else out->costs[i].count += count;
            return;
        }
    if (out->costCount == UNHOLY_EVO_MAX_COSTS)
    { Block(out, UNHOLY_EVO_INVALID, 0); return; }
    struct UnholyEvolutionCost *cost = &out->costs[out->costCount++];
    cost->item = item;
    cost->count = count;
    cost->held = held;
}

static bool32 ValidMon(struct Pokemon *mon)
{
    return mon != NULL && GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
        && !GetMonData(mon, MON_DATA_IS_EGG) && !GetMonData(mon, MON_DATA_SANITY_IS_BAD_EGG);
}

static void Evaluate(struct Pokemon *mon, enum Species species, u16 index, u8 partySlot,
                     struct UnholyEvolutionOption *out)
{
    memset(out, 0, sizeof(*out));
    out->source = species;
    out->entry = index;
    const struct Evolution *row = Entry(species, index);
    if (row == NULL || !Visible(row)) { out->status = UNHOLY_EVO_INVALID; return; }
    out->target = row->targetSpecies;
    out->minimumLevel = row->method == EVO_LEVEL || row->method == EVO_LEVEL_BATTLE_ONLY ? row->param : 0;
    if (!ValidMon(mon)) { out->status = UNHOLY_EVO_INVALID; return; }
    if (GetMonData(mon, MON_DATA_SPECIES) != species) { out->status = UNHOLY_EVO_PREVIEW; return; }
    u16 held = GetMonData(mon, MON_DATA_HELD_ITEM);
    if (GetItemHoldEffect(held) == HOLD_EFFECT_PREVENT_EVOLVE
     && (P_KADABRA_EVERSTONE < GEN_4 || species != SPECIES_KADABRA))
        Block(out, UNHOLY_EVO_EVERSTONE, 0);
    if (GetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR) && GetGMaxTargetSpecies(species) != species
     && GetGMaxTargetSpecies(out->target) == out->target)
        Block(out, UNHOLY_EVO_SPECIAL, 0);

    if (row->method == EVO_LEVEL || row->method == EVO_LEVEL_BATTLE_ONLY)
    {
        if (GetMonData(mon, MON_DATA_LEVEL) < row->param) Block(out, UNHOLY_EVO_LEVEL, 0);
    }
    else if (row->method == EVO_ITEM) Cost(out, row->param, 1, FALSE);
    else if (row->method != EVO_TRADE && row->method != EVO_BATTLE_END)
        Block(out, UNHOLY_EVO_SPECIAL, 0);

    bool32 specificTradeItem = FALSE;
    for (u16 i = 0; row->params != NULL && row->params[i].condition != CONDITIONS_END; i++)
    {
        const struct EvolutionParam *p = &row->params[i];
        if (p->condition == IF_HOLD_ITEM)
        {
            // Prefer the bag, leaving the current held-item slot alone whenever possible.
            bool8 useHeld = !CheckBagHasItem(p->arg1, 1) && held == p->arg1;
            Cost(out, p->arg1, 1, useHeld);
            specificTradeItem = TRUE;
        }
        else if (p->condition == IF_BAG_ITEM_COUNT) Cost(out, p->arg1, p->arg2, FALSE);
        else if (row->method == EVO_TRADE && p->condition == IF_TRADE_PARTNER_SPECIES)
        {
            // Linking Cord substitutes for the exchange, including its partner check.
        }
        else
        {
            const struct EvolutionParam one[] = {*p, {.condition = CONDITIONS_END}};
            if (!DoesMonMeetAdditionalConditions(mon, one, NULL, partySlot, NULL, CHECK_EVO))
                Block(out, UNHOLY_EVO_CONDITION, i);
        }
    }
    if (row->method == EVO_TRADE && !specificTradeItem) Cost(out, ITEM_LINKING_CORD, 1, FALSE);
    for (u8 i = 0; i < out->costCount; i++)
        if ((out->costs[i].held && (held != out->costs[i].item || out->costs[i].count != 1))
         || (!out->costs[i].held && !CheckBagHasItem(out->costs[i].item, out->costs[i].count)))
            Block(out, UNHOLY_EVO_ITEM, i);
}

bool32 UnholyEvolution_GetOption(struct Pokemon *mon, enum Species viewSpecies, u16 ordinal,
                                u8 partySlot, struct UnholyEvolutionOption *out)
{
    const struct Evolution *rows = GetSpeciesEvolutions(SanitizeSpeciesId(viewSpecies));
    if (rows == NULL) return FALSE;
    u16 n = 0;
    for (u16 i = 0; rows[i].method != EVOLUTIONS_END; i++)
    {
        if (!FirstTarget(rows, i) || n++ != ordinal) continue;
        Evaluate(mon, viewSpecies, i, partySlot, out);
        for (u16 j = i + 1; rows[j].method != EVOLUTIONS_END; j++)
            if (Visible(&rows[j]) && rows[j].targetSpecies == out->target)
            {
                struct UnholyEvolutionOption alternative;
                Evaluate(mon, viewSpecies, j, partySlot, &alternative);
                if ((out->status != UNHOLY_EVO_READY && alternative.status == UNHOLY_EVO_READY)
                 || (out->status == alternative.status && alternative.costCount < out->costCount))
                    *out = alternative;
            }
        return TRUE;
    }
    return FALSE;
}

u8 UnholyEvolution_Prepare(u8 partySlot, u16 entry, struct UnholyEvolutionPlan *plan)
{
    memset(plan, 0, sizeof(*plan));
    plan->partySlot = partySlot;
    if (partySlot >= PARTY_SIZE || partySlot >= gPartiesCount[B_TRAINER_PLAYER] || gMain.inBattle) return plan->option.status = UNHOLY_EVO_INVALID;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][partySlot];
    Evaluate(mon, GetMonData(mon, MON_DATA_SPECIES), entry, partySlot, &plan->option);
    plan->before = *mon;
    return plan->option.status;
}

u8 UnholyEvolution_Consume(struct UnholyEvolutionPlan *plan)
{
    if (plan->consumed || plan->partySlot >= PARTY_SIZE || gMain.inBattle) return UNHOLY_EVO_CHANGED;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][plan->partySlot];
    if (memcmp(mon, &plan->before, sizeof(*mon))) return UNHOLY_EVO_CHANGED;
    struct UnholyEvolutionPlan current;
    u8 status = UnholyEvolution_Prepare(plan->partySlot, plan->option.entry, &current);
    if (status != UNHOLY_EVO_READY) return status;
    if (memcmp(&plan->option, &current.option, sizeof(current.option))) return UNHOLY_EVO_CHANGED;
    // All costs are aggregated and checked before the first resource is consumed.
    for (u8 i = 0; i < current.option.costCount; i++)
    {
        struct UnholyEvolutionCost *cost = &current.option.costs[i];
        if (cost->held)
        {
            u16 none = ITEM_NONE;
            SetMonData(mon, MON_DATA_HELD_ITEM, &none);
        }
        else RemoveBagItem(cost->item, cost->count);
    }
    plan->consumed = TRUE;
    return UNHOLY_EVO_READY;
}

static u8 *Num(u8 *text, u32 value)
{
    return ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEFT_ALIGN, 5);
}

static const u8 *RegionName(u16 region)
{
    static const u8 *const names[] = {
        [REGION_NONE] = COMPOUND_STRING("NONE"), [REGION_KANTO] = COMPOUND_STRING("KANTO"),
        [REGION_JOHTO] = COMPOUND_STRING("JOHTO"), [REGION_HOENN] = COMPOUND_STRING("HOENN"),
        [REGION_SINNOH] = COMPOUND_STRING("SINNOH"), [REGION_UNOVA] = COMPOUND_STRING("UNOVA"),
        [REGION_KALOS] = COMPOUND_STRING("KALOS"), [REGION_ALOLA] = COMPOUND_STRING("ALOLA"),
        [REGION_GALAR] = COMPOUND_STRING("GALAR"), [REGION_HISUI] = COMPOUND_STRING("HISUI"),
        [REGION_PALDEA] = COMPOUND_STRING("PALDEA"),
    };
    return region < ARRAY_COUNT(names) ? names[region] : COMPOUND_STRING("SPECIAL REGION");
}

u16 UnholyEvolution_RequirementCount(const struct UnholyEvolutionOption *option)
{
    const struct Evolution *row = Entry(option->source, option->entry);
    u16 count = 1;
    if (row != NULL && row->params != NULL)
        while (row->params[count - 1].condition != CONDITIONS_END) count++;
    return count;
}

void UnholyEvolution_RequirementText(const struct UnholyEvolutionOption *option, u16 line, u8 *text)
{
    const struct Evolution *row = Entry(option->source, option->entry);
    StringCopy(text, COMPOUND_STRING("--"));
    if (row == NULL || line >= UnholyEvolution_RequirementCount(option)) return;
    u8 *end;
    if (line == 0)
    {
        if (row->method == EVO_LEVEL || row->method == EVO_LEVEL_BATTLE_ONLY)
        { end = StringCopy(text, COMPOUND_STRING("LEVEL ")); Num(end, max(1, row->param)); }
        else if (row->method == EVO_ITEM) StringCopy(text, GetItemName(row->param));
        else if (row->method == EVO_TRADE)
        {
            bool32 item = FALSE;
            for (u16 i = 0; row->params != NULL && row->params[i].condition != CONDITIONS_END; i++)
                item |= row->params[i].condition == IF_HOLD_ITEM;
            StringCopy(text, item ? COMPOUND_STRING("SPECIFIC ITEM - NO TRADE") : GetItemName(ITEM_LINKING_CORD));
        }
        else StringCopy(text, row->method == EVO_BATTLE_END ? COMPOUND_STRING("BATTLE REQUIREMENTS")
                             : row->method == EVO_SPIN ? COMPOUND_STRING("SPIN IN THE OVERWORLD") : COMPOUND_STRING("USE THE STORY TRIGGER"));
        return;
    }
    const struct EvolutionParam *p = &row->params[line - 1];
    switch (p->condition)
    {
    case IF_HOLD_ITEM: StringCopy(text, GetItemName(p->arg1)); StringAppend(text, COMPOUND_STRING(" (BAG / HELD)")); break;
    case IF_BAG_ITEM_COUNT: end = StringCopy(text, GetItemName(p->arg1)); end = StringAppend(end, COMPOUND_STRING(" x")); Num(end, p->arg2); break;
    case IF_MIN_FRIENDSHIP: end = StringCopy(text, COMPOUND_STRING("FRIENDSHIP ")); Num(end, p->arg1); break;
    case IF_GENDER: StringCopy(text, p->arg1 == MON_FEMALE ? COMPOUND_STRING("FEMALE") : COMPOUND_STRING("MALE")); break;
    case IF_TIME: case IF_NOT_TIME:
        end = StringCopy(text, p->condition == IF_NOT_TIME ? COMPOUND_STRING("NOT ") : COMPOUND_STRING("TIME: "));
        StringAppend(end, p->arg1 == TIME_NIGHT ? COMPOUND_STRING("NIGHT") : p->arg1 == TIME_DAY ? COMPOUND_STRING("DAY") : COMPOUND_STRING("DUSK / MORNING")); break;
    case IF_ATK_GT_DEF: StringCopy(text, COMPOUND_STRING("ATTACK > DEFENSE")); break;
    case IF_ATK_EQ_DEF: StringCopy(text, COMPOUND_STRING("ATTACK = DEFENSE")); break;
    case IF_ATK_LT_DEF: StringCopy(text, COMPOUND_STRING("ATTACK < DEFENSE")); break;
    case IF_KNOWS_MOVE: end = StringCopy(text, COMPOUND_STRING("KNOWS ")); StringAppend(end, GetMoveName(p->arg1)); break;
    case IF_SPECIES_IN_PARTY: end = StringCopy(text, COMPOUND_STRING("PARTY: ")); StringAppend(end, GetSpeciesName(p->arg1)); break;
    case IF_TYPE_IN_PARTY: end = StringCopy(text, COMPOUND_STRING("PARTY TYPE: ")); StringAppend(end, gTypesInfo[p->arg1].name); break;
    case IF_KNOWS_MOVE_TYPE: end = StringCopy(text, COMPOUND_STRING("KNOWS TYPE: ")); StringAppend(end, gTypesInfo[p->arg1].name); break;
    case IF_NATURE: end = StringCopy(text, COMPOUND_STRING("NATURE: ")); StringAppend(end, gNaturesInfo[p->arg1].name); break;
    case IF_AMPED_NATURE: StringCopy(text, COMPOUND_STRING("AMPED NATURE")); break;
    case IF_LOW_KEY_NATURE: StringCopy(text, COMPOUND_STRING("LOW KEY NATURE")); break;
    case IF_MIN_BEAUTY: case IF_MIN_COOLNESS: case IF_MIN_SMARTNESS: case IF_MIN_TOUGHNESS: case IF_MIN_CUTENESS:
        end = StringCopy(text, p->condition == IF_MIN_BEAUTY ? COMPOUND_STRING("BEAUTY ") : COMPOUND_STRING("CONTEST STAT ")); Num(end, p->arg1); break;
    case IF_USED_MOVE_X_TIMES: end = StringCopy(text, GetMoveName(p->arg1)); end = StringAppend(end, COMPOUND_STRING(" x")); Num(end, p->arg2); break;
    case IF_MIN_OVERWORLD_STEPS: end = StringCopy(text, COMPOUND_STRING("LEAD MON STEPS: ")); Num(end, p->arg1); break;
    case IF_CRITICAL_HITS_GE: end = StringCopy(text, COMPOUND_STRING("CRITICAL HITS: ")); Num(end, p->arg1); break;
    case IF_RECOIL_DAMAGE_GE: case IF_CURRENT_DAMAGE_GE: end = StringCopy(text, COMPOUND_STRING("DAMAGE REQUIRED: ")); Num(end, p->arg1); break;
    case IF_DEFEAT_X_WITH_ITEMS: end = StringCopy(text, COMPOUND_STRING("DEFEAT ")); end = StringAppend(end, GetSpeciesName(p->arg1)); end = StringAppend(end, COMPOUND_STRING(" x")); Num(end, p->arg3); break;
    case IF_TRADE_PARTNER_SPECIES: StringCopy(text, COMPOUND_STRING("REPLACED BY LINKING CORD")); break;
    case IF_IN_MAP:
        end = StringCopy(text, COMPOUND_STRING("AT: "));
        GetMapName(end, Overworld_GetMapHeaderByGroupAndId(p->arg1 >> 8, p->arg1 & 0xFF)->regionMapSectionId, 0);
        break;
    case IF_IN_MAPSEC: end = StringCopy(text, COMPOUND_STRING("AT: ")); GetMapName(end, p->arg1, 0); break;
    case IF_REGION: case IF_NOT_REGION:
        end = StringCopy(text, p->condition == IF_REGION ? COMPOUND_STRING("REGION: ") : COMPOUND_STRING("OUTSIDE "));
        StringAppend(end, RegionName(p->arg1)); break;
    case IF_WEATHER:
        StringCopy(text, p->arg1 == WEATHER_RAIN ? COMPOUND_STRING("LOCAL WEATHER: RAIN")
                       : p->arg1 == WEATHER_FOG ? COMPOUND_STRING("LOCAL WEATHER: FOG") : COMPOUND_STRING("REQUIRED LOCAL WEATHER")); break;
    default: StringCopy(text, COMPOUND_STRING("THIS POKéMON'S NATURAL BRANCH")); break;
    }
}

void UnholyEvolution_StatusText(const struct UnholyEvolutionOption *option, u8 *text)
{
    u8 *end;
    switch (option->status)
    {
    case UNHOLY_EVO_READY: StringCopy(text, COMPOUND_STRING("READY TO EVOLVE")); break;
    case UNHOLY_EVO_LEVEL: end = StringCopy(text, COMPOUND_STRING("NEEDS LEVEL ")); Num(end, option->minimumLevel); break;
    case UNHOLY_EVO_EVERSTONE: StringCopy(text, COMPOUND_STRING("REMOVE THE EVERSTONE")); break;
    case UNHOLY_EVO_ITEM:
        if (option->missingCondition < option->costCount)
        { end = StringCopy(text, COMPOUND_STRING("NEED ")); StringAppend(end, GetItemName(option->costs[option->missingCondition].item)); }
        else StringCopy(text, COMPOUND_STRING("MISSING EVOLUTION ITEM"));
        break;
    case UNHOLY_EVO_CONDITION: UnholyEvolution_RequirementText(option, option->missingCondition + 1, text); break;
    case UNHOLY_EVO_PREVIEW: StringCopy(text, COMPOUND_STRING("PREVIEW - NOT CURRENT SPECIES")); break;
    case UNHOLY_EVO_SPECIAL: StringCopy(text, COMPOUND_STRING("USE ITS SPECIAL EVOLUTION METHOD")); break;
    default: StringCopy(text, COMPOUND_STRING("REVIEW THE POKéMON AGAIN")); break;
    }
}
