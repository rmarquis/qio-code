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

class wolfAnimCondition_c {

};
class wolfAnimConditions_c {
	arraySTD_c<wolfAnimCondition_c> conditions;
public:
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
// set of animations
class wolfStateSet_c {
friend class wolfAnimScript_c;
	// "idle", "walk", etc.
	str setName;
	arraySTD_c<wolfConditionalAnim_c> conditionalCases;
};
class wolfState_c {
friend class wolfAnimScript_c;
	// RELAXED, ALERT, etc.
	str stateName; 
	arraySTD_c<wolfStateSet_c> sets;
};
class wolfAnimScript_c {
	arraySTD_c<wolfState_c> states;

	bool parseConditions(class parser_c &p, wolfAnimConditions_c &o);
	bool parseConditionalAnim(class parser_c &p, wolfConditionalAnim_c &o);
	bool parseState(class parser_c &p);
public:
	bool parse(const char *fileName);
};

#endif // __SHARED_WOLFANIMATIONSCRIPT_H__

