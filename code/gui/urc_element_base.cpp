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
#include <api/rAPI.h>
#include <api/guiAPI.h>

urcElementBase_c::urcElementBase_c() {
	linkCvarToMat = false;
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
		return true;
	}
	if(p.atWord("linkcvartoshader")) {
		linkCvarToMat = true;
		return true;
	}
	return false;
}
