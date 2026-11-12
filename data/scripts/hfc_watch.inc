@ Hand-over of the HFC Watch tiers. Each script gives its tier and removes the lower ones,
@ so the bag keeps a single watch; running one twice changes nothing.
@ Mom gives the first tier, Norman the Super, and Wattson the HF.
EventScript_GivePokeWatch::
	checkitem ITEM_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GivePokeWatch_Done
	checkitem ITEM_SUPER_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GivePokeWatch_Done
	checkitem ITEM_HF_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GivePokeWatch_Done
	giveitem ITEM_POKE_WATCH
EventScript_GivePokeWatch_Done::
	return

EventScript_GiveSuperPokeWatch::
	checkitem ITEM_SUPER_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GiveSuperPokeWatch_Done
	checkitem ITEM_HF_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GiveSuperPokeWatch_Done
	giveitem ITEM_SUPER_POKE_WATCH
	goto_if_eq VAR_0x8007, FALSE, EventScript_GiveSuperPokeWatch_Done
	setvar VAR_0x8004, ITEM_SUPER_POKE_WATCH
	callnative ScriptUpgradeRegisteredHfcWatch
	removeitem ITEM_POKE_WATCH, 1
EventScript_GiveSuperPokeWatch_Done::
	return

EventScript_GiveHFPokeWatch::
	checkitem ITEM_HF_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GiveHFPokeWatch_Done
	giveitem ITEM_HF_POKE_WATCH
	goto_if_eq VAR_0x8007, FALSE, EventScript_GiveHFPokeWatch_Done
	setvar VAR_0x8004, ITEM_HF_POKE_WATCH
	callnative ScriptUpgradeRegisteredHfcWatch
	removeitem ITEM_POKE_WATCH, 1
	removeitem ITEM_SUPER_POKE_WATCH, 1
EventScript_GiveHFPokeWatch_Done::
	return
