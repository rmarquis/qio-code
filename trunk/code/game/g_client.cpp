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
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include <math/vec3.h>
#include "classes/ModelEntity.h"
#include "classes/Player.h"
#include "classes/VehicleCar.h"

// g_client.c -- client functions that don't happen every frame

//======================================================================

/*
================
ClientRespawn
================
*/
void ClientRespawn( edict_s *ent ) {


	ClientSpawn(ent);
}

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum ) {
	Player *pl = dynamic_cast<Player*>(g_entities[clientNum].ent);

	char buf[BIG_INFO_STRING];
	g_server->GetUserinfo(clientNum,buf,sizeof(buf));
	const char *s = Info_ValueForKey(buf, "name");
	if(s == 0) {
		pl->setNetName("UnnamedPlayer");
	} else {
		pl->setNetName(s);
	}
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
const char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	Player *pl;
	edict_s	*ent;

	ent = &g_entities[ clientNum ];

	if(ent->ent) {
		G_Printf(S_COLOR_YELLOW"ClientBegin: freeing old player class\n");
		delete ent->ent;
	}
	// create a player class for given edict
	BE_SetForcedEdict(ent);
	ent->ent = pl = new Player;

	// they can connect
	pl->pers.connected = CON_CONNECTING;

	// get and distribute relevent paramters
	G_Printf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		g_server->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", pl->getNetName()) );
	}

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	edict_s	*ent;

	ent = g_entities + clientNum;

	if(ent->ent == 0) {
		// this should never happen
		g_core->DropError("ClientBegin on edict without entity!");
		return;
	}

	Player *pl = dynamic_cast<Player*>(ent->ent);

	pl->pers.connected = CON_CONNECTED;
	pl->pers.enterTime = level.time;

	// locate ent at a spawn point
	ClientSpawn( ent );

	g_server->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", pl->getNetName()) );

	G_Printf( "ClientBegin: %i\n", clientNum );

}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(edict_s *ent) {
	int		index;
	vec3_c	spawn_origin, spawn_angles;
	char	userinfo[MAX_INFO_STRING];

	index = ent - g_entities;

	if(ent->ent == 0) {
		// this should never happen
		g_core->DropError("ClientSpawn on edict without entity!");
		return;
	}
	Player *pl = dynamic_cast<Player*>(ent->ent);

	BaseEntity *spawnPoint = G_GetRandomEntityOfClass("InfoPlayerStart");

	VectorClear(spawn_origin);
	VectorClear(spawn_angles);
	if(spawnPoint) {
		spawn_origin = spawnPoint->getOrigin();
		//spawn_origin.z += 128;
	}
	//	VectorSet(spawn_origin,1400,1340,470);

	VectorClear(pl->ps.delta_angles);
	VectorClear(pl->ps.velocity);


	g_server->GetUserinfo( index, userinfo, sizeof(userinfo) );
	// set max health
	pl->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( pl->pers.maxHealth < 1 || pl->pers.maxHealth > 100 ) {
		pl->pers.maxHealth = 100;
	}

	pl->ps.clientNum = index;

	VectorCopy( spawn_origin, pl->ps.origin );
	pl->ps.viewheight = 26;

	g_server->GetUsercmd( index, &pl->pers.cmd );
	pl->setClientViewAngle(spawn_angles );
	// don't allow full run speed for a bit

	pl->createCharacterControllerCapsule(48,16);

	// run a pl frame to drop exactly to the floor,
	// initialize animations and other things
	pl->ps.commandTime = level.time - 100;
	pl->pers.cmd.serverTime = level.time;
	ClientThink( index );
	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	//BG_PlayerStateToEntityState( &pl->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	edict_s	*ent;

	ent = g_entities + clientNum;
	if (!ent->ent) {
		return;
	}
	//// ||
	//Player *pl = dynamic_cast<Player*>(ent->ent);
	//if(pl->pers.connected == CON_DISCONNECTED) {
	//	return;
	//}

	if(ent->ent) {
		delete ent->ent;
		ent->ent = 0;
	}

//ent->classname = "disconnected";
	//ent->client->pers.connected = CON_DISCONNECTED;

#ifdef CS_PLAYERS
	g_server->SetConfigstring( CS_PLAYERS + clientNum, "");
#endif
}

void ClientCommand( int clientNum ) {
	Player *pl = (Player*)g_entities[clientNum].ent;
	if(pl == 0) {
		g_core->Print("ClientCommand: client %i is NULL\n",clientNum);
		return;
	}

	const char *cmd = g_core->Argv(0);
	if(!stricmp(cmd,"noclip")) {
		pl->toggleNoclip();
	} else if(!stricmp(cmd,"shootbox")) {
		vec3_c p = pl->getOrigin();
		p.z += pl->getViewHeight();
		p += pl->getForward() * 32.f;
		BT_CreateBoxEntity(p,vec3_c(16,16,16),pl->getForward());
	} else if(!stricmp(cmd,"createkubelwagen") || !stricmp(cmd,"createporshe")) {
		VehicleCar *veh = new VehicleCar;
		veh->spawnPhysicsVehicle();
		if(!stricmp(cmd,"createkubelwagen")) {
			veh->setRenderModel("models/vehicles/kubeldakwre/kubeldakwre.obj");
		} else {
			veh->setRenderModel("models/vehicles/Porsche_911/porsche-911.obj");
		}
	} else {
		vec3_c tmp(1400,1340,470);
		//BT_CreateVehicle(tmp);
		BT_CreateBoxEntity(tmp + vec3_c(0,0,128),vec3_c(16,16,16),0);
	}
}

playerState_s dummy;
struct playerState_s *ClientGetPlayerState(u32 clientNum) {
	edict_s *ed = &g_entities[clientNum];
	if(ed->s == 0) {
		g_core->Print(S_COLOR_RED"ClientGetPlayerState: edict %i isnt activeb (has NULL state)\n",clientNum);
		memset(&dummy,0,sizeof(dummy));
		return &dummy;
	}
	Player *pl = dynamic_cast<Player*>(ed->ent);
	if(pl == 0) {
		g_core->Print(S_COLOR_RED"ClientGetPlayerState: can't get Player class of edict %i\n",clientNum);
		memset(&dummy,0,sizeof(dummy));
		return &dummy;
	}
	return pl->getPlayerState();
}

