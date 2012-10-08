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
// sv_game.c -- interface to the game dll

#include "server.h"
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/gameAPI.h>

static moduleAPI_i *sv_gameDLL = 0;
gameAPI_s *g_game = 0;
gameClientAPI_s *g_gameClients = 0;

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( edict_s *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

edict_s *SV_GentityNum( int num ) {
	edict_s *ent;

	ent = (edict_s *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_s *SV_GameClientNum( int num ) {
	playerState_s	*ps;

	ps = (playerState_s *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( edict_s *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

edict_s *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( edict_s *ent, const char *name ) {

}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS (const vec3_t p1, const vec3_t p2)
{

	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	
	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( edict_s *ent, qboolean open ) {

}


/*
==================
SV_EntityContact
==================
*/
qboolean	SV_EntityContact( vec3_t mins, vec3_t maxs, const edict_s *gEnt, int capsule ) {

	return qfalse;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( edict_s *gEnts, int numGEntities, int sizeofGEntity_t,
					   playerState_s *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_s *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

static int	FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !sv_gameDLL ) {
		return;
	}
	g_game->ShutdownGame(qfalse);
	g_moduleMgr->unload(&sv_gameDLL);
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( qboolean restart ) {
	int		i;

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}
	
	// use the current msec count for a random seed
	// init for this gamestate
	g_game->InitGame(sv.time, Com_Milliseconds(), restart);
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !sv_gameDLL ) {
		return;
	}
	g_game->ShutdownGame(qtrue);

	// do a restart instead of a free
	sv_gameDLL = g_moduleMgr->restart(sv_gameDLL, qtrue);
	if ( !sv_gameDLL ) {
		Com_Error( ERR_FATAL, "VM_Restart on game failed" );
	}

	SV_InitGameVM( qtrue );
}

/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {
	// load the dll or bytecode
	sv_gameDLL = g_moduleMgr->load("qagame");
	if ( !sv_gameDLL ) {
		Com_Error( ERR_FATAL, "VM_Create on game failed" );
	}
	g_iFaceMan->registerIFaceUser(&g_game,GAME_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_gameClients,GAMECLIENTS_API_IDENTSTR);
	SV_InitGameVM( qfalse );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	//if ( sv.state != SS_GAME ) {
		return qfalse;
	//}
	// TODO
	//return VM_Call( gvm, GAME_CONSOLE_COMMAND );
}

