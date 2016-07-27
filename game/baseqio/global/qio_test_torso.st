state STAND
{
	movetype legs

	entrycommands
	{
		viewmodelanim idle
	}
	
	torso
	{
		none : default // no torso animation
	}

	states
	{
		KILLED				: KILLED

		PUTAWAY_MAIN			: PUTAWAYMAIN
		RAISE_WEAPON			: NEW_WEAPON

		CHECK_PRIMARY_ATTACK_FULLAUTO	: !IS_WEAPON_SEMIAUTO ATTACK_PRIMARY IS_WEAPON_READY_TO_FIRE "dualhand"
		CHECK_PRIMARY_ATTACK_SEMIAUTO	: IS_WEAPON_SEMIAUTO +ATTACK_PRIMARY IS_WEAPON_READY_TO_FIRE "dualhand"

		RELOAD_WEAPON			: RELOAD
	}
}
state RELOAD_WEAPON
{
	movetype legs

	torso
	{
		m249_reload :  IS_WEAPON_ACTIVE "dualhand" "m249"
		unarmed_reload : default // stop torso animation
	}
	entrycommands
	{
		weaponCommand mainHand clip_fill
		viewmodelanim reload
	}
	
	states
	{
		KILLED				: KILLED
		STAND		:  ANIMDONE_TORSO
	}
	
}

state CHECK_PRIMARY_ATTACK_FULLAUTO
{
	movetype legs
	
	torso
	{
		none : default // no torso animation
	}
	states
	{
		KILLED				: KILLED
		
		STAND					: RELOAD
		STAND					: NEW_WEAPON
		STAND					: !IS_WEAPON_READY_TO_FIRE "dualhand"
		PRIMARY_ATTACK_FULLAUTO		: ATTACK_PRIMARY
		
		STAND					: !ATTACK_PRIMARY
	}
}
state CHECK_PRIMARY_ATTACK_SEMIAUTO
{
	movetype legs
	
	torso
	{
		none : default // no torso animation
	}
	states
	{
		KILLED				: KILLED
		
		STAND					: RELOAD
		STAND					: NEW_WEAPON
		STAND					: !IS_WEAPON_READY_TO_FIRE "dualhand"
		PRIMARY_ATTACK_SEMIAUTO		: ATTACK_PRIMARY
		
		STAND					: !ATTACK_PRIMARY
	}
}
state PRIMARY_ATTACK_FULLAUTO
{
	movetype legs

	torso
	{
		m249_attack_h :  IS_WEAPON_ACTIVE "dualhand" "m249"
		unarmed_attack_h : default // stop torso animation
	}
	
	entrycommands
	{
		viewmodelanim fire
		weaponCommand mainHand shoot
	}	
	states
	{
		KILLED				: KILLED
		PUTAWAY_MAIN			: PUTAWAYMAIN ANIMDONE_TORSO
		RAISE_WEAPON			: NEW_WEAPON ANIMDONE_TORSO

		CHECK_PRIMARY_ATTACK_FULLAUTO	: !IS_WEAPON_SEMIAUTO ATTACK_PRIMARY ANIMDONE_TORSO IS_WEAPON_READY_TO_FIRE "dualhand"
		STAND		:  ANIMDONE_TORSO
	}
}
state PRIMARY_ATTACK_SEMIAUTO
{
	movetype legs

	torso
	{
		m249_attack_h :  IS_WEAPON_ACTIVE "dualhand" "m249"
		unarmed_attack_h : default // stop torso animation
	}
	
	entrycommands
	{
		viewmodelanim fire
		weaponCommand mainHand shoot
	}	
	states
	{
		KILLED				: KILLED
		PUTAWAY_MAIN			: PUTAWAYMAIN ANIMDONE_TORSO
		RAISE_WEAPON			: NEW_WEAPON ANIMDONE_TORSO

		STAND		:  ANIMDONE_TORSO
	}
}
state RAISE_WEAPON
{	
	movetype legs

	torso
	{
		m249_select : IS_WEAPON_ACTIVE "dualhand" "m249"
		unarmed_select : default // default animation, should be something flexible
	}
	entrycommands
	{
		viewmodelanim pullout
	}
	exitcommands
	{
		activatenewweapon
	}
	states
	{
		KILLED				: KILLED
		STAND			: ANIMDONE_TORSO
	}
}

state PUTAWAY_MAIN
{	
	movetype legs

	torso
	{
		m249_unselect : IS_WEAPON_ACTIVE "dualhand" "m249"
		unarmed_unselect : default // default animation, should be something flexible
	}
	entrycommands
	{
		viewmodelanim putaway
	}
	exitcommands
	{
		deactivateweapon dualhand
	}
	states
	{
		KILLED				: KILLED
		STAND			: ANIMDONE_TORSO
	}
}

state KILLED
{
	movetype none
	
	entrycommands
	{
		modheight prone
		printMsg "ENTERING KILLED STATE"
	}
	torso
	{
		none : default
	}
	
}



