EventScript_CraftSuperExpShare::
	lock
	message Crafting_Text_MenuPrompt
	waitmessage
	dynmultipush Crafting_Text_MenuSuperExpShare, 0
	dynmultipush Crafting_Text_MenuExit, 1
	dynmultistack 0, 0, FALSE, 2, 0, 0, DYN_MULTICHOICE_CB_NONE
	closemessage
	switch VAR_RESULT
	case 0, EventScript_CraftSuperExpShare_Recipe
	goto EventScript_CraftSuperExpShare_End

EventScript_CraftSuperExpShare_Recipe:
	checkflag FLAG_BADGE01_GET
	goto_if_unset FLAG_BADGE01_GET, EventScript_CraftSuperExpShare_NoBadge
	checkitem ITEM_EXP_SHARE, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CraftSuperExpShare_NoExpShare
	checkitemspace ITEM_SUPER_EXP_SHARE, 1
	goto_if_eq VAR_RESULT, FALSE, EventScript_CraftSuperExpShare_NoSpace
	msgbox Crafting_Text_ConfirmSuperExpShare, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq EventScript_CraftSuperExpShare_End
	removeitem ITEM_EXP_SHARE, 1
	additem ITEM_SUPER_EXP_SHARE, 1
	clearflag FLAG_UNHOLY_EXP_SHARE_ENABLED
	setflag FLAG_UNHOLY_SUPER_EXP_SHARE_ENABLED
	msgbox Crafting_Text_SuperExpShareComplete, MSGBOX_SIGN
	goto EventScript_CraftSuperExpShare_End

EventScript_CraftSuperExpShare_NoBadge:
	msgbox Crafting_Text_SuperExpShareNoBadge, MSGBOX_SIGN
	goto EventScript_CraftSuperExpShare_End

EventScript_CraftSuperExpShare_NoExpShare:
	msgbox Crafting_Text_SuperExpShareNoBase, MSGBOX_SIGN
	goto EventScript_CraftSuperExpShare_End

EventScript_CraftSuperExpShare_NoSpace:
	msgbox Crafting_Text_SuperExpShareNoSpace, MSGBOX_SIGN

EventScript_CraftSuperExpShare_End:
	releaseall
	end

Crafting_Text_MenuPrompt:
	.string "CRAFTING - Choose a recipe.$"

Crafting_Text_MenuSuperExpShare:
	.string "SUPER EXP. SHARE$"

Crafting_Text_MenuExit:
	.string "EXIT$"

Crafting_Text_ConfirmSuperExpShare:
	.string "Craft a SUPER EXP. SHARE?\n"
	.string "It requires the STONE BADGE and\n"
	.string "your current EXP. SHARE.$"

Crafting_Text_SuperExpShareComplete:
	.string "The EXP. SHARE became a\n"
	.string "SUPER EXP. SHARE!$"

Crafting_Text_SuperExpShareNoBadge:
	.string "The STONE BADGE is required\n"
	.string "for this recipe.$"

Crafting_Text_SuperExpShareNoBase:
	.string "You need an EXP. SHARE first.$"

Crafting_Text_SuperExpShareNoSpace:
	.string "There is no room for the crafted\n"
	.string "item in your Bag.$"
