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
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include <api/clientAPI.h>
#include <api/cmAPI.h>
#include <api/rAPI.h>
#include <api/vfsAPI.h>
#include <api/materialSystemAPI.h>
#include <api/declManagerAPI.h>
#include <shared/str.h>


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;

	info = CG_ConfigString( CS_SERVERINFO );

	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	mapname = Info_ValueForKey( info, "mapname" );
	// fix and format the world map name
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	if(g_vfs->FS_FileExists(cgs.mapname) == false) {
		// if there is no .bsp file, fall back to .proc (Doom3 .bsp tree with portals but without PVS)
		Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.proc", mapname );
		if(g_vfs->FS_FileExists(cgs.mapname) == false) {	
			// check for ET:QW proc binary
			Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.procb", mapname );
			if(g_vfs->FS_FileExists(cgs.mapname) == false) {
				// and finally, if there is no .proc file fall back to .map
				// (NOTE: .bsp files are a compiled .map files with some extra info
				// like lightmaps, lightgrid, PVS, etc...)
				Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.map", mapname );
			}
		}
	}

}


/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	g_client->GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if(num == CS_WORLD_SKYMATERIAL) {
		rf->setSkyMaterial(str);
	} else if(num == CS_WORLD_WATERLEVEL) {
		rf->setWaterLevel(str);
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = rf->registerModel( str );
	} else if ( num >= CS_ANIMATIONS && num < CS_ANIMATIONS+MAX_ANIMATIONS ) {
		cgs.gameAnims[ num-CS_ANIMATIONS ] = rf->registerAnimation_getAPI( str );
	} else if ( num >= CS_RAGDOLLDEFSS && num < CS_RAGDOLLDEFSS+MAX_RAGDOLLDEFS ) {
		cgs.gameAFs[ num-CS_RAGDOLLDEFSS ] = g_declMgr->registerAFDecl( str );
	} else if ( num >= CS_COLLMODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameCollModels[ num-CS_COLLMODELS ] = cm->registerModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		cgs.gameSounds[ num-CS_SOUNDS] = 0;//trap_S_RegisterSound( str, qfalse );
	} else if ( num >= CS_SKINS && num < CS_SKINS+MAX_SKINS ) {
		//cgs.gameSkins[ num-CS_SKINS] = rf->registerSkin(str);
	} else if ( num >= CS_MATERIALS && num < CS_MATERIALS+MAX_MATERIALS ) {
		cgs.gameMaterials[ num-CS_MATERIALS] = rf->registerMaterial(str);
	}
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {


//	cg.mapRestart = qtrue;



}

#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/rEntityAPI.h>

aCvar_c cg_debugDrawBulletAttack("cg_debugDrawBulletAttack","0");

static void CG_TestBulletAttack() {
	vec3_c p, d;
	str decalMaterialName = CG_Argv(1);
	p.x = atof(CG_Argv(2));
	p.y = atof(CG_Argv(3));
	p.z = atof(CG_Argv(4));
	d.x = atof(CG_Argv(5));
	d.y = atof(CG_Argv(6));
	d.z = atof(CG_Argv(7));
	int skipEntityNum = atoi(CG_Argv(8));
	CG_Printf("CG_TestBulletAttack: from %f %f %f, dir %f %f %f\n",p.x,p.y,p.z,d.x,d.y,d.z);
	trace_c tr;
	tr.setupRay(p,d*100000.f);
	if(CG_RayTrace(tr,skipEntityNum) == false) {
		CG_Printf("CG_TestBulletAttack: no hit\n");
		return; // no hit
	}
	//mtrAPI_i *decalMaterial = g_ms->registerMaterial("qiotests/testdecalmaterial");
	mtrAPI_i *decalMaterial = g_ms->registerMaterial(decalMaterialName);
	float radius = 8.f;
	centity_s *hit = tr.getHitCGEntity();
	if(hit == &cg_entities[ENTITYNUM_WORLD]) {
		CG_Printf("CG_TestBulletAttack: hit Worldspawn\n");
		if(cg_debugDrawBulletAttack.getInt()) {
			rf->addDebugLine(tr.getHitPos(),tr.getHitPos() + radius * tr.getHitPlaneNormal(),vec3_c(1,0,0),5.f);
		}
		rf->addWorldMapDecal(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		return;
	} else {
		CG_Printf("CG_TestBulletAttack: hit entity\n");
		if(hit->rEnt) {
			hit->rEnt->addDecalWorldSpace(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		} else {
			CG_Printf("CG_TestBulletAttack: hit centity has NULL rEnt\n");
		}
	}
}

static void CG_DoExplosionEffect() {
	vec3_c p;
	float radius;
	str matName;
	p.x = atof(CG_Argv(1));
	p.y = atof(CG_Argv(2));
	p.z = atof(CG_Argv(3));
	radius = atof(CG_Argv(4));
	matName = CG_Argv(5);

	rf->addExplosion(p,radius,matName);
}
static void CG_DoRailgunEffect() {
//	va("doRailgunEffect railCore railDisc railExplosion %f %f %f %f %f %f %i",muzzle.x,muzzle.y,muzzle.z,
	vec3_c p, d;
	str coreMaterial = CG_Argv(1);
	str discMaterial = CG_Argv(2);
	str explosionMaterial = CG_Argv(3);
	p.x = atof(CG_Argv(4));
	p.y = atof(CG_Argv(5));
	p.z = atof(CG_Argv(6));
	d.x = atof(CG_Argv(7));
	d.y = atof(CG_Argv(8));
	d.z = atof(CG_Argv(9));
	int skipEntityNum = atoi(CG_Argv(10));
	CG_Printf("CG_DoRailgunEffect: from %f %f %f, dir %f %f %f\n",p.x,p.y,p.z,d.x,d.y,d.z);
	trace_c tr;
	tr.setupRay(p,d*100000.f);
	if(CG_RayTrace(tr,skipEntityNum) == false) {
		CG_Printf("CG_DoRailgunEffect: no hit\n");
		return; // no hit
	}
	//mtrAPI_i *decalMaterial = g_ms->registerMaterial("qiotests/testdecalmaterial");
	mtrAPI_i *decalMaterial = g_ms->registerMaterial("gfx/damage/plasma_mrk");
	float radius = 32.f;
	centity_s *hit = tr.getHitCGEntity();
	if(hit == &cg_entities[ENTITYNUM_WORLD]) {
		CG_Printf("CG_DoRailgunEffect: hit Worldspawn\n");
		if(cg_debugDrawBulletAttack.getInt()) {
			rf->addDebugLine(tr.getHitPos(),tr.getHitPos() + radius * tr.getHitPlaneNormal(),vec3_c(1,0,0),5.f);
		}
		rf->addWorldMapDecal(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		return;
	} else {
		CG_Printf("CG_DoRailgunEffect: hit entity\n");
		if(hit->rEnt) {
			hit->rEnt->addDecalWorldSpace(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		} else {
			CG_Printf("CG_DoRailgunEffect: hit centity has NULL rEnt\n");
		}
	}
}
void CG_CreateDecal(const vec3_c &p, const vec3_c &dir, float radius, const char *matName) {
	trace_c tr;
	tr.setupRay(p,dir*100000.f);
	if(CG_RayTrace(tr,-1) == false) {
		CG_Printf("CG_DoRailgunEffect: no hit\n");
		return; // no hit
	}
	mtrAPI_i *decalMaterial = g_ms->registerMaterial(matName);
	centity_s *hit = tr.getHitCGEntity();
	if(hit == &cg_entities[ENTITYNUM_WORLD]) {
		CG_Printf("CG_CreateDecal: creating world decal\n");
		rf->addWorldMapDecal(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		return;
	} else {
		CG_Printf("CG_CreateDecal: creating entity decal\n");
		if(hit->rEnt) {
			hit->rEnt->addDecalWorldSpace(tr.getHitPos(),tr.getHitPlaneNormal(),radius,decalMaterial);
		} else {
			CG_Printf("CG_CreateDecal: hit centity has NULL rEnt\n");
		}
	}
}
static void CG_CreateDecalCommand() {
	vec3_c p, d;
	p.x = atof(CG_Argv(1));
	p.y = atof(CG_Argv(2));
	p.z = atof(CG_Argv(3));
	d.x = atof(CG_Argv(4));
	d.y = atof(CG_Argv(5));
	d.z = atof(CG_Argv(6));
	float radius = atof(CG_Argv(7));
	const char *matName = CG_Argv(8);
	CG_CreateDecal(p,d,radius,matName);
}


/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	} else if ( !strcmp( cmd, "print" ) ) {
		CG_Printf( "%s", CG_Argv(1) );
		return;
	} else if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	} else if ( !strcmp( cmd, "test_bulletAttack" ) ) {
		CG_TestBulletAttack();
		return;
	} else if ( !strcmp( cmd, "doExplosionEffect" ) ) {
		CG_DoExplosionEffect();
		return;
	} else if ( !strcmp( cmd, "doRailgunEffect" ) ) {
		CG_DoRailgunEffect();
		return;
	} else if ( !strcmp( cmd, "createDecal" ) ) {
		CG_CreateDecalCommand();
		return;
	}

	CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( g_client->GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
