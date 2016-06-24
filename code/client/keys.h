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
#include "keycodes.h"
#include <api/vfsAPI.h>
#include <shared/str.h>
#include <shared/field.h>

struct keyBind_s {
	bool	down;
	int			repeats;		// if > 1, it is autorepeating
	str		binding;
};

extern	keyBind_s		keys[MAX_KEYS];

#define		COMMAND_HISTORY		32
extern	field_s	historyEditLines[COMMAND_HISTORY];

extern	field_s	g_consoleField;
extern	field_s	chatField;
extern	int				anykeydown;
extern	bool	chat_team;
extern	int			chat_playerNum;

void Key_WriteBindings( fileHandle_t f );
void Key_SetBinding( int keynum, const char *binding );
const char *Key_GetBinding( int keynum );
bool Key_IsDown( int keynum );
void Key_ClearStates();
int Key_GetKey(const char *binding);
