/*
============================================================================
Copyright (C) 2012 V.

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
// Door.cpp
#include "Door.h"
#include <api/serverAPI.h>

DEFINE_CLASS(Door, "ModelEntity");
DEFINE_CLASS_ALIAS(Door, func_door);
// RTCW rotating door
DEFINE_CLASS_ALIAS(Door, func_door_rotating);
// MoHAA/FAKK rotating door
DEFINE_CLASS_ALIAS(Door, func_rotatingdoor);

Door::Door() {
	bPhysicsBodyKinematic = true;
}
void Door::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"angle")) {
		// that's door opening angle and not the ModelEntity orientation angle
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void Door::postSpawn() {
	// close touched areaportal (but only if we're using new areaPortals system)
	//u32 touchingAreas = getNumTouchingAreas();
	//if(touchingAreas > 1) {
	//	int area0 = this->getTouchingArea(0);
	//	int area1 = this->getTouchingArea(1);
	//	// mark portal as closed (by this doors)
	//	g_server->adjustAreaPortalState(area0,area1,false);
	//}
}

