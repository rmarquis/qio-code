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
#include "wolfAnimScript.h"
#include <shared/parser.h>

enum wolfAnimScriptSection_e {
	WASS_NOT_SET,
	WASS_DEFINES,
	WASS_ANIMATIONS,
	WASS_CANNED_ANIMATIONS,
	WASS_EVENTS,
	WASS_STATECHANGES,
	
};
/*

		// example conditions string
		// 	weapons pistols, movetype walk AND turnleft AND turnright AND idlecr AND idle
		
		// parse conditions
		while(p.atChar('{')==false) {
			*/
bool wolfAnimScript_c::parseConditionalAnim(class parser_c &p, wolfConditionalAnim_c &o) {
	if(parseConditions(p,o.conditions)) {
		g_core->RedWarning("wolfAnimScript_c::parseState: error parsing conditions at line %i of %s - found %s\n",
			p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
		return true;
	}
	// parse animations
	while(p.atChar('}')==false) {
		if(p.atWord("torso")) {
			o.def.torso = p.getToken();
		} else if(p.atWord("both")) {
			o.def.both = p.getToken();
		} else if(p.atWord("legs")) {
			o.def.legs = p.getToken();
		} else {
			g_core->RedWarning("wolfAnimScript_c::parseState: unknown token %s at line %i of %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
		}
	}
	return false;
}
bool wolfAnimScript_c::parseConditions(class parser_c &p, wolfAnimConditions_c &o) {
	// 	weapons pistols, movetype walk AND turnleft AND turnright AND idlecr AND idle
	while(p.atChar('{')==false) {
		if(p.atWord("weapons")) {
			p.skipLine();
		} else if(p.atWord("moveType")) {
			p.skipLine();
		} else if(p.atWord("special_condition")) {
			p.skipLine();
		} else if(p.atWord("mounted")) {
			p.skipLine();
		} else if(p.atWord("default")) {

		} else {
			g_core->RedWarning("wolfAnimScript_c::parseState: unknown condition %s at line %i of %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
		}
	}
	return false;
}
bool wolfAnimScript_c::parseState(class parser_c &p) {
	wolfState_c &state = states.pushBack();
	str stateName = p.getToken();
	state.stateName = stateName;
	g_core->Print("wolfAnimScript_c::parseState: state %s\n",stateName.c_str());
	if(p.atChar('{')==false) {
		g_core->RedWarning("wolfAnimScript_c::parseState: expected '{' at line %i of %s (after state %s) - found %s\n",
			p.getCurrentLineNumber(),p.getDebugFileName(),stateName.c_str(),p.getToken());
		return true;
	}
	while(p.atChar('}')==false) {
		wolfStateSet_c &set = state.sets.pushBack();
		str setName = p.getToken();
		set.setName = setName;
		if(p.atChar('{')==false) {
			g_core->RedWarning("wolfAnimScript_c::parseState: expected '{' at line %i of %s (after set %s) - found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),setName.c_str(),p.getToken());
			return true;
		}
		while(p.atChar('}')==false) {
			wolfConditionalAnim_c &ca = set.conditionalCases.pushBack();
			if(parseConditionalAnim(p,ca)) {

			}
		}
	}
	return false;
}

bool wolfAnimScript_c::parse(const char *fileName) {
	parser_c p;
	if(p.openFile(fileName)) {
		return true;
	}
	wolfAnimScriptSection_e curSec = WASS_NOT_SET;
	while(p.atEOF() == false) {
		if(p.atWord("DEFINES")) {
			curSec = WASS_DEFINES;
		} else if(p.atWord("ANIMATIONS")) {
			curSec = WASS_ANIMATIONS;
		} else if(p.atWord("CANNED_ANIMATIONS")) {
			curSec = WASS_CANNED_ANIMATIONS;
		} else if(p.atWord("EVENTS")) {
			curSec = WASS_EVENTS;
		} else if(p.atWord("STATECHANGES")) {
			curSec = WASS_STATECHANGES;
		} else if(p.atWord("STATE")) {
			if(parseState(p)) {

			}
		} else if(p.atWord("set")) {
			p.skipLine();
		} else  {
			p.skipLine();
		}
	}
	return false; // no error
}