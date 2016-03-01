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
// urc_element_checkbox.cpp
#include "urc_element_checkbox.h"
#include <shared/parser.h>
#include "urc_mgr.h"
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/cvarAPI.h>

void urcElementCheckbox_c::onURCElementParsed() {
	// get linked cvar value and set checkbox state
	if(linkCvar.size()) {
		char tmp[512];
		g_cvars->Cvar_VariableStringBuffer(linkCvar,tmp,sizeof(tmp));
		checkBoxState = atoi(tmp);
	}
}
bool urcElementCheckbox_c::parseURCProperty(class parser_c &p) {
	if(p.atWord("checked_shader")) {
		p.getToken(checkedMaterial);
		return true;
	}
	if(p.atWord("unchecked_shader")) {
		p.getToken(uncheckedMaterial);
		return true;
	}
	return false;
}
void urcElementCheckbox_c::toggleCheckBox() {
	checkBoxState = !checkBoxState;
	if(checkBoxState) {
		if(checkCommand.size()) {
			g_core->Cbuf_AddText(checkCommand);
		}
	} else {
		if(uncheckCommand.size()) {
			g_core->Cbuf_AddText(uncheckCommand);
		}
	}
	if(linkCvar.size()) {
		if(checkBoxState) {
			g_cvars->Cvar_Set(linkCvar,"1");
		} else {
			g_cvars->Cvar_Set(linkCvar,"0");
		}
	}
}
void urcElementCheckbox_c::renderURCElement(class urcMgr_c *pMgr) {
	const guiRenderer_i *gr = pMgr->getGUIRenderer();
	const rect_c &r = this->getRect();
	const char *matName;
	if(checkBoxState) {
		matName = checkedMaterial;
	} else {
		matName = uncheckedMaterial;
	}
	int checkboxSize = 16;
	if(matName[0]) {
		if(0) {
			g_core->Print("Material %s\n",matName);
		}
		gr->drawStretchPic(r.getX()+6,r.getCenterY()-checkboxSize/2,checkboxSize,checkboxSize,0,0,1,1,matName);
	}
}

