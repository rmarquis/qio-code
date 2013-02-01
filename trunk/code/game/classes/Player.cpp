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
#include "Weapon.h"
#include "../g_local.h"
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>

static aCvar_c g_printPlayerPositions("g_printPlayerPositions","0");

DEFINE_CLASS(Player, "ModelEntity");

enum sharedGameAnim_e {
	SGA_BAD,
	SGA_IDLE,
	SGA_WALK,
	SGA_RUN,
	SGA_WALK_BACKWARDS,
	SGA_RUN_BACKWARDS,
	SGA_JUMP,
};
const char *sharedGameAnimNames[] = {
	"bad",
	"idle",
	"walk",
	"run",
	"walk_backwards",
	"run_backwards",
	"jump",
};
class playerAnimControllerAPI_i {
public:
	virtual ~playerAnimControllerAPI_i() {
	}

	virtual void setGameEntity(class ModelEntity *ent) = 0;
	virtual void setAnimBoth(enum sharedGameAnim_e anim) = 0;
	virtual void setModelName(const char *newModelName) = 0;
	//virtual void setTorsoAnim(enum sharedGameAnim_e anim) = 0;
};
#include <shared/quake3Anims.h>
class q3PlayerAnimController_c : public playerAnimControllerAPI_i {
	ModelEntity *ctrlEnt;
	str modelName;
public:
	virtual ~q3PlayerAnimController_c() {
	}

	virtual void setGameEntity(class ModelEntity *ent) {
		ctrlEnt = ent;
	}
	virtual void setModelName(const char *newModelName) {
		modelName = newModelName;
	}
	virtual void setAnimBoth(enum sharedGameAnim_e anim) {
		if(anim == SGA_IDLE) {
			ctrlEnt->setInternalAnimationIndex(LEGS_IDLE);
		} else if(anim == SGA_WALK) {
			ctrlEnt->setInternalAnimationIndex(LEGS_WALK);
		} else if(anim == SGA_RUN) {
			ctrlEnt->setInternalAnimationIndex(LEGS_RUN);
		} else if(anim == SGA_JUMP) {
			ctrlEnt->setInternalAnimationIndex(LEGS_JUMP);
		} else if(anim == SGA_RUN_BACKWARDS) {
			ctrlEnt->setInternalAnimationIndex(LEGS_RUN);
		} else if(anim == SGA_WALK_BACKWARDS) {
			ctrlEnt->setInternalAnimationIndex(LEGS_WALK);
		}
	}

};

class qioPlayerAnimController_c : public playerAnimControllerAPI_i {
	ModelEntity *ctrlEnt;
	str modelName;
	str animsDir;
public:
	virtual ~qioPlayerAnimController_c() {
	}

	virtual void setGameEntity(class ModelEntity *ent) {
		ctrlEnt = ent;
	}
	virtual void setModelName(const char *newModelName) {
		modelName = newModelName;
		animsDir = modelName;
		animsDir.stripExtension();
		animsDir.toDir();
	}
	virtual void setAnimBoth(enum sharedGameAnim_e anim) {
		str newAnimPath = animsDir;
		const char *animName = sharedGameAnimNames[anim];
		newAnimPath.append(animName);
		newAnimPath.append(".md5anim");
		ctrlEnt->setAnimation(newAnimPath);
	}
};


Player::Player() {
	this->characterController = 0;
	memset(&pers,0,sizeof(pers));
	buttons = 0;
	oldButtons = 0;
	noclip = false;
	useHeld = false;
	fireHeld = false;
	onGround = false;
	vehicle = 0;
	curWeapon = 0;
	animHandler = 0;
}
Player::~Player() {
	if(characterController) {
		BT_FreeCharacter(this->characterController);
		characterController = 0;
	}
	if(curWeapon) {
		delete curWeapon;
	}
	if(animHandler) {
		delete animHandler;
	}
}
void Player::setOrigin(const vec3_c &newXYZ) {
	ModelEntity::setOrigin(newXYZ);
	if(characterController) {
#if 0
		BT_SetCharacterPos(characterController,newXYZ);
#else
		disableCharacterController();
		enableCharacterController();
#endif
	}
}
void Player::setLinearVelocity(const vec3_c &newVel) {
	if(characterController) {
		BT_SetCharacterVelocity(characterController,newVel);
	}
}
void Player::setVehicle(class VehicleCar *newVeh) {
	vehicle = newVeh;
	disableCharacterController();
}
void Player::setPlayerModel(const char *newPlayerModelName) {
	this->disableCharacterController();
	setRenderModel(newPlayerModelName);
	if(animHandler) {
		delete animHandler;
	}
	if(newPlayerModelName[0] == '$') {
		// QuakeIII three-part player model
		animHandler = new q3PlayerAnimController_c;
		this->setRenderModelSkin("default");
		// NOTE: Q3 player model origin is in the center of the model.
		// Model feet are at 0,0,-24
		float h = 30;
		this->createCharacterControllerCapsule(h,15);
		this->setCharacterControllerZOffset(h-24);
		this->ps.viewheight = 26;
	} else {
		animHandler = new qioPlayerAnimController_c;
		// NOTE: shina models origin is on the ground, between its feet
		this->createCharacterControllerCapsule(48,19);
		this->setCharacterControllerZOffset(48);
		this->ps.viewheight = 82;
	}
	animHandler->setGameEntity(this);
	animHandler->setModelName(newPlayerModelName);
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
void Player::setCharacterControllerZOffset(float ofs) {
	characterControllerOffset.set(0,0,ofs);
}
void Player::enableCharacterController() {
	if(this->cmod == 0) {
		return;
	}
	cmCapsule_i *c = this->cmod->getCapsule();
	float h = c->getHeight();
	float r = c->getRadius();
	BT_FreeCharacter(this->characterController);
	this->characterController = BT_CreateCharacter(20.f, this->ps.origin+characterControllerOffset, h, r);
}
#include "../bt_include.h"
void Player::createCharacterControllerCapsule(float cHeight, float cRadius) {
	cmCapsule_i *m;
	m = cm->registerCapsule(cHeight,cRadius);
	this->setColModel(m);
	enableCharacterController();
}
#include "Trigger.h"
void Player::touchTriggers() {
	arraySTD_c<class Trigger*> triggers;
	G_BoxTriggers(this->getAbsBounds(), triggers);
	for(u32 i = 0; i < triggers.size(); i++) {
		Trigger *t = triggers[i];
		t->onTriggerContact(this);
	}
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
		bool bJumped = false;
		bool bLanding = false;
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
				ModelEntity::setOrigin(newOrigin);
				onGround = false;
			} else {
				dir[2] = 0;
				VectorScale(dir,0.75f,dir);
				G_RunCharacterController(dir,this->characterController, newOrigin);
				bool isNowOnGround = BT_IsCharacterOnGround(this->characterController);
				if(isNowOnGround) {
					if(ucmd->upmove) {
						bJumped = G_TryToJump(this->characterController);
					}
					if(onGround == false) {
						bLanding = true;
						g_core->Print("Player::runPlayer: LANDING\n");
					}
				}
				onGround = isNowOnGround;
				ModelEntity::setOrigin(newOrigin-characterControllerOffset);
			}
			ps.angles.set(0,ps.viewangles[1],0);
		}
		float groundDist = 0.f;
		if(onGround == false) {
			trace_c tr;
			tr.setupRay(this->getOrigin()+characterControllerOffset,this->getOrigin()-vec3_c(0,0,32.f));
			BT_TraceRay(tr);
			groundDist = tr.getTraveled();
			//G_Printf("GroundDist: %f\n",groundDist);
		}
		//if(0) {
			//this->setAnimation("models/player/shina/attack.md5anim");
		//} else if(bLanding) {
		//	this->setAnimation("models/player/shina/run.md5anim");
		//} else
		if(bJumped) {
			animHandler->setAnimBoth(SGA_JUMP);
			//this->setAnimation("models/player/shina/jump.md5anim");
		} else if(groundDist > 32.f) {
			animHandler->setAnimBoth(SGA_JUMP);
			//this->setAnimation("models/player/shina/jump.md5anim");
		} else if(ucmd->hasMovement()) {
			if(ucmd->forwardmove < 82) {
				if(ucmd->forwardmove < 0) {
					if(ucmd->forwardmove < -82) {
						animHandler->setAnimBoth(SGA_RUN_BACKWARDS);
						//this->setAnimation("models/player/shina/run_backwards.md5anim");
					} else {
						animHandler->setAnimBoth(SGA_WALK_BACKWARDS);
						//this->setAnimation("models/player/shina/walk_backwards.md5anim");
					}
				} else {
					animHandler->setAnimBoth(SGA_WALK);
					//this->setAnimation("models/player/shina/walk.md5anim");
				}
			} else {
				animHandler->setAnimBoth(SGA_RUN);
				//this->setAnimation("models/player/shina/run.md5anim");
			}
		} else {
			animHandler->setAnimBoth(SGA_IDLE);
			//this->setAnimation("models/player/shina/idle.md5anim");
		}
	}

	if(carryingEntity) {
		vec3_c pos = carryingEntity->getOrigin();
		vec3_c neededPos = this->getEyePos() + this->getForward() * 60.f;
		vec3_c delta = neededPos - pos;

		carryingEntity->setLinearVelocity(carryingEntity->getLinearVelocity()*0.5f);
		carryingEntity->setAngularVelocity(carryingEntity->getAngularVelocity()*0.5f);
		carryingEntity->applyCentralImpulse(delta);
		//carryingEntity->setOrigin(neededPos);
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

	if(this->pers.cmd.buttons & BUTTON_ATTACK) {
		if(fireHeld) {
			G_Printf("Fire held\n");
			onFireKeyHeld();
		} else {
			G_Printf("Fire pressed\n");
			fireHeld = true;
			onFireKeyDown();
		}
	} else {
		if(fireHeld) {
			G_Printf("Fire released\n");
			fireHeld = false;
		}
	}

	if(noclip == false) {
		touchTriggers();
	}

#if 1
	if(curWeapon) {
		curWeapon->setOrigin(this->getEyePos());
		curWeapon->setAngles(this->getAngles());
	}
#endif

	if(g_printPlayerPositions.getInt()) {
		G_Printf("Player::runPlayer: client %i is at %f %f %f\n",myEdict->s->number,myEdict->s->origin[0],myEdict->s->origin[1],myEdict->s->origin[2]);
	}
	//if (g_smoothClients.integer) {
	//	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	//}
	//else {
	//	BG_PlayerStateToEntityState( &this->ps, &myEdict->s, qtrue );
	//}

	this->ps.commandTime = ucmd->serverTime;
	// swap and latch button actions
	this->oldButtons = this->buttons;
	this->buttons = ucmd->buttons;
	this->latchedButtons |= this->buttons & ~this->oldButtons;
}
#include <shared/trace.h>
void Player::onUseKeyDown() {
	if(this->isCarryingEntity()) {
		this->dropCarryingEntity();
		return;
	}
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
	tr.setupRay(eye,eye + dir * 96.f);
	if(G_TraceRay(tr,this)) {
		BaseEntity *hit = tr.getHitEntity();
		if(hit == 0) {
			G_Printf("Player::onUseKeyDown: WARNING: null hit entity\n");
			return;
		}
		G_Printf("Use trace hit\n");
		if(hit->doUse(this) == false && hit->isDynamic()) {
			ModelEntity *me = dynamic_cast<ModelEntity*>(hit);
			this->pickupPhysicsProp(me);
		}
	}
}
void Player::onFireKeyHeld() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		curWeapon->onFireKeyHeld();
		return;
	}
}
void Player::onFireKeyDown() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		curWeapon->onFireKeyDown();
		return;
	}
	//G_BulletAttack(this->getEyePos(),this->ps.viewangles.getForward());
}
void Player::pickupPhysicsProp(class ModelEntity *ent) {
	if(carryingEntity) {
		return;
	}
	g_core->Print("Picked up %s\n",ent->getClassName());
	carryingEntity = ent;
}
bool Player::isCarryingEntity() const {
	if(carryingEntity.getPtr()) {
		return true;
	}
	return false;
}
void Player::dropCarryingEntity() {
	if(carryingEntity == 0)
		return;
	g_core->Print("Player::dropCarryingEntity: dropping %s\n",carryingEntity->getClassName());
	carryingEntity = 0;
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
void Player::addWeapon(class Weapon *newWeapon) {
#if 1
	if(curWeapon) {
		//dropCurWeapon();
	}
	curWeapon = newWeapon;
	if(curWeapon == 0) {
		ps.curWeaponEntNum = ENTITYNUM_NONE;
		ps.customViewRModelIndex = 0;
	} else {
		ps.curWeaponEntNum = curWeapon->getEntNum();
		if(curWeapon->hasCustomViewModel()) {
			ps.customViewRModelIndex = G_RenderModelIndex(curWeapon->getCustomViewModelName());
		} else {
			ps.customViewRModelIndex = 0;
		}
		curWeapon->setParent(this,getBoneNumForName("MG_ATTACHER"));
	}
#else

#endif
}