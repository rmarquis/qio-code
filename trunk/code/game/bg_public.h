/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Qio source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qio source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// bg_public.h -- definitions shared by both the server game and client game modules
#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#include <shared/typedefs.h>
#include <protocol/netLimits.h>

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#define	GAME_VERSION		"qioTestGame"

//#define	DEFAULT_GRAVITY		800

//#define	MINS_Z				-24
//#define	DEFAULT_VIEWHEIGHT	82

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined elsewhere?


#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define CS_WORLD_SKYMATERIAL	22
#define CS_WORLD_WATERLEVEL		23
// zFar value (far clip plane)
#define CS_WORLD_FARPLANE		24

// renderer models
#define	CS_MODELS				32
// collision models (cm module)
#define	CS_COLLMODELS			(CS_MODELS+MAX_MODELS)
// sounds
#define	CS_SOUNDS				(CS_COLLMODELS+MAX_MODELS)
#define	CS_ANIMATIONS			(CS_SOUNDS+MAX_SOUNDS)
#define CS_RAGDOLLDEFSS			(CS_ANIMATIONS+MAX_ANIMATIONS)
#define CS_SKINS				(CS_RAGDOLLDEFSS+MAX_RAGDOLLDEFS)
#define CS_MATERIALS				(CS_SKINS+MAX_SKINS)
//#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)
//#define CS_PARTICLES			(CS_LOCATIONS+MAX_LOCATIONS) 
//
//#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)
#define CS_MAX					(CS_MATERIALS+MAX_MATERIALS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

#endif // __BG_PUBLIC_H__
