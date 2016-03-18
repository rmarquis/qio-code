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
#include "StateMachine.h"
#include <shared/parser.h>
#include <api/coreAPI.h>
#include <api/stateConditionsHandlerAPI.h>



stAnim_c::stAnim_c() {
	bIsPattern = false;
}
bool stCondition_c::parseCondition(class parser_c &p) {
	if(p.atWord_dontNeedWS("-")) {
		conditionType = CT_EDGEFALSE;
	} else if(p.atWord_dontNeedWS("+")) {
		conditionType = CT_EDGETRUE;
	} else if(p.atWord_dontNeedWS("!")) {
		conditionType = CT_NEGATE;
	} else {
		conditionType = CT_NORMAL;
	}
	this->name = p.getToken();
	while(p.isNextTokenInQuotationMarks()) {
		const char *arg = p.getToken();
		arguments.addString(arg);
	}
	return false; // ok
}
bool stCondition_c::checkCondition(class stateConditionsHandler_i *handler, class patternMatcher_c *patternMatcher) const {
	return handler->isConditionTrue(this->conditionType,this->name.c_str(),&this->arguments,patternMatcher);
}
bool stConditionsList_c::parseConditions(class parser_c &p) {
	if(p.atWord_dontNeedWS(":") == false) {
		int line = p.getCurrentLineNumber();
		str token = p.getToken();
		g_core->RedWarning("stConditionsList_c::parseConditions: expected ':' before conditions list, found %s at line %i of %s\n",token.c_str(),line,p.getDebugFileName());
		return true; // error
	}
	while(p.isAtEOL() == false) {
		stCondition_c &c = list.pushBack();
		if(c.parseCondition(p)) {
			return true;
		}
	}
	return false; // ok
}
bool stConditionsList_c::checkConditions(class stateConditionsHandler_i *handler, class patternMatcher_c *patternMatcher) const {
	for(u32 i = 0; i < list.size(); i++) {
		const stCondition_c &c = list[i];
		if(c.checkCondition(handler,patternMatcher)==false)
			return false;
	}
	return true;
}
bool stAnimsList_c::parseStateAnims(class parser_c &p) {
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		str token = p.getToken();
		g_core->RedWarning("stAnimsList_c::parseStateAnims: expected '{' to follow state animations list, found %s at line %i of %s\n",token.c_str(),line,p.getDebugFileName());
		return true; // error
	}
	while(p.atWord_dontNeedWS("}") == false) {
		stAnim_c &an = list.pushBack();
		if(p.atWord("pattern")) {
			an.setIsPattern(true);
		}
		an.setAnimation(p.getToken());
		if(an.getConditions().parseConditions(p)) {
			return true;
		}
	}
	return false;
}
const char *stAnimsList_c::selectAnimation(class stateConditionsHandler_i *handler) const {
	for(u32 i = 0; i < list.size(); i++) {
		const stAnim_c &a = list[i];
		if(a.getConditions().checkConditions(handler,0)) {
			return a.getAnimation();
		}
	}
	return 0;
}

bool stateCommandsList_c::parseCommandsList(class parser_c &p) {
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		str token = p.getToken();
		g_core->Print("stateCommandsList_c::parseCommandsList: expected '{' to follow commands block,found %s at line %i of %s\n",token.c_str(),line,p.getDebugFileName());
		return true; // error
	}
	bool bStop = false;
	while(p.atWord_dontNeedWS("}") == false) {
		str command = p.getToken();
		str arguments;
		if(p.isAtEOL()) {
			arguments = "";
		} else if(p.atChar('}')) {
			bStop = true;
		} else {
			const char *line = p.getLine();
			arguments = line;
		}
		if(p.atWord_dontNeedWS("(")) {
			str s = arguments;
			while(p.atWord_dontNeedWS(")") == false) {
				s.append(p.getLine());
				s.append("\n");
			}
			s.append(" )");
			//addCommand(command.c_str(),s.c_str());
		} else {
			//addCommand(command.c_str(),arguments.c_str());
		}
		if(bStop)
			break;
	}
	return false; // ok
}
bool stState_c::parseState(class stateMachine_c *m, class parser_c &p) {
	stateName = p.getToken();
	if(p.atWord_dontNeedWS("{") == false) {
		int line = p.getCurrentLineNumber();
		str token = p.getToken();
		g_core->RedWarning("stState_c::parseState: expected '{' to follow state %s,found %s at line %i of %s\n",stateName.c_str(),token.c_str(),line,m->getName());
		return true; // error
	}
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atWord("movetype")) {
			if(p.atWord("legs")) {
				moveType = EMT_LEGS;
			} else if(p.atWord("none")) {
				moveType = EMT_NONE;
			} else if(p.atWord("climbwall")) {
				moveType = EMT_CLIMBWALL;
			} else if(p.atWord("anim")) {
				moveType = EMT_ANIM;
			} else {
				moveType = EMT_BAD;
				p.getToken();
			}
		} else if(p.atWord("camera")) {
			p.getToken();
		} else if(p.atWord("entryCommands")) {
			if(entryCommands.parseCommandsList(p)) {
				return true;
			}
		} else if(p.atWord("exitCommands")) {
			if(exitCommands.parseCommandsList(p)) {
				return true;
			}
		} else if(p.atWord("legs")) {
			if(legs.parseStateAnims(p)) {
				return true;
			}
		} else if(p.atWord("action")) {
			if(actions.parseStateAnims(p)) {
				return true;
			}
		// only for FAKK
		} else if(p.atWord("torso")) {
			if(torso.parseStateAnims(p)) {
				return true;
			}
		} else if(p.atWord("states")) {
			if(p.atWord_dontNeedWS("{") == false) {
				int line = p.getCurrentLineNumber();
				str token = p.getToken();
				g_core->RedWarning("stState_c::parseState: expected '{' to follow state transitions in state %s,found %s at line %i of %s\n",stateName.c_str(),token.c_str(),line,m->getName());
				return true; // error
			}
			while(p.atWord_dontNeedWS("}") == false) {
				stTransition_c &tr = transitions.pushBack();
				tr.setTargetStateName(p.getToken());
				if(tr.getConditions().parseConditions(p)) {
					return true;
				}
			}
		} else if(p.atWord("behavior")) {
			// FAKK state machine for AI
			behaviourName = p.getToken();
			if(p.isAtEOL() == false) {
				behaviourArguments = p.getLine();
			}
		} else if(p.atWord("time")) {
			// FAKK state machine for AI
			time.time[0] = p.getFloat();
			if(p.isAtEOL()) {
				time.time[1] = time.time[0];
			} else {
				time.time[1] = p.getFloat();
			}
		} else {
			int line = p.getCurrentLineNumber();
			str token = p.getToken();
			g_core->RedWarning("stState_c::parseState: unknown token %s in state %s at line %i of %s\n",token.c_str(),stateName.c_str(),line,m->getName());
			return true; // error
		}
	}
	return false; // ok
}
const char *stState_c::transitionState(class stateConditionsHandler_i *handler) const {
	for(u32 i = 0; i < transitions.size(); i++) {
		const stTransition_c &tr = transitions[i];
		const stConditionsList_c &c = tr.getConditions();
		if(c.checkConditions(handler,false)) {
			return tr.getTargetStateName();
		}
	}
	return 0;
}
const char *stState_c::getLegsAnim(class stateConditionsHandler_i *handler) const {
	return legs.selectAnimation(handler);
}
const char *stState_c::getActionAnim(class stateConditionsHandler_i *handler) const {
	return actions.selectAnimation(handler);
}
const char *stState_c::getTorsoAnim(class stateConditionsHandler_i *handler) const {
	return torso.selectAnimation(handler);
}
void stState_c::iterateStateEntryCommands(class stCommandHandler_i *callback) const {
//	entryCommands.executeCommandsOn(callback);
}
void stState_c::iterateStateExitCommands(class stCommandHandler_i *callback) const {
//	exitCommands.executeCommandsOn(callback);
}
stateMachine_c::stateMachine_c() {
	hashNext = 0;
}
stateMachine_c::~stateMachine_c() {
	while(states.size()) {
		stState_c *s = states[0];
		states.removeEntry(s);
		delete s;
	}
}
bool stateMachine_c::parseStateMachine() {
	parser_c p;
	if(p.openFile(fileName.c_str())) {
		g_core->RedWarning("stateMachine_c::parseStateMachine(): cannot open %s for reading\n",fileName.c_str());
		return true; // error
	}
	while(p.atEOF() == false) {
		if(p.atWord("state")) {
			// parse single state
			stState_c *newState = new stState_c;
			if(newState->parseState(this,p)) {
				delete newState;
				return true;
			}
			states.addObject(newState);
		} else {
			int line = p.getCurrentLineNumber();
			str token = p.getToken();
			g_core->RedWarning("stateMachine_c::parseStateMachine(): unknown token %s at line %i of %s\n",token.c_str(),line,fileName.c_str());
			return true; // error
		}
	}
	return false;
}
// stateMachineAPI_i implementation
bool stateMachine_c::hasState(const char *stateName) const {
	if(states.getEntry(stateName))
		return true;
	return false;
}
const char *stateMachine_c::getStateName(u32 stateIndex) const {
	if(stateIndex >= states.size()) {
		return "INVALID_INDEX_ERROR_STATE";
	}
	if(states[stateIndex] == 0) {
		return "NULL_ERROR_STATE";
	}
	return states[stateIndex]->getName();
}

const char *stateMachine_c::transitionState(const char *curStateName, class stateConditionsHandler_i *handler) const {
	const stState_c *s = states.getEntry(curStateName);
	if(s == 0) {
		g_core->RedWarning("stateMachine_c::transitionState: state %s does not exist\n",curStateName);
		return 0;
	}
	return s->transitionState(handler);
}

const char *stateMachine_c::getStateLegsAnim(const char *curStateName, class stateConditionsHandler_i *handler) const {
	const stState_c *s = states.getEntry(curStateName);
	if(s == 0)
		return 0;
	return s->getLegsAnim(handler);
}
const char *stateMachine_c::getStateActionAnim(const char *curStateName, class stateConditionsHandler_i *handler) const {
	const stState_c *s = states.getEntry(curStateName);
	if(s == 0)
		return 0;
	return s->getActionAnim(handler);
}
const char *stateMachine_c::getStateTorsoAnim(const char *curStateName, class stateConditionsHandler_i *handler) const {
	const stState_c *s = states.getEntry(curStateName);
	if(s == 0)
		return 0;
	return s->getTorsoAnim(handler);
}	
const stTime_s *stateMachine_c::getStateTime(const char *curStateName) const {
	const stState_c *s = states.getEntry(curStateName);
	if(s == 0)
		return 0;
	return s->getStateTime();
}
void stateMachine_c::getStateBehaviour(const char *stateName, const char **bName, const char **bArgs) const {
	const stState_c *s = states.getEntry(stateName);
	if(s == 0) {
		*bName = 0;
		*bArgs = 0;
		return;
	}
	*bName = s->getBehaviourName();
	*bArgs = s->getBehaviourArgs();
}
void stateMachine_c::iterateStateEntryCommands(const char *stateName, class stCommandHandler_i *callback) const {
	const stState_c *s = states.getEntry(stateName);
	if(s == 0)
		return;
	return s->iterateStateEntryCommands(callback);
}
void stateMachine_c::iterateStateExitCommands(const char *stateName, class stCommandHandler_i *callback) const {
	const stState_c *s = states.getEntry(stateName);
	if(s == 0)
		return;
	return s->iterateStateExitCommands(callback);
}
enum stMoveType_e stateMachine_c::getStateMoveType(const char *stateName) const {
	const stState_c *s = states.getEntry(stateName);
	if(s == 0)
		return EMT_BAD;
	return s->getMoveType();
}
