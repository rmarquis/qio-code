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
// Actor.h
#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "ModelEntity.h"
#include "../stateMachineShared.h"

class Actor : public ModelEntity {
	str st_curState;
	// used to force a transition next frame
	str forceState;
	// selected time for current state (TIME_DONE condition)
	float st_timeLimit;
	// passed time
	float st_passedTime;
	class stateMachineAPI_i *st;
	genericConditionsHandler_t<Actor> *st_handler;
	class bhBase_c *behaviour;
	class ModelEntity *enemy;
	u32 painTime;

	class ModelEntity *nearestSentient;
	float nearestSentientDist;

	class physCharacterControllerAPI_i *characterController;
	vec3_c characterControllerOffset;

	void findNearestSentient();
	float getDistanceToEnemy() const;
	void resetStateTimer();
	void runActorStateMachines();
	void loadAIStateMachine(const char *fname);
	void setBehaviour(const char *behaviourName, const char *args);

	bool canSee(BaseEntity *other) const;
public:
	Actor();
	~Actor();

	DECLARE_CLASS( Actor );

	ModelEntity *getEnemy() {
		return enemy;
	}
	void disableCharacterController();
	void enableCharacterController();

	virtual void runFrame();
	virtual void postSpawn();
	virtual void setOrigin(const vec3_c &newXYZ);

	virtual void setKeyValue(const char *key, const char *value);

	virtual void onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damage);

	virtual void onDeath();

	const char *getRandomPainAnimationName() const;

	//
	// state conditions
	// (for FAKK ai/*.st files support)
	//
	bool checkAnimDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkMovingActorRange(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkChance(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkTimeDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkName(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHaveEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkCanSeeEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);

	bool checkRange(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkDone(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkCanShootEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHealth(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkEnemyRelativeYaw(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkAllowHangBack(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkOnGround(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkSmallPain(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkJumpToEnemy(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkMeleeHit(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkInWater(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkHeld(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkMOD(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
	bool checkOnFire(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);

};

#endif // __ACTOR_H__
