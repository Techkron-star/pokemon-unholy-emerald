NavelRock_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, NavelRock_B1F_GenesisOnTransition
	.byte 0

NavelRock_B1F_GenesisOnTransition:
	call NavelRock_B1F_EventScript_GenesisFaunaReset
	end


@ Genesis island fauna: each specimen can be encountered once per map visit.
NavelRock_B1F_EventScript_GenesisFaunaReset::
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	return

NavelRock_B1F_EventScript_GenesisFaunaDREEPY::
	lockall
	waitse
	playmoncry SPECIES_DREEPY, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_DREEPY, 44
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, NavelRock_B1F_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_1
	removeobject VAR_LAST_TALKED
	releaseall
	end

NavelRock_B1F_EventScript_GenesisFaunaDURALUDON::
	lockall
	waitse
	playmoncry SPECIES_DURALUDON, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_DURALUDON, 48
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, NavelRock_B1F_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_2
	removeobject VAR_LAST_TALKED
	releaseall
	end

NavelRock_B1F_EventScript_GenesisFaunaDRAMPA::
	lockall
	waitse
	playmoncry SPECIES_DRAMPA, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_DRAMPA, 55
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, NavelRock_B1F_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_3
	removeobject VAR_LAST_TALKED
	releaseall
	end

NavelRock_B1F_EventScript_GenesisFaunaDiscovered::
	setflag FLAG_UNHOLY_GENESIS_NAVEL_FAUNA_DISCOVERED
	return
