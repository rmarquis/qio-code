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
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include <protocol/configStrings.h>
#include "cg_emitter_base.h"
#include <api/coreAPI.h>
#include <api/clientAPI.h>
#include <api/cvarAPI.h>
#include <api/cmAPI.h>
#include <api/rAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <api/declManagerAPI.h>
#include <shared/autoCvar.h>
#include <shared/autoCmd.h>
#include <shared/infoString.h>
#include <stdarg.h>

cg_t				cg;
cgs_t				cgs;
centity_s			cg_entities[MAX_GENTITIES];

aCvar_c		cg_lagometer("cg_lagometer","0");
aCvar_c		cg_drawFPS("cg_drawFPS","0");
aCvar_c		cg_draw2D("cg_draw2D","1");
aCvar_c		cg_fov("cg_fov","90");
aCvar_c		cg_thirdPersonRange("cg_thirdPersonRange","100");
aCvar_c		cg_thirdPersonAngle("cg_thirdPersonAngle","0");
aCvar_c		cg_thirdPerson("cg_thirdPerson","0");
aCvar_c		cg_timescale("timescale","1");
aCvar_c		cg_timescaleFadeEnd("cg_timescaleFadeEnd","1");
aCvar_c		cg_timescaleFadeSpeed("cg_timescaleFadeSpeed","0");

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	char var[64];
	// see if we are also running the server on this machine
	g_cvars->Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );
}
																															
void CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	g_core->Print( text );
}

void CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	g_core->DropError( text );
}

void Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	g_core->Error( level, text );
}

void Com_Printf( const char *msg, ... ) {
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

	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("CG_Init: clientNum %i\n",clientNum);
	}
	
	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= rf->registerMaterial( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= rf->registerMaterial( "white" );

	// init cgame console variables
	AUTOCVAR_RegisterAutoCvars();
	CG_RegisterCvars();
	// init console commands
	AUTOCMD_RegisterAutoConsoleCommands();

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

	s = CG_ConfigString(CS_WORLD_SKYMATERIAL);
	if(s && s[0]) {
		rf->setSkyMaterial(s);
	}
	s = CG_ConfigString(CS_WORLD_WATERLEVEL);
	if(s && s[0]) {
		rf->setWaterLevel(s);
	}
	s = CG_ConfigString(CS_WORLD_FARPLANE);
	if(s && s[0]) {
		cg.farPlane = atof(s);
	}
	CG_ParseServerinfo();

	// clear any references to old media
	rf->clearEntities();
	//trap_R_ClearScene();

	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("CG_Init: loading world map \"%s\"...",cgs.mapname);
	}
	// load world map first so inline models are present when requested
	rf->loadWorldMap( cgs.mapname );

	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("done.\n");
	}

	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("CG_Init: registering render models...");
	}
	u32 c_renderModelsLoaded = 0;
	for(u32 i = 0; i < MAX_MODELS; i++) {
		const char *str = CG_ConfigString(CS_MODELS+i);
		if(str && str[0]) {
			cgs.gameModels[i] = rf->registerModel(str);
			c_renderModelsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i rModels\n",c_renderModelsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering skins...");
	}
	u32 c_renderSkinsLoaded = 0;
	for(u32 i = 0; i < MAX_SKINS; i++) {
		const char *str = CG_ConfigString(CS_SKINS+i);
		if(str && str[0]) {
			//cgs.gameSkins[i] = rf->registerSkin(str);
			c_renderSkinsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i rSkins\n",c_renderSkinsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering animations...");
	}
	u32 c_animationsLoaded = 0;
	for(u32 i = 0; i < MAX_ANIMATIONS; i++) {
		const char *str = CG_ConfigString(CS_ANIMATIONS+i);
		if(str && str[0]) {
			cgs.gameAnims[i] = rf->registerAnimation_getAPI(str);
			c_animationsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i animations\n",c_animationsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering articulated figures decls...");
	}
	u32 c_afsLoaded = 0;
	for(u32 i = 0; i < MAX_RAGDOLLDEFS; i++) {
		const char *str = CG_ConfigString(CS_RAGDOLLDEFSS+i);
		if(str && str[0]) {
			cgs.gameAFs[i] = g_declMgr->registerAFDecl(str);
			c_afsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i AFs\n",c_afsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering collision models...");
	}
	u32 c_collisionModelsLoaded = 0;
	for(u32 i = 0; i < MAX_MODELS; i++) {
		const char *str = CG_ConfigString(CS_COLLMODELS+i);
		if(str && str[0]) {
			cgs.gameCollModels[i] = cm->registerModel(str);
			c_collisionModelsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i cmModels\n",c_collisionModelsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering materials...");
	}
	u32 c_renderMaterialsLoaded = 0;
	for(u32 i = 0; i < MAX_MATERIALS; i++) {
		const char *str = CG_ConfigString(CS_MATERIALS+i);
		if(str && str[0]) {
			cgs.gameMaterials[i] = rf->registerMaterial(str);
			c_renderMaterialsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i materials\n",c_renderMaterialsLoaded);
		g_loadingScreen->addLoadingString("CG_Init: registering sounds...");
	}
	u32 c_soundsLoaded = 0;
	for(u32 i = 0; i < MAX_SOUNDS; i++) {
		const char *str = CG_ConfigString(CS_SOUNDS+i);
		if(str && str[0]) {
//			cgs.gameSounds[i] = 0;//snd->registerSound(str);
			c_soundsLoaded++;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" %i sounds\n",c_soundsLoaded);
	}

//	cg.loading = false;	// future players will be deferred
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	for(u32 i = 0; i < MAX_GENTITIES; i++) {
		centity_s *cent = &cg_entities[i];
		if(cent->rEnt) {
			rf->removeEntity(cent->rEnt);
			cent->rEnt = 0;
		}
		if(cent->rLight) {
			rf->removeLight(cent->rLight);
			cent->rLight = 0;
		}
		if(cent->emitter) {
			delete cent->emitter;
			cent->emitter = 0;
		}
	}
	// shutdown test material
	CG_FreeTestMaterialClass();
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
	// unlink autocvars
	AUTOCVAR_UnregisterAutoCvars();
	// unlink autoCmds
	AUTOCMD_UnregisterAutoConsoleCommands();
}


