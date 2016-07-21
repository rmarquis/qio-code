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
// rVertex.h - 3D vertex class
#ifndef __RVERTEX_H__
#define __RVERTEX_H__

#include <math/math.h>
#include <math/vec3.h>
#include <math/vec2.h>

#if 1
#define RVERT_STORE_TANGENTS
#endif

class rVert_c {
public:
	vec3_c xyz;
	vec3_c normal;
	union {
		byte color[4];
		int colorAsInt;
	};
	vec2_c tc;
	vec2_c lc;
#ifdef RVERT_STORE_TANGENTS
	vec3_c tan;
	vec3_c bin;
#endif

	rVert_c() {
		memset(color,0xff,sizeof(color));
	}
	rVert_c(const vec3_c &newXYZ) {
		this->xyz = newXYZ;
		memset(color,0xff,sizeof(color));
	}
	rVert_c(const vec2_c &newTC, const vec3_c &newXYZ) {
		this->xyz = newXYZ;
		this->tc = newTC;
		memset(color,0xff,sizeof(color));
	}
	rVert_c(float nPX, float nPY, float nPZ,
		float nNX, float nNY, float nNZ,
		float nTCS, float nTCT) {
		xyz.set(nPX,nPY,nPZ);
		normal.set(nNX,nNY,nNZ);
		tc.set(nTCS,nTCT);
	}
	void setXYZ(const float *f) {
		xyz[0] = f[0];
		xyz[1] = f[1];
		xyz[2] = f[2];
	}
	void setXYZST(const float *p, const float *pst) {
		xyz = p;
		tc = pst;
	}
	void setXYZ(float nX, float nY, float nZ) {
		xyz[0] = nX;
		xyz[1] = nY;
		xyz[2] = nZ;
	}
	void setUV(const float *f) {
		tc[0] = f[0];
		tc[1] = f[1];
	}
	void setUV(float u, float v) {
		tc[0] = u;
		tc[1] = v;
	}
	void setRGBA(byte r, byte g, byte b, byte a) {
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
	}
	void setColor(int newColorAsInt) {
		colorAsInt = newColorAsInt;
	}
	// returns the result of quadratic interpolation between this vertex and two other vertices
	rVert_c getInterpolated_quadratic(rVert_c &a, rVert_c &b, float s) {
		rVert_c out;
		G_GetInterpolated_quadraticn(3,out.xyz,xyz,a.xyz,b.xyz,s);
		G_GetInterpolated_quadraticn(2,out.tc,tc,a.tc,b.tc,s);
		G_GetInterpolated_quadraticn(2,out.lc,lc,a.lc,b.lc,s);
		G_GetInterpolated_quadraticn(3,out.normal,normal,a.normal,b.normal,s);
#ifdef RVERT_STORE_TANGENTS
		G_GetInterpolated_quadraticn(3,out.bin,bin,a.bin,b.bin,s);
		G_GetInterpolated_quadraticn(3,out.tan,tan,a.tan,b.tan,s);
#endif
		vec3_c ct, ca, cb;
		ct.fromByteRGB(this->color);
		ca.fromByteRGB(a.color);
		cb.fromByteRGB(b.color);
		vec3_c res;
		G_GetInterpolated_quadraticn(3,res,ct,ca,cb,s);
		res.colorToBytes(out.color);
		return out;
	}	
	void lerpAll(const rVert_c &a, const rVert_c &b, const float f) {
		xyz = a.xyz + f * ( b.xyz - a.xyz );
		tc = a.tc + f * ( b.tc - a.tc );
		lc = a.lc + f * ( b.lc - a.lc );
		normal = a.normal + f * ( b.normal - a.normal );
#ifdef RVERT_STORE_TANGENTS
		tan = a.tan + f * ( b.tan - a.tan );
		bin = a.bin + f * ( b.bin - a.bin );
#endif
		color[0] = (byte)( a.color[0] + f * ( b.color[0] - a.color[0] ) );
		color[1] = (byte)( a.color[1] + f * ( b.color[1] - a.color[1] ) );
		color[2] = (byte)( a.color[2] + f * ( b.color[2] - a.color[2] ) );
		color[3] = (byte)( a.color[3] + f * ( b.color[3] - a.color[3] ) );
	}
	// texgen enviromental CPU implementation
	// NOTE: texgen requires vertex NORMALS to be calculated!
	inline void calcEnvironmentTexCoords(const vec3_c &viewerOrigin) {
		vec3_c dir = viewerOrigin - this->xyz;
		dir.normalize();
		float dot = this->normal.dotProduct(dir);
		float twoDot = 2.f * dot;

		vec3_c reflected;
		reflected.x = this->normal.x * twoDot - dir.x;
		reflected.y = this->normal.y * twoDot - dir.y;
		reflected.z = this->normal.z * twoDot - dir.z;

		this->tc.x = 0.5f + reflected.y * 0.5f;
		this->tc.y = 0.5f - reflected.z * 0.5f;
	}
};

#endif // __RVERTEX_H__
