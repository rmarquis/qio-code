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
// vec3.h - 3d vector class (used for pitch-yaw-roll euler angles as well)
#ifndef __MATH_VEC3_H__
#define __MATH_VEC3_H__

#include "math.h"
#include <stdio.h> // sscanf
#include <string.h> // memcmp

class vec3_c {
public:
	float x, y, z;

	vec3_c() {
		x = 0;
		y = 0;
		z = 0;
	}
	vec3_c(const float *f) {
		x = f[0];
		y = f[1];
		z = f[2];
	}
	vec3_c(const int *i) {
		x = i[0];
		y = i[1];
		z = i[2];
	}
	vec3_c(const double *d) {
		x = d[0];
		y = d[1];
		z = d[2];
	}
	vec3_c(float x,float y, float z){
		this->x = x;
		this->y = y;
		this->z = z;
	}
	vec3_c(const char *text) {
		this->fromString(text);
	}

	void set(float x,float y, float z){
		this->x = x;
		this->y = y;
		this->z = z;
	}	
	void set(const float *f) {
		x = f[0];
		y = f[1];
		z = f[2];
	}
	void clear() {
		x = y = z = 0;
	}
	void zero() {
		x = y = z = 0;
	}
	void swapYZ() {
		float tmp = y;
		y = z;
		z = tmp;
	}
	void negate() {
		x = -x;
		y = -y;
		z = -z;
	}
	void snap() {
		x = (int)x;
		y = (int)y;
		z = (int)z;
	}
	bool isNull() const {
		if(x != 0)
			return false;
		if(y != 0)
			return false;
		if(z != 0)
			return false;
		return true;
	}
	inline bool compare(const vec3_c &other, const float eps = 0.01f) const {
#if 0
		if(abs(this->x - other.x) > eps)
			return false;
		if(abs(this->y - other.y) > eps)
			return false;
		if(abs(this->z - other.z) > eps)
			return false;
		return true;
#else
		return !memcmp(this,&other,sizeof(vec3_c));
#endif
	}
	bool operator == (const vec3_c &other) const {
		if(x != other.x)
			return false;
		if(y != other.y)
			return false;
		if(z != other.z)
			return false;
		return true;
	}

	friend vec3_c operator*(const vec3_c& a, const float f );
	friend vec3_c operator*(const float f, const vec3_c& b );
	friend vec3_c operator/(const vec3_c& a, const float f );
	friend vec3_c operator/(const float f, const vec3_c& b );
	friend vec3_c operator+(const vec3_c& a, const vec3_c& b );
	friend vec3_c operator-(const vec3_c& a, const vec3_c& b );

	vec3_c operator - () const {
		return vec3_c(-x,-y,-z);
	}
	void operator += ( const vec3_c &other ) {
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
	}
	void operator -= ( const vec3_c &other ) {
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
	}
	vec3_c getNormalized() const {
		vec3_c ret;
		float lengthSQ = x*x + y*y + z*z;
		if ( lengthSQ ) {
			float iLength = G_rsqrt(lengthSQ);
			ret.x = x * iLength;
			ret.y = y * iLength;
			ret.z = z * iLength;
		}
		return ret;
	}
	vec3_c getABS() const {
		vec3_c r;
		r.x = fabs(this->x);
		r.y = fabs(this->y);
		r.z = fabs(this->z);
		return r;
	}
	void normalize();
	void normalizeFast();

	// returns previous vector lenght value
	float normalize2() {
		float length = x*x + y*y + z*z;
		length = sqrt (length);
		if ( length ) {
			float iLength = 1.f/length;
			x *= iLength;
			y *= iLength;
			z *= iLength;
		}
		return length;
	}
	// classic 3d vector dot product
	float dotProduct(const vec3_c &v) const {
		return (x*v.x + y*v.y + z*v.z);
	}
	// sets this vector to a cross product result of two argument vectors
	vec3_c&	crossProduct(const vec3_c &v1, const vec3_c &v2) {
		x = v1.y*v2.z - v1.z*v2.y;
		y = v1.z*v2.x - v1.x*v2.z;
		z = v1.x*v2.y - v1.y*v2.x;
		return *this;
	}
	// returns the result of cross product between this and other vector
	vec3_c crossProduct(const vec3_c &v) const {
		vec3_c o;
		o.x = this->y*v.z - this->z*v.y;
		o.y = this->z*v.x - this->x*v.z;
		o.z = this->x*v.y - this->y*v.x;
		return o;
	}
	vec3_c crossProductINV(const vec3_c &v) const {
		return v.crossProduct(*this);
	}
	// returns the lenght squared
	float lenSQ() const {
		return x*x + y*y + z*z;
	}
	// returns vector len
	float len() const {
		return sqrt(x*x + y*y + z*z);
	}
	// returns squared distance between two 3d points
	float distSQ(const vec3_c &other) const {
		return (*this - other).lenSQ();
	}
	// returns distance between two 3d points
	float dist(const vec3_c &other) const {
		return (*this - other).len();
	}
	// sets this vector to the interpolation results of other two vectors
	void lerpResult(const vec3_c &v0, const vec3_c &v1, const float f) {
		vec3_c delta = v1 - v0;
		*this = v0 + delta * f;
	}
	void setLen(const float newLen) {
		normalize();
		this->scale(newLen);
	}
	void vectorMA(const vec3_c &base, const vec3_c &dir, const float scale) {
		x = base.x + scale * dir.x;
		y = base.y + scale * dir.y;
		z = base.z + scale * dir.z;
	}
	// rotates this vector by given set of euler angles
	void rotate(const vec3_c &angles) {
		vec3_c vWork, va;
		vWork = va = *this;

		u32 nIndex[3][2] = {
			{ 1, 2 }, 
			{ 0, 1 },
			{ 2, 0 },
		};

		for (u32 i = 0; i < 3; i++) {
			if (angles[i] != 0) {
				float dAngle = angles[i] * M_PI / 180.0f;
				float c = (float)cos(dAngle);
				float s = (float)sin(dAngle);
				vWork[nIndex[i][0]] = va[nIndex[i][0]] * c - va[nIndex[i][1]] * s;
				vWork[nIndex[i][1]] = va[nIndex[i][0]] * s + va[nIndex[i][1]] * c;
			}
			va = vWork;
		}
		*this = vWork;
	}
	vec3_c toAngles() const {
		float	forward;
		float	yaw, pitch;
		
		if ( y == 0 && x == 0 ) {
			yaw = 0;
			if ( z > 0 ) {
				pitch = 90;
			}
			else {
				pitch = 270;
			}
		}
		else {
			if ( x ) {
				yaw = ( atan2 ( y, x ) * 180 / M_PI );
			}
			else if ( y > 0 ) {
				yaw = 90;
			}
			else {
				yaw = 270;
			}
			if ( yaw < 0 ) {
				yaw += 360;
			}

			forward = sqrt ( x*x + y*y );
			pitch = ( atan2(z, forward) * 180 / M_PI );
			if ( pitch < 0 ) {
				pitch += 360;
			}
		}

		vec3_c angles;
		angles[PITCH] = -pitch;
		angles[YAW] = yaw;
		angles[ROLL] = 0;
		return angles;
	}
	void angleVectors(vec3_t forward, vec3_t right, vec3_t up) const {
		// pitch
		float angle = DEG2RAD(this->x);
		float sp = sin(angle);
		float cp = cos(angle);
		// yaw
		angle = DEG2RAD(this->y);
		float sy = sin(angle);
		float cy = cos(angle);
		// roll
		angle = DEG2RAD(this->z);
		float sr = sin(angle);
		float cr = cos(angle);

		if (forward) {
			forward[0] = cp*cy;
			forward[1] = cp*sy;
			forward[2] = -sp;
		}
		if (right) {
			right[0] = (-1*sr*sp*cy+-1*cr*-sy);
			right[1] = (-1*sr*sp*sy+-1*cr*cy);
			right[2] = -1*sr*cp;
		}
		if (up) {
			up[0] = (cr*sp*cy+-sr*-sy);
			up[1] = (cr*sp*sy+-sr*cy);
			up[2] = cr*cp;
		}
	}
	// converts vector values to RGB color bytes (0-255 range)
	void colorToBytes( byte *outColorRGB ) const {
		vec3_c sample = *this;

		// clamp with color normalization
		float max = sample[0];
		if ( sample[1] > max ) {
			max = sample[1];
		}
		if ( sample[2] > max ) {
			max = sample[2];
		}
		if ( max > 255 ) {
			sample *= (255.f/max);
		}
		outColorRGB[ 0 ] = sample[0];
		outColorRGB[ 1 ] = sample[1];
		outColorRGB[ 2 ] = sample[2];
	}
	void fromByteRGB(const byte *rgb) {
		x = rgb[0];
		y = rgb[1];
		z = rgb[2];
	}
	void rad2deg() {
		x = RAD2DEG(x);
		y = RAD2DEG(y);
		z = RAD2DEG(z);
	}
	int getLargestAxis() const {
		if(x >= y) {
			if(x >= z) {
				return 0;
			}
			return 2;
		}
		// y is larger than x
		if(y > z) {
			return 1;
		}
		return 2;
	}
	float getLargestAxisLen() const {
		if(x >= y) {
			if(x >= z) {
				return x;
			}
			return z;
		}
		// y is larger than x
		if(y > z) {
			return y;
		}
		return z;
	}
	vec3_c getProjected( const vec3_c &normal ) const {
		float invDenom = normal.lenSQ();
		
		if(invDenom == 0.0f)
			invDenom = (float)1.0e-20; // avoid dividing by zero
			
		invDenom = 1.0f / invDenom;
		
		float d = this->dotProduct( normal ) * invDenom;

		vec3_c n;
		n.x  = normal.x * invDenom;
		n.y = normal.y * invDenom;
		n.z = normal.z * invDenom;

		vec3_c ret;
		ret.x  = this->x - d * n.x;
		ret.y = this->y - d * n.y;
		ret.z = this->z - d * n.z;
		return ret;
	}
	vec3_c getPerpendicular() const {
		float minelem = 1.0f;
		// find the smallest magnitude axially aligned vector
		int pos;
		int i;
		for ( pos = 0, i = 0; i < 3; i++ ) {
			if ( fabs( (*this)[i] ) < minelem ) {
				pos = i;
				minelem = fabs( (*this)[i] );
			}
		}
		vec3_c tempVec(0,0,0);
		tempVec[pos] = 1.0f;

		// project the point onto the plane defined by src
		vec3_c dst = tempVec.getProjected( *this );

		//normalize the result
		dst.normalize();
		return dst;
	}
	void makeNormalVectors(vec3_c &left, vec3_c &down) const {
		float d = x * x + y * y;
		if ( !d ) {
			left[0] = 1;
			left[1] = 0;
			left[2] = 0;
		} else {
			d = G_rsqrt(d);
			left[0] = -y * d;
			left[1] = x * d;
			left[2] = 0;
		}
		down = left.crossProduct( *this );
	}
	bool isAxial() const {
		const float eps = 0.000001f;
		float f0 = abs(x);
		float f1 = abs(y);
		float f2 = abs(z);
		if(f0 < eps) {
			if(f1 < eps) {
				// 0, 0, x 
				return true;
			} else {
				if(f2 < eps) {
					// 0, x, 0
					return true;
				}
			}
		} else {
			if(f1 < eps && f2 < eps) {
				// x, 0, 0
				return true;
			}
		}
		return false;
	}
	// 	assumes that *this == to - from
	float getYaw() const {
		return (atan2(y, x) * 180 / M_PI);
	}
	float getPitch() const {
		return (asin((-z)/(len()))* 180 / M_PI);
	}
	bool isInSameDirection(const vec3_c &other) const {
		float d = this->dotProduct(other);
		if(d > 0) {
			return true;
		} else {
			return false;
		}
    }
	bool isInOppositeDirection(const vec3_c &other) const {
		float d = this->dotProduct(other);
		if(d < 0) {
			return true;
		} else {
			return false;
		}	
	}
	bool isAlmostZero() const {
		if(fabsf(x)>1e-6 || fabsf(y)>1e-6 || fabsf(z)>1e-6)
			return false;
		return true;
	}
	bool hasComponentLargerThan(float val) const {
		if(x > val)
			return true;
		if(y > val)
			return true;
		if(z > val)
			return true;
		return false;
	}
	bool isBetweenPoints(const vec3_c &begin, const vec3_c &end) const {
		const float f = (end - begin).lenSQ();
		return distSQ(begin) <= f && distSQ(end) <= f;
	}
	void fromString(const char *str) {
		sscanf(str,"%f %f %f",&this->x,&this->y,&this->z);
	}
	vec3_c lerp(const vec3_c &other, float frac) const {
		vec3_c ret = *this + frac * (other - *this);
		return ret;
	}
	void lerp(const vec3_c &from, const vec3_c &to, float frac) {
		*this = from + frac * (to - from);
	}
	void operator *= ( const float f ) {
		this->x *= f;
		this->y *= f;
		this->z *= f;
	}
	void operator /= ( const float f ) {
		this->x /= f;
		this->y /= f;
		this->z /= f;
	}
	void scaleXYZ(float sx, float sy, float sz) {
		x *= sx;
		y *= sy;
		z *= sz;
	}
	void scaleXYZ(const float *scaleXYZ) {
		x *= scaleXYZ[0];
		y *= scaleXYZ[1];
		z *= scaleXYZ[2];
	}
	void scale(float scale) {
		x *= scale;
		y *= scale;
		z *= scale;
	}
	// for angles
	// returns the forward vector for pitch/yaw/roll angles
	vec3_c getForward() const {
		float angle = (*this)[YAW] * (M_PI*2 / 360);
		float sy = sin(angle);
		float cy = cos(angle);
		angle = (*this)[PITCH] * (M_PI*2 / 360);
		float sp = sin(angle);
		float cp = cos(angle);

		vec3_c forward;
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
		return forward;
	}


	const float *floatPtr() const {
		return &x;
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
inline vec3_c operator*(const vec3_c& a, const float f ) {
	vec3_c o;
	o.x = a.x * f;
	o.y = a.y * f;
	o.z = a.z * f;
	return o;
}
inline vec3_c operator*(const float f, const vec3_c& b ) {
	vec3_c o;
	o.x = b.x * f;
	o.y = b.y * f;
	o.z = b.z * f;
	return o;
}
inline vec3_c operator/(const vec3_c& a, const float f ) {
	vec3_c o;
	o.x = a.x / f;
	o.y = a.y / f;
	o.z = a.z / f;
	return o;
}
inline vec3_c operator/(const float f, const vec3_c& b ) {
	vec3_c o;
	o.x = f / b.x;
	o.y = f / b.y;
	o.z = f / b.z;
	return o;
}
inline vec3_c operator+(const vec3_c& a, const vec3_c& b ) {
	vec3_c o;
	o.x = a.x + b.x;
	o.y = a.y + b.y;
	o.z = a.z + b.z;
	return o;
}
inline vec3_c operator-(const vec3_c& a, const vec3_c& b ) {
	vec3_c o;
	o.x = a.x - b.x;
	o.y = a.y - b.y;
	o.z = a.z - b.z;
	return o;
}

#endif // __MATH_VEC3_H__
