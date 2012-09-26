/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef MISSIONPACK
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[32];
	char		attackerName[32];
	gender_t	gender;
	clientInfo_t	*ci;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "suicides";
		break;
	case MOD_FALLING:
		message = "cratered";
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
	case MOD_SLIME:
		message = "melted";
		break;
	case MOD_LAVA:
		message = "does a back flip into the lava";
		break;
	case MOD_TARGET_LASER:
		message = "saw the light";
		break;
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
	default:
		message = NULL;
		break;
	}

	if (attacker == target) {
		gender = ci->gender;
		switch (mod) {
#ifdef MISSIONPACK
		case MOD_KAMIKAZE:
			message = "goes out with a bang";
			break;
#endif
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
#ifdef MISSIONPACK
		case MOD_PROXIMITY_MINE:
			if( gender == GENDER_FEMALE ) {
				message = "found her prox mine";
			} else if ( gender == GENDER_NEUTER ) {
				message = "found its prox mine";
			} else {
				message = "found his prox mine";
			}
			break;
#endif
		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum ) {
		char	*s;

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else {
			s = va("You fragged %s", targetName );
		}
#ifdef MISSIONPACK
		if (!(cg_singlePlayerActive.integer && cg_cameraOrbit.integer)) {
			CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} 
#else
		CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
#endif

		// print the text message as well
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
		case MOD_GAUNTLET:
			message = "was pummeled by";
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			break;
		case MOD_GRENADE:
			message = "ate";
			message2 = "'s grenade";
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			break;
		case MOD_ROCKET_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			break;
		case MOD_LIGHTNING:
			message = "was electrocuted by";
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;
#ifdef MISSIONPACK
		case MOD_NAIL:
			message = "was nailed by";
			break;
		case MOD_CHAINGUN:
			message = "got lead poisoning from";
			message2 = "'s Chaingun";
			break;
		case MOD_PROXIMITY_MINE:
			message = "was too close to";
			message2 = "'s Prox Mine";
			break;
		case MOD_KAMIKAZE:
			message = "falls to";
			message2 = "'s Kamikaze blast";
			break;
		case MOD_JUICED:
			message = "was juiced by";
			break;
#endif
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		default:
			message = "was killed by";
			break;
		}

		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	
}

/*
================
CG_WaterLevel

Returns waterlevel for entity origin
================
*/
int CG_WaterLevel(centity_t *cent) {
	vec3_t point;
	int contents, sample1, sample2, anim, waterlevel;

	// get waterlevel, accounting for ducking
	waterlevel = 0;
	VectorCopy(cent->lerpOrigin, point);
	point[2] += MINS_Z + 1;
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR) {
		point[2] += CROUCH_VIEWHEIGHT;
	} else {
		point[2] += DEFAULT_VIEWHEIGHT;
	}

	contents = CG_PointContents(point, -1);

	if (contents & MASK_WATER) {
		sample2 = point[2] - MINS_Z;
		sample1 = sample2 / 2;
		waterlevel = 1;
		point[2] = cent->lerpOrigin[2] + MINS_Z + sample1;
		contents = CG_PointContents(point, -1);

		if (contents & MASK_WATER) {
			waterlevel = 2;
			point[2] = cent->lerpOrigin[2] + MINS_Z + sample2;
			contents = CG_PointContents(point, -1);

			if (contents & MASK_WATER) {
				waterlevel = 3;
			}
		}
	}

	return waterlevel;
}

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	// play a gurp sound instead of a normal pain sound
	if (CG_WaterLevel(cent) >= 1) {
		if (rand()&1) {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp1.wav"));
		} else {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp2.wav"));
		}
	} else {
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, snd));
	}
	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {


}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {

}

