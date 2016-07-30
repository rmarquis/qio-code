/*
============================================================================
Copyright (C) 2013 V.

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
// Actor.cpp
#include "Actor.h"
#include "Player.h"
#include "../g_local.h"
#include <api/cmAPI.h>
#include <api/serverAPI.h>
#include <api/coreAPI.h>
#include <api/physAPI.h>
#include <api/physObjectAPI.h>
#include <api/physCharacterControllerAPI.h>
#include <api/stateMachineAPI.h>
#include <api/stateConditionsHandlerAPI.h>
#include <shared/stringList.h>
#include <shared/autoCvar.h>
#include <shared/parser.h>
#include <shared/trace.h>

DEFINE_CLASS(Actor, "ModelEntity");
 
static aCvar_c g_actor_st_debugAnimDone("g_actor_st_debugAnimDone","0");
static aCvar_c g_actor_debugCanSee("g_actor_debugCanSee","0");
static aCvar_c g_actor_printStateChange("g_actor_printStateChange","0");
static aCvar_c g_actor_printActiveStateFiles("g_actor_printActiveStateFiles","0");


conditionFunction_s g_actorConditions [] = {

	GETFUNC("ANIM_DONE",Actor::checkAnimDone)
	GETFUNC("MOVING_ACTOR_RANGE",Actor::checkMovingActorRange)
	GETFUNC("CHANCE",Actor::checkChance)
	GETFUNC("TIME_DONE",Actor::checkTimeDone)
	GETFUNC("NAME",Actor::checkName)
	GETFUNC("HAVE_ENEMY",Actor::checkHaveEnemy)
	GETFUNC("CAN_SEE_ENEMY",Actor::checkCanSeeEnemy)
	GETFUNC("RANGE",Actor::checkRange)
	GETFUNC("DONE",Actor::checkDone)
	GETFUNC("PAIN",Actor::checkPain)
	GETFUNC("SMALL_PAIN",Actor::checkSmallPain)
	GETFUNC("MELEE_HIT",Actor::checkMeleeHit)
	GETFUNC("IN_WATER",Actor::checkInWater)
	GETFUNC("HELD",Actor::checkHeld)
	// MeansOfDeath - type of damage
	GETFUNC("MOD",Actor::checkMOD)
	
	GETFUNC("ON_FIRE",Actor::checkOnFire)
	

	// is this the same?
	GETFUNC("BEHAVIOR_DONE",Actor::checkDone)


	
	GETFUNC("CAN_JUMP_TO_ENEMY",Actor::checkJumpToEnemy)
	GETFUNC("CAN_SHOOT_ENEMY",Actor::checkCanShootEnemy)
	// is health lower than given value
	GETFUNC("HEALTH",Actor::checkHealth)
	GETFUNC("ENEMY_RELATIVE_YAW",Actor::checkEnemyRelativeYaw)
	GETFUNC("ALLOW_HANGBACK",Actor::checkAllowHangBack)
	GETFUNC("ON_GROUND",Actor::checkOnGround)
	
	{ 0, 0 }
};


conditionsTable_c g_actorConditionsTable(g_actorConditions,sizeof(g_actorConditions)/sizeof(g_actorConditions[0]));

class bhBase_c {
protected:
	Actor * const self;
	float timeElapsed;
public:
	bhBase_c(Actor *a) : self(a) {
		timeElapsed = 0.f;
	}
	virtual void advanceTime(float f) {
		timeElapsed += f;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		return true;
	}	
	virtual const char *getAnimName() const {
		return 0;
	}
	virtual bool isDone() const {
		return false;
	}
	virtual ~bhBase_c() {

	}
};
// Left
// behavior turn "1" 
// Right
// behavior turn "-1"
// AimAndMelee behaviour
// just a fast experiment
class bhTurn_c : public bhBase_c {
	float arg;
public:
	bhTurn_c(Actor *self, const char *args) : bhBase_c(self) {
		parser_c p(args);
		arg = p.getFloat();
	}
	virtual void advanceTime(float f) {
		bhBase_c::advanceTime(f);
		vec3_c a = self->getAngles();
		a.y += f * arg;
		self->setAngles(a);
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		o.clear();
		return true;
	}
	virtual bool isDone() const {
		return false;
	}
	virtual ~bhTurn_c() {

	}
};
// behavior TurnToEnemy "turnleft" "turnright" "3" "1"
// just a fast experiment
class bhTurnToEnemy_c : public bhBase_c {
	str animLeft, animRight;
	float arg, arg2;
	bool bReached;
public:
	bhTurnToEnemy_c(Actor *self, const char *args) : bhBase_c(self) {
		parser_c p(args);
		animLeft = p.getToken();
		animRight = p.getToken();
		arg = p.getFloat();
		arg2 = p.getFloat();
		bReached = false;
	}
	virtual void advanceTime(float f) {
		bhBase_c::advanceTime(f);
		float step = f * 10.f; // FIXME
		ModelEntity *e = self->getEnemy();
		if(e) {
			vec3_c d = e->getOrigin() - self->getOrigin();
			d.setZ(0);
			float neededYaw = d.getYaw();
			vec3_c a = self->getAngles();
			float curYaw = a.y;
			neededYaw = AngleNormalize360(neededYaw);
			curYaw = AngleNormalize360(curYaw);
			float delta = neededYaw - curYaw;
			if(delta > step) {
				self->setAnimation(animLeft);
				delta = step;
			} else if(delta < -step) {
				self->setAnimation(animRight);
				delta = -step;
			} else {
				bReached = true;
			}
			curYaw += delta;
			a.y = curYaw;
			self->setAngles(a);
		}
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		o.clear();
		return true;
	}
	virtual bool isDone() const {
		return bReached;
	}
	virtual ~bhTurnToEnemy_c() {

	}
};
// GetCloseToEnemy behaviour
class bhGetCloseToEnemy_c : public bhBase_c {
	str animName;
public:
	bhGetCloseToEnemy_c(Actor *self, const char *args) : bhBase_c(self) {
		animName = args;
		animName.removeCharacter('"');
	}
	virtual const char *getAnimName() const {
		return animName;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		if(self->getEnemy()) {
			vec3_c d = self->getEnemy()->getOrigin()-self->getOrigin();
			d.setZ(0);
			d.normalize();
			o = d;
			self->setAngles(d.toAngles());
		} else {
			o.clear();
		}
		return true;
	}
	virtual ~bhGetCloseToEnemy_c() {

	}
};
// Flee behaviour
class bhFlee_c : public bhBase_c {
	str animName;
public:
	bhFlee_c(Actor *self, const char *args) : bhBase_c(self) {
		animName = args;
		animName.removeCharacter('"');
	}
	virtual const char *getAnimName() const {
		return animName;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		if(self->getEnemy()) {
			vec3_c d = self->getEnemy()->getOrigin()-self->getOrigin();
			d.setZ(0);
			d.normalize();
			d *= -1;
			o = d;
			self->setAngles(d.toAngles());
		} else {
			o.clear();
		}
		return true;
	}
	virtual ~bhFlee_c() {

	}
};
// Watch behaviour
class bhWatch_c : public bhBase_c {

public:
	bhWatch_c(Actor *self, const char *args) : bhBase_c(self) {
		str tmp = args;
		tmp.removeCharacter('"');

	}
	virtual const char *getAnimName() const {
		return 0;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		o.clear();
		if(self->getEnemy()) {
			vec3_c d = self->getEnemy()->getOrigin()-self->getOrigin();
			d.setZ(0);
			d.normalize();
			self->setAngles(d.toAngles());
		}
		return true;
	}
	virtual ~bhWatch_c() {

	}
};
// AimAndMelee behaviour
class bhAimAndMelee_c : public bhBase_c {
	str animName;
	float timeNeeded;
public:
	bhAimAndMelee_c(Actor *self, const char *args) : bhBase_c(self) {
		parser_c p(args);
		animName = p.getToken();
		timeNeeded = self->getAnimationTotalTimeMs(animName) * 0.001f;
	}
	virtual const char *getAnimName() const {
		return animName;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		if(self->getEnemy()) {
			vec3_c d = self->getEnemy()->getOrigin()-self->getOrigin();
			d.setZ(0);
			d.normalize();
			self->setAngles(d.toAngles());
		}
		o.clear();
		return true;
	}
	virtual bool isDone() const {
		if(0) {
			g_core->Print("Needed: %f, elapsed: %f\n",timeNeeded,timeElapsed);
		}
		return timeNeeded < timeElapsed;
	}
	virtual ~bhAimAndMelee_c() {

	}
};

// Pain behaviour
class bhPain_c : public bhBase_c {
	str animName;
	float timeNeeded;
public:
	bhPain_c(Actor *self, const char *args) : bhBase_c(self) {
		animName = self->getRandomPainAnimationName();
		timeNeeded = self->getAnimationTotalTimeMs(animName) * 0.001f;
	}
	virtual const char *getAnimName() const {
		return animName;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		o.clear();
		return true;
	}
	virtual bool isDone() const {
		if(0) {
			g_core->Print("Needed: %f, elapsed: %f\n",timeNeeded,timeElapsed);
		}
		return timeNeeded < timeElapsed;
	}
	virtual ~bhPain_c() {

	}
};

// Idle behaviour
class bhIdle_c : public bhBase_c {
public:
	bhIdle_c(Actor *self, const char *args) : bhBase_c(self) {
	}
	virtual const char *getAnimName() const {
		return 0;
	}
	virtual bool setupMoveDir(vec3_c &o) const {
		o.clear();
		return true;
	}
	virtual bool isDone() const {
		return 0;
	}
	virtual ~bhIdle_c() {

	}
};
Actor::Actor() {
	health = 100;
	bTakeDamage = true;
	st = 0;
	behaviour = 0;
	st_curState = "IDLE";
	st_handler = 0;
	this->characterController = 0;
	this->enemy = 0;
	painTime = 0;

	nearestSentient = 0;
	nearestSentientDist = 0.f;
}
Actor::~Actor() {
	if(characterController) {
		g_physWorld->freeCharacter(this->characterController);
		characterController = 0;
	}
}

void Actor::findNearestSentient() {

}
void Actor::enableCharacterController() {
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
	this->characterController = g_physWorld->createCharacter(this->getOrigin()+characterControllerOffset, h, r);
	if(this->characterController) {
		this->characterController->setCharacterEntity(this);
	}
}
void Actor::disableCharacterController() {
	if(characterController) {
		g_physWorld->freeCharacter(this->characterController);
		characterController = 0;
	}
}
void Actor::setOrigin(const vec3_c &newXYZ) {
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
void Actor::setBehaviour(const char *behaviourName, const char *args) {

	bhBase_c *nb;
	if(!stricmp(behaviourName,"GetCloseToEnemy")) {
		nb = new bhGetCloseToEnemy_c(this,args);
	} else if(!stricmp(behaviourName,"Watch")) {
		nb = new bhWatch_c(this,args);
	} else if(!stricmp(behaviourName,"Turn")) {
		nb = new bhTurn_c(this,args);
	} else if(!stricmp(behaviourName,"TurnToEnemy")) {
		nb = new bhTurnToEnemy_c(this,args);
	} else if(!stricmp(behaviourName,"AimAndMelee")) {
		nb = new bhAimAndMelee_c(this,args);
	} else if(!stricmp(behaviourName,"Idle")) {
		nb = new bhIdle_c(this,args);
	} else if(!stricmp(behaviourName,"Pain")) {
		nb = new bhPain_c(this,args);
	} else if(!stricmp(behaviourName,"Flee")) {
		nb = new bhFlee_c(this,args);
	} else {
		g_core->RedWarning("Actor::setBehaviour: unknown behaviour name '%s'\n",behaviourName);
		nb = 0;
	}
	if(behaviour) {
		delete behaviour;
		behaviour = 0;
	}
	behaviour = nb;
}
float Actor::getDistanceToEnemy() const {
	if(enemy == 0)
		return -1;
	vec3_c d = getOrigin()-enemy->getOrigin();
	return d.len();
}
void Actor::resetStateTimer() {
	const stTime_s *time = st->getStateTime(st_curState);
	if(time) {
		st_timeLimit = time->selectTime();
	} else {
		st_timeLimit = 0.f;
	}
	st_passedTime = 0.f; // reset timer
}
void Actor::runActorStateMachines() {
	if(g_actor_printActiveStateFiles.getInt()) {
		g_core->Print("Actor::runActorStateMachines: %s is using %s\n",getRenderModelName(),st->getName());
	}
	st_passedTime += level.frameTime;
	if(st_handler == 0) {
		st_handler = new genericConditionsHandler_t<Actor>(&g_actorConditionsTable,this);
	}
	st_handler->updateFrame();
	bool bChanged = false;
	if(forceState.length() == 0) {
		for(u32 i = 0; i < 2; i++) {
			const char *next = st->transitionState(st_curState,st_handler);
			if(next && next[0]) {
				if(g_actor_printStateChange.getInt()) 
					g_core->Print("Actor::runActorStateMachines: time %i: changing from %s to %s\n",level.time,st_curState.c_str(),next);
				st->iterateStateExitCommands(st_curState,this);
				st_curState = next;
				st->iterateStateEntryCommands(st_curState,this);
				bChanged = true;
				if(st->stateHasBehaviour(st_curState) && st->hasBehaviorOfType(st_curState,"idle")==false)
					break;
			}
		}
	} else {			
		st->iterateStateExitCommands(st_curState,this);
		st_curState = forceState;
		st->iterateStateEntryCommands(st_curState,this);
		if(g_actor_printStateChange.getInt()) 
			g_core->Print("Forced transition to %s\n",forceState.c_str());
		bChanged = true;
		forceState = "";
	}
	if(bChanged) {
		resetStateTimer();
		const char *bName, *bArgs;
		st->getStateBehaviour(st_curState,&bName,&bArgs);
		if(bName) {
			this->setBehaviour(bName,bArgs);
		}
	}
	const char *anim = st->getStateLegsAnim(st_curState,st_handler);
	if(anim == 0 || anim[0] == 0) {
		//g_core->Print("No animation found for state %s\n",st_curState.c_str());
		//return;
		if(behaviour) {
			anim = behaviour->getAnimName();
		//	g_core->Print("Actor:: anm from behaviour: %s\n",anim);
		}
	}
	if(0) {
		g_core->Print("Actor::runActorStateMachines: selected anim %s for state %s\n",anim,st_curState.c_str());
	}
	//anim = "death";
	this->setTorsoAnimation(0);
	this->setAnimation(anim);

	// reset timer, so TIMER_DONE CHANCE 0.5 won't get called over and over again every next frame
	if(st_passedTime > st_timeLimit) {
		resetStateTimer();
	}
}
bool Actor::canSee(BaseEntity *other) const {
	if(other == 0)
		return false;
	trace_c tr;
	for(u32 i = 0; i < 3; i++) {
		if(i == 0) {
			tr.setupRay(getCenter(),other->getCenter());
		} else if(i == 1) {
			vec3_c ofs(0,0,16);
			tr.setupRay(getCenter()+ofs,other->getCenter()+ofs);
		} else {
			tr.setupRay(getOrigin(),other->getOrigin());
		}
		if(g_physWorld) {
			if(!g_physWorld->traceRay(tr,this)) {
				if(g_actor_debugCanSee.getInt()) {
					g_core->Print("Actor::canSee (self %i, other %i): No hit\n",
						getEntNum(),other->getEntNum());
				}
				return true;
			}
		}
		if(tr.getHitEntity() == other) {
			if(g_actor_debugCanSee.getInt()) {
				g_core->Print("Actor::canSee (self %i, other %i): Hit target\n",
					getEntNum(),other->getEntNum());
			}
			return true;
		}
	}
	if(g_actor_debugCanSee.getInt()) {
		g_core->Print("Actor::canSee (self %i, other %i): Hit obstacle\n",
				getEntNum(),other->getEntNum());
	}
	return false;
}
void Actor::runFrame() {
	updateAnimations();
	if(st) {
		runActorStateMachines();
	}
	enemy = G_GetPlayer(0);
	if(canSee(enemy)==false)
		enemy = 0;

	if(characterController) {
		vec3_c dir(0,0,0);
		if(behaviour) {
			behaviour->setupMoveDir(dir);
			dir *= 5.f;
		}
		this->characterController->update(dir);
		vec3_c p = this->characterController->getPos() - characterControllerOffset;
		myEdict->s->origin = p;
		if(p.z < -10000) {
			g_core->RedWarning("Actor::runFrame: actor %s has abnormal origin %f %f %f\n",
				getRenderModelName(),p.x,p.y,p.z);
		}
		recalcABSBounds();
	}
	if(behaviour) {
		behaviour->advanceTime(level.frameTime);
	}
}
void Actor::loadAIStateMachine(const char *fname) {
	st = G_LoadStateMachine(fname);
	if(st == 0) {
		g_core->RedWarning("Actor::loadAIStateMachine: failed to load %s\n",fname);
	}
}
void Actor::setKeyValue(const char *key, const char *value) {
	if(!stricmp("statemap",key)) {
		loadAIStateMachine(value);
	} else if(!stricmp("behaviour",key)) {
	} else if(!stricmp("suicide",key)) {
		// in shgliek.st it's used like "suicide 1",
		// but g_allclasses.html does not describe any arguments...
		health = 0;
		onDeath();
	}
	ModelEntity::setKeyValue(key,value);
}
void Actor::onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damage) {
	painTime = level.time;
}
void Actor::onDeath() {
	forceState = "DEATH";
}
void Actor::postSpawn() {
	executeTIKIInitCommands();
	enableCharacterController();
}

const char *Actor::getRandomPainAnimationName() const {
	const char *painAnimNames[] = {
		"pain1", "pain", "pain2", "pain3"
	};
	u32 numPainAnims = sizeof(painAnimNames) / sizeof(painAnimNames[0]);
	u32 r = rand()%numPainAnims;
	for(u32 i = 0; i < numPainAnims; i++) {
		u32 idx = (i+r) % numPainAnims;
		const char *name = painAnimNames[idx];
		if(this->findAnimationIndex(name)!= -1)
			return name;
	}
	return "";
}
bool Actor::checkMovingActorRange(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	float range = atof(arguments->getString(0));
	for(u32 i = 0; i < MAX_CLIENTS; i++) {
		Player *pl = G_GetPlayer(i);
		if(pl == 0)
			continue;
		float d = pl->getOrigin().dist(this->getOrigin());
		if(d < range)
			return true;
	}
	return false;
}
bool Actor::checkChance(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	float f = atof(arguments->getString(0));
	float r = (rand() % 1000)*0.001f;
	if(f > r)
		return true;
	return false;
}
bool Actor::checkTimeDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(st_passedTime > st_timeLimit)
		return true;
	return false;
}	

bool Actor::checkCanSeeEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(enemy==0)
		return false;
	return true;
}
bool Actor::checkHaveEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(enemy)
		return true;
	return false;
}
bool Actor::checkRange(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(enemy == 0)
		return false;
	float f = atof(arguments->getString(0));
	float r = getDistanceToEnemy();
	if(r < f) 
		return true;
	return false;
}
bool Actor::checkDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(behaviour == 0)
		return true;
	return behaviour->isDone();
}
bool Actor::checkCanShootEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	return true;
}
bool Actor::checkHealth(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	float f = atof(arguments->getString(0));
	if(health < f)
		return true;
	return false;
}
bool Actor::checkAllowHangBack(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return true;
}
bool Actor::checkOnGround(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return true;
}
bool Actor::checkJumpToEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return false;
}
bool Actor::checkMeleeHit(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return false;
}
bool Actor::checkInWater(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	return false;
}
bool Actor::checkHeld(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// player can hold Shgliek
	return false;
}

bool Actor::checkMOD(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// check type of damage
	return false;
}
bool Actor::checkOnFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {

	return false;
}


bool Actor::checkSmallPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	u32 del = 100;
	if(level.time < this->painTime + del)
		return true;
	return false;
}
bool Actor::checkPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	u32 del = 100;
	if(level.time < this->painTime + del)
		return true;
	return false;
}
bool Actor::checkEnemyRelativeYaw(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	if(enemy == 0)
		return false;
	float f = atof(arguments->getString(0));
	vec3_c d = getOrigin() - enemy->getOrigin();
	d.normalize();
	vec3_c a = d.toAngles();
	// yaw
	float anglesDelta = enemy->getAngles().y - a.y;
	if(anglesDelta > 180.f) {
		anglesDelta -= 360.f;
	}
	g_core->Print("Angle: %f\n",anglesDelta);
	if (anglesDelta < f)
		return true;
	return false;
}

bool Actor::checkName(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	// TODO
		return true;
	return false;
}
bool Actor::checkAnimDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	int needed = getCurrentAnimationTotalTimeMs();
	if(legsAnimationTime > needed) {
		if(g_actor_st_debugAnimDone.getInt())
			g_core->Print("Anim done! %i > %i \n",legsAnimationTime,needed);
		return true;
	}
	if(g_actor_st_debugAnimDone.getInt()) 
		g_core->Print("Anim not yet done... %i <= %i \n",legsAnimationTime,needed);
	return false;
}
