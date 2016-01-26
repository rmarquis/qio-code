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

wolfSetArray_c g_weaponsSet;
wolfSetArray_c g_moveTypesSet;
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
			wolfAnimCondition_c &weaponsCondition = o.conditions.pushBack();
			weaponsCondition.type = WC_WEAPONS;
			const char *w = p.getToken();
			const wolfSet_c *extended = g_weaponsSet.findSet(w);
			if(extended) {
				extended->addValuesTo(weaponsCondition.data);
			} else {
				weaponsCondition.data.push_back(w);
			}
		} else if(p.atWord("moveType")) {
			p.skipLine();
		} else if(p.atWord("special_condition")) {
			wolfAnimCondition_c &con = o.conditions.pushBack();
			con.type = WC_SPECIAL;
			p.skipLine();
		} else if(p.atWord("mounted")) {
			wolfAnimCondition_c &con = o.conditions.pushBack();
			con.type = WC_MOUNTED;
			p.skipLine();
		} else if(p.atWord("default")) {
			wolfAnimCondition_c &defaultCondition = o.conditions.pushBack();
			defaultCondition.type = WC_DEFAULT;
		} else if(p.atWord("firing")) {
			wolfAnimCondition_c &con = o.conditions.pushBack();
			con.type = WC_FIRING;
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
		wolfStateMove_c &set = state.moveStates.pushBack();
		str moveType = p.getToken();
		set.moveType = moveType;
		if(p.atChar('{')==false) {
			g_core->RedWarning("wolfAnimScript_c::parseState: expected '{' at line %i of %s (after moveType %s) - found %s\n",
				p.getCurrentLineNumber(),p.getDebugFileName(),moveType.c_str(),p.getToken());
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

bool wolfAnimScript_c::parseSet(class parser_c &p, wolfSet_c &o) {
	o.alias = p.getToken();
	if(p.atChar('=')==false) {	
		g_core->RedWarning("wolfAnimScript_c::parse: expected '=' after 'set weapons %s', found %s at line %i of %s\n",o.alias.c_str(),p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
		return true;
	}
	g_core->Print("wolfAnimScript_c::parseSet: alias: <%s>\n",o.alias.c_str());
	str caseName;
	while(p.isAtEOL() == false) {
		if(p.atWord("AND")) {
			g_core->Print("wolfAnimScript_c::parseSet: caseName: <%s>\n",caseName.c_str());
			o.values.push_back(caseName);
			caseName.clear();
		} else {
			if(caseName.size()) {
				caseName.append(" ");
			}
			caseName.append(p.getToken());
		}
	}
	return false;
}
const wolfState_c *wolfAnimScript_c::findState(const char *stateName) const {
	for(u32 i = 0; i < states.size(); i++) {
		if(!stricmp(states[i].stateName,stateName))
			return &states[i];
	}
	return 0;
}
bool wolfStateMove_c::hasMoveType(const char *name) const {
	if(!stricmp(moveType,name))
		return true;
	return false;
}
const wolfStateMove_c *wolfState_c::findMoveType(const wolfAnimScriptInput_s &in) const {
	for(u32 i = 0; i < moveStates.size(); i++) {
		if(moveStates[i].hasMoveType(in.moveType)) {
			return &moveStates[i];
		}
	}
	return 0;
}
bool wolfAnimConditions_c::checkConditions(const wolfAnimScriptInput_s &in) const {
	for(u32 i = 0; i < conditions.size(); i++) {
		const wolfAnimCondition_c &con = conditions[i];
		if(con.type == WC_DEFAULT) {
			continue;
		}
		if(con.type == WC_WEAPONS) {
			// is current weapon on list?
			if(con.isOnDataList(in.weapon)==false) {
				return false; // not met
			}
		}
	}
	return true;
}
const wolfAnimDef_c *wolfAnimScript_c::findAnimDef(const wolfAnimScriptInput_s &in) const {
	const char *stateName = in.state;
	const wolfState_c *state = findState(stateName);
	if(state == 0) {
		return 0;
	}
	const wolfStateMove_c *m = state->findMoveType(in);
	if(m == 0) {
		return 0;
	}
	for(u32 i = 0; i < m->conditionalCases.size(); i++) {
		const wolfAnimConditions_c &cons = m->conditionalCases[i].conditions;
		if(cons.checkConditions(in)) {
			// all conditions met - use first one
			return &m->conditionalCases[i].def;
		}
	}
	return 0;
}
bool wolfAnimScript_c::parse(const char *fileName) {
	parser_c p;
	if(p.openFile(fileName)) {
		return true;
	}
	g_moveTypesSet.clear();
	g_weaponsSet.clear();
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
			if(p.atWord("weapons")) {
				wolfSet_c &newSet = g_weaponsSet.nextSet();
				if(parseSet(p,newSet)) {
					return true;
				}
			} else if(p.atWord("moveType")) {
				wolfSet_c &newSet = g_moveTypesSet.nextSet();
				if(parseSet(p,newSet)) {
					return true;
				}
			} else {
				g_core->RedWarning("wolfAnimScript_c::parse: unknown set type %s at line %i of %s\n",p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
				p.skipLine();
			}
		} else  {
			p.skipLine();
		}
	}
	// testing
	wolfAnimScriptInput_s s;
	s.state = "COMBAT";
	s.moveType = "walk";
	s.weapon = "mp40"; // it's two handed
	const wolfAnimDef_c *ad = findAnimDef(s); // should be both alert_run_2h
	s.weapon = "luger"; // it's one handed
	const wolfAnimDef_c *ad2 = findAnimDef(s); // should be both alert_run_1h
	s.weapon = "none"; // it's none
	const wolfAnimDef_c *ad3 = findAnimDef(s); // should be both alert_run_no

	return false; // no error
}