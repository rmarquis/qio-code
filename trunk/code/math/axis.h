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
// axis.h - FLU vectors axis
#ifndef __MATH_AXIS_H__
#define __MATH_AXIS_H__

#include "vec3.h"

class axis_c {
public:
	vec3_c mat[3];

	axis_c() {
		//identity();	
	}
	axis_c(float v0x, float v0y, float v0z, 
		float v1x, float v1y, float v1z, 
		float v2x, float v2y, float v2z) {
		// forward
		mat[0][0] = v0x;
		mat[0][1] = v0y;
		mat[0][2] = v0z;

		// left
		mat[1][0] = v1x;
		mat[1][1] = v1y;
		mat[1][2] = v1z;

		// up
		mat[2][0] = v2x;
		mat[2][1] = v2y;
		mat[2][2] = v2z;
	}
	axis_c(const vec3_t ax[3]) {
		mat[0] = ax[0];
		mat[1] = ax[1];
		mat[2] = ax[2];
	}

	inline void identity() {
		this->mat[0].set(1,0,0);
		this->mat[1].set(0,1,0);
		this->mat[2].set(0,0,1);
	}
	void inverse() {
		mat[0] *= -1;
		mat[1] *= -1;
		mat[2] *= -1;
	}
	void fromAngles(const vec3_c &angles) {
		float angle = angles[YAW] * (M_PI*2 / 360);
		float sy = sin(angle);
		float cy = cos(angle);
		angle = angles[PITCH] * (M_PI*2 / 360);
		float sp = sin(angle);
		float cp = cos(angle);
		angle = angles[ROLL] * (M_PI*2 / 360);
		float sr = sin(angle);
		float cr = cos(angle);

		this->mat[0][0] = cp*cy;
		this->mat[0][1] = cp*sy;
		this->mat[0][2] = -sp;

		this->mat[1][0] = -(-1*sr*sp*cy+-1*cr*-sy);
		this->mat[1][1] = -(-1*sr*sp*sy+-1*cr*cy);
		this->mat[1][2] = 1*sr*cp;

		this->mat[2][0] = (cr*sp*cy+-sr*-sy);
		this->mat[2][1] = (cr*sp*sy+-sr*cy);
		this->mat[2][2] = cr*cp;
	}	
	void fromString(const char *str) {
		sscanf(str,"%f %f %f %f %f %f %f %f %f",&this->mat[0].x,&this->mat[0].y,&this->mat[0].z
			,&this->mat[1].x,&this->mat[1].y,&this->mat[1].z,&this->mat[2].x,&this->mat[2].y,&this->mat[2].z);
	}
	inline vec3_c&	operator [] (const int index) {
		return mat[index];
	}
	inline const vec3_c&	operator [] (const int index) const {
		return mat[index];
	}
	inline operator vec3_t * () {
		return (vec3_t *)&mat;
	}
	inline operator const vec3_t * () const {
		return (vec3_t *)&mat;
	}

	vec3_c &getForward() {
		return mat[0];
	}
	const vec3_c &getForward() const {
		return mat[0];
	}
	vec3_c &getLeft() {
		return mat[1];
	}
	const vec3_c &getLeft() const {
		return mat[1];
	}
	vec3_c &getUp() {
		return mat[2];
	}
	const vec3_c &getUp() const {
		return mat[2];
	}
	float *floatPtr() {
		return &mat[0].x;
	}
	vec3_c toAngles() const {
		float yaw, pitch, roll = 0.0f;
		if(mat[0][1] == 0 && mat[0][0] == 0) {
			yaw = 0;
			if(mat[0][2] > 0) {
				pitch = 90;
			} else {
				pitch = 270;
			}
		} else {
			if(mat[0][0]) {
				yaw = (atan2(mat[0][1], mat[0][0]) * 180 / M_PI);
			} else if(mat[0][1] > 0) {
				yaw = 90;
			} else {
				yaw = 270;
			} if(yaw < 0) {
				yaw += 360;
			}

			float length1 = sqrt(mat[0][0] * mat[0][0] + mat[0][1] * mat[0][1]);
			pitch = (atan2(mat[0][2], length1) * 180 / M_PI);
			if(pitch < 0) {
				pitch += 360;
			}

			roll = (atan2(mat[1][2], mat[2][2]) * 180 / M_PI);
			if(roll < 0) {
				roll += 360;
			}
		}

		vec3_c angles;
		angles[PITCH] = -pitch;
		angles[YAW] = yaw;
		angles[ROLL] = roll;
		return angles;
	}

	const char *toString() const;
};

#endif // __MATH_AXIS_H__
