/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

#define	EQUAL_EPSILON	0.001
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

class edVec3_c {
public:
	union {
		vec_t _v[3];
		struct {
			float x, y, z;
		};
	};
public:
	edVec3_c() {

	}
	edVec3_c(float a, float b, float c) {
		_v[0] = a;
		_v[1] = b;
		_v[2] = c;
	}
	edVec3_c(const float *p) {
		if(p == 0) {
			_v[0] = _v[1] = _v[2] = 0.f;
			return;
		}
		_v[0] = p[0];
		_v[1] = p[1];
		_v[2] = p[2];
	}
	void clear() {
		x = y = z = 0.0f;
	}
	void set(float a, float b, float c) {
		_v[0] = a;
		_v[1] = b;
		_v[2] = c;
	}
	void set(float abc) {
		_v[0] = abc;
		_v[1] = abc;
		_v[2] = abc;
	}
	float vectorLength() const {
		float length = 0.0f;
		for (int i = 0; i < 3; i++)
			length += _v[i]*_v[i];
		length = sqrt (length);
		return length;
	}
	void vectorSnap() {
		for (int i = 0; i < 3; i++) {
			_v[i] = floor (_v[i] + 0.5);
		}
	}

	float normalize() {
		float length = 0.0f;
		for (int i = 0; i < 3; i++)
			length += _v[i]*_v[i];
		length = sqrt (length);
		if (length == 0)
			return 0.0;
		for (int i = 0; i < 3; i++)
			_v[i] /= length;	
		return length;
	}
	void crossProduct (const float *v1, const float *v2) {
		_v[0] = v1[1]*v2[2] - v1[2]*v2[1];
		_v[1] = v1[2]*v2[0] - v1[0]*v2[2];
		_v[2] = v1[0]*v2[1] - v1[1]*v2[0];
	}
	float dotProduct(const float *x) const {
		return (x[0]*_v[0]+x[1]*_v[1]+x[2]*_v[2]);
	}
	// sets this vector to a MA (multiply add) result of given arguments
	void vectorMA (const edVec3_c &base, float scale, const edVec3_c &dir) {
		_v[0] = base[0] + scale*dir[0];
		_v[1] = base[1] + scale*dir[1];
		_v[2] = base[2] + scale*dir[2];
	}
	void makeAngleVectors(vec3_t forward, vec3_t right, vec3_t up) {
		float		angle;
		static float		sr, sp, sy, cr, cp, cy;
		// static to help MS compiler fp bugs
		
		angle = _v[YAW] * (M_PI*2 / 360);
		sy = sin(angle);
		cy = cos(angle);
		angle = _v[PITCH] * (M_PI*2 / 360);
		sp = sin(angle);
		cp = cos(angle);
		angle = _v[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);
		
		if (forward)
		{
			forward[0] = cp*cy;
			forward[1] = cp*sy;
			forward[2] = -sp;
		}
		if (right)
		{
			right[0] = -sr*sp*cy+cr*sy;
			right[1] = -sr*sp*sy-cr*cy;
			right[2] = -sr*cp;
		}
		if (up)
		{
			up[0] = cr*sp*cy+sr*sy;
			up[1] = cr*sp*sy-sr*cy;
			up[2] = cr*cp;
		}
	}

	void operator *=(float f) {
		x *= f;
		y *= f;
		z *= f;
	}
	void operator +=(const edVec3_c &other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}
	void operator -=(const edVec3_c &other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}
	bool vectorCompare(const edVec3_c &other, float epsilon = EQUAL_EPSILON) const {
		for (int i = 0; i < 3; i++) {
			if (fabs(_v[i]-other._v[i]) > epsilon) {
				return false;
			}
		}			
		return true;
	}
	friend edVec3_c operator +(const edVec3_c &a, const edVec3_c &b) {
		return edVec3_c(a.x+b.x,a.y+b.y,a.z+b.z);
	}
	friend edVec3_c operator -(const edVec3_c &a, const edVec3_c &b) {
		return edVec3_c(a.x-b.x,a.y-b.y,a.z-b.z);
	}
	friend edVec3_c operator *(const edVec3_c &a, float b) {
		return edVec3_c(a.x*b,a.y*b,a.z*b);
	}
	edVec3_c operator -() const {
		return edVec3_c(-x,-y,-z);
	}
	void setupPolar(float radius, float theta, float phi) {
 		_v[0]=float(radius * cos(theta) * cos(phi));
		_v[1]=float(radius * sin(theta) * cos(phi));
		_v[2]=float(radius * sin(phi));
	}
	inline float operator[](int index) const {
		return _v[index];
	}
	inline float &operator[](int index) {
		return _v[index];
	}
	inline operator const float *() const {
		return _v;
	}
	inline operator float *() {
		return _v;
	}
};

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

vec_t Q_rint (vec_t in);

void _Vector53Copy (vec5_t in, vec3_t out);
void _Vector5Scale (vec5_t v, vec_t scale, vec5_t out);
void _Vector5Add (vec5_t va, vec5_t vb, vec5_t out);

#include <float.h>

class edAABB_c {
	edVec3_c mins;
	edVec3_c maxs;
public:
	void clear(float inf = FLT_MAX) {
		mins.set(inf);
		maxs.set(-inf);
	}
	void addPoint(const float *p) {
		for(int i = 0; i < 3; i++) {
			float f = p[i];
			if(f < mins[i])
				mins[i] = f;
			if(f > maxs[i])
				maxs[i] = f;
		}
	}
	edVec3_c getSizes() const {
		return maxs - mins;
	}
	void addBox(const edAABB_c &bb) {
		addPoint(bb.getMins());
		addPoint(bb.getMaxs());
	}
	const edVec3_c &getMaxs() const {
		return maxs;
	}
	const edVec3_c &getMins() const {
		return mins;
	}
};

#define ZERO_EPSILON 1.0E-6
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )
#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0f )


#endif
