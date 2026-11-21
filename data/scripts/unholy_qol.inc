@ L opens the persistent overworld QoL menu. B exits; the final row audits
@ every route/zone in small batches, then repairs the current map safely.
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
	callnative UnholyWorldUpdate_BeginAudit
	fadescreen FADE_FROM_BLACK
EventScript_UnholyQolMenu_UpdateWorldNextChunk:
	callnative UnholyWorldUpdate_ProcessChunk
	compare VAR_RESULT, TRUE
	goto_if_eq EventScript_UnholyQolMenu_UpdateWorldFinished
	delay 1
	goto EventScript_UnholyQolMenu_UpdateWorldNextChunk
EventScript_UnholyQolMenu_UpdateWorldFinished:
	callnative UnholyWorldUpdate_RefreshCurrentMap
	callnative UnholyWorldUpdate_FinishAudit
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto EventScript_UnholyQolMenu_Exit
EventScript_UnholyQolMenu_Exit:
	releaseall
	end
