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

DEFINE_CLASS(Actor, "ModelEntity");
// Doom3 AI
DEFINE_CLASS_ALIAS(Actor, idAI);
 
static aCvar_c g_actor_st_debugAnimDone("g_actor_st_debugAnimDone","0");

conditionFunction_s g_actorConditions [] = {

	GETFUNC("ANIM_DONE",Actor::checkAnimDone)
	GETFUNC("MOVING_ACTOR_RANGE",Actor::checkMovingActorRange)
	GETFUNC("CHANCE",Actor::checkChance)
	GETFUNC("TIME_DONE",Actor::checkTimeDone)
	GETFUNC("NAME",Actor::checkName)
	GETFUNC("HAVE_ENEMY",Actor::checkHaveEnemy)
	GETFUNC("RANGE",Actor::checkRange)
	GETFUNC("DONE",Actor::checkDone)
	
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
		g_core->Print("Needed: %f, elapsed: %f\n",timeNeeded,timeElapsed);
		return timeNeeded < timeElapsed;
	}
	virtual ~bhAimAndMelee_c() {

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
}
Actor::~Actor() {
	if(characterController) {
		g_physWorld->freeCharacter(this->characterController);
		characterController = 0;
	}
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
	} else if(!stricmp(behaviourName,"AimAndMelee")) {
		nb = new bhAimAndMelee_c(this,args);
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
	st_passedTime += level.frameTime;
	if(st_handler == 0) {
		st_handler = new genericConditionsHandler_t<Actor>(&g_actorConditionsTable,this);
	}
	st_handler->updateFrame();
	bool bChanged = false;
	for(u32 i = 0; i < 2; i++) {
		const char *next = st->transitionState(st_curState,st_handler);
		if(next && next[0]) {
			g_core->Print("Actor::runActorStateMachines: time %i: changing from %s to %s\n",level.time,st_curState.c_str(),next);
			st_curState = next;
			bChanged = true;
			if(st->stateHasBehaviour(st_curState))
				break;
		}
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
			g_core->Print("Actor:: anm from behaviour: %s\n",anim);
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
void Actor::runFrame() {
	updateAnimations();
	if(st) {
		runActorStateMachines();
	}
	enemy = G_GetPlayer(0);
	if(characterController) {
		vec3_c dir(0,0,0);
		if(behaviour) {
			behaviour->setupMoveDir(dir);
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
	}
	ModelEntity::setKeyValue(key,value);
}
void Actor::postSpawn() {
	executeTIKIInitCommands();
	enableCharacterController();
}

bool Actor::checkMovingActorRange(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	float range = atof(arguments->getString(0));
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
