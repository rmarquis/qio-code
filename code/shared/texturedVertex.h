/*
============================================================================
Copyright (C) 2015 V.

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
// texturedVertex.h - simple vertex with XYZ positon and ST texture coordinates
#ifndef __SHARED_TEXTUREDVERTEX_H__
#define __SHARED_TEXTUREDVERTEX_H__

#include <math/vec3.h>
#include <math/vec2.h>

class texturedVertex_c {
public:
	vec3_c xyz;
	vec2_c st;


	void setXYZ(const float *newXYZ) {
		xyz = newXYZ;
	}
	const vec3_c &getXYZ() const {
		return xyz;
	}
	float dotProduct(const float *pVec3) const {
		return xyz.dotProduct(pVec3);
	}
	bool vectorCompare(const vec3_c &other, float epsilon = EQUAL_EPSILON) const {
		return xyz.vectorCompare(other,epsilon);
	}
	void setTC(float s, float t) {
		st[0] = s;
		st[1] = t;
	}
	const float operator [] (int index) const {
		return (&xyz.x)[index];
	}
	float &operator [] (int index) {
		return (&xyz.x)[index];
	}
	operator vec3_c &() {
		return xyz;
	}
	operator float *() {
		return &xyz.x;
	}
};

#endif // __SHARED_TEXTUREDVERTEX_H__
