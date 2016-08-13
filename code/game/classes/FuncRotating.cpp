/*
============================================================================
Copyright (C) 2013 V.

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
// FuncRotating.cpp
#include "FuncRotating.h"
#include "../g_local.h"
#include <shared/autoCvar.h>
#include <api/coreAPI.h>

DEFINE_CLASS(FuncRotating, "ModelEntity");
DEFINE_CLASS_ALIAS(FuncRotating, func_rotating);

static aCvar_c g_funcRotating_printOrientation("g_funcRotating_printOrientation","0");

FuncRotating::FuncRotating() {
	//bPhysicsBodyKinematic = true;
	bRigidBodyPhysicsEnabled = false;
	// HACK for q3dm0 rotating skeleton models!
	pvsBoundsSkinWidth = 32.f;
	// default rotation axis
	rotationAxis = 1; // rotate around Z axis - yaw 
}

void FuncRotating::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"y_axis")) {
		// "y_axis" "1" is used on Prey's maps/game/shuttlea.map
		int bYAxis = atoi(value);
		if(bYAxis) {
			rotationAxis = 0; // rotate around Y
		}
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void FuncRotating::runFrame() {
	float rotateSpeed = 10;
	float delta = rotateSpeed * g_frameTime;
	vec3_c a = this->getAngles();
	// rotationAxis == 1 -> rotate around Z axis
	// rotationAxis == 0 -> rotate around Y axis
	a[rotationAxis] += delta;
	this->setAngles(a);

	if(g_funcRotating_printOrientation.getInt()) {
		g_core->Print("FuncRotating::runFrame(): pos %f %f %f rot %f %f %f\n",getOrigin().x,getOrigin().y,getOrigin().z,a.x,a.y,a.z);
	}
}

