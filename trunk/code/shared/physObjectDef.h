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
// physObjectDef.h
#ifndef __PHYSOBJECTDEF_H__
#define __PHYSOBJECTDEF_H__

#include <math/matrix.h>

struct physObjectDef_s {
	// 0 mass means that object is non-moveable
	float mass;
	// physics object creation will fail if collisionModel pointer is NULL
	const class cMod_i *collisionModel;
	// model starting transform
	matrix_c transform;
	// extra bounciness
	// used for Q3 grenade launcher projectiles
	float bounciness;
	// model scale
	vec3_c scale;

	physObjectDef_s() {
		mass = 0.f;
		collisionModel = 0;
		bounciness = 0.f;
		scale.set(1,1,1);
	}
	physObjectDef_s(const vec3_c &newXYZ, const vec3_c &newAngles, const vec3_c &newScale, const class cMod_i *newCMod,
		float newMass, bool newBUseDynamicConvexForTrimeshCMod, float newBounciness = 0.f) {
		transform.fromAnglesAndOrigin(newAngles,newXYZ);
		collisionModel = newCMod;
		mass = newMass;
		bounciness = newBounciness;
		scale = newScale;
	}
	bool isStatic() const {
		if(mass == 0.f) {
			return true;
		}
		return false;
	}

	inline void setBounciness(float newBounciness) {
		bounciness = newBounciness;
	}
};

#endif // __PHYSOBJECTDEF_H__

