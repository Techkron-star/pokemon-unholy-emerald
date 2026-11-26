FarawayIsland_Entrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FarawayIsland_Entrance_OnTransition
	.byte 0

FarawayIsland_Entrance_OnTransition:
	call FarawayIsland_Entrance_EventScript_GenesisFaunaReset
	setflag FLAG_ARRIVED_ON_FARAWAY_ISLAND
	end

FarawayIsland_Entrance_EventScript_SetCloudsWeather::
	setweather WEATHER_SUNNY_CLOUDS
	doweather
	end

FarawayIsland_Entrance_EventScript_ClearWeather::
	setweather WEATHER_NONE
	doweather
	end

FarawayIsland_Entrance_EventScript_Sailor::
	lock
	faceplayer
	msgbox FarawayIsland_Entrance_Text_SailorReturn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FarawayIsland_Entrance_EventScript_AsYouLike
	msgbox EventTicket_Text_SailHome, MSGBOX_DEFAULT
	closemessage
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	delay 30
	hideobjectat LOCALID_FARAWAY_ISLAND_SAILOR, MAP_FARAWAY_ISLAND_ENTRANCE
	setvar VAR_0x8004, LOCALID_FARAWAY_ISLAND_SS_TIDAL
	call Common_EventScript_FerryDepartIsland
	warp MAP_LILYCOVE_CITY_HARBOR, 8, 11
	waitstate
	release
	end

FarawayIsland_Entrance_EventScript_AsYouLike::
	msgbox EventTicket_Text_AsYouLike, MSGBOX_DEFAULT
	release
	end

FarawayIsland_Entrance_EventScript_Sign::
	msgbox FarawayIsland_Entrance_Text_Sign, MSGBOX_SIGN
	end


@ Genesis island fauna: each specimen can be encountered once per map visit.
FarawayIsland_Entrance_EventScript_GenesisFaunaReset::
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	clearflag FLAG_TEMP_4
	return

FarawayIsland_Entrance_EventScript_GenesisFaunaORANGURU::
	lockall
	waitse
	playmoncry SPECIES_ORANGURU, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_ORANGURU, 48
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FarawayIsland_Entrance_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_1
	removeobject VAR_LAST_TALKED
	releaseall
	end

FarawayIsland_Entrance_EventScript_GenesisFaunaPASSIMIAN::
	lockall
	waitse
	playmoncry SPECIES_PASSIMIAN, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_PASSIMIAN, 48
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FarawayIsland_Entrance_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_2
	removeobject VAR_LAST_TALKED
	releaseall
	end

FarawayIsland_Entrance_EventScript_GenesisFaunaKOMALA::
	lockall
	waitse
	playmoncry SPECIES_KOMALA, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_KOMALA, 42
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FarawayIsland_Entrance_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_3
	removeobject VAR_LAST_TALKED
	releaseall
	end

FarawayIsland_Entrance_EventScript_GenesisFaunaTOEDSCOOL::
	lockall
	waitse
	playmoncry SPECIES_TOEDSCOOL, CRY_MODE_ENCOUNTER
	delay 24
	waitmoncry
	setwildbattle SPECIES_TOEDSCOOL, 43
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	call_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FarawayIsland_Entrance_EventScript_GenesisFaunaDiscovered
	setflag FLAG_TEMP_4
	removeobject VAR_LAST_TALKED
	releaseall
	end

FarawayIsland_Entrance_EventScript_GenesisFaunaDiscovered::
	setflag FLAG_UNHOLY_GENESIS_FARAWAY_FAUNA_DISCOVERED
	return
