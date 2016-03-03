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
// urc_element_slider.cpp
#include "urc_element_slider.h"
#include "urc_mgr.h"
#include <shared/parser.h>
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/fontAPI.h>
#include <api/cvarAPI.h>

urcElementSlider_c::urcElementSlider_c() {
	currentFrac = 0;
	rangeMin = 0.f;
	rangeMax = 1.f;
	stepSize = 0.1f;
}
void urcElementSlider_c::onURCElementParsed() {
	// get linked cvar value and set field text
	if(linkCvar.size()) {
		char tmp[512];
		//g_cvars->Cvar_VariableStringBuffer(linkCvar,tmp,sizeof(tmp));
		//this->currentText = tmp;
	}
}
void urcElementSlider_c::setFraction(float f) {
	currentFrac = f;
	// map the [0,1] fraction to desired range
	float range = rangeMax - rangeMin;
	float fracStep = stepSize / range;
	float clampedFrac = ceil(f/fracStep)*fracStep;
	this->currentFrac = clampedFrac;
	float val = rangeMin + currentFrac * range;
	if(linkCvar.size()) {
		char tmp[128];
		sprintf(tmp,"%f",val);
		g_cvars->Cvar_Set(linkCvar,tmp);
	}
}
bool urcElementSlider_c::parseURCProperty(class parser_c &p) {
	if(p.atWord("slidertype")) {
		// slidertype float
	} else if(p.atWord("setrange")) {
		// setrange 0 1
		rangeMin = p.getFloat();
		rangeMax = p.getFloat();
	} else if(p.atWord("stepsize")) {
		// stepsize 0.1
		stepSize = p.getFloat();
	} else {

	}
	return false;
}
void urcElementSlider_c::renderURCElement(class urcMgr_c *pMgr) {
	const guiRenderer_i *gr = pMgr->getGUIRenderer();
	const rect_c &r = this->getRect();
	const char *matName = this->getMatName();
	if(matName[0]) {
		if(0) {
			g_core->Print("Material %s\n",matName);
		}
		u32 sizeX = r.getW();
		u32 ofsX = r.getX() + sizeX * currentFrac;
		gr->drawStretchPic(r.getX(),r.getY(),r.getW(),r.getH(),0,0,1,1,matName);
		gr->drawStretchPic(ofsX,r.getY(),14,r.getH(),0,0,1,1,"textures/menu/slider_thumb");
	} else {
		//gr->fillRectRGBA(r.getX(),r.getY(),r.getW(),r.getH(),bgColor.toPointer());
	}
}

