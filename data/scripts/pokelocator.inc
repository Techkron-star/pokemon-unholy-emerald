@ Idempotent hand-over of the Pokelocator key item. Safe to call more than once.
@ DISENO.md section 7 says the father gives it in the town after the guided Ralts
@ capture; that story scene is not scripted yet, so nothing in the story calls this
@ script. Call it from that scene once it exists:  call EventScript_GivePokelocator
EventScript_GivePokelocator::
	checkitem ITEM_POKELOCATOR, 1
	goto_if_eq VAR_RESULT, TRUE, EventScript_GivePokelocator_Done
	giveitem ITEM_POKELOCATOR
EventScript_GivePokelocator_Done::
	return

@ Search enters the standard scripted wild-battle path, with normal catching.
EventScript_PokelocatorSearchBattle::
	lockall
	dowildbattle
	releaseall
	end

EventScript_PokelocatorSearchUnavailable::
	msgbox PokelocatorText_SearchUnavailable, MSGBOX_SIGN
	releaseall
	end

PokelocatorText_SearchUnavailable:
	.string "This Pokémon can't be found here\n"
	.string "right now. Try searching again.$"
