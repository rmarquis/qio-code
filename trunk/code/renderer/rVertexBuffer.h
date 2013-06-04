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

#if 1
#define RVERT_STORE_TANGENTS
#endif

class rVert_c {
public:
	vec3_c xyz;
	vec3_c normal;
	byte color[4];
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
	// NOTE: texgen requires vertex NORMALS to be calculated!!!
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
class rVertexBuffer_c {
	arraySTD_c<rVert_c> data;
	union {
		u32 handleU32;
		void *handleV;
	};
	u32 numVerts;
public:
	rVertexBuffer_c() {
		handleU32 = 0;
		numVerts = 0;
	}
	~rVertexBuffer_c() {
		if(handleU32) {
			unloadFromGPU();
		}
	}
	void ensureAllocated(u32 needVerts) {
		if(data.size() >= needVerts)
			return;
		data.resize(needVerts);
	}
	void operator = (const rVertexBuffer_c &other) {
		// free the previous GPU buffer
		unloadFromGPU();
		// copy vertex data
		this->ensureAllocated(other.size());
		memcpy(this->data.getArray(),other.data.getArray(),other.getSizeInBytes());
		this->numVerts = other.numVerts;
		// DONT upload new buffer automatically,
		// if it's needed it must be done manually
		//uploadToGPU();
	}
	void addArray(const arraySTD_c<rVert_c> &ar) {
		ensureAllocated(numVerts+ar.size());
		memcpy(data.getArray()+numVerts,ar.getArray(),ar.getSizeInBytes());
		numVerts += ar.size();
	}
	void uploadToGPU() {
		rb->createVBO(this);
	}
	void unloadFromGPU() {
		rb->destroyVBO(this);
	}
	void resize(u32 newSize) {
		ensureAllocated(newSize);
		numVerts = newSize;
	}
	void push_back(const rVert_c &nv) {
		this->ensureAllocated(numVerts+1);
		data[numVerts] = nv;
		numVerts++;
	}
	u32 getSizeInBytes() const {
		return numVerts*sizeof(rVert_c);
	}
	u32 size() const {
		return numVerts;
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
		for(u32 i = 0; i < numVerts; i++, v++) {
			v->normal.clear();
		}
	}
	inline void normalizeNormals() {
		rVert_c *v = this->getArray();
		for(u32 i = 0; i < numVerts; i++, v++) {
			v->normal.normalize();
		}
	}
#ifdef RVERT_STORE_TANGENTS
	inline void nullTBN() {
		rVert_c *v = this->getArray();
		for(u32 i = 0; i < numVerts; i++, v++) {
			v->normal.clear();
			v->tan.clear();
			v->bin.clear();
		}
	}
	void calcTBNForIndices(const class rIndexBuffer_c &indices);
	inline void normalizeTBN() {
		rVert_c *v = this->getArray();
		for(u32 i = 0; i < numVerts; i++, v++) {
			v->normal.normalize();
			v->tan.normalize();
			v->bin.normalize();
		}
	}
#endif // RVERT_STORE_TANGENTS

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

	// rVertexBuffer.cpp - CPU texgens/colorgens/texmods, etc
	void calcEnvironmentTexCoords(const class vec3_c &viewerOrigin);
	void calcEnvironmentTexCoordsForReferencedVertices(const class rIndexBuffer_c &ibo, const class vec3_c &viewerOrigin);
	void setVertexColorsToConstValue(byte val);
	void setVertexColorsToConstValues(byte *rgbVals);
	void setVertexColorsToConstValuesVec3255(const float *vec3_255) {
		byte rgbVals[3];
		rgbVals[0] = vec3_255[0];
		rgbVals[1] = vec3_255[1];
		rgbVals[2] = vec3_255[2];
		setVertexColorsToConstValues(rgbVals);
	}
	void setVertexAlphaToConstValue(byte val);

	void transform(const class matrix_c &mat);

	// returns true if vertices are not on the same plane
	bool getPlane(class plane_c &pl) const;
	bool getPlane(const class rIndexBuffer_c &ibo, class plane_c &pl) const;
	// returns the center of vertices referenced in IBO
	void getCenter(const class rIndexBuffer_c &ibo, class vec3_c &p) const;

	void getReferencedPoints(rVertexBuffer_c &out, const class rIndexBuffer_c &ibo) const;
};

#endif // __RVERTEXBUFFER_H__
