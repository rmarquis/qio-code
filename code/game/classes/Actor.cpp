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
#include "../g_local.h"
#include <api/cmAPI.h>
#include <api/serverAPI.h>
#include <api/coreAPI.h>
#include <api/physAPI.h>
#include <api/physObjectAPI.h>
#include <api/physCharacterControllerAPI.h>
#include <api/stateMachineAPI.h>
#include <api/stateConditionsHandlerAPI.h>

DEFINE_CLASS(Actor, "ModelEntity");
// Doom3 AI
DEFINE_CLASS_ALIAS(Actor, idAI);
 

conditionFunction_s g_actorConditions [] = {

	GETFUNC("ANIM_DONE",Actor::checkAnimDone)
	
	{ 0, 0 }
};


conditionsTable_c g_actorConditionsTable(g_actorConditions,sizeof(g_actorConditions)/sizeof(g_actorConditions[0]));



Actor::Actor() {
	health = 100;
	bTakeDamage = true;
	st = 0;
	st_curState = "IDLE";
	st_handler = 0;
	this->characterController = 0;
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
void Actor::runActorStateMachines() {

	if(st_handler == 0) {
		st_handler = new genericConditionsHandler_t<Actor>(&g_actorConditionsTable,this);
	}
	st_handler->updateFrame();
	const char *next = st->transitionState(st_curState,st_handler);
	if(next && next[0]) {
		st_curState = next;
	}
	const char *anim = st->getStateLegsAnim(st_curState,st_handler);
	if(anim == 0 || anim[0] == 0) {
		g_core->Print("No animation found for state %s\n",st_curState.c_str());
		return;
	}
	g_core->Print("Actor::runActorStateMachines: selected anim %s for state %s\n",anim,st_curState.c_str());
	//anim = "death";
	this->setTorsoAnimation(0);
	this->setAnimation(anim);
}
void Actor::runFrame() {
	updateAnimations();
	if(st) {
		runActorStateMachines();
	}
	vec3_c p = this->characterController->getPos() - characterControllerOffset;
	myEdict->s->origin = p;
	if(p.z < -10000) {
		g_core->RedWarning("Actor::runFrame: actor %s has abnormal origin %f %f %f\n",
			getRenderModelName(),p.x,p.y,p.z);
	}
	recalcABSBounds();
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
	}
	ModelEntity::setKeyValue(key,value);
}
void Actor::postSpawn() {
	executeTIKIInitCommands();
	enableCharacterController();
}


bool Actor::checkAnimDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
	int needed = getCurrentAnimationTotalTimeMs();
	if(legsAnimationTime > needed) {
		if(1)
			g_core->Print("Anim done! %i > %i \n",legsAnimationTime,needed);
		return true;
	}
	if(1) 
		g_core->Print("Anim not yet done... %i <= %i \n",legsAnimationTime,needed);
	return false;
}
