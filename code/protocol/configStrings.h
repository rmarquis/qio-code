/*
============================================================================
Copyright (C) 2012-2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// protocol/configStrings.h
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
#include "netLimits.h"

// an info string with all the serverinfo cvars
#define	CS_SERVERINFO			0
// an info string for server system to client system configuration (timescale, etc)
#define	CS_SYSTEMINFO			1

#define GAME_VERSION "qioTestGame" // value for CS_GAME_VERSION
#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define CS_WORLD_SKYMATERIAL	22
#define CS_WORLD_WATERLEVEL		23
// zFar value (far clip plane)
#define CS_WORLD_FARPLANE		24

// renderer models and TIKIs
#define	CS_MODELS				32
// collision models (cm module)
#define	CS_COLLMODELS			(CS_MODELS+MAX_MODELS)
// sounds
#define	CS_SOUNDS				(CS_COLLMODELS+MAX_MODELS)
#define	CS_ANIMATIONS			(CS_SOUNDS+MAX_SOUNDS)
// Articulated Figures (AF files)
#define CS_RAGDOLLDEFSS			(CS_ANIMATIONS+MAX_ANIMATIONS)
#define CS_SKINS				(CS_RAGDOLLDEFSS+MAX_RAGDOLLDEFS)
#define CS_MATERIALS			(CS_SKINS+MAX_SKINS)
#define CS_MAX					(CS_MATERIALS+MAX_MATERIALS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif
