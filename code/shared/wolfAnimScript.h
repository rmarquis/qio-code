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
// wolfAnimScript.h - RTCW/ET anim .script files loader
#ifndef __SHARED_WOLFANIMATIONSCRIPT_H__
#define __SHARED_WOLFANIMATIONSCRIPT_H__

#include <shared/typedefs.h>
#include <shared/array.h>
#include <shared/str.h>

enum wolfConditionType_e {
	// check if player has any of given weapons active
	WC_WEAPONS, 
	// always met
	WC_DEFAULT,
	// for scripted sequences?
	WC_SPECIAL,
	// using mg42?
	WC_MOUNTED,
	// shooting a weapon
	WC_FIRING,
};
class wolfAnimCondition_c {
friend class wolfAnimScript_c;
friend class wolfAnimConditions_c;
	wolfConditionType_e type;
	arraySTD_c<str> data;
public:
	bool isOnDataList(const char *s) const {
		for(u32 i = 0; i < data.size(); i++) {
			if(!stricmp(data[i],s)) {
				return true;
			}
		}
		return false;
	}
};
class wolfAnimConditions_c {
friend class wolfAnimScript_c;
	arraySTD_c<wolfAnimCondition_c> conditions;
public:
	bool checkConditions(const struct wolfAnimScriptInput_s &in) const;
};
class wolfAnimDef_c {
friend class wolfAnimScript_c;
	// hardcoded legs, torso and full body animations
	str legs, torso, both;
	// custom events, eg. "sound"
	arraySTD_c<str> customEvents;
public:
};
class wolfConditionalAnim_c {
friend class wolfAnimScript_c;
	wolfAnimConditions_c conditions;
	wolfAnimDef_c def;
public:
};
// set of animations for given movetype
class wolfStateMove_c {
friend class wolfAnimScript_c;
	// "idle", "walk", etc.
	str moveType;
	arraySTD_c<wolfConditionalAnim_c> conditionalCases;
public:
	bool hasMoveType(const char *name) const;
};
class wolfState_c {
friend class wolfAnimScript_c;
	// RELAXED, ALERT, etc.
	str stateName; 
	arraySTD_c<wolfStateMove_c> moveStates;

public:
	const wolfStateMove_c *findMoveType(const struct wolfAnimScriptInput_s &in) const;
};
class wolfSet_c {
friend class wolfAnimScript_c;
	str alias;
	arraySTD_c<str> values;

public:
	void addValuesTo(arraySTD_c<str> &o) const {
		for(u32 i = 0; i < values.size(); i++) {
			o.push_back(values[i]);
		}
	}
	const char *getAlias() const {
		return alias.c_str();
	}
};
class wolfSetArray_c {
	arraySTD_c<wolfSet_c> sets;
public: 
	void clear() {
		sets.clear();
	}
	wolfSet_c &nextSet() {
		return sets.pushBack();
	}
	const wolfSet_c *findSet(const char *name) const {
		for(u32 i = 0; i < sets.size(); i++) {
			if(!stricmp(sets[i].getAlias(),name)) {
				return &sets[i];
			}
		}
		return 0;
	}
};
// NOTE: wolfAnim.script first matches the state,
// then the move type, and finally weapon and other conditions.
struct wolfAnimScriptInput_s {
	// RELAXED, ALERT or COMBAT
	str state;
	// idle, walk
	str moveType;
	// mp40, thompson, sten, mauser rifle, snooper rifle, sniper scope, flamethrower, etc. 
	str weapon;
};
class wolfAnimScript_c {
	arraySTD_c<wolfState_c> states;

	bool parseConditions(class parser_c &p, wolfAnimConditions_c &o);
	bool parseConditionalAnim(class parser_c &p, wolfConditionalAnim_c &o);
	bool parseSet(class parser_c &p, wolfSet_c &o);
	bool parseState(class parser_c &p);

	const wolfState_c *findState(const char *stateName) const;
public:
	bool parse(const char *fileName);

	const wolfAnimDef_c *findAnimDef(const wolfAnimScriptInput_s &in) const;
};

#endif // __SHARED_WOLFANIMATIONSCRIPT_H__

