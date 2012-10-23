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
// math.h - stateless mathematical routines
#ifndef __MATH_MATH_H__
#define __MATH_MATH_H__

#include <cmath>
#include "../shared/typedefs.h"

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

inline float G_rsqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
}

inline float G_sqrt2(float n) {
    float r = 0.f;
    float i = 1.f;
    while((!(r*r>n || ((r+=i) && 0)) || ((r-=i) && (i*=0.1f))) && i>0.0001f);
    return r;
}	

// quadratic interpolation for n-dimensional vector
inline void G_GetInterpolated_quadraticn(int rows, float *out, const float *v1, const float *v2, const float *v3, f32 d)
{
	const f32 inv = 1.0 - d;
	const f32 mul0 = inv * inv;
	const f32 mul1 =  2.0 * d * inv;
	const f32 mul2 = d * d;
	for(int i = 0; i < rows; i++) {
		out[i] = (v1[i] * mul0 + v2[i] * mul1 + v3[i] * mul2);
	}
}

#endif // __MATH_MATH_H__
