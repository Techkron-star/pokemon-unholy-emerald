@ Hand-over of the HFC Watch tiers. Each script gives its tier and removes the lower ones,
@ so the bag keeps a single watch; running one twice changes nothing.
@ The player's mother gives the first tier. Story calls for Super and HF are TBD.
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
	removeitem ITEM_POKE_WATCH, 1
EventScript_GiveSuperPokeWatch_Done::
	return

EventScript_GiveHFPokeWatch::
	checkitem ITEM_HF_POKE_WATCH, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GiveHFPokeWatch_Done
	giveitem ITEM_HF_POKE_WATCH
	goto_if_eq VAR_0x8007, FALSE, EventScript_GiveHFPokeWatch_Done
	removeitem ITEM_POKE_WATCH, 1
	removeitem ITEM_SUPER_POKE_WATCH, 1
EventScript_GiveHFPokeWatch_Done::
	return
