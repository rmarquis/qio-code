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
// aabb.h - axis aligned bounding box defined by mins/maxs vectors
#ifndef __MATH_AABB_H__
#define __MATH_AABB_H__

#include "vec3.h"

#define AABB_INFINITE 99999999.f

class aabb {
public:
	aabb() {
		maxs.set(-AABB_INFINITE, -AABB_INFINITE, -AABB_INFINITE);
		mins.set(AABB_INFINITE, AABB_INFINITE, AABB_INFINITE);
	}
	aabb(const vec3_c &p) {
		mins = maxs = p;
	}
	void clear() {
		maxs.set(-AABB_INFINITE, -AABB_INFINITE, -AABB_INFINITE);
		mins.set(AABB_INFINITE, AABB_INFINITE, AABB_INFINITE);
	}
	void fromTwoPoints(const vec3_c &p0, const vec3_c &p1) {
		maxs = mins = p0;
		addPoint(p1);
	}
	void fromThreePoints(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) {
		maxs = mins = p0;
		addPoint(p1);
		addPoint(p2);
	}
	void fromRadius(const float radius) {
		mins.x = -radius;
		mins.y = -radius;
		mins.z = -radius;
		maxs.x = radius;
		maxs.y = radius;
		maxs.z = radius;
	}
	void fromCapsuleZ(float h, float r) {
		maxs.z = h * 0.5 + r;
		mins.z = -maxs.z;
		mins.x = mins.y = -r;
		maxs.x = maxs.y = r;
	}
	void fromPointAndRadius(const vec3_c &p, float radius) {
		mins.x = -radius;
		mins.y = -radius;
		mins.z = -radius;
		maxs.x = radius;
		maxs.y = radius;
		maxs.z = radius;
		mins += p;
		maxs += p;
	}
	void fromHalfSize(float halfSize) {
		mins.x = -halfSize;
		mins.y = -halfSize;
		mins.z = -halfSize;
		maxs.x = halfSize;
		maxs.y = halfSize;
		maxs.z = halfSize;
	}
	void fromHalfSizes(const vec3_c &halfSizes) {
		mins.x = -halfSizes.x;
		mins.y = -halfSizes.y;
		mins.z = -halfSizes.z;
		maxs.x = halfSizes.x;
		maxs.y = halfSizes.y;
		maxs.z = halfSizes.z;
	}
	bool isValid() const {
		for(u32 i = 0; i < 3; i++) {
			if(mins[i] > maxs[i]) {
				return false;
			}
		}
		return true;
	}

	void addPoint(float x, float y, float z) {
		if (x > maxs.x)
			maxs.x = x;
		if (y > maxs.y)
			maxs.y = y;
		if (z > maxs.z)
			maxs.z = z;
		if (x < mins.x)
			mins.x = x;
		if (y < mins.y)
			mins.y = y;
		if (z < mins.z)
			mins.z = z;
	}
	void reset(const vec3_c &v) {
		this->mins = this->maxs = v;
	}
	void addPoint(const vec3_c &v) {
		addPoint(v.x,v.y,v.z);
	}
	void addBox(const aabb &bb) {
		if(bb.isValid() == false)
			return;
		addPoint(bb.mins);
		addPoint(bb.maxs);
	}
	bool intersect(const aabb& other) const {
		return mins.x <= other.maxs.x &&
			mins.y <= other.maxs.y &&
			mins.z <= other.maxs.z &&
			maxs.x >= other.mins.x &&
			maxs.y >= other.mins.y &&
			maxs.z >= other.mins.z;
	}
	bool intersectExt(const vec3_c& oMins, const vec3_c &oMaxs, u32 nDim0, u32 nDim1) const {
		return mins[nDim0] <= oMaxs[nDim0] &&
			mins[nDim1] <= oMaxs[nDim1] &&
			maxs[nDim0] >= oMins[nDim0] &&
			maxs[nDim1] >= oMins[nDim1];
	}
	bool isInside(const vec3_c& xyz) const {
		return mins.x <= xyz.x &&
			mins.y <= xyz.y &&
			mins.z <= xyz.z &&
			maxs.x >= xyz.x &&
			maxs.y >= xyz.y &&
			maxs.z >= xyz.z;
	}
	bool isInsideExt(const vec3_c& oMins, const vec3_c &oMaxs, u32 nDim0, u32 nDim1) const {
		return mins[nDim0] >= oMins[nDim0] &&
			mins[nDim1] >= oMins[nDim1] &&
			maxs[nDim0] <= oMaxs[nDim0] &&
			maxs[nDim1] <= oMaxs[nDim1];
	}
	bool isInside(const aabb &o) const {
		return mins[0] >= o.mins[0] &&
			mins[1] >= o.mins[1] &&
			mins[2] >= o.mins[2] &&
			maxs[0] <= o.maxs[0] &&
			maxs[1] <= o.maxs[1] &&
			maxs[2] <= o.maxs[2];
	}
	void capTo(const aabb &o) {
		for(u32 i = 0; i < 3; i++) {
			if(o.mins[i] > mins[i])
				mins[i] = o.mins[i];
			if(o.maxs[i] < maxs[i])
				maxs[i] = o.maxs[i];
		}
	}
	vec3_c getCenter() const {
		return (mins + maxs) * 0.5;
	}
	vec3_c getSizes() const {
		vec3_c out;
		out.x = maxs.x - mins.x;
		out.y = maxs.y - mins.y;
		out.z = maxs.z - mins.z;
		//G_assert(out.x >= 0 && out.y >= 0  && out.z >= 0);
		return out;
	}
	float getRadius() const {
		vec3_c corner;
		for(u32 i = 0; i < 3; i++) {
			float min = abs(mins[i]);
			float max = abs(maxs[i]);
			corner[i] = min > max ? min : max;
		}
		return corner.len();
	}

	// return the axis on which box extents are the largest
	int getLargestAxis() const {
		vec3_c sizes = getSizes();
		return sizes.getLargestAxis();
	}
	float getLargestAxisLen() const {
		vec3_c sizes = getSizes();
		return sizes.getLargestAxisLen();
	}
	bool hasExtentLargerThan(float val) const {
		vec3_c sizes = getSizes();
		if(sizes.hasComponentLargerThan(val))
			return true;
		return false;
	}
	// returns one of AABB corners
    vec3_c getPoint(int _index) const {
        switch(_index) {
            case 0: return vec3_c(mins.x, mins.y, mins.z);
            case 1: return vec3_c(maxs.x, mins.y, mins.z);
            case 2: return vec3_c(mins.x, maxs.y, mins.z);
            case 3: return vec3_c(maxs.x, maxs.y, mins.z);
            case 4: return vec3_c(mins.x, mins.y, maxs.z);
            case 5: return vec3_c(maxs.x, mins.y, maxs.z);
            case 6: return vec3_c(mins.x, maxs.y, maxs.z);
            case 7: return vec3_c(maxs.x, maxs.y, maxs.z);
            default: return vec3_c(0,0,0);
        }
    }
	// this is for GJK algorithm
	vec3_c getSupportPoint(const class vec3_c &dir) const {
		vec3_c best = getPoint(0);
		float bestDot = dir.dotProduct(best);
		for(u32 i = 1; i < 8; i++) {
			const vec3_c p = getPoint(i);
			float dot = dir.dotProduct(p);
			if(dot > bestDot) {
				bestDot = dot;
				best = p;
			}
		}
		return best;
	}
	// returns one of AABB side normals
    static vec3_c getFaceNormal(int _index) {
        switch(_index) {
            case 0: return vec3_c(-1,  0,  0);
            case 1: return vec3_c( 1,  0,  0);
            case 2: return vec3_c( 0, -1,  0);
            case 3: return vec3_c( 0,  1,  0);
            case 4: return vec3_c( 0,  0, -1);
            case 5: return vec3_c( 0,  0,  1);
            default: return vec3_c(0,0,0);
        }
    }
	// returns one of AABB sides planes
    class plane_c getPlane(int _index) const;

	void translate(const vec3_c &delta) {
		maxs += delta;
		mins += delta;
	}
	aabb getTranslated(const vec3_c &delta) const {
		aabb ret;
		ret.mins = this->mins + delta;
		ret.maxs = this->maxs + delta;
		return ret;
	}
	aabb getTransformed(const vec3_c &pos, const class axis_c &ax) const;
	void scaleBB(const float scale) {
		maxs *= scale;
		mins *= scale;
	}
	void swapYZ() {
		maxs.swapYZ();
		mins.swapYZ();
	}
	void extend(float scalar) {
		mins.x -= scalar;
		mins.y -= scalar;
		mins.z -= scalar;
		maxs.x += scalar;
		maxs.y += scalar;
		maxs.z += scalar;
	}
	aabb getExtended(const float f) const {
		aabb ret;
		ret.mins.x = this->mins.x - f;
		ret.mins.y = this->mins.y - f;
		ret.mins.z = this->mins.z - f;
		ret.maxs.x = this->maxs.x + f;
		ret.maxs.y = this->maxs.y + f;
		ret.maxs.z = this->maxs.z + f;
		return ret;
	}
	// calculates the cohen-sutherland outcode for a point and this bounding box.
	char calcOutCode( const vec3_c &point ) const;
	// determines if a linesegment intersects a bounding box. this is based on
	//  the cohen-sutherland line-clipping algorithm.
	bool intersect(const vec3_c &from, const vec3_c &to, vec3_c &outIntercept) const;
	
	// classic mins/maxs access. Index 0 is for mins, index 1 is maxs;
	inline const vec3_c &operator[]( const int index ) const {
		if(index == 0)
			return mins;
		if(index == 1)
			return maxs;
//		assert(index >= 0 && index < 2);
		static vec3_c dummy;
		dummy.clear();
		return dummy;
	}
	inline vec3_c &operator[]( const int index ) {
		if(index == 0)
			return mins;
		if(index == 1)
			return maxs;
//		assert(index >= 0 && index < 2);
		static vec3_c dummy;
		dummy.clear();
		return dummy;
	}

	vec3_c maxs;
	vec3_c mins;
};

#endif // __MATH_AABB_H__
