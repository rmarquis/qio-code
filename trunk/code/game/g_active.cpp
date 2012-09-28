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

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	int			msec;
	usercmd_t	*ucmd;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}


	// update the viewangles
	PM_UpdateViewAngles( &ent->client->ps, ucmd );
	{
		vec3_t f,r,u;
		vec3_t v = { 0, ent->client->ps.viewangles[1], 0 };
		//G_Printf("Yaw %f\n",ent->client->ps.viewangles[1]);
		AngleVectors(v,f,r,u);
		VectorScale(f,level.frameTime*ucmd->forwardmove,f);
		VectorScale(r,level.frameTime*ucmd->rightmove,r);
		VectorScale(u,level.frameTime*ucmd->upmove,u);
		VectorAdd(ent->client->ps.origin,f,ent->client->ps.origin);
		VectorAdd(ent->client->ps.origin,r,ent->client->ps.origin);
		VectorAdd(ent->client->ps.origin,u,ent->client->ps.origin);
	}


	//if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	//}
	//else {
	//	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	//}

	client->ps.commandTime = ucmd->serverTime;
	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	//client->latched_buttons |= client->buttons & ~client->oldbuttons;
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );
}


void G_RunClient( gentity_t *ent ) {
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
}


