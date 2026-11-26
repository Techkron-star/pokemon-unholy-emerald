BirthIsland_Exterior_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, BirthIsland_Exterior_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, BirthIsland_Exterior_OnResume
	.byte 0

BirthIsland_Exterior_OnTransition:
	call BirthIsland_Exterior_EventScript_GenesisFaunaReset
	setflag FLAG_MAP_SCRIPT_CHECKED_DEOXYS
	setflag FLAG_HIDE_DEOXYS
	setflag FLAG_HIDE_BIRTH_ISLAND_DEOXYS_TRIANGLE
	goto_if_set FLAG_BATTLED_DEOXYS, BirthIsland_Exterior_EventScript_End
	goto_if_set FLAG_DEOXYS_ROCK_COMPLETE, BirthIsland_Exterior_EventScript_ShowDeoxys
	clearflag FLAG_HIDE_BIRTH_ISLAND_DEOXYS_TRIANGLE
	end

BirthIsland_Exterior_EventScript_ShowDeoxys::
	clearflag FLAG_HIDE_DEOXYS
	end

BirthIsland_Exterior_EventScript_End::
	end

BirthIsland_Exterior_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, BirthIsland_Exterior_EventScript_TryRemoveDeoxys
	end

BirthIsland_Exterior_EventScript_TryRemoveDeoxys::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject LOCALID_BIRTH_ISLAND_DEOXYS
	return

BirthIsland_Exterior_EventScript_Triangle::
	lockall
	goto_if_set FLAG_BATTLED_DEOXYS, BirthIsland_Exterior_EventScript_Release
	playse SE_PIN
	msgbox BirthIsland_Exterior_Text_GenesisSignal, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_DEOXYS_ROCK_COMPLETE
	setflag FLAG_HIDE_BIRTH_ISLAND_DEOXYS_TRIANGLE
	removeobject LOCALID_BIRTH_ISLAND_EXTERIOR_ROCK
	clearflag FLAG_HIDE_DEOXYS
	addobject LOCALID_BIRTH_ISLAND_DEOXYS
	releaseall
	end

BirthIsland_Exterior_EventScript_Deoxys::
	lockall
	goto_if_set FLAG_BATTLED_DEOXYS, BirthIsland_Exterior_EventScript_Release
	playbgm MUS_RG_ENCOUNTER_DEOXYS, FALSE
	waitse
	playmoncry SPECIES_DEOXYS_NORMAL, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	seteventmon SPECIES_DEOXYS_NORMAL, 60
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, BirthIsland_Exterior_EventScript_Caught
	setflag FLAG_HIDE_DEOXYS
	removeobject LOCALID_BIRTH_ISLAND_DEOXYS
	releaseall
	end

BirthIsland_Exterior_EventScript_Caught::
	setflag FLAG_BATTLED_DEOXYS
	clearflag FLAG_DEFEATED_DEOXYS
	setflag FLAG_HIDE_DEOXYS
	setflag FLAG_HIDE_BIRTH_ISLAND_DEOXYS_TRIANGLE
	releaseall
	end

BirthIsland_Exterior_EventScript_Release::
	releaseall
	end

BirthIsland_Exterior_Text_GenesisSignal:
	.string "The stone pulses with a strange\n"
	.string "light. Something has appeared!$"

@ Genesis island fauna: each specimen can be encountered once per map visit.
BirthIsland_Exterior_EventScript_GenesisFaunaReset::
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	call_if_unset FLAG_BATTLED_DEOXYS, BirthIsland_Exterior_EventScript_GenesisFaunaHide3
	return

BirthIsland_Exterior_EventScript_GenesisFaunaELGYEM::
	lockall
	waitse
	playmoncry SPECIES_ELGYEM, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_ELGYEM, 36
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, BirthIsland_Exterior_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_1
	removeobject VAR_LAST_TALKED
	releaseall
	end

BirthIsland_Exterior_EventScript_GenesisFaunaBELDUM::
	lockall
	waitse
	playmoncry SPECIES_BELDUM, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_BELDUM, 36
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, BirthIsland_Exterior_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_2
	removeobject VAR_LAST_TALKED
	releaseall
	end

BirthIsland_Exterior_EventScript_GenesisFaunaHide3::
	setflag FLAG_TEMP_3
	return

BirthIsland_Exterior_EventScript_GenesisFaunaBEHEEYEM::
	lockall
	waitse
	playmoncry SPECIES_BEHEEYEM, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_BEHEEYEM, 48
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, BirthIsland_Exterior_EventScript_GenesisFaunaBeheeyemDiscovered
	setflag FLAG_TEMP_3
	removeobject VAR_LAST_TALKED
	releaseall
	end

BirthIsland_Exterior_EventScript_GenesisFaunaDiscovered::
	setflag FLAG_UNHOLY_GENESIS_BIRTH_FAUNA_DISCOVERED
	return

BirthIsland_Exterior_EventScript_GenesisFaunaBeheeyemDiscovered::
	setflag FLAG_UNHOLY_GENESIS_BIRTH_FAUNA_DISCOVERED
	setflag FLAG_UNHOLY_GENESIS_BEHEEYEM_DISCOVERED
	return
