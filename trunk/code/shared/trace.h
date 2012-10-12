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
// trace.h
#ifndef __TRACE_H__
#define __TRACE_H__

#include "../math/vec3.h"
#include "../math/aabb.h"
//#include "../math/plane.h"

class trace_c {
	// input
	vec3_c from;
	vec3_c to;
	// derived from input
	aabb traceBounds;
	vec3_c delta;
	float len;
	// output
	vec3_c hitPos;
	float fraction;
	float traveled;
	//plane_c hitPlane;

	void updateForNewHitPos();
public:

	void recalcRayTraceBounds();
	void setupRay(const vec3_c &newFrom, const vec3_c &newTo);

	bool clipByTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2, bool twoSided = false);

	const vec3_c &getStartPos() const {
		return from;
	}
	const vec3_c &getHitPos() const {
		return hitPos;
	}
	const vec3_c getDir() const {
		return delta.getNormalized();
	}
	float getFraction() const {
		return fraction;
	}
	const aabb &getTraceBounds() const {
		return traceBounds;
	}

};

#endif // __TRACE_H__
