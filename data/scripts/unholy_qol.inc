@ L opens the persistent overworld QoL menu. B exits; the final row safely
@ reconciles the current map without resetting native item-ball pickup flags.
EventScript_UnholyQolMenu::
	lockall
EventScript_UnholyQolMenu_Loop:
	callnative UnholyQolDrawMenu
	waitstate
	compare VAR_RESULT, MULTI_B_PRESSED
	goto_if_eq EventScript_UnholyQolMenu_Exit
	compare VAR_RESULT, 7
	goto_if_eq EventScript_UnholyQolMenu_UpdateWorld
	callnative UnholyQolToggleSelection
	goto EventScript_UnholyQolMenu_Loop
EventScript_UnholyQolMenu_UpdateWorld:
	fadescreen FADE_TO_BLACK
	callnative UnholyWorldUpdate_RefreshCurrentMap
	fadescreen FADE_FROM_BLACK
	msgbox UnholyQol_Text_WorldUpdated, MSGBOX_DEFAULT
	goto EventScript_UnholyQolMenu_Exit
EventScript_UnholyQolMenu_Exit:
	releaseall
	end

UnholyQol_Text_WorldUpdated:
	.string "Mundo actualizado.\nTus objetos recogidos se conservan.$"
