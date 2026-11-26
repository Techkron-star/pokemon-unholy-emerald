CrystalCave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CrystalCave_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CrystalCave_OnResume
	.byte 0

CrystalCave_OnTransition:
	setflag FLAG_GENESIS_MEWTWO_HIDE
	call_if_unset FLAG_GENESIS_MEWTWO_CAUGHT, CrystalCave_ShowMewtwo
	end

CrystalCave_ShowMewtwo::
	clearflag FLAG_GENESIS_MEWTWO_HIDE
	return

CrystalCave_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, CrystalCave_TryRemoveMewtwo
	end

CrystalCave_TryRemoveMewtwo::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject LOCALID_CRYSTAL_CAVE_MEWTWO
	return

CrystalCave_EventScript_Mewtwo::
	lockall
	specialvar VAR_RESULT, GenesisEvent_MewtwoAvailable
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_GenesisUnworthy
	waitse
	playmoncry SPECIES_MEWTWO, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_MEWTWO, 70
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, CrystalCave_MewtwoCaught
	setflag FLAG_GENESIS_MEWTWO_HIDE
	removeobject LOCALID_CRYSTAL_CAVE_MEWTWO
	releaseall
	end

CrystalCave_MewtwoCaught::
	setflag FLAG_GENESIS_MEWTWO_CAUGHT
	setflag FLAG_GENESIS_MEWTWO_HIDE
	releaseall
	end
