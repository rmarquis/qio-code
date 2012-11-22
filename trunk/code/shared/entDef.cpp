/*
============================================================================
Copyright (C) 2012 V.

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
// entDef.cpp
#include "entDef.h"
#include "../qcommon/q_shared.h"
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <shared/parser.h>
#include <shared/str.h>

bool entDef_c::parseSingleEntDef(class parser_c &p) {
	bool hasOpeningBrace = false;
	if(p.atChar('{')) {
		hasOpeningBrace = true;
	} else {
		hasOpeningBrace = false;
	}
	while(p.atChar('}') == false) {
		if(p.atEOF()) {
			break;
		}
		if(p.atChar('}')) {
			g_core->RedWarning("entDef_c::fromString: unexpected '}'\n");
			return true; // error
		}
		if(p.atChar('{')) {
			g_core->RedWarning("entDef_c::fromString: unexpected '{'\n");
			return true; // error
		}
		str key = p.getToken();
		str val = p.getToken();
		this->setKeyValue(key,val);
	}
	return false;
}
bool entDef_c::fromString(const char *text) {
	parser_c p;
	p.setup(text);	
	return parseSingleEntDef(p);
}

bool entDef_c::readFirstEntDefFromFile(const char *fileName) {
	parser_c p;
	if(p.openFile(fileName)) {
		g_core->RedWarning("entDef_c::readFirstEntDefFromFile: cannot open \"%s\"\n",fileName);
		return true; // error
	}
	return parseSingleEntDef(p);
}
