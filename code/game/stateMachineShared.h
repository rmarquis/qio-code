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
// stateMachineShared.h - classes and structures 
// used to make a bridge between game class and state machine
// Currently state machine system is used in the following classes:
// - Player (legs and torso state)
// - Actor (generic AI state)
#ifndef __STATEMACHINESHARED_H__
#define __STATEMACHINESHARED_H__

#include <shared/hashTableTemplate.h>

struct conditionFunction_s {
	const char *name;
	//bool (BaseEntity::*func)();
	s64 func;
	conditionFunction_s *hashNext;
	int index;

	const char *getName() const {
		return name;
	}
	conditionFunction_s *getHashNext() const {
		return hashNext;
	}	
	void setHashNext(conditionFunction_s *newHashNext) {
		hashNext = newHashNext;
	}
	bool isValid() const {
		if(name == 0)
			return false;
		if(func == 0)
			return false;
		return true;
	}
};

template <typename d, typename s>
d &hack_cast(s v) {
  return reinterpret_cast<d&>(v);
}
//#define GETFUNC(name, ptr) { name, reinterpret_cast<s64>(&ptr^), 0 },
#define GETFUNC(name, ptr) { name, hack_cast<s64>(&ptr), 0 },


class conditionsTable_c {
	hashTableTemplateExt_c<conditionFunction_s> table;
public:
	conditionsTable_c(conditionFunction_s *funcs, u32 size) {
		conditionFunction_s *f = funcs;
		for(u32 i = 0; i < size; i++, f++) {
			if(f->isValid()) {
				f->index = table.size();
				table.addObject(f);
			}
		}
	}
	const conditionFunction_s *findFunction(const char *name) const {
		return table.getEntry(name);
	}
	u32 getSize() const {
		return table.size();
	}
};


struct conditionState_s {
	bool result;
	bool prevResult;

	void updateFrame() {
		prevResult = result;
	}
};

template<typename TYPE>
class genericConditionsHandler_t : public stateConditionsHandler_i {
	const class conditionsTable_c *table;
	arraySTD_c<conditionState_s> states;
	TYPE *ent;

	virtual bool hasAnim(const char *animName) const {
		return ent->findAnimationIndex(animName)!=-1;
	}
	virtual bool isConditionTrue(enum stConditionType_e conditionType, const char *conditionName, const class stringList_c *arguments, class patternMatcher_c *patternMatcher) {
		if(!stricmp(conditionName,"default"))
			return true;

		const conditionFunction_s *f = table->findFunction(conditionName);
		if(f == 0)
			return false;
		int index = f->index;
		// the arguments are not taken into account while checking edgetrue/edgefalse stuff
		conditionState_s &s = states[index];

		union {
			bool (TYPE::*pFunc)(const class stringList_c *arguments, class patternMatcher_c *patternMatcher);
			s64 p;
			byte rawData[8];
		};
		p = f->func;
		s.result = (ent->*pFunc)(arguments,patternMatcher);

		if(conditionType == CT_NORMAL)
			return s.result;
		if(conditionType == CT_NEGATE)
			return !s.result;
		if(conditionType == CT_EDGEFALSE) {
			if(!s.result && s.prevResult)
				return true;
			return false;
		}
		if(conditionType == CT_EDGETRUE) {
			if(s.result && !s.prevResult)
				return true;
			return false;
		}
		return false;
	}
public:
	genericConditionsHandler_t(const conditionsTable_c *pTable, TYPE *newEnt) {
		table = pTable;
		ent = newEnt;
		states.resize(table->getSize());
		states.nullMemory();
	}
	~genericConditionsHandler_t() {

	}
	void updateFrame() {
		for(u32 i = 0; i < table->getSize(); i++) {
			states[i].updateFrame();
		}
	}
};

#endif
