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
#include "VehicleCar.h"
#include "../g_local.h"
#include <api/cmAPI.h>

DEFINE_CLASS(Player, "ModelEntity");

Player::Player() {
	this->characterController = 0;
	memset(&ps,0,sizeof(ps));
	memset(&pers,0,sizeof(pers));
	buttons = 0;
	oldbuttons = 0;
	noclip = false;
	useHeld = false;
	vehicle = 0;
}
Player::~Player() {
	if(characterController) {
		BT_FreeCharacter(this->characterController);
		characterController = 0;
	}
}
void Player::setVehicle(class VehicleCar *newVeh) {
	vehicle = newVeh;
	disableCharacterController();
}
void Player::toggleNoclip() {
	noclip = !noclip;
	if(noclip) {
		disableCharacterController();
	} else {
		enableCharacterController();
	}
}
void Player::disableCharacterController() {
	if(characterController) {
		BT_FreeCharacter(this->characterController);
		characterController = 0;
	}
}
void Player::enableCharacterController() {
	if(this->cmod == 0) {
		return;
	}
	cmCapsule_i *c = this->cmod->getCapsule();
	float h = c->getHeight();
	float r = c->getRadius();
	BT_FreeCharacter(this->characterController);
	this->characterController = BT_CreateCharacter(8.f, this->ps.origin, h, r);
}
#include "../bt_include.h"
void Player::createCharacterControllerCapsule(float cHeight, float cRadius) {
	cmCapsule_i *m;
	m = cm->registerCapsule(48,16);
	this->setColModel(m);
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


	if(vehicle) {
		this->setOrigin(vehicle->getOrigin()+vec3_c(0,0,64.f));
		vehicle->steerUCmd(ucmd);
		//this->setClientViewAngle(vehicle->getAngles());
	} else {
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
			vec3_c dir(0,0,0);
			VectorAdd(dir,f,dir);
			VectorAdd(dir,r,dir);
			VectorAdd(dir,u,dir);
			vec3_c newOrigin;
			if(noclip) {
				dir.scale(4.f);
				VectorAdd(this->ps.origin,dir,newOrigin);
			} else {
				dir[2] = 0;
				VectorScale(dir,0.75f,dir);
				G_RunCharacterController(dir,this->characterController, newOrigin);
				if(ucmd->upmove) {
					G_TryToJump(this->characterController);
				}
			}
			ps.angles.set(0,ps.viewangles[1],0);
			ModelEntity::setOrigin(newOrigin);
		}
	}

	this->link();

	if(noclip == false && this->pers.cmd.buttons & BUTTON_USE_HOLDABLE) {
		if(useHeld) {
			//G_Printf("Use held\n");
		} else {
			//G_Printf("Use pressed\n");
			useHeld = true;
			onUseKeyDown();
		}
	} else {
		if(useHeld) {
			//G_Printf("Use released\n");
			useHeld = false;
		}
	}

	//G_Printf("at %f %f %f\n",ent->client->ps.origin[0],ent->client->ps.origin[1],ent->client->ps.origin[2]);

	//if (g_smoothClients.integer) {
	//	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	//}
	//else {
	//	BG_PlayerStateToEntityState( &this->ps, &myEdict->s, qtrue );
	//}

	this->ps.commandTime = ucmd->serverTime;
	// swap and latch button actions
	this->oldbuttons = this->buttons;
	this->buttons = ucmd->buttons;
	//client->latched_buttons |= client->buttons & ~client->oldbuttons;
}
#include <shared/trace.h>
void Player::onUseKeyDown() {
	if(this->vehicle) {
		this->vehicle->detachPlayer(this);
		this->setOrigin(this->getOrigin()+vec3_c(0,0,64));
		this->vehicle = 0;
		this->enableCharacterController();
		return;
	}
	vec3_c eye = this->getEyePos();
	trace_c tr;
	vec3_c dir;
	AngleVectors(this->ps.viewangles,dir,0,0);
	tr.setupRay(eye,eye + dir * 64.f);
	if(G_TraceRay(tr)) {
		BaseEntity *hit = tr.getHitEntity();
		if(hit == 0) {
			G_Printf("Player::onUseKeyDown: WARNING: null hit entity\n");
			return;
		}
		G_Printf("Use trace hit\n");
		hit->doUse(this);
	}
}
void Player::setClientViewAngle(const vec3_c &angle) {
	// set the delta angle
	for(u32 i = 0; i < 3; i++) {
		int cmdAngle = ANGLE2SHORT(angle[i]);
		this->ps.delta_angles[i] = cmdAngle - this->pers.cmd.angles[i];
	}
	// set the pitch/yaw view angles
	VectorCopy(angle, this->ps.viewangles);
	// set the model angle - only yaw (turning left/right)
	VectorSet(this->ps.angles,0,angle[YAW],0);
}
void Player::setNetName(const char *newNetName) {
	netName = newNetName;
}
const char *Player::getNetName() const {
	return netName;
}
int Player::getViewHeight() const {
	return this->ps.viewheight;
}
vec3_c Player::getEyePos() const {
	vec3_c ret = this->ps.origin;
	ret.z += this->ps.viewheight;
	return ret;
}
struct playerState_s *Player::getPlayerState() {
	return &this->ps;
}