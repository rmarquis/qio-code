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
// quat.h - quaternion class for rotations
#ifndef __QUAT_H__
#define __QUAT_H__
// NOTE: w is a fourth component
class quat_c {
public:
	union {
		struct {
			float x, y, z, w;
		};
		float val[4];
	};

	quat_c() {
		//identity();
	}
	quat_c(const float *f) {
		x = f[0];
		y = f[1];
		z = f[2];
		w = f[3];
	}
	quat_c(float nX, float nY, float nZ, float nW) {
		x = nX;
		y = nY;
		z = nZ;
		w = nW;
	}
	void set(float nX, float nY, float nZ, float nW) {
		x = nX;
		y = nY;
		z = nZ;
		w = nW;
	}
	// NOTE: negated quat represents the same rotation
	void negate() {
		this->x = -this->x;
		this->y = -this->y;
		this->z = -this->z;
		this->w =- this->w;	
	}
	void conjugate() {
		this->x = -this->x;
		this->y = -this->y;
		this->z = -this->z;
		//this->w = this->w;
	}
	void identity() {
		x = 0;
		y = 0;
		z = 0;
		w = 1;
	}
	// returns the conjugated quaterion
	quat_c getInverse() const {
		quat_c ret;
		ret.x = -x;
		ret.y = -y;
		ret.z = -z;
		ret.w = w;
		return ret;
	}
	bool compare(const quat_c &other) const {
		if(other.x != this->x)
			return false;
		if(other.y != this->y)
			return false;
		if(other.z != this->z)
			return false;
		if(other.w != this->w)
			return false;
		return true;
	}
	void slerp(const quat_c &from, const quat_c &to, float frac) {
#if 0
		double cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3];

		quat_c to1;
		if(cosom < 0.0) {
			cosom = -cosom;

			to1 = to;
			to1.negate();
		} else {
			to1 = to;
		}

		double scale0, scale1;
		if((1.0 - cosom) > 0) {
			double omega = acos(cosom);
			double sinom = sin(omega);
			scale0 = sin((1.0 - frac) * omega) / sinom;
			scale1 = sin(frac * omega) / sinom;
		} else {
			scale0 = 1.0 - frac;
			scale1 = frac;
		}

		this->val[0] = scale0 * from[0] + scale1 * to1[0];
		this->val[1] = scale0 * from[1] + scale1 * to1[1];
		this->val[2] = scale0 * from[2] + scale1 * to1[2];
		this->val[3] = scale0 * from[3] + scale1 * to1[3];
#else
		/*
		   Slerping Clock Cycles
		   February 27th 2005
		   J.M.P. van Waveren

		   http://www.intel.com/cd/ids/developer/asmo-na/eng/293747.htm
		 */
		float           cosom, absCosom, sinom, sinSqr, omega, scale0, scale1;

		if(frac <= 0.0f) {
			*this = from;
			return;
		}

		if(frac >= 1.0f) {
			*this = to;
			return;
		}

		if(from.compare(to)) {
			*this = from;
			return;
		}

		cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3];
		absCosom = fabs(cosom);

		if((1.0f - absCosom) > 1e-6f) {
			sinSqr = 1.0f - absCosom * absCosom;
			sinom = 1.0f / sqrt(sinSqr);
			omega = atan2(sinSqr * sinom, absCosom);

			scale0 = sin((1.0f - frac) * omega) * sinom;
			scale1 = sin(frac * omega) * sinom;
		} else {
			scale0 = 1.0f - frac;
			scale1 = frac;
		}

		scale1 = (cosom >= 0.0f) ? scale1 : -scale1;

		this->val[0] = scale0 * from[0] + scale1 * to[0];
		this->val[1] = scale0 * from[1] + scale1 * to[1];
		this->val[2] = scale0 * from[2] + scale1 * to[2];
		this->val[3] = scale0 * from[3] + scale1 * to[3];
#endif
	}
	// NOTE: calculated W will always be negative.
	void calcW() {
		// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
		float temp = 1.0f - (x*x) - (y*y) - (z*z);
		if (temp < 0.0f)
			w = 0.0f;
		else
			w = -sqrt(temp);
	}
	void toAngles(vec3_t angles) {
		float          q2[4];

		q2[0] = val[0] * val[0];
		q2[1] = val[1] * val[1];
		q2[2] = val[2] * val[2];
		q2[3] = val[3] * val[3];

		angles[PITCH] = RAD2DEG(asin(-2 * (val[2] * val[0] - val[3] * val[1])));
		angles[YAW] = RAD2DEG(atan2(2 * (val[2] * val[3] + val[0] * val[1]), (q2[2] - q2[3] - q2[0] + q2[1])));
		angles[ROLL] = RAD2DEG(atan2(2 * (val[3] * val[0] + val[2] * val[1]), (-q2[2] - q2[3] + q2[0] + q2[1])));
	}

	const float *floatPtr() const {
		return &x; // component order is such: X Y Z W
	}


	// fast-access operators
	inline operator float *() const {
		return (float*)&x;
	}
	inline operator float *() {
		return (float*)&x;
	}
	inline float operator [] (const int index) const {
		return ((float*)this)[index];
	}
	inline float &operator [] (const int index) {
		return ((float*)this)[index];
	}
};

#endif // __QUAT_H__
