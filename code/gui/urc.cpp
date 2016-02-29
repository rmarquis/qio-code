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
// urc.cpp
#include "urc.h"
#include "urc_mgr.h"
#include "urc_element_label.h"
#include "urc_element_button.h"
#include "urc_element_field.h"
#include "urc_element_checkbox.h"
#include "urc_element_pulldown.h"
#include <shared/parser.h>
#include <api/coreAPI.h>

bool urc_c::filterURCElement(const class urcElementBase_c *el) const {
	if(!stricmp(el->getName(),"disconnect")) {
		return true;
	}
	if(!stricmp(el->getName(),"backtogame")) {
		return true;
	}
	return false;
}
void urc_c::drawURC() {	
	for(u32 i = 0; i < elements.size(); i++) {
		urcElementBase_c *el = elements[i];
		// allow certain elements to be hidden
		if(filterURCElement(el))
			continue;
		el->renderURCElement();
	}
}
void urc_c::onKeyDown(int keyCode) {
}
void urc_c::onMouseDown(int keyCode, int mouseX, int mouseY, class urcMgr_c *mgr) {
	for(u32 i = 0; i < elements.size(); i++) {
		urcElementBase_c *el = elements[i];
		// allow certain elements to be hidden
		if(filterURCElement(el))
			continue;
		if(el->isClickable()==false)
			continue;
		const rect_c &r = el->getRect();
		if(r.isInside(mouseX,mouseY)) {
			g_core->Print("Clicked element %s with material %s\n",el->getName(),el->getMatName());
			// NOTE: multiple commands can be separated by ;
			g_core->Cbuf_AddText(el->getStuffCommand());

			if(el->isField()) {
				mgr->setActiveField(dynamic_cast<urcElementField_c*>(el));
			} else if(el->isCheckBox()) {
				(dynamic_cast<urcElementCheckbox_c*>(el))->toggleCheckBox();
			}
			break;
		}
	}
}
void urc_c::onMouseMove(int mouseX, int mouseY) {
	for(u32 i = 0; i < elements.size(); i++) {
		urcElementBase_c *el = elements[i];
		// allow certain elements to be hidden
		if(filterURCElement(el))
			continue;
		if(el->isClickable()==false)
			continue;
		const rect_c &r = el->getRect();
		if(r.isInside(mouseX,mouseY)) {
			const char *hcmd = el->getHoverCommand();
			if(hcmd && hcmd[0]) {
				if(1) {
					g_core->Print("Executing hover command %s\n",hcmd);
				}
				// NOTE: multiple commands can be separated by ;
				g_core->Cbuf_AddText(hcmd);
			}
			break;
		}
	}
}
bool urc_c::parseURCFile(class parser_c &p) {

	while(p.atEOF() == false) {
		if(p.atWord("resource")) {
			urcElementBase_c *el;
			if(p.atWord("Label")) {
				el = new urcElementLabel_c();
			} else if(p.atWord("Button")) {
				el = new urcElementButton_c();
			} else if(p.atWord("Field")) {
				el = new urcElementField_c();
			} else if(p.atWord("CheckBox")) {
				el = new urcElementCheckbox_c();
			} else if(p.atWord("PulldownMenuContainer")) {
				el = new urcElementPullDown_c();
			} else {
				str type = p.getToken();
				el = new urcElementLabel_c();
			}
			if(el->parseURCElement(p)) {
				delete el;
				return true;
			}
			elements.push_back(el);
		} else if(p.atWord("include")) {
			parser_c inc;
			const char *name = p.getToken();
			if(inc.openFile(name)) {
				g_core->RedWarning("urc_c::parseURCFile: failed to open include file %s (included in %s)\n",name,p.getDebugFileName());
			} else {
				if(parseURCFile(inc))
					return true;
			}
		} else {
			p.getToken();
		}
	}
	return false;
}
bool urc_c::loadURCFile() {
	parser_c p;
	if(p.openFile(fname))
		return true;

	return parseURCFile(p);
}
