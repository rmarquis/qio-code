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
#ifndef __API_STATEMACHINE_H__
#define __API_STATEMACHINE_H__

enum stMoveType_e {
	EMT_BAD,
	EMT_LEGS,
	// for sword combo
	EMT_ANIM,
	// for ladder
	EMT_CLIMBWALL,
	// disable steering
	EMT_NONE, 
};

// used only for AI state files
struct stTime_s {
	float time[2];

	stTime_s() {
		time[0] = time[1] = 0.f;
	}
	float selectTime() const {
		float dif = time[1] - time[0];
		float f = (rand()%10000)*0.0001f;
		return time[0] + dif * f;
	}
};

class stateMachineAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual bool hasState(const char *stateName) const = 0;
	virtual const char *getStateName(u32 stateIndex) const = 0;
	virtual const char *transitionState(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual const char *getStateLegsAnim(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual const char *getStateActionAnim(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual const char *getStateTorsoAnim(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual void iterateStateEntryCommands(const char *stateName, class keyValueSetter_i *callback) const = 0;
	virtual void iterateStateExitCommands(const char *stateName, class keyValueSetter_i *callback) const = 0;
	virtual enum stMoveType_e getStateMoveType(const char *curStateName) const = 0;
	// for AI state machines
	virtual const stTime_s *getStateTime(const char *curStateName) const = 0;
	virtual void getStateBehaviour(const char *stateName, const char **bName, const char **bArgs) const = 0;
	virtual bool stateHasBehaviour(const char *stateName) const = 0;
	virtual bool hasBehaviorOfType(const char *stateName, const char *s) const = 0;

};

#endif // __API_STATEMACHINE_H__
