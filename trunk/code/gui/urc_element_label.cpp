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
// urc_element_label.cpp
#include "urc_element_label.h"
#include <shared/parser.h>

bool urcElementBase_c::parseURCElement(class parser_c &p) {
	if(p.atChar('{')==false) {
		return true;
	}
	while(p.atChar('}')==false) {
		if(!parseURCProperty(p)) {
			// maybe a common property
			if(!urcElementBase_c::parseURCProperty(p)) {
				p.skipLine();
			}
		}
	}
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
	return false;
}
bool urcElementLabel_c::parseURCProperty(class parser_c &p) {
	
	return false;
}
