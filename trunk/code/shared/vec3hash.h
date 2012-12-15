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
#ifndef __VEC3HASH_H__
#define __VEC3HASH_H__

#include <math/vec3.h>
#include "array.h"

#define HASHVEC3_NONE 0xffffffff

class hashVec3_c {
public:
	vec3_c v;
	u32 next;

	hashVec3_c() {
		next = HASHVEC3_NONE;
	}
	inline bool hasNext() const {
		if(next == HASHVEC3_NONE)
			return false;
		return true;
	}
};

class vec3Hash_c {
	arraySTD_c<hashVec3_c> data;
	u32 table[1024];
	int numVecs;
	float equalVertexEpsilon;

	u32 hashForVec3(const vec3_c &v) {
		return ((u32(v.x) + u32(v.y) + u32(v.z)) % 1024);
	}
public:
	vec3Hash_c() {
		memset(table,0xff,sizeof(table));
		equalVertexEpsilon = 0.0001f;
		numVecs = 0;
	}
	void clear() {
		memset(table,0xff,sizeof(table));
		data.clear();
		numVecs = 0;
	}
	void setEqualVertexEpsilon(float newEpsilon) {
		this->equalVertexEpsilon = newEpsilon; 
	}
	void setNullCount() {
		memset(table,0xff,sizeof(table));
		numVecs = 0;
	}
	u32 registerVec3(const vec3_c &v) {
		u32 hash = hashForVec3(v);
		//assert(hash < (sizeof(table)/sizeof(table[0])));
		u32 idx = table[hash];
		// see if similiar vertex is already on list
		while(idx != HASHVEC3_NONE) {
			if(data[idx].v.compare(v,equalVertexEpsilon)) {
				return idx;
			}
			idx = data[idx].next;
		}
		// add a new vertex
		u32 ret = numVecs;
		if(numVecs+1 > data.size()) {
			data.resize(numVecs+1);
		}
		hashVec3_c &nv = data[numVecs];
		numVecs++;
		nv.next = table[hash];
		table[hash] = ret;
		nv.v = v;
		return ret;
	}
	void ensureAllocated(u32 neededVertCount) {
		data.reserve(neededVertCount);
	}
	u32 size() const {
		return numVecs;
	}
	u32 getSizeInBytes() const {
		return numVecs*sizeof(hashVec3_c);
	}
	inline hashVec3_c operator [] (const int index) const {
		return data[index];
	}
	inline hashVec3_c &operator [] (const int index) {
		return data[index];
	}
	inline const vec3_c &getVec3(const int index) const {
		return data[index].v;
	}
	inline vec3_c &getVec3(const int index) {
		return data[index].v;
	}
	const hashVec3_c *getArray() const {
		return data.getArray();
	}
	hashVec3_c *getArray() {
		return data.getArray();
	}
};

#endif // __VEC3HASH_H__
