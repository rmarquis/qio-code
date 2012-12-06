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
// rVertexBuffer.h 
#ifndef __RVERTEXBUFFER_H__
#define __RVERTEXBUFFER_H__

#include <math/math.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <shared/array.h>
#include <api/rbAPI.h>

class rVert_c {
public:
	vec3_c xyz;
	vec3_c normal;
	byte color[4];
	vec2_c tc;
	vec2_c lc;
	//vec2_c tan;
	//vec2_c bin;

	rVert_c() {
		memset(color,0xff,sizeof(color));
	}
	// returns the result of quadratic interpolation between this vertex and two other vertices
	rVert_c getInterpolated_quadratic(rVert_c &a, rVert_c &b, float s) {
		rVert_c out;
		G_GetInterpolated_quadraticn(3,out.xyz,xyz,a.xyz,b.xyz,s);
		G_GetInterpolated_quadraticn(2,out.tc,tc,a.tc,b.tc,s);
		G_GetInterpolated_quadraticn(2,out.lc,lc,a.lc,b.lc,s);
		G_GetInterpolated_quadraticn(3,out.normal,normal,a.normal,b.normal,s);
		//G_GetInterpolated_quadraticn(3,out.bin,bin,a.bin,b.bin,s);
		//G_GetInterpolated_quadraticn(3,out.tan,tan,a.tan,b.tan,s);
		vec3_c ct, ca, cb;
		ct.fromByteRGB(this->color);
		ca.fromByteRGB(a.color);
		cb.fromByteRGB(b.color);
		vec3_c res;
		G_GetInterpolated_quadraticn(3,res,ct,ca,cb,s);
		res.colorToBytes(out.color);
		return out;
	}
};
class rVertexBuffer_c {
	arraySTD_c<rVert_c> data;
	union {
		u32 handleU32;
		void *handleV;
	};
public:
	rVertexBuffer_c() {
		handleU32 = 0;
	}
	~rVertexBuffer_c() {
		if(handleU32) {
			unloadFromGPU();
		}
	}
	void uploadToGPU() {
		rb->createVBO(this);
	}
	void unloadFromGPU() {
		rb->destroyVBO(this);
	}
	void resize(u32 newSize) {
		data.resize(newSize);
	}
	void push_back(const rVert_c &nv) {
		data.push_back(nv);
	}
	u32 getSizeInBytes() const {
		return data.getSizeInBytes();
	}
	u32 size() const {
		return data.size();
	}
	const rVert_c &operator [] (u32 index) const {
		return data[index];
	}
	rVert_c &operator [] (u32 index) {
		return data[index];
	}
	const rVert_c *getArray() const {
		return data.getArray();
	}
	rVert_c *getArray() {
		return data.getArray();
	}
	void destroy() {
		data.clear();
		unloadFromGPU();
	}

	inline void nullNormals() {
		rVert_c *v = this->getArray();
		for(u32 i = 0; i < this->size(); i++, v++) {
			v->normal.clear();
		}
	}
	inline void normalizeNormals() {
		rVert_c *v = this->getArray();
		for(u32 i = 0; i < this->size(); i++, v++) {
			v->normal.normalize();
		}
	}

	u32 getInternalHandleU32() const {
		return handleU32;
	}
	void setInternalHandleU32(u32 nh) {
		handleU32 = nh;
	}
	void *getInternalHandleVoid() const {
		return handleV;
	}
	void setInternalHandleVoid(void *nh) {
		handleV = nh;
	}
};

#endif // __RVERTEXBUFFER_H__
