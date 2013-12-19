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
// vec2.h - 2d vector class - for menu graphics and texture coordinates
#ifndef __MATH_VEC2_H__
#define __MATH_VEC2_H__

#include "math.h"

class vec2_c {
public:
	float x, y;

	vec2_c() {
		x = 0;
		y = 0;
	}
	vec2_c(const float *f) {
		x = f[0];
		y = f[1];
	}
	vec2_c(const float newX, const float newY) {
		x = newX;
		y = newY;
	}

	void set(const float newX, const float newY) {
		x = newX;
		y = newY;
	}
	bool operator == (const vec2_c &other) const {
		if(x != other.x)
			return false;
		if(y != other.y)
			return false;
		return true;
	}
	void operator *= ( const float f ) {
		this->x *= f;
		this->y *= f;
	}
	void operator /= ( const float f ) {
		this->x /= f;
		this->y /= f;
	}
	// returns vector len
	float len() const {
		return sqrt(x*x + y*y);
	}
	void normalize() {
		float lengthSQ = x*x + y*y;
		if ( lengthSQ ) {
			float iLength = G_rsqrt(lengthSQ);
			x *= iLength;
			y *= iLength;
		}
	}
	void setLen(const float newLen) {
		normalize();
		this->scale(newLen);
	}
	void scale(float scale) {
		x *= scale;
		y *= scale;
	}

	friend vec2_c operator*(const vec2_c& a, const float f );
	friend vec2_c operator*(const float f, const vec2_c& b );
	friend vec2_c operator+(const vec2_c& a, const vec2_c& b );
	friend vec2_c operator-(const vec2_c& a, const vec2_c& b );

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
inline vec2_c operator*(const vec2_c& a, const float f ) {
	vec2_c o;
	o.x = a.x * f;
	o.y = a.y * f;
	return o;
}
inline vec2_c operator*(const float f, const vec2_c& b ) {
	vec2_c o;
	o.x = b.x * f;
	o.y = b.y * f;
	return o;
}
inline vec2_c operator+(const vec2_c& a, const vec2_c& b ) {
	vec2_c o;
	o.x = a.x + b.x;
	o.y = a.y + b.y;
	return o;
}
inline vec2_c operator-(const vec2_c& a, const vec2_c& b ) {
	vec2_c o;
	o.x = a.x - b.x;
	o.y = a.y - b.y;
	return o;
}

#endif // __MATH_VEC2_H__
