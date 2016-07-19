/*
============================================================================
Copyright (C) 2016 V.

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
#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#include <shared/str.h>
#include <shared/hashTableTemplate.h>
#include <api/stateMachineAPI.h>
#include <shared/stringList.h>
//#include <shared/conditionType.h>


class stCondition_c {
	enum stConditionType_e conditionType;
	stringList_c arguments;
	str name;
public:
	bool parseCondition(class parser_c &p);
	bool checkCondition(class stateConditionsHandler_i *handler, class patternMatcher_c *patternMatcher) const;
};

class stConditionsList_c {
	arraySTD_c<stCondition_c> list;
public:
	bool parseConditions(class parser_c &p);
	bool checkConditions(class stateConditionsHandler_i *handler, class patternMatcher_c *patternMatcher) const;
};
class stAnim_c {
	str name;
	// TODO - animation patterns
	bool bIsPattern;
	// conditions which must be met to play the animatiom
	stConditionsList_c conditions;
public:
	stAnim_c();

	void setIsPattern(bool newBIsPattern) {
		bIsPattern = newBIsPattern;
	}
	void setAnimation(const char *newAnimName) {
		name = newAnimName;
	}
	const char *getAnimation() const {
		return name.c_str();
	}
	bool isPattern() const {
		return bIsPattern;
	}
	stConditionsList_c &getConditions() {
		return conditions;
	}
	const stConditionsList_c &getConditions() const {
		return conditions;
	}
};
class stAnimsList_c {
	arraySTD_c<stAnim_c> list;
public:
	bool parseStateAnims(class parser_c &p);
	const char *selectAnimation(class stateConditionsHandler_i *handler) const;
};

class stTransition_c {
	// target state name
	str target;
	// conditions which must be met to do the transition
	stConditionsList_c conditions;
public:

	void setTargetStateName(const char *newTargetStateName) {
		target = newTargetStateName;
	}
	const char *getTargetStateName() const {
		return target.c_str();
	}
	const stConditionsList_c &getConditions() const {
		return conditions;
	}
	stConditionsList_c &getConditions() {
		return conditions;
	}
};
class stCommand_c {
friend class stateCommandsList_c;
	str cmd;
	str args;
};
class stateCommandsList_c {
	arraySTD_c<stCommand_c> cmds;
public:
//	~stateCommandsList_c();

	void addCommand(const char *cmd, const char *args) {
		stCommand_c c;
		c.cmd = cmd;
		c.args = args;
		cmds.push_back(c);
	}


	void executeCommandsOn(class keyValueSetter_i *p) const;
	bool parseCommandsList(class parser_c &p);
};
class stState_c {
	str stateName;
	stState_c *hashNext;
	enum stMoveType_e moveType;
	enum ECameraType cameraType;

	stateCommandsList_c entryCommands;
	stateCommandsList_c exitCommands;
	
	stAnimsList_c actions;
	stAnimsList_c legs;
	// added for FAKK
	stAnimsList_c torso;
	arraySTD_c<stTransition_c> transitions;
	// for AI states
	stTime_s time;
	str behaviourName;
	str behaviourArguments;
public:
	stState_c() {
		moveType = EMT_LEGS;
	}
	const char *getBehaviourName() const {
		return behaviourName;
	}
	const char *getBehaviourArgs() const {
		return behaviourArguments;
	}
	const stTime_s *getStateTime() const {
		return &time;
	}
	static stState_c *newInstance() {
		return new stState_c();
	}
	const char *getName() const {
		return stateName.c_str();
	}
	class stState_c *getHashNext() const {
		return hashNext;
	}
	void setHashNext(class stState_c *ns) {
		hashNext = ns;
	}
	enum stMoveType_e getMoveType() const {
		return moveType;
	}
	bool parseState(class stateMachine_c *m, class parser_c &p);
	const char *transitionState(class stateConditionsHandler_i *handler) const;	
	const char *getLegsAnim(class stateConditionsHandler_i *handler) const;	
	const char *getTorsoAnim(class stateConditionsHandler_i *handler) const;	
	const char *getActionAnim(class stateConditionsHandler_i *handler) const;	
	void iterateStateEntryCommands(class keyValueSetter_i *callback) const;
	void iterateStateExitCommands(class keyValueSetter_i *callback) const;
};


class stateMachine_c : public stateMachineAPI_i {
	str fileName;
	hashTableTemplateExt_c<class stState_c> states;
	stateMachine_c *hashNext;

	// stateMachineAPI_i implementation
	virtual bool hasState(const char *stateName) const;
	virtual const char *getStateName(u32 stateIndex) const;
	virtual const char *transitionState(const char *curStateName, class stateConditionsHandler_i *handler) const;
	virtual const char *getStateLegsAnim(const char *curStateName, class stateConditionsHandler_i *handler) const;
	virtual const char *getStateActionAnim(const char *curStateName, class stateConditionsHandler_i *handler) const;
	virtual const char *getStateTorsoAnim(const char *curStateName, class stateConditionsHandler_i *handler) const;
	virtual const stTime_s *getStateTime(const char *curStateName) const;
	virtual void iterateStateEntryCommands(const char *stateName, class keyValueSetter_i *callback) const;
	virtual void iterateStateExitCommands(const char *stateName, class keyValueSetter_i *callback) const;
	virtual enum stMoveType_e getStateMoveType(const char *stateName) const;
	virtual void getStateBehaviour(const char *stateName, const char **bName, const char **bArgs) const;
	virtual bool stateHasBehaviour(const char *stateName) const;
	virtual bool hasBehaviorOfType(const char *stateName, const char *s) const;
public:
	stateMachine_c();
	~stateMachine_c();

	bool parseStateMachine();

	void setName(const char *newName) {
		fileName = newName;
	}
	const char *getName() const {
		return fileName.c_str();
	}
	void setHashNext(stateMachine_c *newHashNext) {
		hashNext = newHashNext;
	}
	stateMachine_c *getHashNext() const {
		return hashNext;
	}
	bool isValid() const {
		return states.size();
	}
};



#endif // __STATEMACHINE_H__

