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
// urc_element_pulldown.cpp
#include "urc_element_pulldown.h"
#include <shared/parser.h>
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/fontAPI.h>
#include <api/cvarAPI.h>

urcElementPullDown_c::urcElementPullDown_c() {

}
void urcElementPullDown_c::onURCElementParsed() {
	
}
bool urcElementPullDown_c::parseURCProperty(class parser_c &p) {
	if(p.atWord("menushader")) {
		p.getToken(); // "MENU"
		p.getToken(menuMaterial); // material name
		return true;
	} else if(p.atWord("selmenushader")) {
		p.getToken(); // "MENU"
		p.getToken(selMenuMaterial); // material name
		return true;
	} else if(p.atWord("addpopup")) {
		pullDownOption_c pd;
		// addpopup "MENU" "None" command "set cg_drawviewmodel 0"
		p.getToken(); // "MENU"
		p.getToken(pd.label);
		if(p.atWord("command")) {
			p.getToken(pd.command); // eg. "set cg_drawviewmodel 0"
			options.push_back(pd);
		} else {

		}
		return true;
	}
	return false;
}
void urcElementPullDown_c::renderURCElement() {
	const rect_c &r = this->getRect();
	const char *matName = menuMaterial;
	if(matName[0]) {
		if(0) {
			g_core->Print("Material %s\n",matName);
		}
		rf->drawStretchPic(r.getX(),r.getY(),r.getW(),r.getH(),0,0,1,1,matName);
	}
}


