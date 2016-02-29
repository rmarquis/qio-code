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
// urc_element_field.cpp
#include "urc_element_field.h"
#include <shared/parser.h>
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/fontAPI.h>
#include <api/cvarAPI.h>

urcElementField_c::urcElementField_c() {
	currentCursor = 0;
}
void urcElementField_c::onURCElementParsed() {
	// get linked cvar value and set field text
	if(linkCvar.size()) {
		char tmp[512];
		g_cvars->Cvar_VariableStringBuffer(linkCvar,tmp,sizeof(tmp));
		this->currentText = tmp;
	}
}
bool urcElementField_c::parseURCProperty(class parser_c &p) {

	return false;
}
void urcElementField_c::onKeyDown(int keyCode) {
	if(keyCode == 127) {
		if(currentCursor) {
			currentText.removeCharBefore(currentCursor);
			currentCursor--;
		}
	} else if(keyCode == 135) {
		if(currentCursor <= currentText.size()) {
			currentCursor++;
		}
	} else if(keyCode == 134) {
		if(currentCursor) {
			currentCursor--;
		}
	} else if(isalnum(keyCode) || keyCode == ' ') {
		currentText.insertAt(currentCursor,keyCode);
		currentCursor++;
	}
	if(linkCvar.size()) {
		if(1) {
			g_core->Print("urcElementField_c::onKeyDown: updating CVar %s\n",linkCvar.c_str());
		}
		g_cvars->Cvar_Set(linkCvar,currentText);
	}
}
void urcElementField_c::renderURCElement(class urcMgr_c *pMgr) {
	const rect_c &r = this->getRect();
	const char *matName = this->getMatName();
	if(matName[0]) {
		if(0) {
			g_core->Print("Material %s\n",matName);
		}
		rf->drawStretchPic(r.getX(),r.getY(),r.getW(),r.getH(),0,0,1,1,matName);
	}
	fontAPI_i *f = rf->registerFont("Arial");
	if(f) {
		str tmp = currentText;
		tmp.insertAt(currentCursor,'|');
		f->drawString(r.getX(),r.getY(),tmp);
	}
}

