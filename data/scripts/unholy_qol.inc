@ L opens the persistent overworld QoL menu. Selecting a row toggles it
@ and redraws the menu; B/Close returns control to the player.
EventScript_UnholyQolMenu::
	lockall
EventScript_UnholyQolMenu_Loop:
	callnative UnholyQolDrawMenu
	waitstate
	compare VAR_RESULT, MULTI_B_PRESSED
	goto_if_eq EventScript_UnholyQolMenu_Exit
	compare VAR_RESULT, 7
	goto_if_eq EventScript_UnholyQolMenu_Exit
	callnative UnholyQolToggleSelection
	goto EventScript_UnholyQolMenu_Loop
EventScript_UnholyQolMenu_Exit:
	releaseall
	end
