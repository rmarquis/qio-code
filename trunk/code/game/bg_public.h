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
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#define	GAME_VERSION		BASEGAME "-1"

//#define	DEFAULT_GRAVITY		800

//#define	MINS_Z				-24
//#define	DEFAULT_VIEWHEIGHT	82

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h


#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define CS_WORLD_SKYMATERIAL	22
#define CS_WORLD_WATERLEVEL		23

// renderer models
#define	CS_MODELS				32
// collision models (cm module)
#define	CS_COLLMODELS			(CS_MODELS+MAX_MODELS)
// sounds
#define	CS_SOUNDS				(CS_COLLMODELS+MAX_MODELS)
#define	CS_ANIMATIONS			(CS_SOUNDS+MAX_SOUNDS)
#define CS_RAGDOLLDEFSS			(CS_ANIMATIONS+MAX_ANIMATIONS)
#define CS_SKINS				(CS_RAGDOLLDEFSS+MAX_RAGDOLLDEFS)
//#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)
//#define CS_PARTICLES			(CS_LOCATIONS+MAX_LOCATIONS) 
//
//#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)
#define CS_MAX					(CS_SKINS+MAX_SKINS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_s *ps, const usercmd_s *cmd );

//===================================================================================

//---------------------------------------------------------
//
// entityState_s->eType
//
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	// dynamic light entity
	ET_LIGHT,
	// camera view portal; used for example on q3dm0
	ET_PORTAL, // classname: misc_portal_surface
	// server-only entity for triggers
	ET_TRIGGER, 
} entityType_t;


void	BG_PlayerStateToEntityState( playerState_s *ps, entityState_s *s, qboolean snap );
