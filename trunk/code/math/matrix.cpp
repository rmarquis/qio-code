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
// matrix.cpp

#include "matrix.h"
#include "aabb.h"
#include "axis.h"
#include "quat.h"

void matrix_c::transformAABB(const aabb &in, aabb &out) const {
#if 0
	out.clear();
	for(int i = 0; i < 8; i++) {
		vec3_c p = in.getPoint(i);
		this->transformPoint(p);
		out.addPoint(p);
	}
#else
	// based upon Real-Time Collision Detection, Christer Ericson

	// for all three axes
	for (int i = 0; i < 3; i++) {
		// start by adding in translation
		out.mins[i] = out.maxs[i] = ( (float*)&this->_v[12] )[i];
		// form extent by summing smaller and larger terms respectively
		for (int j = 0; j < 3; j++) {
			float e = this->_m[j][i] * in.mins[j];
			float f = this->_m[j][i] * in.maxs[j];
			if (e < f) {
				out.mins[i] += e;
				out.maxs[i] += f;
			} else {
				out.mins[i] += f;
				out.maxs[i] += e;
			}
		}
	}
#endif
}

class axis_c matrix_c::getAxis() const {
	axis_c r;
	r.mat[0] = getForward();
	r.mat[1] = getLeft();
	r.mat[2] = getUp();
	return r;
}
void matrix_c::fromAxisAndOrigin(const axis_c &ax, const vec3_c &origin) {
	fromVectorsFLUAndOrigin(ax.mat[0],ax.mat[1],ax.mat[2],origin);
}
#if 0
void matrix_c::fromQuat(const quat_c &q) {
#if 0
	float xx,xy,xz,xw,yy,yz,yw,zz,zw;

	xx      = q[0] * q[0];
	xy      = q[0] * q[1];
	xz      = q[0] * q[2];
	xw      = q[0] * q[3];

	yy      = q[1] * q[1];
	yz      = q[1] * q[2];
	yw      = q[1] * q[3];

	zz      = q[2] * q[2];
	zw      = q[2] * q[3];

	_v[0]  = 1 - 2 * ( yy + zz );
	_v[4]  =     2 * ( xy - zw );
	_v[8]  =     2 * ( xz + yw );

	_v[1]  =     2 * ( xy + zw );
	_v[5]  = 1 - 2 * ( xx + zz );
	_v[9]  =     2 * ( yz - xw );

	_v[2]  =     2 * ( xz - yw );
	_v[6]  =     2 * ( yz + xw );
	_v[10] = 1 - 2 * ( xx + yy );

	_v[3]  = _v[7] = _v[11] = _v[12] = _v[13] = _v[14] = 0;
	_v[15] = 1;
#else
	/*
	From Quaternion to Matrix and Back
	February 27th 2005
	J.M.P. van Waveren

	http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
	*/
	float			x2, y2, z2, w2;
	float			yy2, xy2;
	float			xz2, yz2, zz2;
	float			wz2, wy2, wx2, xx2;

	x2 = q[0] + q[0];
	y2 = q[1] + q[1];
	z2 = q[2] + q[2];
	w2 = q[3] + q[3];

	yy2 = q[1] * y2;
	xy2 = q[0] * y2;

	xz2 = q[0] * z2;
	yz2 = q[1] * z2;
	zz2 = q[2] * z2;

	wz2 = q[3] * z2;
	wy2 = q[3] * y2;
	wx2 = q[3] * x2;
	xx2 = q[0] * x2;

	_v[ 0] = - yy2 - zz2 + 1.0f;
	_v[ 1] =   xy2 + wz2;
	_v[ 2] =   xz2 - wy2;

	_v[ 4] =   xy2 - wz2;
	_v[ 5] = - xx2 - zz2 + 1.0f;
	_v[ 6] =   yz2 + wx2;

	_v[ 8] =   xz2 + wy2;
	_v[ 9] =   yz2 - wx2;
	_v[10] = - xx2 - yy2 + 1.0f;

	_v[ 3] = _v[ 7] = _v[11] = _v[12] = _v[13] = _v[14] = 0;
	_v[15] = 1;
#endif
}
quat_c matrix_c::getQuat() const {
	quat_c q;
#if 1
	/*
	   From Quaternion to Matrix and Back
	   February 27th 2005
	   J.M.P. van Waveren

	   http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
	 */
	float           t, s;

	if(_v[0] + _v[5] + _v[10] > 0.0f)
	{
		t = _v[0] + _v[5] + _v[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[3] = s * t;
		q[2] = (_v[1] - _v[4]) * s;
		q[1] = (_v[8] - _v[2]) * s;
		q[0] = (_v[6] - _v[9]) * s;
	}
	else if(_v[0] > _v[5] && _v[0] > _v[10])
	{
		t = _v[0] - _v[5] - _v[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[0] = s * t;
		q[1] = (_v[1] + _v[4]) * s;
		q[2] = (_v[8] + _v[2]) * s;
		q[3] = (_v[6] - _v[9]) * s;
	}
	else if(_v[5] > _v[10])
	{
		t = -_v[0] + _v[5] - _v[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[1] = s * t;
		q[0] = (_v[1] + _v[4]) * s;
		q[3] = (_v[8] - _v[2]) * s;
		q[2] = (_v[6] + _v[9]) * s;
	}
	else
	{
		t = -_v[0] - _v[5] + _v[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[2] = s * t;
		q[3] = (_v[1] - _v[4]) * s;
		q[0] = (_v[8] + _v[2]) * s;
		q[1] = (_v[6] + _v[9]) * s;
	}

#else
	float           trace;

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm

	trace = 1.0f + _v[0] + _v[5] + _v[10];

	if(trace > 0.0f)
	{
		vec_t           s = 0.5f / sqrt(trace);

		q[0] = (_v[6] - _v[9]) * s;
		q[1] = (_v[8] - _v[2]) * s;
		q[2] = (_v[1] - _v[4]) * s;
		q[3] = 0.25f / s;
	}
	else
	{
		if(_v[0] > _v[5] && _v[0] > _v[10])
		{
			// column 0
			float           s = sqrt(1.0f + _v[0] - _v[5] - _v[10]) * 2.0f;

			q[0] = 0.25f * s;
			q[1] = (_v[4] + _v[1]) / s;
			q[2] = (_v[8] + _v[2]) / s;
			q[3] = (_v[9] - _v[6]) / s;
		}
		else if(_v[5] > _v[10])
		{
			// column 1
			float           s = sqrt(1.0f + _v[5] - _v[0] - _v[10]) * 2.0f;

			q[0] = (_v[4] + _v[1]) / s;
			q[1] = 0.25f * s;
			q[2] = (_v[9] + _v[6]) / s;
			q[3] = (_v[8] - _v[2]) / s;
		}
		else
		{
			// column 2
			float           s = sqrt(1.0f + _v[10] - _v[0] - _v[5]) * 2.0f;

			q[0] = (_v[8] + _v[2]) / s;
			q[1] = (_v[9] + _v[6]) / s;
			q[2] = 0.25f * s;
			q[3] = (_v[4] - _v[1]) / s;
		}
	}

	q.normalize();
#endif		
	return q;
}
#endif

void matrix_c::fromQuat(const quat_c &q) {
#if 0
	float xx,xy,xz,xw,yy,yz,yw,zz,zw;

	xx      = q[0] * q[0];
	xy      = q[0] * q[1];
	xz      = q[0] * q[2];
	xw      = q[0] * q[3];

	yy      = q[1] * q[1];
	yz      = q[1] * q[2];
	yw      = q[1] * q[3];

	zz      = q[2] * q[2];
	zw      = q[2] * q[3];

	_v[0]  = 1 - 2 * ( yy + zz );
	_v[4]  =     2 * ( xy - zw );
	_v[8]  =     2 * ( xz + yw );

	_v[1]  =     2 * ( xy + zw );
	_v[5]  = 1 - 2 * ( xx + zz );
	_v[9]  =     2 * ( yz - xw );

	_v[2]  =     2 * ( xz - yw );
	_v[6]  =     2 * ( yz + xw );
	_v[10] = 1 - 2 * ( xx + yy );

	_v[3]  = _v[7] = _v[11] = _v[12] = _v[13] = _v[14] = 0;
	_v[15] = 1;
#else
	/*
	From Quaternion to Matrix and Back
	February 27th 2005
	J.M.P. van Waveren

	http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
	*/
	float			x2, y2, z2, w2;
	float			yy2, xy2;
	float			xz2, yz2, zz2;
	float			wz2, wy2, wx2, xx2;

	x2 = q[0] + q[0];
	y2 = q[1] + q[1];
	z2 = q[2] + q[2];
	w2 = q[3] + q[3];

	yy2 = q[1] * y2;
	xy2 = q[0] * y2;

	xz2 = q[0] * z2;
	yz2 = q[1] * z2;
	zz2 = q[2] * z2;

	wz2 = q[3] * z2;
	wy2 = q[3] * y2;
	wx2 = q[3] * x2;
	xx2 = q[0] * x2;

	_v[ 0] = - yy2 - zz2 + 1.0f;
	_v[ 1] =   xy2 + wz2;
	_v[ 2] =   xz2 - wy2;

	_v[ 4] =   xy2 - wz2;
	_v[ 5] = - xx2 - zz2 + 1.0f;
	_v[ 6] =   yz2 + wx2;

	_v[ 8] =   xz2 + wy2;
	_v[ 9] =   yz2 - wx2;
	_v[10] = - xx2 - yy2 + 1.0f;

	_v[ 3] = _v[ 7] = _v[11] = _v[12] = _v[13] = _v[14] = 0;
	_v[15] = 1;
#endif
}
void matrix_c::print() const {
	for(u32 i = 0; i < 4; i++) {
		printf("%f %f %f %f\n",_m[i][0],_m[i][1],_m[i][2],_m[i][3]);
	}
}