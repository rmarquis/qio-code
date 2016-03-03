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
// urc_element_base.cpp
#include "urc_element_base.h"
#include <shared/parser.h>
#include <shared/cvarFlags.h>
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/cvarAPI.h>

urcElementBase_c::urcElementBase_c() {
	linkCvarToMat = false;
	// set default font
	font = "Arial";
	borderStyle = BS_NOT_SET;
}
void urcElementBase_c::translate(int dX, int dY) {
	rect.translate(dX,dY);
}
bool urcElementBase_c::parseURCElement(class parser_c &p) {
	if(p.atChar('{')==false) {
		return true;
	}
	while(p.atChar('}')==false) {
		if(p.atEOF()) {
			g_core->RedWarning("urcElementBase_c::parseURCElement: unexpected EOF reached while parsing %s\n",p.getDebugFileName());
			return true;
		}
		if(!parseURCProperty(p)) {
			// maybe a common property
			if(!urcElementBase_c::parseURCProperty(p)) {
				p.skipLine();
			}
		}
	}
	onURCElementParsed();
	return false;
}
bool urcElementBase_c::parseURCProperty(class parser_c &p) {
	if(p.atWord("rect")) {
		rect.setMinX(p.getFloat());
		rect.setMinY(p.getFloat());
		rect.setW(p.getFloat());
		rect.setH(p.getFloat());
		return true;
	}
	if(p.atWord("shader")) {
		matName = p.getToken();
		return true;
	}
	if(p.atWord("name")) {
		name = p.getToken();
		return true;
	}
	if(p.atWord("linkcvar")) {
		linkCvar = p.getToken();
		// create CVar if not present
		g_cvars->Cvar_Get(linkCvar,"",CVAR_ARCHIVE);
		return true;
	}
	if(p.atWord("font")) {
		font = p.getToken();
		return true;
	}
	if(p.atWord("linkcvartoshader")) {
		linkCvarToMat = true;
		return true;
	}
	if(p.atWord("bgColor")) {
		bgColor.setR(p.getFloat());
		bgColor.setG(p.getFloat());
		bgColor.setB(p.getFloat());
		bgColor.setA(p.getFloat());
		return true;
	}
	if(p.atWord("fgColor")) {
		fgColor.setR(p.getFloat());
		fgColor.setG(p.getFloat());
		fgColor.setB(p.getFloat());
		fgColor.setA(p.getFloat());
		return true;
	}
	if(p.atWord("borderStyle")) {
		if(p.atWord("RAISED")) {
			borderStyle = BS_RAISED;
		} else if(p.atWord("NONE")) {
			borderStyle = BS_NONE;
		} else if(p.atWord("3D_BORDER")) {
			borderStyle = BS_3D;
		} else {
			g_core->RedWarning("Unknown border type %s in file %s at line %i\n",
				p.getToken(),p.getDebugFileName(),p.getCurrentLineNumber());
		}
		return true;
	}


	
	return false;
}
