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
	// for ladder
	EMT_CLIMBWALL,
	// disable steering
	EMT_NONE, 
};

class stateMachineAPI_i {
public:
	virtual bool hasState(const char *stateName) const = 0;
	virtual const char *getStateName(u32 stateIndex) const = 0;
	virtual const char *transitionState(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual const char *getStateLegsAnim(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual const char *getStateActionAnim(const char *curStateName, class stateConditionsHandler_i *handler) const = 0;
	virtual void iterateStateEntryCommands(const char *stateName, class stCommandHandler_i *callback) const = 0;
	virtual void iterateStateExitCommands(const char *stateName, class stCommandHandler_i *callback) const = 0;
	virtual enum stMoveType_e getStateMoveType(const char *curStateName) const = 0;
};

#endif // __API_STATEMACHINE_H__
