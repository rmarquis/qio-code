
state STAND
{
	entrycommands
	{
		modheight "stand"
	}
	legs
	{
		m249_hold_f :  IS_WEAPON_ACTIVE "mainhand" "m249"
		unarmed_hold_f : default
	}
	states
	{
		RUN_FORWARD			: FORWARD
		RUN_BACKWARD		: BACKWARD 
		RUN_LEFT			: STRAFE_LEFT
		RUN_BACKWARD		: STRAFE_RIGHT 
		
		// restart if animation is done
		STAND				: ANIMDONE_LEGS
	}
}


state RUN_FORWARD
{
	legs
	{
		m249_run_f :  IS_WEAPON_ACTIVE "mainhand" "m249"
		unarmed_run_f : default
	}

	states
	{
		STAND				: !HAS_VELOCITY
		
		RUN_BACKWARD		: BACKWARD
		RUN_LEFT			: STRAFE_LEFT !FORWARD !BACKWARD
		RUN_RIGHT			: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}


state RUN_BACKWARD
{
	legs
	{
		m249_run_b :  IS_WEAPON_ACTIVE "mainhand" "m249"
		unarmed_run_b : default
	}

	states
	{
		STAND				: !HAS_VELOCITY

		RUN_FORWARD			: FORWARD
		RUN_LEFT			: STRAFE_LEFT !FORWARD !BACKWARD
		RUN_RIGHT			: STRAFE_RIGHT !FORWARD !BACKWARD
	}
}

state RUN_LEFT
{
	legs
	{
		m249_run_l :  IS_WEAPON_ACTIVE "mainhand" "m249"
		unarmed_run_l : default
	}

	states
	{
		STAND				: !HAS_VELOCITY

		RUN_FORWARD			: FORWARD !STRAFE_LEFT !STRAFE_RIGHT
		RUN_BACKWARD		: BACKWARD !STRAFE_LEFT !STRAFE_RIGHT
		RUN_RIGHT			: STRAFE_RIGHT
	}
}

state RUN_RIGHT
{
	legs
	{
		m249_run_r :  IS_WEAPON_ACTIVE "mainhand" "m249"
		unarmed_run_r : default
	}

	states
	{
		STAND				: !HAS_VELOCITY
		
		RUN_FORWARD			: FORWARD !STRAFE_LEFT !STRAFE_RIGHT
		RUN_BACKWARD		: BACKWARD !STRAFE_LEFT !STRAFE_RIGHT
		RUN_LEFT			: STRAFE_LEFT
	}
}
