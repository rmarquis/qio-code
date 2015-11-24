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
// frustumExt.h - extended frustum class (with the variable number of planes)
#ifndef __FRUSTUMEXT_H__
#define __FRUSTUMEXT_H__

#include <shared/array.h>
#include "plane.h"

// V: it seems that reallocating those planes is infact a bit slow
#if 1
#define FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY 
#endif

#define FRUSTUM_EXT_MAX_PLANES 16

class frustumExt_c {
public:
#ifdef FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY
	plane_c planes[FRUSTUM_EXT_MAX_PLANES];
	u32 curCount;
#else
	arraySTD_c<plane_c> planes;
#endif

	frustumExt_c() {
#ifdef FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY
		curCount = 0;
#endif
	}
	frustumExt_c(const class frustum_c &fr);

	enum cullResult_e cull(const class aabb &bb) const;
	void adjustFrustum(const frustumExt_c &other, const vec3_c &eye, const class cmWinding_c &points, const plane_c &plane);
	void adjustFrustum(const frustumExt_c &other, const vec3_c &eye, const class vec3_c *points, u32 numPoints, const plane_c &plane);
	void fromPointAndWinding(const vec3_c &p, const class cmWinding_c &points, const plane_c &plane);

	inline const plane_c&	operator [] (const int index) const {
		return planes[index];
	}
	inline plane_c&	operator [] (const int index) {
		return planes[index];
	}
	inline void addPlane(const plane_c &newPlane) {
#ifdef FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY
		if(curCount >= FRUSTUM_EXT_MAX_PLANES) {
			return;
		}
		planes[curCount] = newPlane;
		curCount++;
#else
		planes.push_back(newPlane);
#endif
	}
	inline void clear() {
#ifdef FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY
		curCount = 0;
#else
		planes.clear();
#endif
	}
	inline u32 size() const {
#ifdef FRUSTUM_EXT_USE_FIXED_SIZE_ARRAY
		return curCount;
#else
		return planes.size();
#endif
	}
};

#endif // __FRUSTUMEXT_H__
