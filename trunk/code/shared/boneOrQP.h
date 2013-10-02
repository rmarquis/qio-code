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
// shared/boneOrQP.h
#ifndef __BONEORQP_H_
#define __BONEORQP_H_

#include <math/vec3.h>
#include <math/quat.h>

// bone orientation defined by 3d vector and a quaterion - total 28 bytes
// (NOTE: boneOr_s from skelUtils.h is more than two times larger: 68 bytes)
class boneOrQP_c {
	vec3_c p;
	quat_c q;
public:
	void setPos(const float *newPos) {
		p = newPos;
	}
	void setQuatXYZ(const vec3_c &xyzQuat) {
		q.x = xyzQuat.x;
		q.y = xyzQuat.y;
		q.z = xyzQuat.z;
		q.calcW();
	}
	void setQuat(const quat_c &newQuat) {
		q = newQuat;
	}
	const quat_c &getQuat() const {
		return q;
	}
	const vec3_c &getPos() const {
		return p;
	}
};

class boneOrQPArray_t : public arraySTD_c<boneOrQP_c> {
public:
	void setVec3(u32 boneIndex, const vec3_c &v) {
		(*this)[boneIndex].setPos(v);
	}
	void setQuat(u32 boneIndex, const quat_c &q) {
		(*this)[boneIndex].setQuat(q);
	}
};

#endif // __BONEORQP_H_
