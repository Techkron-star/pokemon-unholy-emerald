VictoryRoad_B2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, VictoryRoad_B2F_OnTransition
	.byte 0

VictoryRoad_B2F_OnTransition:
	call_if_set FLAG_DEFEATED_SUPER_LEAGUE, VictoryRoad_B2F_ResetWildBoss
	end

VictoryRoad_B2F_ResetWildBoss:
	clearflag FLAG_UNHOLY_WILD_BOSS_AEGIS_CLEARED
	return

VictoryRoad_B2F_EventScript_Vito::
	trainerbattle_single TRAINER_VITO, VictoryRoad_B2F_Text_VitoIntro, VictoryRoad_B2F_Text_VitoDefeat
	msgbox VictoryRoad_B2F_Text_VitoPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Owen::
	trainerbattle_single TRAINER_OWEN, VictoryRoad_B2F_Text_OwenIntro, VictoryRoad_B2F_Text_OwenDefeat
	msgbox VictoryRoad_B2F_Text_OwenPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Caroline::
	trainerbattle_single TRAINER_CAROLINE, VictoryRoad_B2F_Text_CarolineIntro, VictoryRoad_B2F_Text_CarolineDefeat
	msgbox VictoryRoad_B2F_Text_CarolinePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Julie::
	trainerbattle_single TRAINER_JULIE, VictoryRoad_B2F_Text_JulieIntro, VictoryRoad_B2F_Text_JulieDefeat
	msgbox VictoryRoad_B2F_Text_JuliePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Felix::
	trainerbattle_single TRAINER_FELIX, VictoryRoad_B2F_Text_FelixIntro, VictoryRoad_B2F_Text_FelixDefeat
	msgbox VictoryRoad_B2F_Text_FelixPostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_EventScript_Dianne::
	trainerbattle_single TRAINER_DIANNE, VictoryRoad_B2F_Text_DianneIntro, VictoryRoad_B2F_Text_DianneDefeat
	msgbox VictoryRoad_B2F_Text_DiannePostBattle, MSGBOX_AUTOCLOSE
	end

VictoryRoad_B2F_Text_VitoIntro:
	.string "I trained together with my whole family,\n"
	.string "every one of us!\l"
	.string "I'm not losing to anyone!$"

VictoryRoad_B2F_Text_VitoDefeat:
	.string "Better than my family?!\n"
	.string "Is that possible?!$"

VictoryRoad_B2F_Text_VitoPostBattle:
	.string "I was better than everyone in my\n"
	.string "family. I've never lost before…\p"
	.string "I've lost my confidence…\n"
	.string "Maybe I'll go home…$"

VictoryRoad_B2F_Text_OwenIntro:
	.string "I'd heard that there was a tough\n"
	.string "little kid around. Do they mean you?$"

VictoryRoad_B2F_Text_OwenDefeat:
	.string "The little shrimp is tough!$"

VictoryRoad_B2F_Text_OwenPostBattle:
	.string "The rumors I heard, that tough little\n"
	.string "kid is from PETALBURG CITY.$"

VictoryRoad_B2F_Text_CarolineIntro:
	.string "You must be getting a little tired.$"

VictoryRoad_B2F_Text_CarolineDefeat:
	.string "No signs of tiring at all!$"

VictoryRoad_B2F_Text_CarolinePostBattle:
	.string "VICTORY ROAD and the POKéMON LEAGUE\n"
	.string "are long and grueling challenges.\l"
	.string "Beware of fatigue!$"

VictoryRoad_B2F_Text_JulieIntro:
	.string "You shouldn't get complacent just\n"
	.string "because you have a lot of GYM BADGES.\p"
	.string "There's always going to be someone\n"
	.string "who's better than you!$"

VictoryRoad_B2F_Text_JulieDefeat:
	.string "You're better than me!$"

VictoryRoad_B2F_Text_JuliePostBattle:
	.string "Gaze on your collected BADGES and\n"
	.string "remember the TRAINERS you've faced.$"

VictoryRoad_B2F_Text_FelixIntro:
	.string "I've come this far, but the tension's\n"
	.string "giving me awful stomach pain…$"

VictoryRoad_B2F_Text_FelixDefeat:
	.string "Ooh…\n"
	.string "It hurts…$"

VictoryRoad_B2F_Text_FelixPostBattle:
	.string "I can't help getting all tense knowing\n"
	.string "that I'm nearing the POKéMON LEAGUE.\p"
	.string "It's all I can do to feign calm.$"

VictoryRoad_B2F_Text_DianneIntro:
	.string "The elite among the elite gather in\n"
	.string "this cave.\p"
	.string "How are you finding it?$"

VictoryRoad_B2F_Text_DianneDefeat:
	.string "Not rattled in the least bit!$"

VictoryRoad_B2F_Text_DiannePostBattle:
	.string "You do have guts! I like that!\n"
	.string "Keep it going!$"



// Unholy Wild Boss: Aegis guardian. Captures preserve Alpha, not battle boosts.
VictoryRoad_B2F_EventScript_AegisBoss::
	lock
	faceplayer
	msgbox VictoryRoad_B2F_Text_AegisChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, VictoryRoad_B2F_EventScript_AegisLeave
	msgbox VictoryRoad_B2F_Text_AegisWarning, MSGBOX_DEFAULT
	setwildbattle SPECIES_REGISTEEL, 62
	setvar VAR_0x8004, UNHOLY_WILD_BOSS_AEGIS
	special UnholyWildBoss_Arm
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, VictoryRoad_B2F_EventScript_AegisWon
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, VictoryRoad_B2F_EventScript_AegisCaught
	release
	end

VictoryRoad_B2F_EventScript_AegisWon::
	setvar VAR_0x8000, ITEM_GEAR_ESCUDO_AEGIS
	setvar VAR_0x8001, 1
	call Std_ObtainItem
	goto_if_eq VAR_RESULT, FALSE, VictoryRoad_B2F_EventScript_AegisBagFull
	setflag FLAG_UNHOLY_WILD_BOSS_AEGIS_CLEARED
	removeobject VAR_LAST_TALKED
	msgbox VictoryRoad_B2F_Text_AegisReward, MSGBOX_DEFAULT
	release
	end

VictoryRoad_B2F_EventScript_AegisCaught::
	setflag FLAG_UNHOLY_WILD_BOSS_AEGIS_CLEARED
	removeobject VAR_LAST_TALKED
	msgbox VictoryRoad_B2F_Text_AegisCaptured, MSGBOX_DEFAULT
	release
	end

VictoryRoad_B2F_EventScript_AegisBagFull::
	msgbox VictoryRoad_B2F_Text_AegisBagFull, MSGBOX_DEFAULT
	release
	end

VictoryRoad_B2F_EventScript_AegisLeave::
	release
	end

VictoryRoad_B2F_Text_AegisChallenge:
	.string "Un REGISTEEL Alpha vigila este\n"
	.string "paso. ¿Quieres desafiarlo?$"

VictoryRoad_B2F_Text_AegisWarning:
	.string "Su poder es anormal, pero\n"
	.string "puedes intentar capturarlo.$"

VictoryRoad_B2F_Text_AegisReward:
	.string "¡El guardián dejó su\n"
	.string "Escudo de Aegis!$"

VictoryRoad_B2F_Text_AegisCaptured:
	.string "Lo capturaste, pero el\n"
	.string "Escudo de Aegis desapareció.$"

VictoryRoad_B2F_Text_AegisBagFull:
	.string "No hay espacio para Aegis.\n"
	.string "Haz espacio y vuelve a luchar.$"
