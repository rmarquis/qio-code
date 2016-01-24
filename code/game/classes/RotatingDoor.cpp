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
// RotatingDoor.cpp
#include "RotatingDoor.h"
#include <api/serverAPI.h>
#include <api/physAPI.h>

DEFINE_CLASS(RotatingDoor, "ModelEntity");
// RTCW rotating door
DEFINE_CLASS_ALIAS(RotatingDoor, func_door_rotating);
// MoHAA/FAKK rotating door
DEFINE_CLASS_ALIAS(RotatingDoor, func_rotatingdoor);

RotatingDoor::RotatingDoor() {
	bRigidBodyPhysicsEnabled = true;
	hinge = 0;
}
RotatingDoor::~RotatingDoor() {
	if(this->hinge) {
		g_physWorld->destroyPhysicsConstraint(hinge);
		hinge = 0;
	}
}
void RotatingDoor::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"angle")) {
		// that's door opening angle and not the ModelEntity orientation angle
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void RotatingDoor::postSpawn() {
	ModelEntity::postSpawn();


	vec3_c axis(0,0,1);
	hinge = g_physWorld->createConstraintHinge(this->getOrigin(),axis,this->getRigidBody(),0);
}

