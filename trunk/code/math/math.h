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

#ifndef M_PI_64
#define M_PI_64		3.1415926535897932384626433832795028841971693993751
#endif

#ifndef M_TWOPI
#define M_TWOPI			6.28318530717958647692
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2		0.7071067811865475244008443621048490
#endif

#ifndef M_ONEDIVPI
#define M_ONEDIVPI		1.f/M_PI
#endif

#ifndef M_ONEDIVPI64
#define M_ONEDIVPI64		1.f/M_PI_64
#endif


#ifndef M_PIDIV180
#define M_PIDIV180			0.01745329251994329576923690768488f		// M_PI / 180.f
#endif

#ifndef M_180DIVPI
#define M_180DIVPI			57.295779513082320876798154814105f		// 180.f / M_PI
#endif

// half of pi, 1.570796326794897
#ifndef M_PIDIV2
#define M_PIDIV2		(M_PI*0.5f)
#endif

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// returns a clamped value in the range [min, max].
#define Q_clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

//#define DISABLE_ALL_FAST_SQRTS

inline float G_rsqrt(float x) {
#ifdef DISABLE_ALL_FAST_SQRTS
	float tmp = sqrt(x);
	//if(tmp == 0.f)
	//	return 0.f;
	return 1.f/tmp;
#else
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    return x;
#endif
}

inline float G_sqrt2(float n) {
#ifdef DISABLE_ALL_FAST_SQRTS
	return sqrt(n);
#else
    float r = 0.f;
    float i = 1.f;
    while((!(r*r>n || ((r+=i) && 0)) || ((r-=i) && (i*=0.1f))) && i>0.0001f);
    return r;
#endif
}	

#ifdef DISABLE_ALL_FAST_SQRTS
inline float G_rsqrt3(float x) {
	float tmp = sqrt(x);
	//if(tmp == 0.f)
	//	return 0.f;
	return 1.f/tmp;
}
#else
inline float __declspec(naked) __fastcall G_rsqrt3(float x) {
    __asm {
        mov	eax, 0be6eb508h
        mov	dword ptr [esp-12], 03fc00000h
        sub	eax,	dword ptr [esp+4]
        sub	dword ptr [esp+4], 800000h
        shr	eax,	1
        mov	dword ptr [esp-8], eax
        fld	dword ptr [esp-8]
        fmul	st,	st
        fld	dword ptr [esp-8]
        fxch	st(1)
        fmul	dword ptr [esp+4]
        fld	dword ptr [esp-12]
        fld	st(0)
        fsub	st,	st(2)
        fld	st(1)
        fxch	st(1)
        fmul	st(3),	st
        fmul	st(3),	st
        fmulp	st(4),	st
        fsub	st,	st(2)
        fmul	st(2),	st
        fmul	st(3),	st
        fmulp	st(2),	st
        fxch	st(1)
        fsubp	st(1),	st
        fmulp	st(1),	st
        ret 4
    }
}
#endif

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
