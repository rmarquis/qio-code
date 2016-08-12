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

#include "g_local.h"
#include <api/serverAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <api/cmAPI.h>
#include "classes/BaseEntity.h"
#include "classes/World.h"
#include <shared/autoCvar.h>
#include <shared/autoCmd.h>
#include <game/bg_public.h>
#include "g_pathNodes.h"

static aCvar_c g_printEntityPositions("g_printEntityPositions","0");
static aCvar_c g_printModelEntityRenderModelNames("g_printModelEntityRenderModelNames","0");
static aCvar_c g_printEntityClasses("g_printModelEntityRenderModelNames","0");

level_locals_t	level;

edict_s		g_entities[MAX_GENTITIES];


/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed, int restart ) {
	char mapName[128];

	g_core->Print ("------- Game Initialization -------\n");
	g_core->Print ("gamename: %s\n", GAME_VERSION);
	g_core->Print ("gamedate: %s\n", __DATE__);

	srand( randomSeed );

	// init autoCvars
	AUTOCVAR_RegisterAutoCvars();
	// init autoCmds
	AUTOCMD_RegisterAutoConsoleCommands();

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

//	for ( i=0 ; i<MAX_CLIENTS ; i++ ) {
////		g_entities[i].classname = "clientslot";
//	}

	//G_AnimationIndex("noanim");

	// let the server system know where the entites are
	g_server->LocateGameData( level.gentities, level.num_entities );

	// make some data visible to connecting client
	g_server->SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	g_server->SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	g_cvars->Cvar_Set( "g_gravity", "800" );

	g_cvars->Cvar_VariableStringBuffer("mapname",mapName,sizeof(mapName));

	G_InitScriptedClasses();

	// init bullet physics library
	G_InitPhysicsEngine();
	// load ammoTypes list from .def files	
	G_InitAmmoTypes();
	// init .st files manager (must be done before spawning entities)
	G_InitStateMachines();
	// load map for Bullet
	G_LoadMap(mapName);
	// init RTCW / ET scripts (must be done before spawning entities,
	// so spawn events can be called later)
	G_InitWolfScripts(mapName);
	// load map entities and spawn them
	G_SpawnMapEntities(mapName);
	// init pathnodes
	G_InitPathnodesSystem();
	
#ifdef G_ENABLE_LUA_SCRIPTING
	G_InitLua();
#endif

	//wsScript_c test;
	//test.loadScriptFile("maps/escape1.script");
	g_core->Print ("-----------------------------------\n");
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame( int restart ) {
	g_core->Print ("==== ShutdownGame ====\n");
	for(u32 i = 0; i < level.num_entities; i++) {
		edict_s *e = &g_entities[i];
		if(e->s == 0)
			continue;
		//g_core->Print("G_ShutdownGame: freeing entity %i (phys %i)\n",i,e->ent->getRigidBody());
		delete e->ent;
		e->ent = 0;
	}
#ifdef G_ENABLE_LUA_SCRIPTING
	G_ShutdownLua();
#endif
	G_ShutdownPhysicsEngine();
	G_ShutdownScriptedClasses();
	G_ShutdownPathnodesSystem();
	if(cm) {
		cm->freeAllModels();
	}
	if(g_declMgr) {
		g_declMgr->onGameShutdown();
	}
	AUTOCVAR_UnregisterAutoCvars();
	AUTOCMD_UnregisterAutoConsoleCommands();
}

void G_TestSafePtrs() {
	BaseEntity *ne = new BaseEntity;
	safePtr_c<BaseEntity> np = ne;
	delete ne;
	if(np.getPtr()) {
		g_core->DropError("G_TestSafePtrs: ERROR\n");
	}
	np = new BaseEntity;
	safePtr_c<BaseEntity> others[8];
	safePtr_c<BaseEntity> others2[8];
	for(u32 i = 0; i < 8; i++) {
		others[i] = np;
	}
	delete np;
	for(u32 i = 0; i < 8; i++) {
		if(others[i].getPtr()) {
			g_core->DropError("G_TestSafePtrs: ERROR\n");
		}
		if(others2[i].getPtr()) {
			g_core->DropError("G_TestSafePtrs: ERROR\n");
		}
	}
	for(u32 i = 0; i < 8; i++) {
		others[i] = new BaseEntity;
		np = others[i];
		others2[i] = np;
	}
	delete np;
	for(u32 i = 0; i < 8; i++) {
		if(others[i]) {
			delete others[i];
		}
	}
	for(u32 i = 0; i < 8; i++) {
		if(others[i].getPtr()) {
			g_core->DropError("G_TestSafePtrs: ERROR\n");
		}
		if(others2[i].getPtr()) {
			g_core->DropError("G_TestSafePtrs: ERROR\n");
		}
	}
}

void G_ProcessEntityEvents() {
	u32 c_numProcessedEvents = 0;
	edict_s	*ed = &g_entities[0];
	for(u32 i = 0; i < level.num_entities; i++, ed++) {
		if(ed->s == 0) {
			// edict is not active
			continue;
		}
		if(ed->ent == 0) {
			continue;
		}
		c_numProcessedEvents += ed->ent->processPendingEvents();
	}
}

/*
================
G_RunFrame
================
*/
void G_RunFrame( int levelTime ) {
	u32 i;
	edict_s	*ed;

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	level.frameTimeMs = (level.time-level.previousTime);
	level.frameTime = (level.time-level.previousTime)*0.001f;

	//G_TestSafePtrs();

	G_RunPhysics();
	G_ProcessEntityEvents();

	g_world.runWorldFrame();

	//
	// go through all allocated objects
	//
	ed = &g_entities[0];
	for (i = 0; i < level.num_entities; i++, ed++) {
		if (ed->s == 0) {
			// edict is not active
			continue;
		}
		// see if the entity number is correctly set (it should always be)
		if(ed->s->number != i) {
			g_core->RedWarning("G_RunFrame: fixing entity %i number\n",i);
			ed->s->number = i;
		}
		if (i < MAX_CLIENTS) {
			G_RunClient( ed );
			continue;
		}
		BaseEntity *e = ed->ent;

		if(g_printEntityPositions.getInt()) {
			g_core->Print("Entity %i (%s - %s) is at %f %f %f\n",e->getEntNum(),e->getClassName(),e->getRenderModelName(),e->getOrigin().x,e->getOrigin().y,e->getOrigin().z);
		}
		if(g_printModelEntityRenderModelNames.getInt()) {
			const char *modelName = e->getRenderModelName();
			if(modelName != 0 && modelName[0] != 0) {
				g_core->Print("Entity with model: %i (%s - %s) is at %f %f %f\n",e->getEntNum(),e->getClassName(),e->getRenderModelName(),e->getOrigin().x,e->getOrigin().y,e->getOrigin().z);
			}
		}
		if(g_printEntityClasses.getInt()) {
			g_core->Print("Entity: %i (%s - %s) is at %f %f %f\n",e->getEntNum(),e->getClassName(),e->getRenderModelName(),e->getOrigin().x,e->getOrigin().y,e->getOrigin().z);
		}
		e->runFrame();
		if(ed->ent == 0)
			continue; // fried during runFrame
		e->runWolfScript();
		if(ed->ent == 0)
			continue; // fried during runWolfScript
		e->runLuaFrameHandlers();
	}

	// perform final fixups on the players
	ed = &g_entities[0];
	for (i = 0; i < MAX_CLIENTS; i++, ed++ ) {
		if (ed->s == 0) {
			// edict is not active
			continue;
		}
		ClientEndFrame( ed );
	}

}
