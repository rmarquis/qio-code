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
#include <api/serverAPI.h>
#include <api/physAPI.h>
#include <api/physObjectAPI.h>
#include <api/physCharacterControllerAPI.h>
#include <api/modelDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/entDefAPI.h>
#include <api/entityDeclAPI.h>
#include <api/stateMachineAPI.h>
#include <api/stateConditionsHandlerAPI.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <shared/animationFlags.h>
#include <shared/hashTableTemplate.h>
#include <shared/stringList.h>


static aCvar_c g_printPlayerPositions("g_printPlayerPositions","0");
static aCvar_c g_printPlayersHealth("g_printPlayersHealth","0");
static aCvar_c g_printPlayerWeaponState("g_printPlayerWeaponState","0");
static aCvar_c g_printPlayerPrimaryFireState("g_printPlayerPrimaryFireState","0");
static aCvar_c g_printPlayerSecondaryPrimaryFireState("g_printPlayerSecondaryPrimaryFireState","0");
static aCvar_c g_printPlayerLanding("g_printPlayerLanding","0");
static aCvar_c g_printPlayerStateChange("g_printPlayerStateChange","0");
static aCvar_c g_playerForceAnimation("g_playerForceAnimation","0");
static aCvar_c g_useTraceHit_printAreas("g_useTraceHit_printAreas","0");

DEFINE_CLASS(Player, "ModelEntity");

enum sharedGameAnim_e {
	SGA_BAD,
	SGA_IDLE,
	SGA_WALK,
	SGA_RUN,
	SGA_WALK_BACKWARDS,
	SGA_RUN_BACKWARDS,
	SGA_JUMP,
	SGA_DEATH,
	SGA_PAIN,
	SGA_PAIN_CHEST,
	SGA_PAIN_HEAD,
	SGA_PAIN_RIGHT_ARM,
	SGA_PAIN_LEFT_ARM,
	SGA_ATTACK,
};
const char *sharedGameAnimNames[] = {
	"bad", // SGA_BAD
	"idle", // SGA_IDLE
	"walk", // SGA_WALK
	"run", // SGA_RUN
	"walk_backwards", // etc...
	"run_backwards",
	"jump",
	"death", // FIXME
	"pain", // FIXME
	"pain_chest",
	"pain_head",
	"pain_right_arm",
	"pain_left_arm",
	"attack", // SGA_ATTACK
};

static u32 g_numSharedAnimNames = sizeof(sharedGameAnimNames) / sizeof(sharedGameAnimNames[0]);

static sharedGameAnim_e G_FindSharedAnim(const char *name) {
	for(u32 i = 0; i < g_numSharedAnimNames; i++) {
		const char *iName = sharedGameAnimNames[i];
		if(!_stricmp(iName,name)) {
			return (sharedGameAnim_e)i;
		}
	}
	return SGA_BAD;
}

class playerAnimControllerAPI_i {
public:
	virtual ~playerAnimControllerAPI_i() {
	}

	virtual void setGameEntity(class ModelEntity *ent) = 0;
	virtual void setAnimBoth(enum sharedGameAnim_e anim) = 0;
	virtual void setModelName(const char *newModelName) = 0;
	virtual void setAnimTorso(enum sharedGameAnim_e anim) = 0;
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
		} else if(anim == SGA_DEATH) {
			ctrlEnt->setInternalAnimationIndex(BOTH_DEATH1);
		}
	}
	virtual void setAnimTorso(enum sharedGameAnim_e anim) {

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
	virtual void setAnimTorso(enum sharedGameAnim_e anim) {
		str newAnimPath = animsDir;
		const char *animName = sharedGameAnimNames[anim];
		newAnimPath.append(animName);
		newAnimPath.append(".md5anim");
		ctrlEnt->setTorsoAnimation(newAnimPath);
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
	bLoopFire = false;
	bLoopFireRight = false;
	bLoopFireLeft = false;
	vehicle = 0;
	curWeapon = 0;
	animHandler = 0;
	bTakeDamage = true;
	weaponState = WP_NONE;
	lastPainTime = 0;
	st_legs = 0;
	st_torso = 0;
	bJumped = false;
	bLanding = false;
	groundDist = 0.f;
	st_handler = 0;
	st_curStateLegs = "STAND";
	st_curStateTorso = "STAND";
	bPutaway = false;
}
Player::~Player() {
	if(characterController) {
		g_physWorld->freeCharacter(this->characterController);
		characterController = 0;
	}
	if(curWeapon) {
		delete curWeapon;
	}
	if(animHandler) {
		delete animHandler;
	}
}

void Player::stopFiringWeapon(const char *handName) {
	if(bPutaway) {
		g_core->RedWarning("Player::stopFiringWeapon: ERROR, bPutaway flag set, can't stopFire during putaway!\n");
		return;
	}
	if(!stricmp(handName,"left") || !stricmp(handName,"lefthand")) {
		if(curWeaponLeft.getPtr() == 0) {
			g_core->RedWarning("Player::stopFiringWeapon: can't stopFire lefthand weapon because curWeaponLeft ptr is NULL\n");
		} else {
		}
		bLoopFireLeft = false;
	} else if(!stricmp(handName,"right") || !stricmp(handName,"righthand")) {
		if(curWeaponRight.getPtr() == 0) {
			g_core->RedWarning("Player::stopFiringWeapon: can't stopFire righthand weapon because curWeaponRight ptr is NULL\n");
		} else {
		}
		bLoopFireRight = false;
	} else {
		if(curWeapon.getPtr() == 0) {
			g_core->RedWarning("Player::stopFiringWeapon: can't stopFire dualhand weapon because curWeapon ptr is NULL\n");
		} else {
		}
		bLoopFire = false;
	}
}
void Player::fireWeapon(const char *handName) {
	if(bPutaway) {
		g_core->RedWarning("Player::deactivateWeapon: ERROR, bPutaway flag set, can't fire during putaway!\n");
		return;
	}
	if(!stricmp(handName,"left") || !stricmp(handName,"lefthand")) {
		if(curWeaponLeft.getPtr() == 0) {
			g_core->RedWarning("Player::fireWeapon: can't fire lefthand weapon because curWeaponLeft ptr is NULL\n");
		} else {
			curWeaponLeft->fireEvent();
			bLoopFireLeft = curWeaponLeft->getBLoopFire();
		}
	} else if(!stricmp(handName,"right") || !stricmp(handName,"righthand")) {
		if(curWeaponRight.getPtr() == 0) {
			g_core->RedWarning("Player::fireWeapon: can't fire righthand weapon because curWeaponRight ptr is NULL\n");
		} else {
			curWeaponRight->fireEvent();
			bLoopFireRight = curWeaponRight->getBLoopFire();
		}
	} else {
		if(curWeapon.getPtr() == 0) {
			g_core->RedWarning("Player::fireWeapon: can't fire dualhand weapon because curWeapon ptr is NULL\n");
		} else {
			curWeapon->fireEvent();
			bLoopFire = curWeapon->getBLoopFire();
		}
	}
}
void Player::deactivateWeapon(const char *handName) {
	if(bPutaway == false) {
		g_core->RedWarning("Player::deactivateWeapon: ERROR, bPutaway flag not set!\n");
	}
	bPutaway = false;
	if(!stricmp(handName,"left") || !stricmp(handName,"lefthand")) {
		if(curWeaponLeft.getPtr() == 0) {
			g_core->RedWarning("Player::deactivateWeapon: can't deactivate lefthand weapon because curWeaponLeft ptr is NULL\n");
		} else {
			curWeaponLeft->detachFromParent();
			curWeaponLeft->hideEntity();
		}
		curWeaponLeft = 0;
	} else if(!stricmp(handName,"right") || !stricmp(handName,"righthand")) {
		if(curWeaponRight.getPtr() == 0) {
			g_core->RedWarning("Player::deactivateWeapon: can't deactivate righthand weapon because curWeaponRight ptr is NULL\n");
		} else {
			curWeaponRight->detachFromParent();
			curWeaponRight->hideEntity();
		}
		curWeaponRight = 0;
	} else {
		if(curWeapon.getPtr() == 0) {
			g_core->RedWarning("Player::deactivateWeapon: can't deactivate dualhand weapon because curWeapon ptr is NULL\n");
		} else {
			curWeapon->detachFromParent();
			curWeapon->hideEntity();
		}
		curWeapon = 0;
	}
	nextWeapon = 0;
}
void Player::activateNewWeapon() {
	if(nextWeapon.getPtr() == 0) {
		g_core->RedWarning("Player::activateNewWeapon: ERROR, nextWeapon not set!\n");
		return;
	}
	if(nextWeaponHand == WH_LEFT) {
		curWeaponLeft = nextWeapon;
		curWeaponLeft->showEntity();
		curWeaponLeft->setParent(this,"tag_weapon_left");
	} else if(nextWeaponHand == WH_RIGHT) {
		curWeaponRight = nextWeapon;
		curWeaponRight->showEntity();
		curWeaponRight->setParent(this,"tag_weapon_right");
	} else if(nextWeaponHand == WH_DUALHAND) {
		curWeapon = nextWeapon;
		curWeapon->showEntity();
		curWeapon->setParent(this,curWeapon->getDualAttachToTag());
	} else {

	}
	nextWeapon = 0;
}
void Player::setKeyValue(const char *key, const char *value) {
 	if(!stricmp(key,"jumpxy")) {
		// called from .tik animation commands list
		if(this->characterController) {
			bJumped = this->characterController->tryToJump();
		} else {
			bJumped = false;
		}
	} else if(!stricmp(key,"jump")) {
		// called from .tik animation commands list
		if(this->characterController) {
			bJumped = this->characterController->tryToJump();
		} else {
			bJumped = false;
		}
	} else if(!stricmp(key,"activatenewweapon")) {
		// called from .tik animation commands list
		activateNewWeapon();
	} else if(!stricmp(key,"deactivateweapon")) {
		// called from .tik animation commands list
		deactivateWeapon(value);
	} else if(!stricmp(key,"fire")) {
		// called from .tik animatiom commands list
		// NOTE: it seems that Weapons with "loopfire" key should fire until "stopfire" event is received
		g_core->Print("Fire event on player\n");
		fireWeapon(value);
	} else if(!stricmp(key,"stopfire")) {
		// Called from state file exitCommands block
		g_core->Print("Stop event on player\n");
		stopFiringWeapon(value);
	} else {
		ModelEntity::setKeyValue(key,value);
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
		characterController->setCharacterVelocity(newVel);
	}
}
void Player::setVehicle(class VehicleCar *newVeh) {
	vehicle = newVeh;
	this->hideEntity();
	disableCharacterController();
}
void Player::loadStateMachineLegs(const char *fname) {
	st_legs = G_LoadStateMachine(fname);
}
void Player::loadStateMachineTorso(const char *fname) {
	st_torso = G_LoadStateMachine(fname);
	setTorsoAnimation(0); // FIXME
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
		this->ps.viewheight = 26; // so eye is 24+26 = 50 units above ground
	} else {
		animHandler = new qioPlayerAnimController_c;
		// For Dim's request and TCQB support - player size controlled by .def file
		entityDeclAPI_i *qioPlayerModelDef = g_declMgr->registerEntityDecl("qioPlayerModelCfg");
		if(qioPlayerModelDef) {
			float h = qioPlayerModelDef->getEntDefAPI()->getKeyFloat("capsuleHeight",30);
			float r = qioPlayerModelDef->getEntDefAPI()->getKeyFloat("capsuleRadius",19);
			this->createCharacterControllerCapsule(h,r);
			this->setCharacterControllerZOffset(qioPlayerModelDef->getEntDefAPI()->getKeyFloat("zOffset",30));
			this->ps.viewheight = qioPlayerModelDef->getEntDefAPI()->getKeyFloat("viewHeight",72);
		} else {
			// NOTE: shina models origin is on the ground, between its feet
			this->createCharacterControllerCapsule(48,19);
			this->setCharacterControllerZOffset(48);
			this->ps.viewheight = 82; // so eye is 82 units above ground
		}
	}
	animHandler->setGameEntity(this);
	animHandler->setModelName(newPlayerModelName);
}
bool Player::hasUserCmdForward() const {
	if(pers.cmd.forwardmove > 0)
		return true;
	return false;
}
bool Player::hasUserCmdBackward() const {
	if(pers.cmd.forwardmove < 0)
		return true;
	return false;
}
bool Player::hasUserCmdLeft() const {
	if(pers.cmd.rightmove < 0)
		return true;
	return false;
}
bool Player::hasUserCmdRight() const {
	if(pers.cmd.rightmove > 0)
		return true;
	return false;
}
bool Player::hasUserCmdDown() const {
	if(pers.cmd.upmove < 0)
		return true;
	return false;
}
bool Player::hasUserCmdUp() const {
	if(pers.cmd.upmove > 0)
		return true;
	return false;
}
void Player::cmdTestBulletAttack() {
	vec3_c p = getEyePos();
	vec3_c f = getForward();
	G_MultiBulletAttack(p,f,this,1,5.f,50.f);
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
		g_physWorld->freeCharacter(this->characterController);
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
	float h, r;
	if(this->cmod->isCapsule()) {
		cmCapsule_i *c = this->cmod->getCapsule();
		h = c->getHeight();
		r = c->getRadius();
	} else if(this->cmod->isBBMinsMaxs()) {
		aabb cb;
		cmod->getBounds(cb);
		vec3_c cbSizes = cb.getSizes();
		h = cbSizes.z;
		//r = sqrt(Square(cbSizes.x)+Square(cbSizes.y));
		r = cbSizes.x*0.5f;
		h -= r;
		characterControllerOffset.set(0,0,h*0.5f+r);
	} else {
		return;
	}
	g_physWorld->freeCharacter(this->characterController);
	this->characterController = g_physWorld->createCharacter(this->ps.origin+characterControllerOffset, h, r);
	if(this->characterController) {
		this->characterController->setCharacterEntity(this);
	}
}
#include "../bt_include.h"
void Player::createCharacterControllerCapsule(float cHeight, float cRadius) {
	if(cm == 0) {

		return;
	}
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
void Player::setPlayerAnimBoth(enum sharedGameAnim_e type) {
	if(animHandler) {
		animHandler->setAnimBoth(type);
	} else {
		if(type == SGA_RUN) {
			if(hasDeclAnimation("run")) {
				setAnimation("run");
			} else {
				setAnimation("walk");
			}
		} else if(type == SGA_PAIN) {
			setAnimation("pain");
		} else if(type == SGA_PAIN_CHEST) {
			setAnimation("pain_chest");
		} else if(type == SGA_PAIN_HEAD) {
			setAnimation("pain_head");
		} else if(type == SGA_PAIN_RIGHT_ARM) {
			setAnimation("pain_right_arm");
		} else if(type == SGA_PAIN_LEFT_ARM) {
			setAnimation("pain_left_arm");
		} else {
			setAnimation("idle");
		}
	}
}
void Player::setPlayerAnimTorso(enum sharedGameAnim_e type) {
	if(animHandler) {
		animHandler->setAnimTorso(type);
	} else {

	}
}
void Player::playPainAnimation(const char *newPainAnimationName, u32 animTime) {
	lastPainTime = level.time;
	curPainAnimationName = newPainAnimationName;
	if(modelDecl) {
		curPainAnimationTime = modelDecl->getAnimationTimeMSec(newPainAnimationName);
	}
}
bool Player::isPainAnimActive() const {
	if(level.time - lastPainTime > curPainAnimationTime)
		return false;
	if(curPainAnimationName.length() == 0)
		return false;
	return true;
}
void Player::runPlayerAnimation_gameCode() {
	userCmd_s *ucmd = &this->pers.cmd;
	// update animation
	//if(0) {
		//this->setAnimation("models/player/shina/attack.md5anim");
	//} else if(bLanding) {
	//	this->setAnimation("models/player/shina/run.md5anim");
	//} else
	if(isPainAnimActive()) {
		setPlayerAnimBoth(G_FindSharedAnim(curPainAnimationName.c_str()));
	} else if(bJumped) {
		setPlayerAnimBoth(SGA_JUMP);
		//this->setAnimation("models/player/shina/jump.md5anim");
	} else if(groundDist > 32.f) {
		setPlayerAnimBoth(SGA_JUMP);
		//this->setAnimation("models/player/shina/jump.md5anim");
	} else if(ucmd->hasMovement()) {
		if(ucmd->forwardmove < 82) {
			if(ucmd->forwardmove < 0) {
				if(ucmd->forwardmove < -82) {
					setPlayerAnimBoth(SGA_RUN_BACKWARDS);
					//this->setAnimation("models/player/shina/run_backwards.md5anim");
				} else {
					setPlayerAnimBoth(SGA_WALK_BACKWARDS);
					//this->setAnimation("models/player/shina/walk_backwards.md5anim");
				}
			} else {
				setPlayerAnimBoth(SGA_WALK);
				//this->setAnimation("models/player/shina/walk.md5anim");
			}
		} else {
			setPlayerAnimBoth(SGA_RUN);
			//this->setAnimation("models/player/shina/run.md5anim");
		}
	} else {
		setPlayerAnimBoth(SGA_IDLE);
		//this->setAnimation("models/player/shina/idle.md5anim");
	}
	//if(fireHeld) {
	if(weaponState == WP_FIRING) {
		setPlayerAnimTorso(SGA_ATTACK);
	} else {
		setPlayerAnimTorso(SGA_BAD);
	}
}

#if 0
class testPlayerConditionsHandler_c : public stateConditionsHandler_i {
	friend class Player;
	Player *p;
public:
	virtual bool hasAnim(const char *animName) const {
		return p->findAnimationIndex(animName) != -1;
	}
	virtual bool isConditionTrue(enum stConditionType_e conditionType, const char *conditionName, const class stringList_c *arguments, class patternMatcher_c *patternMatcher = 0) {
		bool res = false;
		if(!stricmp(conditionName,"DEFAULT"))
			res = true;	
		if(!stricmp(conditionName,"FORWARD"))
			res = p->hasUserCmdForward();
		if(!stricmp(conditionName,"BACKWARD"))
			res = p->hasUserCmdBackward();
		if(!stricmp(conditionName,"STRAFE_LEFT"))
			res = p->hasUserCmdLeft();
		if(!stricmp(conditionName,"STRAFE_RIGHT"))
			res = p->hasUserCmdRight();
		if(!stricmp(conditionName,"ONGROUND"))
			res = p->onGround;	
		if(!stricmp(conditionName,"FALLING"))
			res = !p->onGround && p->getLinearVelocity().z < 0;	
		if(!stricmp(conditionName,"CAN_MOVE_FORWARD"))
			res = true;	
		if(!stricmp(conditionName,"CAN_MOVE_BACKWARD"))
			res = true;	
		if(!stricmp(conditionName,"CAN_MOVE_RIGHT"))
			res = true;	
		if(!stricmp(conditionName,"CAN_MOVE_LEFT"))
			res = true;	
		//if(!stricmp(conditionName,"RUN"))
		//	res = !(p->pers.cmd.buttons & BUTTON_WALKING);	
		//if(!stricmp(conditionName,"JUMP"))
		//	res = p->bJumped;	
		
		if(conditionType == CT_NEGATE)
			return !res;
		return res;
	}
} g_playerConditionsHandler;
#endif

conditionFunction_s g_playerConditions [] = {
	// movement conditions
	GETFUNC("FALLING",Player::checkFalling)
	GETFUNC("ONGROUND",Player::checkOnGround)
	GETFUNC("FORWARD",Player::checkForward)
	GETFUNC("BACKWARD",Player::checkBackward)
	GETFUNC("STRAFE_LEFT",Player::checkStrafeLeft)
	GETFUNC("STRAFE_RIGHT",Player::checkStrafeRight)
	GETFUNC("RUN",Player::checkRun)
	GETFUNC("HAS_VELOCITY",Player::checkHasVelocity)
	GETFUNC("BLOCKED",Player::checkBlocked)
	GETFUNC("DUCKED_VIEW_IN_WATER",Player::checkDuckedViewInWater)
	GETFUNC("CROUCH",Player::checkCrouch)
	GETFUNC("CHECK_HEIGHT",Player::checkHeight)
	GETFUNC("JUMP",Player::checkJump)
	GETFUNC("LOOKING_UP",Player::checkLookingUp)
	GETFUNC("AT_LADDER",Player::checkAtLadder)

	
	// damage stuff
	GETFUNC("KILLED",Player::checkKilled)
	GETFUNC("PAIN",Player::checkPain)
	// anim conditions
	GETFUNC("ANIMDONE_LEGS",Player::checkAnimDoneLegs)
	GETFUNC("ANIMDONE_TORSO",Player::checkAnimDoneTorso)
	// weapon conditions
	GETFUNC("IS_WEAPON_ACTIVE",Player::checkIsWeaponActive)
	GETFUNC("IS_WEAPONCLASS_ACTIVE",Player::checkIsWeaponClassActive)
	GETFUNC("HAS_WEAPON",Player::checkHasWeapon)
	GETFUNC("NEW_WEAPON",Player::checkNewWeapon)
	GETFUNC("PUTAWAYMAIN",Player::checkPutawayMain)
	// IS_NEW_WEAPONCLASS handname weaponclassname
	// IS_NEW_WEAPON handname weaponname
	GETFUNC("IS_NEW_WEAPONCLASS",Player::checkIsNewWeaponClass)
	GETFUNC("IS_NEW_WEAPON",Player::checkIsNewWeapon)
	GETFUNC("ATTACK_PRIMARY",Player::checkAttackPrimary)
	GETFUNC("ATTACK_SECONDARY",Player::checkAttackSecondary)
	GETFUNC("IS_WEAPON_SEMIAUTO",Player::checkIsWeaponSemiAuto)
	GETFUNC("IS_WEAPON_READY_TO_FIRE",Player::checkIsWeaponReadyToFire)
	GETFUNC("IS_WEAPONCLASS_READY_TO_FIRE",Player::checkIsWeaponClassReadyToFire)
	GETFUNC("RELOAD",Player::checkReload)

	// for grenades
	// MIN_CHARGE_TIME_MET handname
	GETFUNC("MIN_CHARGE_TIME_MET",Player::checkMinChargeTimeMet)

	// generic conditions
	GETFUNC("CHANCE",Player::checkChance)
	

	// FAKK
	GETFUNC("PUTAWAYLEFT",Player::checkPutawayLeft)
	GETFUNC("PUTAWAYRIGHT",Player::checkPutawayRight)
	GETFUNC("PUTAWAYBOTH",Player::checkPutawayBoth)

	GETFUNC("IS_DUALWEAPON_READY_TO_FIRE",Player::checkIsDualhandWeaponReadyToFire)
	GETFUNC("HAS_AMMO",Player::checkHasAmmo)
	
	GETFUNC("CAN_MOVE_FORWARD",Player::returnTrue)
	GETFUNC("CAN_MOVE_RIGHT",Player::returnTrue)
	GETFUNC("CAN_MOVE_LEFT",Player::returnTrue)
	GETFUNC("CAN_MOVE_BACKWARD",Player::returnTrue)
	GETFUNC("FAKEPLAYERACTIVE",Player::returnFalse)
	GETFUNC("BLOCKED",Player::returnFalse)
	GETFUNC("CAN_FALL",Player::returnFalse)
	GETFUNC("ATTACKLEFT",Player::checkAttackLeft)
	GETFUNC("ATTACKRIGHT",Player::checkAttackRight)
	// what's the difference?
	GETFUNC("ATTACKLEFTBUTTON",Player::checkAttackLeft)
	GETFUNC("ATTACKRIGHTBUTTON",Player::checkAttackRight)




	
	{ 0, 0 }
};


conditionsTable_c g_playerConditionsTable(g_playerConditions,sizeof(g_playerConditions)/sizeof(g_playerConditions[0]));


// use .st files to select proper animation
// (FAKK / MOHAA way)
void Player::runPlayerAnimation_stateMachine() {
#if 0
	// simple old test - left for reference
	g_playerConditionsHandler.p = this;
	const char *next = st_legs->transitionState(st_curStateLegs,&g_playerConditionsHandler);
	if(next && next[0]) {
		st_curStateLegs = next;
	}
	const char *anim = st_legs->getStateLegsAnim(st_curStateLegs,&g_playerConditionsHandler);
	this->setAnimation(anim);
#else
	if(st_handler == 0) {
		st_handler = new genericConditionsHandler_t<Player>(&g_playerConditionsTable,this);
	}
	st_handler->updateFrame();
	stateMachineAPI_i *machines[2] = { st_legs, st_torso };
	const char *names[2] = { "legs" , "torso" };
	str *states[2] = { &st_curStateLegs, &st_curStateTorso };
	bool bChanged[2] = { false, false };
	for(u32 i = 0; i < 2; i++) {
		stateMachineAPI_i *st = machines[i];
		if(st == 0)
			continue;
		const char *name = names[i];
		str *state = states[i];
		///u32 maxStateChanges = 10;
		//for(u32 i = 0; i < maxStateChanges; i++) {
			const char *next = st->transitionState(*state,st_handler);
			if(next && next[0]) {
				if(g_printPlayerStateChange.getInt()) {
					g_core->Print("(Time %i): Changing %s state from %s to %s\n",level.time,name,state->c_str(),next);
				}
				// leaving state, so execute exit commnads
				st->iterateStateExitCommands(state->c_str(),this);
				*state = next;
				// entering state, so execute entry commnads
				st->iterateStateEntryCommands(state->c_str(),this);
				bChanged[i] = true;
		//		continue;
			}
		//	break;
		//}
	}
	if(st_torso) 
		stMoveType = st_torso->getStateMoveType(st_curStateTorso);
	else
		stMoveType = EMT_LEGS;

	if(stMoveType == EMT_LEGS) {
		// legs move type allows us to use animation from legs state
		const char *anim = st_legs->getStateLegsAnim(st_curStateLegs,st_handler);
		if(anim == 0 || anim[0] == 0) {
			g_core->Print("No animation found for state %s\n",st_curStateLegs.c_str());
			return;
		}
		///anim = "stand_to_ready";
		//anim = "ready_to_jump_up_to_rise";
		if(g_playerForceAnimation.getStr()[0] != '0') {
			anim = g_playerForceAnimation.getStr();
		}
		this->setAnimation(anim);
		if(st_torso == 0) {
			this->setTorsoAnimation(0);
		} else {
			if(bChanged[1]) {
				if(st_torso) {
					const char *torsoAnim = st_torso->getStateTorsoAnim(st_curStateTorso,st_handler);
					this->setTorsoAnimation(torsoAnim);
				}
			}
		}
	} else {
		const char *torsoAnim = st_torso->getStateLegsAnim(st_curStateTorso,st_handler);
		this->setAnimation(torsoAnim);
		this->setTorsoAnimation(0);
	}
#endif
}
void Player::runPlayer() {
	userCmd_s *ucmd = &this->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		g_core->Print("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		g_core->Print("serverTime >>>>>\n" );
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

	if(health <= 0) {
		// player must wait 1 second before respawning again
		if(level.time - lastDeathTime > 1000) {
			if(ucmd->buttons & BUTTON_ATTACK) {
				ClientSpawn(this->myEdict);
			}
		}
	} else {
		if(vehicle) {
			this->setOrigin(vehicle->getOrigin()+vec3_c(0,0,64.f));
			vehicle->steerUCmd(ucmd);
			//this->setClientViewAngle(vehicle->getAngles());
		} else {
			bJumped = false;
			bLanding = false;
			// update the viewangles
			ps.updateViewAngles(ucmd);
			{
				vec3_c v( 0, this->ps.viewangles[1], 0 );
				vec3_c dir;;
				if(isPainAnimActive()) {
					dir.clear();
				} else if(st_legs && stMoveType != EMT_LEGS) {
					dir.clear();
				} else {
					vec3_c f,r,u;
					//g_core->Print("Yaw %f\n",ent->client->ps.viewangles[1]);
					v.angleVectors(f,r,u);
					f *= level.frameTime*ucmd->forwardmove;
					r *= level.frameTime*ucmd->rightmove;
					u *= level.frameTime*ucmd->upmove;
					dir += f;
					dir += r;
					dir += u;
				}
				vec3_c newOrigin;
				if(noclip || (characterController==0)) {
					dir.scale(4.f);
					newOrigin = ps.origin + dir;
					ModelEntity::setOrigin(newOrigin);
					linearVelocity = ps.velocity = dir;
					onGround = false;
				} else {
					dir[2] = 0;
					//dir *= 0.75f;
					this->characterController->update(dir);
					newOrigin = this->characterController->getPos();
					linearVelocity = ps.velocity = (newOrigin - ps.origin)-characterControllerOffset;
					bool isNowOnGround = this->characterController->isOnGround();
					if(isNowOnGround) {
						if(ucmd->upmove) {
							// jumping when using .st machine is handled other way
							// There is a jump (jumpxy) event which should be
							// called from .st or .tik animation script.
							if(st_legs == 0) {
								bJumped = this->characterController->tryToJump();
							}
						}
						if(onGround == false) {
							bLanding = true;
							if(g_printPlayerLanding.getInt()) {
								g_core->Print("Player::runPlayer: LANDING (at %f %f %f)\n",ps.origin[0],ps.origin[1],ps.origin[2]);
							}
						}
					}
					onGround = isNowOnGround;
					ModelEntity::setOrigin(newOrigin-characterControllerOffset);
				}
				ps.angles.set(0,ps.viewangles[1],0);
				// add strafe correction to player model angles
				if(ucmd->forwardmove < 0) {
					if(ucmd->rightmove > 0)
						ps.angles.y += 45;
					else if(ucmd->rightmove < 0)
						ps.angles.y -= 45;
				} else {
					if(ucmd->rightmove > 0)
						ps.angles.y -= 45;
					else if(ucmd->rightmove < 0)
						ps.angles.y += 45;
				}
			}
			if(onGround == false) {
				trace_c tr;
				tr.setupRay(this->getOrigin()+characterControllerOffset,this->getOrigin()-vec3_c(0,0,32.f));
//				BT_TraceRay(tr);
				groundDist = tr.getTraveled();
				//g_core->Print("GroundDist: %f\n",groundDist);
				ps.groundEntityNum = ENTITYNUM_NONE;
			} else {
				groundDist = 0.f;
				ps.groundEntityNum = ENTITYNUM_WORLD; // fixme
			}
			if(st_legs) {
				runPlayerAnimation_stateMachine();
			} else {
				runPlayerAnimation_gameCode();
			}
		}

		if(carryingEntity) {
			vec3_c pos = carryingEntity->getRigidBody()->getRealOrigin();
			vec3_c neededPos = this->getEyePos() + this->getForward() * 60.f;
			vec3_c delta = neededPos - pos;

			carryingEntity->setLinearVelocity(carryingEntity->getLinearVelocity()*0.5f);
			carryingEntity->setAngularVelocity(carryingEntity->getAngularVelocity()*0.5f);
			carryingEntity->applyCentralImpulse(delta*50.f);

			//vec3_c anglesDelta = prevAngles - this->getViewAngles();
			//carryingEntity->applyTorque(
			//prevAngles = this->getViewAngles();
			//carryingEntity->setOrigin(neededPos);
			//carryingEntity->setAngles(this->getAngles() - carryingEntityRelAngles);
		}

		if(noclip == false && this->pers.cmd.buttons & BUTTON_USE_HOLDABLE) {
			if(useHeld) {
				//g_core->Print("Use held\n");
			} else {
				//g_core->Print("Use pressed\n");
				useHeld = true;
				onUseKeyDown();
			}
		} else {
			if(useHeld) {
				//g_core->Print("Use released\n");
				useHeld = false;
			}
		}
		// run TIKI loop fires
		if(curWeaponLeft && bLoopFireLeft && curWeaponLeft->canFireAgain()) {
			curWeaponLeft->fireEvent();
		}
		if(curWeaponRight && bLoopFireRight && curWeaponRight->canFireAgain()) {
			curWeaponRight->fireEvent();
		}
		if(curWeapon && bLoopFire && curWeapon->canFireAgain()) {
			curWeapon->fireEvent();
		}

		// hardcoded weapons system for old weapons (Quake 3, etc)
		if(this->pers.cmd.buttons & BUTTON_ATTACK) {
			if(fireHeld) {
				if(g_printPlayerPrimaryFireState.getInt())
					g_core->Print("Fire held\n");
				onFireKeyHeld();
			} else {
				if(g_printPlayerPrimaryFireState.getInt())
					g_core->Print("Fire pressed\n");
				fireHeld = true;
				onFireKeyDown();
			}
		} else {
			if(fireHeld) {
				if(g_printPlayerPrimaryFireState.getInt())
					g_core->Print("Fire released\n");
				fireHeld = false;
			}
		}
		if(this->pers.cmd.buttons & BUTTON_ATTACK_SECONDARY) {
			if(secondaryFireHeld) {
				if(g_printPlayerSecondaryPrimaryFireState.getInt())
					g_core->Print("Secondary fire held\n");
				onSecondaryFireKeyHeld();
			} else {
				if(g_printPlayerSecondaryPrimaryFireState.getInt())
					g_core->Print("Secondary fire pressed\n");
				secondaryFireHeld = true;
				onSecondaryFireKeyDown();
			}
		} else {
			if(secondaryFireHeld) {
				if(g_printPlayerSecondaryPrimaryFireState.getInt())
					g_core->Print("Secondary fire released\n");
				secondaryFireHeld = false;
				onSecondaryFireKeyUp();
			}
		}
	}

	if(ragdoll) {
		// update ragdoll (ModelEntity class)
		runPhysicsObject();
	}

	this->link();

	if(noclip == false) {
		touchTriggers();
	}
	if(st_legs == 0 && st_torso == 0) {
		// default weapon logic
		updatePlayerWeapon();
	} 
	updateAnimations();
	
#if 1
	if(curWeapon) {
		curWeapon->setOrigin(this->getEyePos());
		curWeapon->setAngles(this->getAngles());
	}
#endif

	if(g_printPlayerPositions.getInt()) {
		g_core->Print("Player::runPlayer: client %i is at %f %f %f\n",myEdict->s->number,myEdict->s->origin[0],myEdict->s->origin[1],myEdict->s->origin[2]);
	}
	if(g_printPlayersHealth.getInt()) {
		g_core->Print("Player::runPlayer: client %i health is %i\n",myEdict->s->number,this->health);
	}
	//if (g_smoothClients.integer) {
	//	BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, true );
	//}
	//else {
	//	BG_PlayerStateToEntityState( &this->ps, &myEdict->s, true );
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
		this->showEntity();
		return;
	}
	vec3_c eye = this->getEyePos();
	trace_c tr;
	vec3_c dir = ps.viewangles.getForward();
	tr.setupRay(eye,eye + dir * 96.f);
	if(G_TraceRay(tr,this)) {
		BaseEntity *hit = tr.getHitEntity();
		if(hit == 0) {
			g_core->Print("Player::onUseKeyDown: WARNING: null hit entity\n");
			return;
		}
		g_core->Print("Use trace hit classname %s, modelname %s, entnum %i\n",hit->getClassName(),hit->getRenderModelName(),hit->getEntNum());
		if(g_useTraceHit_printAreas.getInt()) {
			hit->printTouchingAreas();
		}
		if(hit->doUse(this) == false && hit->isDynamic()) {
			ModelEntity *me = dynamic_cast<ModelEntity*>(hit);
			this->pickupPhysicsProp(me);
		}
	}
}
void Player::setViewModelAnim(const char *animName, int animFlags) {
	ps.viewModelAnim = G_AnimationIndex(animName);
	ps.viewModelAnimFlags = animFlags;
}
void Player::onFireKeyHeld() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		if(weaponState == WP_IDLE) {
			// reload weapon if clip is empty
			if(curWeapon->hasEmptyClip()) {
				if(curWeapon->getReloadTime()) {
					weaponTime = level.time;
					weaponState = WP_RELOADING;
					setViewModelAnim("reload",ANIMFLAG_STOPATLASTFRAME);
				} else {
					curWeapon->fillClip(curWeapon->getClipSize());
				}
			} else {
				curWeapon->onFireKeyHeld();

				if(curWeapon->getDelayBetweenShots()) {
					weaponTime = level.time;
					weaponState = WP_FIRING;
					setViewModelAnim("fire",ANIMFLAG_STOPATLASTFRAME);
				}
			}
			// update playerState_s clip state info (for networking)
			updateCurWeaponClipSize();
		}
		return;
	}
}
void Player::onFireKeyDown() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		if(weaponState == WP_IDLE) {
			// reload weapon if clip is empty
			if(curWeapon->hasEmptyClip()) {
				if(curWeapon->getReloadTime()) {
					weaponTime = level.time;
					weaponState = WP_RELOADING;
					setViewModelAnim("reload",ANIMFLAG_STOPATLASTFRAME);
				} else {
					curWeapon->fillClip(curWeapon->getClipSize());
				}
			} else {
				curWeapon->onFireKeyDown();

				if(curWeapon->getDelayBetweenShots()) {
					weaponTime = level.time;
					weaponState = WP_FIRING;
					setViewModelAnim("fire",ANIMFLAG_STOPATLASTFRAME);
				}
			}
			// update playerState_s clip state info (for networking)
			updateCurWeaponClipSize();
		}
		return;
	}
}
void Player::onSecondaryFireKeyHeld() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		curWeapon->onSecondaryFireKeyHeld();
		return;
	}
}
void Player::onSecondaryFireKeyDown() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		curWeapon->onSecondaryFireKeyDown();
		return;
	}
}
void Player::onSecondaryFireKeyUp() {
	if(vehicle) {
		return;
	}
	if(curWeapon) {
		curWeapon->onSecondaryFireKeyUp();
		return;
	}
}
void Player::pickupPhysicsProp(class ModelEntity *ent) {
	if(carryingEntity) {
		return;
	}
	g_core->Print("Picked up %s\n",ent->getClassName());
	carryingEntity = ent;
	//carryingEntityRelAngles = this->getAngles() - carryingEntity->getAngles();
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
void Player::setClientViewAngle(const vec3_c &newAngles) {
	// set the delta angle
	for(u32 i = 0; i < 3; i++) {
		int cmdAngle = ANGLE2SHORT(newAngles[i]);
		this->ps.delta_angles[i] = cmdAngle - this->pers.cmd.angles[i];
	}
	// set the pitch/yaw view angles
	this->ps.viewangles = newAngles;
	// set the model angle - only yaw (turning left/right)
	this->ps.angles.set(0,newAngles[YAW],0);
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
#include <ctime>
void UTIL_GetCurrentTimeHM(char *out) {
	char tmp[16];
	// current date/time based on current system
	time_t now = time(0);
	tm *ltm = localtime(&now);
	if(ltm->tm_hour < 10) {
		sprintf(out,"0%i:",ltm->tm_hour);
	} else {
		sprintf(out,"%i:",ltm->tm_hour);
	}
	if(ltm->tm_min < 10) {
		sprintf(tmp,"0%i",ltm->tm_min);
	} else {
		sprintf(tmp,"%i",ltm->tm_min);
	}
	strcat(out,tmp);
}
void Player::cmdSay(const char *msg) {
	char buffer[8192];
	char timeBuff[64];
	UTIL_GetCurrentTimeHM(timeBuff);
	sprintf(buffer,"chat %s: %s: %s",timeBuff,this->getNetName(),msg);
	g_server->SendServerCommand(-1,buffer);
}
bool Player::canPickUpWeapon(class Weapon *newWeapon) {
	if(weaponState == WP_NONE)
		return true;
	if(weaponState == WP_IDLE)
		return true;
	return false;
}
void Player::updateCurWeaponAttachment() {
	if(curWeapon == 0) {
		ps.curWeaponEntNum = ENTITYNUM_NONE;
		ps.customViewRModelIndex = 0;
		ps.viewModelAngles.zero();
		ps.viewModelOffset.zero();
	} else {
		ps.curWeaponEntNum = curWeapon->getEntNum();
		if(curWeapon->hasCustomViewModel()) {
			ps.customViewRModelIndex = G_RenderModelIndex(curWeapon->getCustomViewModelName());
		} else {
			ps.customViewRModelIndex = 0;
		}
		curWeapon->setParent(this,getBoneNumForName("MG_ATTACHER"));
		curWeapon->setLocalAttachmentAngles(vec3_c(0,-90,-90));
		ps.viewModelAngles = -curWeapon->getViewModelAngles();
		ps.viewModelOffset = curWeapon->getViewModelOffset();
	}
	updateCurWeaponClipSize();
}
void Player::updateCurWeaponClipSize() {
	if(curWeapon == 0) {
		ps.viewWeaponCurClipSize = 0; 
		ps.viewWeaponMaxClipSize = 0; 
	} else {
		ps.viewWeaponCurClipSize = curWeapon->getCurrentClipSize(); 
		ps.viewWeaponMaxClipSize = curWeapon->getClipSize(); 
	}
}
void Player::addWeapon(class Weapon *newWeapon) {
	if(st_torso) {
		weapons.push_back(newWeapon);
		weaponHand_e wh = newWeapon->getWeaponHand();
		// see if we have a free hand
		if(wh == WH_DUALHAND) {
			// both must be free
			if(curWeapon || curWeaponLeft || curWeaponRight)
				return; 
		} else if(wh == WH_RIGHT) {
			// right must be free
			if(curWeapon || curWeaponRight)
				return; 
		} else if(wh == WH_LEFT) {
			// left must be free
			if(curWeapon || curWeaponLeft)
				return; 
		} else if(wh == WH_ANY) {
			// one of them must be free
			if(curWeapon)
				return; 
			if(curWeaponRight && curWeaponLeft)
				return; 
		}
		// unholster
		nextWeaponHand = newWeapon->getWeaponHand();
		if(nextWeaponHand == WH_ANY) {
			if(curWeaponRight) {
				nextWeaponHand = WH_LEFT;
			} else {
				nextWeaponHand = WH_RIGHT;
			}
		}
		nextWeapon = newWeapon;
		return;
	}
#if 0
	if(curWeapon) {
		dropCurrentWeapon();
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
		curWeapon->setParent(this,getBoneNumForName("tag_weapon"));
	}
#else
	if(weaponState == WP_NONE) {
		if(curWeapon.getPtr()) {
			g_core->RedWarning("Player::addWeapon: ERROR: weapon state is NONE but weapon pointer is present\n");
		}
		// immediatelly raise up new weapon
		weaponState = WP_RAISE;
		setViewModelAnim("raise",ANIMFLAG_STOPATLASTFRAME);
		curWeapon = newWeapon;
		updateCurWeaponAttachment();
		weaponTime = level.time;
	} else if(weaponState == WP_IDLE) {
		// putaway old weapon and then raise new one
		if(curWeapon.getPtr() == 0) {
			g_core->RedWarning("Player::addWeapon: ERROR: weapon state is IDLE but weapon pointer is NULL\n");
		}
		weaponState = WP_PUTAWAY;
		setViewModelAnim("putaway",ANIMFLAG_STOPATLASTFRAME);
		// we will bring new weapon up after putting this one away
		nextWeapon = newWeapon;
		weaponTime = level.time;
	}
#endif
}
void Player::holsterWeapon() {
	// can't holster weapon while holstering/raising another one
	if(nextWeapon.getPtr())
		return;
	// find weapon which we can hoslter
	if(curWeaponLeft.getPtr()) {
		nextWeapon = curWeaponLeft;
		nextWeaponHand = WH_LEFT;
		bPutaway = true;
		return;
	} else if(curWeaponRight.getPtr()) {
		nextWeapon = curWeaponRight;
		nextWeaponHand = WH_RIGHT;
		bPutaway = true;
		return;
	} else if(curWeapon.getPtr()) {
		nextWeapon = curWeapon;
		nextWeaponHand = WH_DUALHAND;
		bPutaway = true;
		return;
	}
	// unholster weapon
	if(weapons.size() == 0)
		return;
	nextWeaponHand = weapons[0]->getWeaponHand();
	if(nextWeaponHand == WH_ANY)
		nextWeaponHand = WH_RIGHT;
	nextWeapon = weapons[0];
}
void Player::updatePlayerWeapon() {
	if(weaponState == WP_NONE) {
		if(curWeapon.getPtr()) {
			g_core->RedWarning("Player::updatePlayerWeapon: weapon state is WP_NONE but weapon pointer is not 0\n");
		}
		return;
	}
	if(curWeapon.getPtr() == 0 && nextWeapon.getPtr() == 0) {
		g_core->RedWarning("Player::updatePlayerWeapon: weapon state is not WP_NONE but weapon pointer is NULL\n");
		return;
	}
	if(weaponState == WP_RAISE) {
		u32 elapsed = level.time - weaponTime;
		if(g_printPlayerWeaponState.getInt()) {
			g_core->Print("Player::updatePlayerWeapon: raising weapon (elapsed %i, needed %i)\n",elapsed,curWeapon->getRaiseTime());
		}
		if(elapsed > curWeapon->getRaiseTime()) {
			weaponState = WP_IDLE;
			setViewModelAnim("idle",0);
		}
	} else if(weaponState == WP_PUTAWAY) {
		u32 elapsed = level.time - weaponTime;
		if(g_printPlayerWeaponState.getInt()) {
			g_core->Print("Player::updatePlayerWeapon: puting away weapon (elapsed %i, needed %i)\n",elapsed,curWeapon->getPutawayTime());
		}
		if(elapsed > curWeapon->getPutawayTime()) {
			dropCurrentWeapon();
			// if we have next weapon waiting in queue, start raising it
			if(nextWeapon.getPtr()) {
				weaponState = WP_RAISE;
				setViewModelAnim("raise",ANIMFLAG_STOPATLASTFRAME);
				curWeapon = nextWeapon;
				updateCurWeaponAttachment();
				nextWeapon.nullPtr();
				weaponTime = level.time;
			} else {
				setViewModelAnim("none",0);
			}
		}
	} else if(weaponState == WP_FIRING) {
		u32 elapsed = level.time - weaponTime;
		if(g_printPlayerWeaponState.getInt()) {
			g_core->Print("Player::updatePlayerWeapon: firing weapon (elapsed %i, needed %i)\n",elapsed,curWeapon->getPutawayTime());
		}
		if(elapsed > curWeapon->getDelayBetweenShots()) {
			weaponState = WP_IDLE;
			setViewModelAnim("idle",0);
		}
	} else if(weaponState == WP_RELOADING) {
		u32 elapsed = level.time - weaponTime;
		if(g_printPlayerWeaponState.getInt()) {
			g_core->Print("Player::updatePlayerWeapon: RELOADING weapon (elapsed %i, needed %i)\n",elapsed,curWeapon->getPutawayTime());
		}
		if(elapsed > curWeapon->getReloadTime()) {
			weaponState = WP_IDLE;
			curWeapon->fillClip(curWeapon->getClipSize());
			setViewModelAnim("idle",0);
			// update playerState_s clip state info (for networking)
			updateCurWeaponClipSize();
		}
	} else {
	}
}
void Player::dropCurrentWeapon() {
	if(curWeapon == 0) {
		return;
	}
	//curWeapon->updateAttachmentOrigin();
	curWeapon->detachFromParent();
	vec3_c pos = this->getEyePos() + this->getForward() * 32.f;
	curWeapon->setOrigin(pos);
	curWeapon->setAngles(this->getAngles());
	curWeapon->initRigidBodyPhysics();
	curWeapon->setOwner(0);
	curWeapon = 0;
	ps.curWeaponEntNum = ENTITYNUM_NONE;
	ps.customViewRModelIndex = 0;	
	weaponState = WP_NONE;
}
void Player::postSpawn() {
	this->enableCharacterController();
}
void Player::onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damageCount) {
	// apply hit damage
	this->damage(damageCount);
	// add clientside effect		
	g_server->SendServerCommand(
		-1,va("doExplosionEffect %f %f %f %f %s",hitPosWorld.x,hitPosWorld.y,hitPosWorld.z,
		32.f,"bloodExplosion"));
}

void Player::onDeath() {
	if(health > 0) {
		health = -1; // ensure that this entity is dead
	}
	lastDeathTime = level.time;
	if(curWeapon) {
		this->dropCurrentWeapon();
	}
	this->disableCharacterController();

	// see if we can ragdol the player
	// TODO: make ragdoll stay after player respawn?
	if(ragdollDefName.length() && (ragdoll == 0)) {
		this->initRagdollPhysics();
	} else {
		// just play death animation
		if(animHandler) {
			animHandler->setAnimBoth(SGA_DEATH);
		}
	}
}



bool Player::checkFalling(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// we're not falling if we have a valid groundentitynum
	if(this->ps.groundEntityNum != ENTITYNUM_NONE)
		return false;
	// TODO - check distance to floor?
	if(this->ps.velocity.z > 0)
		return false;
	return true;
}
bool Player::checkOnGround(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// we're on ground if we have a groundentitynum
	if(this->ps.groundEntityNum != ENTITYNUM_NONE)
		return true;
	return false;
}
bool Player::checkForward(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdForward())
		return true;
	return false;
}
bool Player::checkBackward(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdBackward())
		return true;
	return false;
}
bool Player::checkStrafeRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdRight())
		return true;
	return false;
}
bool Player::checkStrafeLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdLeft())
		return true;
	return false;
}
bool Player::checkRun(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(!(pers.cmd.buttons & BUTTON_WALKING))
		return true;
	return false;
}
bool Player::checkHasVelocity(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(ps.velocity.len() > 1.f)
		return true;
	return false;
}
bool Player::checkBlocked(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return false;
}
bool Player::checkDuckedViewInWater(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return false;
}

bool Player::checkCrouch(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdDown())
		return true;
	return false;
}

bool Player::checkJump(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(hasUserCmdUp())
		return true;
	return false;
}
bool Player::checkAtLadder(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// TODO
	return false;
}
// this is used for ladders
bool Player::checkLookingUp(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(arguments->size() == 0) {
		return true;
	}
	const char *angleStr = arguments->getString(0);
	float angle = atof(angleStr);
	float viewAnglePitch = ps.viewangles.getX();
	if(viewAnglePitch > 180)
		viewAnglePitch -= 360;
	if(0) {
		g_core->Print("Player::checkLookingUp: angle %f, required minimum %f\n",-viewAnglePitch,angle);
	}
	if(-viewAnglePitch >= angle)
		return true;
	return false;
}

bool Player::checkHeight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return true;
}

bool Player::checkKilled(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(health <= 0)
		return true;
	return false;
}
bool Player::checkPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// TODO
	return false;
}
bool Player::checkAnimDoneTorso(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	int needed = getCurrentTorsoAnimationTotalTimeMs();
	if(torsoAnimationTime > needed) {
		if(0)
			g_core->Print("Anim done! %i > %i \n",torsoAnimationTime,needed);
		return true;
	}
	if(0) 
		g_core->Print("Anim not yet done... %i <= %i \n",torsoAnimationTime,needed);
	return false;
}
bool Player::checkAnimDoneLegs(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	int needed = getCurrentAnimationTotalTimeMs();
	if(legsAnimationTime > needed) {
		if(0)
			g_core->Print("Anim done! %i > %i \n",legsAnimationTime,needed);
		return true;
	}
	if(0) 
		g_core->Print("Anim not yet done... %i <= %i \n",legsAnimationTime,needed);
	return false;
}
bool Player::checkIsWeaponActive(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	const char *hand = arguments->getString(0);
	if(!stricmp(hand,"lefthand")) {
		// check if there is a weapon in lefthand
		if(curWeaponLeft.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeaponLeft->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	if(!stricmp(hand,"righthand")) {
		// check if there is a weapon in lefthand
		if(curWeaponRight.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeaponRight->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	if(!stricmp(hand,"dualhand")) {
		// check if there is a weapon in mainhand
		if(curWeapon.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeapon->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	return false;
}
bool Player::checkIsWeaponClassActive(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(curWeapon == 0)
		return false;
	// TODO
	return false;
}
bool Player::checkHasWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(curWeapon.getPtr() || curWeaponLeft.getPtr() || curWeaponRight.getPtr())
		return true;
	return false;
}
bool Player::checkNewWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(nextWeapon.getPtr())
		return true;
	return false;
}
bool Player::checkPutawayMain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(bPutaway == false)
		return false;
	if(nextWeaponHand == WH_DUALHAND)
		return true;
	return false;
}
bool Player::checkPutawayLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(bPutaway == false)
		return false;
	if(nextWeaponHand == WH_LEFT)
		return true;
	return false;
}
bool Player::checkPutawayBoth(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(bPutaway == false)
		return false;
	if(nextWeaponHand == WH_DUALHAND)
		return true;
	return false;
}
bool Player::checkHasAmmo(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	const char *handName = arguments->getString(0);
	// "primary" or "secondary"
	const char *fireType = arguments->getString(1);
	
	if(!stricmp(handName,"dualhand")) {
		if(curWeapon == 0)
			return false;
		if(curWeapon->hasAmmoForFireType(fireType))
			return true;
		return false;
	}
	if(!stricmp(handName,"righthand")) {
		if(curWeaponRight == 0)
			return false;
		if(curWeaponRight->hasAmmoForFireType(fireType))
			return true;
		return false;
	}
	if(!stricmp(handName,"lefthand")) {
		if(curWeaponLeft == 0)
			return false;
		if(curWeaponLeft->hasAmmoForFireType(fireType))
			return true;
		return false;
	}


	return false;
}
bool Player::checkIsDualhandWeaponReadyToFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(bPutaway)
		return false;
	if(curWeapon==0)
		return false;
	const char *fireMode = arguments->getString(0);

	if(arguments->size() > 1) {
		const char *weaponName = arguments->getString(1);
		if(stricmp(curWeapon->getWeaponName(),weaponName))
			return false;
	}
//	if(curWeapon->isReadyToFire()) {
		return true;
	//}
	return false;
}

bool Player::checkPutawayRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(bPutaway == false)
		return false;
	if(nextWeaponHand == WH_RIGHT)
		return true;
	return false;
}

bool Player::checkIsNewWeaponClass(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	// TODO
	return false;
}

bool Player::checkIsNewWeapon(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(nextWeapon.getPtr() == 0)
		return false;
	const char *hand = arguments->getString(0);
	const char *weapon = arguments->getString(1);
	if(!stricmp(hand,"lefthand")) {
		if(nextWeaponHand != WH_LEFT)
			return false;
	} else if(!stricmp(hand,"rightthand")) {
		if(nextWeaponHand != WH_RIGHT)
			return false;
	} else if(!stricmp(hand,"dualhand")) {
		if(nextWeaponHand != WH_DUALHAND)
			return false;
	}
	const char *checkName = nextWeapon->getWeaponName();
	if(!stricmp(checkName,weapon))
		return true;
	// TODO
	return false;
}
bool Player::checkAttackRight(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(pers.cmd.buttons & BUTTON_ATTACK_SECONDARY)
		return true;
	return false;
}
bool Player::checkAttackLeft(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(pers.cmd.buttons & BUTTON_ATTACK)
		return true;
	return false;
}
bool Player::checkAttackPrimary(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(pers.cmd.buttons & BUTTON_ATTACK)
		return true;
	return false;
}
bool Player::checkAttackSecondary(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(pers.cmd.buttons & BUTTON_ATTACK_SECONDARY)
		return true;
	return false;
}
bool Player::checkIsWeaponSemiAuto(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	// TODO
	return false;
}
bool Player::checkIsWeaponReadyToFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	const char *hand = arguments->getString(0);
	if(!stricmp(hand,"lefthand")) {
		// check if there is a weapon in lefthand
		if(curWeaponLeft.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeaponLeft->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	if(!stricmp(hand,"righthand")) {
		// check if there is a weapon in lefthand
		if(curWeaponRight.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeaponRight->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	if(!stricmp(hand,"dualhand") || !stricmp(hand,"mainhand")) {
		// check if there is a weapon in dualhand
		if(curWeapon.getPtr() == 0)
			return false;
		if(arguments->size() > 1) {
			const char *wpnName = arguments->getString(1);
			if(stricmp(wpnName,curWeapon->getWeaponName()))
				return false;
		}
		return true; // TODO
	}
	// TODO
	return false;
}
bool Player::checkIsWeaponClassReadyToFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	// TODO
	return false;
}
bool Player::checkReload(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	// TODO
	return false;
}
bool Player::checkMinChargeTimeMet(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	// TODO
	return false;
}
bool Player::checkChance(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	int val = atoi(arguments->getString(0));
	return (rand()%100) < val;
}
