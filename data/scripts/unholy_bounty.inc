@ Repeatable trainer contracts. A single persistent contract prevents paying
@ twice after defeat, and rewards remain claimable when the Bag is full.
EventScript_UnholyBountySeller::
	lock
	faceplayer
	callnative UnholyBounty_Status
	compare VAR_RESULT, 1
	goto_if_eq UnholyBountySeller_Active
	compare VAR_RESULT, 2
	goto_if_eq UnholyBountySeller_Claim
	compare VAR_RESULT, 3
	goto_if_eq UnholyBountySeller_Exit
	callnative UnholyBounty_OpenMenu
	compare VAR_RESULT, TRUE
	goto_if_ne UnholyBountySeller_Exit
	waitstate
	compare VAR_RESULT, MULTI_B_PRESSED
	goto_if_eq UnholyBountySeller_Exit
	copyvar VAR_0x8004, VAR_RESULT
	callnative UnholyBounty_PreparePurchase
	compare VAR_RESULT, TRUE
	goto_if_ne UnholyBountySeller_Message
	msgbox gStringVar4, MSGBOX_YESNO
	compare VAR_RESULT, YES
	goto_if_ne UnholyBountySeller_Exit
	callnative UnholyBounty_Buy
	goto UnholyBountySeller_Message
UnholyBountySeller_Active:
	msgbox gStringVar4, MSGBOX_YESNO
	compare VAR_RESULT, YES
	goto_if_ne UnholyBountySeller_Exit
	callnative UnholyBounty_Abandon
	goto UnholyBountySeller_Exit
UnholyBountySeller_Claim:
	callnative UnholyBounty_ClaimScript
UnholyBountySeller_Message:
	msgbox gStringVar4, MSGBOX_DEFAULT
UnholyBountySeller_Exit:
	release
	end

EventScript_UnholyBountyTarget::
	lock
	faceplayer
	callnative UnholyBounty_Interact
	compare VAR_RESULT, TRUE
	goto_if_eq UnholyBountyTarget_Battle
	compare VAR_RESULT, 2
	goto_if_eq UnholyBountySeller_Message
	goto UnholyBountySeller_Claim
UnholyBountyTarget_Battle:
	waitstate
	goto UnholyBountySeller_Claim
