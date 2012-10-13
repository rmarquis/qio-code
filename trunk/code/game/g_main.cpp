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

#include "g_local.h"
#include <api/serverAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include "classes/BaseEntity.h"

level_locals_t	level;

edict_s		g_entities[MAX_GENTITIES];

void QDECL G_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	g_core->Print( text );
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	Q_vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	g_core->DropError( text );
}

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn( void ) {
	// make some data visible to connecting client
	g_server->SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	g_server->SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	g_cvars->Cvar_Set( "g_gravity", "800" );


//	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
//	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

//	g_entities[ENTITYNUM_NONE].s.number = ENTITYNUM_NONE;
///	g_entities[ENTITYNUM_NONE].classname = "nothing";

	
}
/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	int					i;
	char mapName[128];

	G_Printf ("------- Game Initialization -------\n");
	G_Printf ("gamename: %s\n", GAMEVERSION);
	G_Printf ("gamedate: %s\n", __DATE__);

	srand( randomSeed );

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	// initialize all entities for this game
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	level.gentities = g_entities;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	for ( i=0 ; i<MAX_CLIENTS ; i++ ) {
//		g_entities[i].classname = "clientslot";
	}

	// let the server system know where the entites are
	g_server->LocateGameData( level.gentities, level.num_entities );

	// parse the key/value pairs and spawn gentities
	SP_worldspawn();

	g_cvars->Cvar_VariableStringBuffer("mapname",mapName,sizeof(mapName));

	// init bullet physics library
	G_InitBullet();
	// load map for Bullet
	G_LoadMap(mapName);
	// load map entities and spawn them
	G_SpawnMapEntities(mapName);

	G_Printf ("-----------------------------------\n");
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	G_Printf ("==== ShutdownGame ====\n");
	for(u32 i = 0; i < level.num_entities; i++) {
		edict_s *e = &g_entities[i];
		if(e->s == 0)
			continue;
		delete e->ent;
	}
	G_ShudownBullet();
}



//===================================================================

void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	g_core->DropError( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	g_core->Print( text );
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i;
	edict_s	*ed;

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	level.frameTime = (level.time-level.previousTime)*0.001f;

	G_RunPhysics();


	//
	// go through all allocated objects
	//
	ed = &g_entities[0];
	for (i=0 ; i<level.num_entities ; i++, ed++) {
		if ( ed->s == 0 ) {
			continue;
		}

		if ( i < MAX_CLIENTS ) {
			G_RunClient( ed );
			continue;
		}
		BaseEntity *e = ed->ent;

		e->runFrame();
	}

	// perform final fixups on the players
	ed = &g_entities[0];
	for (i=0 ; i < MAX_CLIENTS; i++, ed++ ) {
		if ( ed->s ) {
			ClientEndFrame( ed );
		}
	}

}
