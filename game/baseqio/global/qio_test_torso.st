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

	viewModelHands 
	{
		beretta_m9_fps_idle 	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_fps_idle 		: IS_WEAPON_ACTIVE "dualhand" "knife"
		m249_fps_idle 		: IS_WEAPON_ACTIVE "dualhand" "m249"
		dragunov_fps_idle 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		mp5_acog_fps_idle 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
	}
	viewModelGun
	{
		idle : default
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
		beretta_m9_reload 	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_reload 		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_reload 		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_reload 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_reload 	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_reload 		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_reload 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_reload 		: default // stop torso animation
	}
	entrycommands
	{
		weaponCommand mainHand clip_fill
	}
	viewModelHands 
	{
		beretta_m9_fps_reload 	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_fps_reload 	: IS_WEAPON_ACTIVE "dualhand" "knife"
		m249_fps_reload 	: IS_WEAPON_ACTIVE "dualhand" "m249"
		dragunov_fps_reload 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		mp5_acog_fps_reload 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
	}
	viewModelGun
	{
		reload : default
	}
	
	states
	{
		KILLED			: KILLED
		STAND			: ANIMDONE_TORSO
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
		KILLED			: KILLED
		
		STAND			: RELOAD
		STAND			: NEW_WEAPON
		STAND			: !IS_WEAPON_READY_TO_FIRE "dualhand"
		PRIMARY_ATTACK_FULLAUTO	: ATTACK_PRIMARY
		
		STAND			: !ATTACK_PRIMARY
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
		KILLED			: KILLED
		
		STAND			: RELOAD
		STAND			: NEW_WEAPON
		STAND			: !IS_WEAPON_READY_TO_FIRE "dualhand"
		PRIMARY_ATTACK_SEMIAUTO	: ATTACK_PRIMARY
		
		STAND			: !ATTACK_PRIMARY
	}
}
state PRIMARY_ATTACK_FULLAUTO
{
	movetype legs

	torso
	{
		beretta_m9_attack_h	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_attack_h	 	: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_attack_h	 	: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_attack_h 		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_attack_h 	: default // stop torso animation
	}
	
	entrycommands
	{
		weaponCommand mainHand shoot
	}	
	viewModelHands 
	{
		beretta_m9_fps_attack 	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_fps_attack 	: IS_WEAPON_ACTIVE "dualhand" "knife"
		m249_fps_attack 	: IS_WEAPON_ACTIVE "dualhand" "m249"
		dragunov_fps_attack 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		mp5_acog_fps_attack 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
	}
	viewModelGun
	{
		attack : default
	}
	states
	{
		KILLED			: KILLED
		PUTAWAY_MAIN		: PUTAWAYMAIN ANIMDONE_TORSO
		RAISE_WEAPON		: NEW_WEAPON ANIMDONE_TORSO

		PRIMARY_ATTACK_FULLAUTO	: !IS_WEAPON_SEMIAUTO ATTACK_PRIMARY ANIMDONE_TORSO IS_WEAPON_READY_TO_FIRE "dualhand"
		STAND			: ANIMDONE_TORSO
	}
}
state PRIMARY_ATTACK_SEMIAUTO
{
	movetype legs

	torso
	{
		beretta_m9_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_attack_h 		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_attack_h 		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_attack_h 		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_attack_h 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_attack_h 	: default // stop torso animation
	}
	
	entrycommands
	{
		weaponCommand mainHand shoot
	}	
	states
	{
		KILLED			: KILLED
		PUTAWAY_MAIN		: PUTAWAYMAIN ANIMDONE_TORSO
		RAISE_WEAPON		: NEW_WEAPON ANIMDONE_TORSO

		STAND			: ANIMDONE_TORSO
	}
}
state RAISE_WEAPON
{	
	movetype legs

	torso
	{
		beretta_m9_select 	: IS_NEW_WEAPON "dualhand" "beretta_m9"
		knife_select 		: IS_NEW_WEAPON "dualhand" "knife"
		awm_select 		: IS_NEW_WEAPON "dualhand" "awm"
		dragunov_select 	: IS_NEW_WEAPON "dualhand" "dragunov"
		steyr_aug_select 	: IS_NEW_WEAPON "dualhand" "steyr_aug"
		m249_select 		: IS_NEW_WEAPON "dualhand" "m249"
		mp5_acog_select 	: IS_NEW_WEAPON "dualhand" "mp5_acog"
		unarmed_select 		: default // default animation, should be something flexible
	}
	viewModelHands 
	{
		beretta_m9_fps_select 	: IS_NEW_WEAPON "dualhand" "beretta_m9"
		knife_fps_select 	: IS_NEW_WEAPON "dualhand" "knife"
		m249_fps_select 	: IS_NEW_WEAPON "dualhand" "m249"
		dragunov_fps_select 	: IS_NEW_WEAPON "dualhand" "dragunov"
		mp5_acog_fps_select 	: IS_NEW_WEAPON "dualhand" "mp5_acog"
	}
	viewModelGun
	{
		select : default
	}
	exitcommands
	{
		activatenewweapon
	}
	states
	{
		KILLED			: KILLED
		STAND			: ANIMDONE_TORSO
	}
}

state PUTAWAY_MAIN
{	
	movetype legs

	torso
	{
		beretta_m9_unselect	: IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_unselect		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_unselect		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_unselect	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_unselect	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_unselect		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_unselect	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_unselect	: default // default animation, should be something flexible
	}
	viewModelHands 
	{
		beretta_m9_fps_unselect : IS_WEAPON_ACTIVE "dualhand" "beretta_m9"
		knife_fps_unselect 	: IS_WEAPON_ACTIVE "dualhand" "knife"
		m249_fps_unselect 	: IS_WEAPON_ACTIVE "dualhand" "m249"
		dragunov_fps_unselect 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		mp5_acog_fps_unselect 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
	}
	viewModelGun
	{
		unselect : default
	}
	exitcommands
	{
		deactivateweapon dualhand
	}
	states
	{
		KILLED			: KILLED
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



