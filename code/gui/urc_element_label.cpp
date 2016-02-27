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
#include <api/rAPI.h>
#include <api/guiAPI.h>
#include <api/cvarAPI.h>

bool urcElementLabel_c::parseURCProperty(class parser_c &p) {
	
	return false;
}

void urcElementLabel_c::renderURCElement() {
	const rect_c &r = this->getRect();

	const char *matName;
	char tmp[512];
	if(linkCvarToMat) {
		g_cvars->Cvar_VariableStringBuffer(linkCvar,tmp,sizeof(tmp));
		matName = tmp;
	} else {
		matName = this->getMatName();
	}
	if(matName[0]) {
		if(0) {
			g_core->Print("Material %s\n",matName);
		}
		rf->drawStretchPic(r.getX(),r.getY(),r.getW(),r.getH(),0,0,1,1,matName);
	}
}

