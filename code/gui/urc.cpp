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
#include "urc_element_slider.h"
#include "urc_element_bindlist.h"
#include <shared/parser.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <shared/autoCVar.h>

static aCvar_c urc_skipLabels("urc_skipLabels","0");

urc_c::urc_c() {
	bVirtualScreen = false;
	verticalAlign = VA_DEFAULT;
	horizontalAlign = HA_DEFAULT;
	virtualScreenW = 640;
	virtualScreenH = 480;
}
bool urc_c::filterURCElement(const class urcElementBase_c *el) const {
	if(!stricmp(el->getName(),"disconnect")) {
		return true;
	}
	if(!stricmp(el->getName(),"backtogame")) {
		return true;
	}
	if(urc_skipLabels.getInt()) {
		if(el->isLabel())
			return true;
	}
	return false;
}
void urc_c::drawURC(class urcMgr_c *pMgr) {	
	for(u32 i = 0; i < elements.size(); i++) {
		urcElementBase_c *el = elements[i];
		// allow certain elements to be hidden
		if(filterURCElement(el))
			continue;
		el->renderURCElement(pMgr);
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
			g_core->Cbuf_AddText(";");
			g_core->Cbuf_AddText(el->getStuffCommand());
			g_core->Cbuf_AddText(";");

			if(el->isField()) {
				mgr->setActiveField(dynamic_cast<urcElementField_c*>(el));
			} else if(el->isCheckBox()) {
				(dynamic_cast<urcElementCheckbox_c*>(el))->toggleCheckBox();
			} else if(el->isPullDown()) {
				mgr->setActivePullDown(dynamic_cast<urcElementPullDown_c*>(el));
			} else if(el->isSlider()) {
				urcElementSlider_c *s = (dynamic_cast<urcElementSlider_c*>(el));
				float f = s->getRect().getXFrac(mouseX);
				s->setFraction(f);
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
				g_core->Cbuf_AddText(";");
				g_core->Cbuf_AddText(hcmd);
				g_core->Cbuf_AddText(";");
			}
			break;
		}
	}
}
bool urc_c::parseURCFile(class parser_c &p) {
	if(p.atWord("menu")==false) {

	}
	p.getToken();
	sizeX = p.getInteger();
	sizeY = p.getInteger();

	while(p.atEOF() == false) {
		if(p.atWord("align")) {
			// All of those combinations should be supported:
			// align right
			// align top right
			// align right top
			// They are used in MoHAA and FAKK
			for(u32 i = 0; i < 2; i++) {
				// this is not used for fullscreen menus,
				// it is used for. eg hud_health 
				// (healthbar which is always in lower left corner)
				if(p.atWord("right")) {	
					horizontalAlign = HA_RIGHT;
				} else if(p.atWord("left")) {
					horizontalAlign = HA_LEFT;
				} else if(p.atWord("top")) {
					verticalAlign = VA_TOP;
				} else if(p.atWord("bottom")) {
					verticalAlign = VA_BOTTOM;
				} else {
					g_core->RedWarning("URC %s has unknown align type %s at line %i\n",getName(),p.getToken(),p.getCurrentLineNumber());
				}
				if(p.isAtEOL())
					break;
			}
		} else if(p.atWord("virtualScreenSize")) {
			// added for Qio
			virtualScreenW = p.getInteger();
			virtualScreenH = p.getInteger();
			//g_core->Print("URC %s is using align keyword\n",getName());
		} else if(p.atWord("virtualscreen")) {
			// TODO: what is the difference between virtualScreen and virtualRes? 
			bVirtualScreen = p.getInteger();
		} else if(p.atWord("virtualres")) {
			// TODO: what is the difference between virtualScreen and virtualRes? 
			bVirtualScreen = p.getInteger();
		} else if(p.atWord("resource")) {
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
			} else if(p.atWord("Slider")) {
				el = new urcElementSlider_c();
			} else if(p.atWord("FAKKBindList")) {
				// special control for FAKK and MoHAA key bindings
				el = new urcElementBindList_c();
			} else if(p.atWord("FAKKLoadGameClass")) {
				// special control for FAKK and MoHAA save games
				el = new urcElementLabel_c();
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
void urc_c::translate(int dX, int dY) {	
	for(u32 i = 0; i < elements.size(); i++) {
		elements[i]->translate(dX,dY);
	}	
	bounds.translate(dX,dY);
}
bool urc_c::loadURCFile() {
	parser_c p;
	if(p.openFile(fname))
		return true;

	if(parseURCFile(p))
		return true;
	
	// calc bounds
	bounds.clear();
	for(u32 i = 0; i < elements.size(); i++) {
		bounds.add(elements[i]->getRect());
	}

	if(verticalAlign != VA_DEFAULT) {
		if(verticalAlign == VA_BOTTOM) {
			int ofs = rf->getWinHeight()-sizeY;
			translate(0,ofs);
		}
	}
	if(horizontalAlign != HA_DEFAULT) {
		if(horizontalAlign == HA_LEFT) {
		} else if(horizontalAlign == HA_RIGHT) {
			int ofs = rf->getWinWidth()-sizeX;
			translate(ofs,0);
		}
	}

	return false;
}
