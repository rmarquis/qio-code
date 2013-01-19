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
	cmWinding_c() {

	}
	cmWinding_c(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) {
		points.push_back(p0);
		points.push_back(p1);
		points.push_back(p2);
	}
	bool createBaseWindingFromPlane(const class plane_c &pl, float maxCoord = 131072.f);
	void addWindingPointsUnique(const vec3_c *addPoints, u32 numPointsToAdd);
	void addWindingPointsUnique(const arraySTD_c<vec3_c> &otherPoints) {
		addWindingPointsUnique(otherPoints.getArray(),otherPoints.size());
	}
	void addWindingPointsUnique(const cmWinding_c &other) {
		addWindingPointsUnique(other.points.getArray(),other.points.size());
	}
	enum planeSide_e clipWindingByPlane(const class plane_c &pl, float epsilon = 0.1f);
	void getBounds(class aabb &out) const;
	void getPlane(class plane_c &out) const;
	void addPointsToBounds(class aabb &out) const;
	void removeDuplicatedPoints(float epsilon = 0.001f);
	void addPointsUnique(const vec3_c *first, u32 numPoints, float epsilon = 0.001f);
	
	void iterateTriangles(void (*triCallback)(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2));
	void iterateTriangles(class staticModelCreatorAPI_i *smc);

	cmWinding_c getReversed() const {
		cmWinding_c ret;
		ret.resize(points.size());
		int j = points.size()-1;
		for(u32 i = 0; i < points.size(); i++) {
			ret.points[j] = points[i];
			j--;
		}
		return ret;
	}

	void resize(u32 newPointsCount) {
		points.resize(newPointsCount);
	}
	const vec3_c *getArray() const {
		return points.getArray();
	}
	vec3_c *getArray() {
		return points.getArray();
	}
	u32 size() const {
		return points.size();
	}
	inline cmWinding_c *copy() const {
		cmWinding_c *ret = new cmWinding_c;
		*ret = *this;
		return ret;
	}
	const arraySTD_c<vec3_c> &getPoints() const {
		return points;
	}
	const vec3_c &operator [] (u32 index) const {
		return points[index];
	}
};

#endif // __CMWINDING_H__
