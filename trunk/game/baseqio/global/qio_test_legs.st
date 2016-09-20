
state STAND
{
	entrycommands
	{
		modheight "stand"
	}
	legs
	{
		knife_hold_f 		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_hold_f 		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_hold_f 	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_hold_f	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_hold_f		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_hold_f 	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_hold_f 		: default
	}
	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		RUN_FORWARD		: FORWARD RUN
		RUN_BACKWARD		: BACKWARD  RUN
		RUN_LEFT		: STRAFE_LEFT RUN
		RUN_BACKWARD		: STRAFE_RIGHT RUN
		
		WALK_FORWARD		: FORWARD
		WALK_BACKWARD		: BACKWARD 
		WALK_LEFT		: STRAFE_LEFT
		WALK_BACKWARD		: STRAFE_RIGHT
		
		// restart if animation is done
		STAND			: ANIMDONE_LEGS
	}
}


state RUN_FORWARD
{
	legs
	{
		knife_run_f		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_run_f		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_run_f		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_run_f 	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_run_f		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_run_f		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_run_f		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		WALK_FORWARD		: !RUN FORWARD
		WALK_BACKWARD		: !RUN BACKWARD
		WALK_LEFT		: !RUN STRAFE_LEFT
		WALK_BACKWARD		: !RUN STRAFE_RIGHT
		
		RUN_BACKWARD		: BACKWARD
		RUN_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
		RUN_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}


state RUN_BACKWARD
{
	legs
	{
		knife_run_b		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_run_b		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_run_b		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_run_b		: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_run_b		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_run_b		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_run_b		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY

		WALK_FORWARD		: !RUN FORWARD
		WALK_BACKWARD		: !RUN BACKWARD
		WALK_LEFT		: !RUN STRAFE_LEFT
		WALK_BACKWARD		: !RUN STRAFE_RIGHT
		
		RUN_FORWARD		: FORWARD
		RUN_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
		RUN_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}

state RUN_LEFT
{
	legs
	{
		knife_run_l		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_run_l		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_run_l		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_run_l		: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_run_l		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_run_l		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_run_l		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY

		WALK_FORWARD		: !RUN FORWARD
		WALK_BACKWARD		: !RUN BACKWARD
		WALK_LEFT		: !RUN STRAFE_LEFT
		WALK_BACKWARD		: !RUN STRAFE_RIGHT
		
		RUN_FORWARD		: FORWARD 
		RUN_BACKWARD		: BACKWARD 
		RUN_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}

state RUN_RIGHT
{
	legs
	{
		knife_run_r		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_run_r		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_run_r		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_run_r		: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_run_r		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_run_r		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_run_r		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		WALK_FORWARD		: !RUN FORWARD
		WALK_BACKWARD		: !RUN BACKWARD
		WALK_LEFT		: !RUN STRAFE_LEFT
		WALK_BACKWARD		: !RUN STRAFE_RIGHT
		
		RUN_FORWARD		: FORWARD
		RUN_BACKWARD		: BACKWARD
		RUN_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
	}
}



state WALK_FORWARD
{
	legs
	{
		knife_walk_f		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_walk_f		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_walk_f		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_walk_f	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_walk_f		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_walk_f		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_walk_f		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		RUN_FORWARD		: RUN FORWARD
		RUN_BACKWARD		: RUN BACKWARD
		RUN_LEFT		: RUN STRAFE_LEFT
		RUN_BACKWARD		: RUN STRAFE_RIGHT
		
		WALK_BACKWARD		: BACKWARD
		WALK_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
		WALK_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}

state WALK_BACKWARD
{
	legs
	{
		knife_walk_b		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_walk_b		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_walk_b		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_walk_b	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_walk_b		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_walk_b		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_walk_b		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		RUN_FORWARD		: RUN FORWARD
		RUN_BACKWARD		: RUN BACKWARD
		RUN_LEFT		: RUN STRAFE_LEFT
		RUN_BACKWARD		: RUN STRAFE_RIGHT
		
		WALK_FORWARD		: FORWARD
		WALK_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
		WALK_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}


state WALK_LEFT
{
	legs
	{
		knife_walk_l		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_walk_l		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_walk_l		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_walk_l	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_walk_l		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_walk_l		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_walk_l		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		RUN_FORWARD		: RUN FORWARD
		RUN_BACKWARD		: RUN BACKWARD
		RUN_LEFT		: RUN STRAFE_LEFT
		RUN_BACKWARD		: RUN STRAFE_RIGHT
		
		WALK_FORWARD		: FORWARD
		WALK_BACKWARD		: BACKWARD
		WALK_RIGHT		: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}

state WALK_RIGHT
{
	legs
	{
		knife_walk_r		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_walk_r		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_walk_r		: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_walk_r	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_walk_r		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_walk_r		: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_walk_r		: default
	}

	states
	{
		JUMP_STANDING		: +JUMP ONGROUND
		
		STAND			: !HAS_VELOCITY
		
		RUN_FORWARD		: RUN FORWARD
		RUN_BACKWARD		: RUN BACKWARD
		RUN_LEFT		: RUN STRAFE_LEFT
		RUN_BACKWARD		: RUN STRAFE_RIGHT
		
		WALK_FORWARD		: FORWARD
		WALK_BACKWARD		: BACKWARD
		WALK_LEFT		: STRAFE_LEFT !FORWARD !BACKWARD
	}
}

state JUMP_STANDING
{
/*	entrycommands
	{
		commanddelay 0.05 jump 56
	}*/
	legs
	{
		knife_jump_bg		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_jump_bg		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_jump_bg	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_jump_bg	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_jump_bg		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_jump_bg	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_jump_bg		: default
	}
	
	entrycommands
	{
		jump 56
	}
	states
	{
		FALL			: ANIMDONE_LEGS
	}
}
state FALL
{
	legs
	{
		knife_jump_id		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_jump_id		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_jump_id	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_jump_id	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_jump_id		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_jump_id	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_jump_id		: default
	}

	states
	{
		LAND			: ONGROUND
	}
}
state LAND
{
	legs
	{
		knife_jump_ed		: IS_WEAPON_ACTIVE "dualhand" "knife"
		awm_jump_ed		: IS_WEAPON_ACTIVE "dualhand" "awm"
		dragunov_jump_ed	: IS_WEAPON_ACTIVE "dualhand" "dragunov"
		steyr_aug_jump_ed	: IS_WEAPON_ACTIVE "dualhand" "steyr_aug"
		m249_jump_ed		: IS_WEAPON_ACTIVE "dualhand" "m249"
		mp5_acog_jump_ed	: IS_WEAPON_ACTIVE "dualhand" "mp5_acog"
		unarmed_jump_ed		: default
	}
	states
	{
		STAND			: ANIMDONE_LEGS
	}
}
