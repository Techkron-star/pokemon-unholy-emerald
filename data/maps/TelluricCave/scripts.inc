TelluricCave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, TelluricCave_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, TelluricCave_OnResume
	.byte 0

TelluricCave_OnTransition:
	setflag FLAG_GENESIS_TELLURIC_VISITED
	setflag FLAG_GENESIS_ZYGARDE_HIDE
	call_if_unset FLAG_GENESIS_ZYGARDE_CAUGHT, TelluricCave_ShowZygarde
	end

TelluricCave_ShowZygarde::
	clearflag FLAG_GENESIS_ZYGARDE_HIDE
	return

TelluricCave_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, TelluricCave_TryRemoveZygarde
	end

TelluricCave_TryRemoveZygarde::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject LOCALID_TELLURIC_CAVE_ZYGARDE
	return

TelluricCave_EventScript_Zygarde::
	lockall
	specialvar VAR_RESULT, GenesisEvent_ZygardeAvailable
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_GenesisUnworthy
	waitse
	playmoncry SPECIES_ZYGARDE_50_POWER_CONSTRUCT, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_ZYGARDE_50_POWER_CONSTRUCT, 75
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, TelluricCave_ZygardeCaught
	setflag FLAG_GENESIS_ZYGARDE_HIDE
	removeobject LOCALID_TELLURIC_CAVE_ZYGARDE
	releaseall
	end

TelluricCave_ZygardeCaught::
	setflag FLAG_GENESIS_ZYGARDE_CAUGHT
	setflag FLAG_GENESIS_ZYGARDE_HIDE
	releaseall
	end
