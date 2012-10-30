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
// cmWinding.h - a set of points (usually lying on the single plane)
#ifndef __CMWINDING_H__
#define __CMWINDING_H__

#include <math/vec3.h>
#include "array.h"

class cmWinding_c {
	arraySTD_c<vec3_c> points;
public:
	bool createBaseWindingFromPlane(const class plane_c &pl, float maxCoord = 8192.f);
	void addWindingPointsUnique(const vec3_c *addPoints, u32 numPointsToAdd);
	void addWindingPointsUnique(const arraySTD_c<vec3_c> &otherPoints) {
		addWindingPointsUnique(otherPoints.getArray(),otherPoints.size());
	}
	void addWindingPointsUnique(const cmWinding_c &other) {
		addWindingPointsUnique(other.points.getArray(),other.points.size());
	}
	enum planeSide_e clipWindingByPlane(const class plane_c &pl, float epsilon = 0.001f);
	void getBounds(class aabb &out) const;
	void addPointsToBounds(class aabb &out) const;
	void removeDuplicatedPoints(float epsilon = 0.001f);
};

#endif // __CMWINDING_H__
