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
// wolfScript.cpp - ET/RTCW script parser, used for both .ai and .script files
#include "wolfScript.h"
#include <shared/parser.h>


bool wsScript_c::parseScriptBlock(wsScriptBlock_c *o, class parser_c &p) {
	if(!p.atChar('{')) {
		g_core->RedWarning("wsScript_c::parseEntity: expected '{' at line %i of %s - found %s\n",
			p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
		return true;
	}
	while(!p.atChar('}')) {
		p.skipLine();

	}

	return false;
}
bool wsScript_c::parseEntity(class parser_c &p, const char *entityName) {
	if(!p.atChar('{')) {
		g_core->RedWarning("wsScript_c::parseEntity: expected '{' after entity %s at line %i of %s - found %s\n",
			entityName,p.getCurrentLineNumber(),p.getDebugFileName(),p.getToken());
		return true;
	}
	wsEntity_c *e = new wsEntity_c(entityName);
	entities.addObject(e);
	while(!p.atChar('}')) {
		if(p.atWord("attributes")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("spawn")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("trigger")) {
			str triggerName;
			p.getToken(triggerName);
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("stateChange")) {
			str stateFrom, stateTo;
			p.getToken(stateFrom);
			p.getToken(stateTo);
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("death")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("enemySight")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("attackSound")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("inspectSoundStart")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("inspectFriendlyCombatStart")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else if(p.atWord("inspectBodyEnd")) {
			wsScriptBlock_c *sb = new wsScriptBlock_c();
			if(parseScriptBlock(sb,p)) {
				delete sb;
				return true;
			}
			delete sb;
		} else {
			g_core->RedWarning("wsScript_c::parseEntity: unknown token %s found after entity %s at line %i of %s\n",
				p.getToken(),entityName,p.getCurrentLineNumber(),p.getDebugFileName());
		}
	}
	return false;
}
bool wsScript_c::loadScriptFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		return true;
	}
	while(p.atEOF()==false) {
		// NOTE: for .ai files it's "ainame", not "targetname"
		str entityName;
		p.getToken(entityName);
		if(parseEntity(p,entityName)) {
			return true;
		}
	}
	return false;
}
