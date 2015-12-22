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
// rPointBuffer.h 
#ifndef __RPOINTBUFFER_H__
#define __RPOINTBUFFER_H__

#include <shared/vec3hash.h>

// NOTE: 
// sizeof(vec3_c) == 12, 
// sizeof(hashVec3_c) == 16 !!!
class rPointBuffer_c {
	vec3Hash_c data;
	union {
		u32 handleU32;
		void *handleV;
	};
public:
	rPointBuffer_c() {
		handleU32 = 0;
	}
	~rPointBuffer_c() {
		if(handleU32) {
			unloadFromGPU();
		}
	}
	void uploadToGPU() {
		//rb->createPointsBufferVBO(this);
	}
	void unloadFromGPU() {
		//rb->destroyPointsBufferVBO(this);
	}
	u32 registerVec3(const vec3_c &v) {
		return data.registerVec3(v);
	}
	u32 addPoint(const vec3_c &v) {
		return data.addVec3(v);
	}
	u32 getSizeInBytes() const {
		return data.getSizeInBytes();
	}
	u32 size() const {
		return data.size();
	}
	const hashVec3_c &operator [] (u32 index) const {
		return data[index];
	}
	hashVec3_c &operator [] (u32 index) {
		return data[index];
	}
	const hashVec3_c *getArray() const {
		return data.getArray();
	}
	hashVec3_c *getArray() {
		return data.getArray();
	}
	void ensureAllocated(u32 neededVertCount) {
		data.ensureAllocated(neededVertCount);
	}
	void destroy() {
		data.clear();
		unloadFromGPU();
	}
	void setNullCount() {
		data.setNullCount();
		unloadFromGPU();
	}
	void setEqualVertexEpsilon(float newEpsilon) {
		data.setEqualVertexEpsilon(newEpsilon); 
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

#endif // __RPOINTBUFFER_H__
