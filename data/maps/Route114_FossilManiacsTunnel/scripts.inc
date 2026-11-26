Route114_FossilManiacsTunnel_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route114_FossilManiacsTunnel_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, Route114_FossilManiacsTunnel_OnLoad
	.byte 0

Route114_FossilManiacsTunnel_OnTransition:
	clearflag FLAG_TEMP_1
	call_if_set FLAG_SYS_GAME_CLEAR, Route114_FossilManiacsTunnel_EventScript_MoveFossilManiac
	end

Route114_FossilManiacsTunnel_EventScript_MoveFossilManiac::
	setobjectxyperm LOCALID_FOSSIL_MANIAC, 6, 5
	setobjectmovementtype LOCALID_FOSSIL_MANIAC, MOVEMENT_TYPE_FACE_DOWN
	return

Route114_FossilManiacsTunnel_OnLoad:
	call_if_unset FLAG_SYS_GAME_CLEAR, Route114_FossilManiacsTunnel_EventScript_CloseDesertUnderpass
	end

Route114_FossilManiacsTunnel_EventScript_CloseDesertUnderpass::
	setmetatile 6, 1, METATILE_Fallarbor_RedRockWall, TRUE
	setmetatile 6, 2, METATILE_Fallarbor_RedRockWall, TRUE
	return

Route114_FossilManiacsTunnel_EventScript_ManiacMentionCaveIn::
	lockall
	applymovement LOCALID_FOSSIL_MANIAC, Common_Movement_WalkInPlaceFasterUp
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox Route114_FossilManiacsTunnel_Text_NotSafeThatWay, MSGBOX_DEFAULT
	setvar VAR_FOSSIL_MANIAC_STATE, 2
	releaseall
	end

Route114_FossilManiacsTunnel_EventScript_FossilManiac::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, Route114_FossilManiacsTunnel_EventScript_GenesisOriginal
	goto_if_set FLAG_TEMP_1, Route114_FossilManiacsTunnel_EventScript_GenesisOriginal
	msgbox Route114_FossilManiacsTunnel_Text_GenesisDig, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, Route114_FossilManiacsTunnel_EventScript_GenesisOriginal
	goto Route114_FossilManiacsTunnel_EventScript_GenesisExcavate
	end

Route114_FossilManiacsTunnel_EventScript_GenesisOriginal::
	goto_if_set FLAG_RECEIVED_REVIVED_FOSSIL_MON, Route114_FossilManiacsTunnel_EventScript_PlayerRevivedFossil
	checkitem ITEM_ROOT_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, Route114_FossilManiacsTunnel_EventScript_PlayerHasFossil
	checkitem ITEM_CLAW_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, Route114_FossilManiacsTunnel_EventScript_PlayerHasFossil
	msgbox Route114_FossilManiacsTunnel_Text_LookInDesertForFossils, MSGBOX_DEFAULT
	release
	end

Route114_FossilManiacsTunnel_EventScript_PlayerHasFossil::
	msgbox Route114_FossilManiacsTunnel_Text_DevonCorpRevivingFossils, MSGBOX_DEFAULT
	release
	end

Route114_FossilManiacsTunnel_EventScript_PlayerRevivedFossil::
	msgbox Route114_FossilManiacsTunnel_Text_FossilsAreWonderful, MSGBOX_DEFAULT
	release
	end

Route114_FossilManiacsTunnel_Text_LookInDesertForFossils:
	.string "I'm the FOSSIL MANIAC…\n"
	.string "I'm a nice guy who loves FOSSILS…\p"
	.string "Do you want a FOSSIL?\p"
	.string "But the FOSSILS around these parts all\n"
	.string "belong to me… None for you…\p"
	.string "If you can't bear to go without\n"
	.string "a FOSSIL, look in a desert where there\l"
	.string "are boulders and sand that may hide\l"
	.string "FOSSILS…$"

Route114_FossilManiacsTunnel_Text_DevonCorpRevivingFossils:
	.string "You found a FOSSIL, didn't you?\n"
	.string "That's so nice… It's so dreamy…\p"
	.string "What are you going to do with that\n"
	.string "FOSSIL?\p"
	.string "From what I've heard, DEVON is doing\n"
	.string "research on reviving POKéMON from\l"
	.string "FOSSILS…\p"
	.string "I love my FOSSILS, so I would never\n"
	.string "do anything like that…$"

Route114_FossilManiacsTunnel_Text_FossilsAreWonderful:
	.string "FOSSILS are so… Wonderful…\n"
	.string "It's so dreamy…$"

Route114_FossilManiacsTunnel_Text_NotSafeThatWay:
	.string "Oh…\n"
	.string "It's not safe that way…\p"
	.string "I was digging away, you see…\n"
	.string "When the whole wall collapsed…\p"
	.string "I think there's a giant cavern\n"
	.string "underneath now…\p"
	.string "But I've left it alone because I don't\n"
	.string "think there are any FOSSILS there…$"


@ One guided excavation per tunnel visit, independent of the original fossil.
Route114_FossilManiacsTunnel_EventScript_GenesisExcavate::
	goto_if_set FLAG_BADGE08_GET, Route114_FossilManiacsTunnel_EventScript_GenesisDeepDig
	random 11
	goto Route114_FossilManiacsTunnel_EventScript_GenesisDigResult
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDeepDig::
	random 15
	goto Route114_FossilManiacsTunnel_EventScript_GenesisDigResult
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDigResult::
	switch VAR_RESULT
	case 0, Route114_FossilManiacsTunnel_EventScript_GenesisDig0
	case 1, Route114_FossilManiacsTunnel_EventScript_GenesisDig1
	case 2, Route114_FossilManiacsTunnel_EventScript_GenesisDig2
	case 3, Route114_FossilManiacsTunnel_EventScript_GenesisDig3
	case 4, Route114_FossilManiacsTunnel_EventScript_GenesisDig4
	case 5, Route114_FossilManiacsTunnel_EventScript_GenesisDig5
	case 6, Route114_FossilManiacsTunnel_EventScript_GenesisDig6
	case 7, Route114_FossilManiacsTunnel_EventScript_GenesisDig7
	case 8, Route114_FossilManiacsTunnel_EventScript_GenesisDig8
	case 9, Route114_FossilManiacsTunnel_EventScript_GenesisDig9
	case 10, Route114_FossilManiacsTunnel_EventScript_GenesisDig10
	case 11, Route114_FossilManiacsTunnel_EventScript_GenesisDig11
	case 12, Route114_FossilManiacsTunnel_EventScript_GenesisDig12
	case 13, Route114_FossilManiacsTunnel_EventScript_GenesisDig13
	case 14, Route114_FossilManiacsTunnel_EventScript_GenesisDig14
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig0::
	giveitem ITEM_ROOT_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig1::
	giveitem ITEM_CLAW_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig2::
	giveitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig3::
	giveitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig4::
	giveitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig5::
	giveitem ITEM_SKULL_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig6::
	giveitem ITEM_ARMOR_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig7::
	giveitem ITEM_COVER_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig8::
	giveitem ITEM_PLUME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig9::
	giveitem ITEM_JAW_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig10::
	giveitem ITEM_SAIL_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig11::
	giveitem ITEM_FOSSILIZED_BIRD
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig12::
	giveitem ITEM_FOSSILIZED_FISH
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig13::
	giveitem ITEM_FOSSILIZED_DRAKE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_EventScript_GenesisDig14::
	giveitem ITEM_FOSSILIZED_DINO
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_TEMP_1
	release
	end

Route114_FossilManiacsTunnel_Text_GenesisDig:
	.string "I've found a new fossil seam.\n"
	.string "Want to help excavate a sample?$"
