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
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include <api/coreAPI.h>
#include <api/clientAPI.h>
#include <api/cvarAPI.h>
#include <api/cmAPI.h>
#include <api/rAPI.h>
#include <shared/autoCvar.h>

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];

vmCvar_t		cg_lagometer;
vmCvar_t		cg_drawFPS;
vmCvar_t		cg_draw2D;
vmCvar_t		cg_fov;
vmCvar_t		cg_thirdPersonRange;
vmCvar_t		cg_thirdPersonAngle;
vmCvar_t		cg_thirdPerson;
vmCvar_t		cg_timescale;
vmCvar_t		cg_timescaleFadeEnd;
vmCvar_t		cg_timescaleFadeSpeed;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
};

static int  cvarTableSize = ARRAY_LEN( cvarTable );

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		g_cvars->Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	g_cvars->Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );


}
																															
/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		g_cvars->Cvar_Update( cv->vmCvar );
	}

	// check for modications here


}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	g_core->Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	g_core->DropError( text );
}

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	g_core->Error( level, text );
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
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	g_core->ArgvBuffer( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	
	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= rf->registerMaterial( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= rf->registerMaterial( "white" );

	// init cgame console variables
	AUTOCVAR_RegisterAutoCvars();
	CG_RegisterCvars();

	// get the rendering configuration from the client system
	cgs.screenXScale = rf->getWinWidth() / 640.0;
	cgs.screenYScale = rf->getWinHeight() / 480.0;

	// get the gamestate from the client system
	g_client->GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();


	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	rf->clearEntities();
	//trap_R_ClearScene();

	// load world map first so inline models are present when requested
	rf->loadWorldMap( cgs.mapname );

	for(u32 i = 0; i < MAX_MODELS; i++) {
		const char *str = CG_ConfigString(CS_MODELS+i);
		if(str && str[0]) {
			cgs.gameModels[i] = rf->registerModel(str);
		}
	}
	for(u32 i = 0; i < MAX_MODELS; i++) {
		const char *str = CG_ConfigString(CS_COLLMODELS+i);
		if(str && str[0]) {
			cgs.gameCollModels[i] = cm->registerModel(str);
		}
	}
	for(u32 i = 0; i < MAX_SOUNDS; i++) {
		const char *str = CG_ConfigString(CS_SOUNDS+i);
		if(str && str[0]) {
			cgs.gameSounds[i] = 0;//snd->registerSound(str);
		}
	}

//	cg.loading = qfalse;	// future players will be deferred
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
	// unlink autocvars
	AUTOCVAR_UnregisterAutoCvars();
}


