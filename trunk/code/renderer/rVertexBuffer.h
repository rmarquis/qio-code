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

#include <shared/array.h>
#include <api/rbAPI.h>
#include <api/coreAPI.h>

#include "rVertex.h"

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
		if(rb == 0) {
			g_core->RedWarning("rVertexBuffer_c::uploadToGPU: rb is NULL, cannot upload VBO\n");
		} else {
			rb->createVBO(this);
		}
	}
	void unloadFromGPU() {
		if(rb == 0) {
			if(handleU32) {
				g_core->RedWarning("rVertexBuffer_c::unloadFromGPU: rb is NULL, cannot free VBO data (GPU memory leak).\n");
			}
		} else {
			rb->destroyVBO(this);
		}
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
	void setVertexArray(const class rVert_c *in, u32 newSize) {
		resize(newSize);
		memcpy(getArray(),in,newSize*sizeof(rVert_c));
	}
	void destroy() {
		data.clear();
		numVerts = 0;
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
	void calcTBNForIndices(const class rIndexBuffer_c &indices, arraySTD_c<plane_c> *trianglePlanes);
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

	// deforms
	bool applyDeformAutoSprite(const vec3_c &leftDir, const vec3_c &upDir);

	// returns true if vertices are not on the same plane
	bool getPlane(class plane_c &pl) const;
	bool getPlane(const class rIndexBuffer_c &ibo, class plane_c &pl) const;
	// returns the center of vertices referenced in IBO
	void getCenter(const class rIndexBuffer_c &ibo, class vec3_c &p) const;

	void getReferencedPoints(rVertexBuffer_c &out, const class rIndexBuffer_c &ibo) const;
};

#endif // __RVERTEXBUFFER_H__
