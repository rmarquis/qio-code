/*
============================================================================
Copyright (C) 2012 V.

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
// Player.cpp - Game Client class
#include "Player.h"
#include "../g_local.h"
#include <api/cmAPI.h>

Player::Player() {
	this->characterController = 0;
	memset(&ps,0,sizeof(ps));
	memset(&pers,0,sizeof(pers));
	buttons = 0;
	oldbuttons = 0;
}

#include "../bt_include.h"
void Player::createCharacterControllerCapsule(float cHeight, float cRadius) {
	cmCapsule_i *m;
	cmod = m = cm->registerCapsule(48,16);
	float h = m->getHeight();
	float r = m->getRadius();
	BT_FreeCharacter(this->characterController);
	this->characterController = BT_CreateCharacter(8.f, this->ps.origin, h, r);
}

void Player::runPlayer(usercmd_s *ucmd) {
	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	int msec = ucmd->serverTime - this->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}


	// update the viewangles
	PM_UpdateViewAngles( &this->ps, ucmd );
	{
		vec3_t f,r,u;
		vec3_t v = { 0, this->ps.viewangles[1], 0 };
		//G_Printf("Yaw %f\n",ent->client->ps.viewangles[1]);
		AngleVectors(v,f,r,u);
		VectorScale(f,level.frameTime*ucmd->forwardmove,f);
		VectorScale(r,level.frameTime*ucmd->rightmove,r);
		VectorScale(u,level.frameTime*ucmd->upmove,u);
		vec3_t dir = {0,0,0};
		VectorAdd(dir,f,dir);
		VectorAdd(dir,r,dir);
		VectorAdd(dir,u,dir);
		if(0) {
			VectorAdd(this->ps.origin,dir,this->ps.origin);
		} else {
			dir[2] = 0;
			VectorScale(dir,0.75f,dir);
			G_RunCharacterController(dir,this->characterController, this->ps.origin);
			if(ucmd->upmove) {
				G_TryToJump(this->characterController);
			}
		}
	}

	//G_Printf("at %f %f %f\n",ent->client->ps.origin[0],ent->client->ps.origin[1],ent->client->ps.origin[2]);

	//if (g_smoothClients.integer) {
	//	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	//}
	//else {
		BG_PlayerStateToEntityState( &this->ps, &myEdict->s, qtrue );
	//}

	this->ps.commandTime = ucmd->serverTime;
	// swap and latch button actions
	this->oldbuttons = this->buttons;
	this->buttons = ucmd->buttons;
	//client->latched_buttons |= client->buttons & ~client->oldbuttons;
}
void Player::setClientViewAngle(const vec3_c &angle) {
	// set the delta angle
	for(u32 i = 0; i < 3; i++) {
		int cmdAngle = ANGLE2SHORT(angle[i]);
		this->ps.delta_angles[i] = cmdAngle - this->pers.cmd.angles[i];
	}
	VectorCopy( angle, myEdict->s.angles );
	VectorCopy (myEdict->s.angles, this->ps.viewangles);
}

struct playerState_s *Player::getPlayerState() {
	return &this->ps;
}