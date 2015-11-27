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
// rIndexBuffer.h 
#ifndef __RINDEXBUFFER_H__
#define __RINDEXBUFFER_H__

#include <shared/array.h>
#include <api/rbAPI.h>

#ifndef U16_MAX
#define U16_MAX  65536
#endif // U16_MAX

enum iboType_e {
	IBO_NOT_SET,
	IBO_U16, // 2 bytes per index
	IBO_U32, // 4 bytes per index
};

class rIndexBuffer_c {
	arraySTD_c<byte> data;
	u32 numIndices;
	iboType_e type;
	union {
		u32 handleU32;
		void *handleV;
	};
public:
	rIndexBuffer_c() {
		type = IBO_NOT_SET;
		numIndices = 0;
		handleV = 0;
	}
	~rIndexBuffer_c() {
		unloadFromGPU();
	}
	u16 *initU16(u32 newCount) {
		destroy();
		type = IBO_U16;
		data.resize(newCount * sizeof(u16));
		numIndices = newCount;
		return (u16*)data.getArray();
	}
	u32 *initU32(u32 newCount) {
		destroy();
		type = IBO_U32;
		data.resize(newCount * sizeof(u32));
		numIndices = newCount;
		return (u32*)data.getArray();
	}
	void setTypeU16() {
		destroy();
		type = IBO_U16;
	}
	u32 getIndexSize() const {
		if(type == IBO_U16)
			return 2;
		if(type == IBO_U32)
			return 4;
		return 0;
	}
	void reserve(u32 newNumIndices) {
		data.resize(newNumIndices*getIndexSize());
	}
	void push_back(u32 nextIndex) {
		ensureAllocated_indices(numIndices+1);
		if(type == IBO_U16) {
			((u16*)data.getArray())[numIndices] = nextIndex;
			numIndices++;
		} else if(type == IBO_U32) {
			((u32*)data.getArray())[numIndices] = nextIndex;
			numIndices++;
		}
	}
	void fromU32Array(u32 numNewIndices, const u32 *newIndices) {
		// see if we need to 32 bit indices
		bool needU32 = false;
		for(u32 i = 0; i < numNewIndices; i++) {
			u32 idx = newIndices[i];
			if(idx >= U16_MAX) {
				needU32 = true;
				break;
			}
		}
		// copy data
		if(needU32) {
			u32 *pNewData32 = initU32(numNewIndices);
			this->numIndices = numNewIndices;
#if 0
			for(u32 i = 0; i < numNewIndices; i++) {
				pNewData32[i] = newIndices[i];
			}
#else
			memcpy(pNewData32,newIndices,sizeof(u32)*this->numIndices);
#endif
		} else {
			u16 *pNewData16 = initU16(numNewIndices);
			this->numIndices = numNewIndices;
			for(u32 i = 0; i < numNewIndices; i++) {
				pNewData16[i] = newIndices[i];
			}
		}
	}
	bool hasU32Index() const {
		if(type == IBO_U32) {
			const u32 *ptr = (const u32*)data.getArray();
			for(u32 i = 0; i < numIndices; i++) {
				u32 idx = ptr[i];
				if(idx+1 >= U16_MAX) {
					return true;
				}
			}
		}
		return false;
	}
	void addU16Array(const u16 *addIndices, u32 numAddIndices) {
		if(numAddIndices == 0)
			return;
		if(type == IBO_NOT_SET) {
			type = IBO_U16;
		}
		if(type == IBO_U16) {
			data.resize((numIndices+numAddIndices)*sizeof(u16));
			u16 *newIndices = (u16 *)&data[numIndices*sizeof(u16)];
			memcpy(newIndices,addIndices,numAddIndices*sizeof(u16));
			numIndices += numAddIndices;	
		} else {
			data.resize((numIndices+numAddIndices)*sizeof(u32));
			u32 *newIndices = (u32 *)&data[numIndices*sizeof(u32)];
			for(u32 i = 0; i < numAddIndices; i++) {
				newIndices[i] = addIndices[i];
			}
			numIndices += numAddIndices;	
		}
	}
	void setNullCount() {
		numIndices = 0;
		type = IBO_U16;
	}
	void convertToU32Buffer() {
		if(type == IBO_U32) {
			return;
		}
		if(type == IBO_NOT_SET) {
			type = IBO_U32;
			return;
		}
		arraySTD_c<u16> tmp;
		tmp.resize(numIndices);
		memcpy(tmp,data,numIndices*sizeof(u16));
		destroy();
		type = IBO_U32;
		addU16Array(tmp.getArray(),tmp.size());
	}
	void addIndexBuffer(const rIndexBuffer_c &add) {
		if(add.numIndices == 0)
			return;
		if(type == IBO_NOT_SET) {
			type = add.type;
			data = add.data;
			numIndices = add.numIndices;
			return;
		}
		unloadFromGPU();
		if(add.type == IBO_U32) {
			if(this->type == IBO_U32 || add.hasU32Index()) {
				convertToU32Buffer();
				data.resize((add.numIndices+numIndices)*sizeof(u32));
				u32 *newIndices = (u32 *)&data[numIndices*sizeof(u32)];
				memcpy(newIndices,add.data.getArray(),add.numIndices*sizeof(u32));
				numIndices += add.numIndices;
			} else {
				const u32 *otherIndices = add.getU32Ptr();
				data.resize((add.numIndices+numIndices)*sizeof(u16));
				u16 *nd = (u16*)&data[numIndices*sizeof(u16)];
				for(u32 i = 0; i < add.numIndices; i++) {
					nd[i] = otherIndices[i];
				}
				numIndices += add.numIndices;
			}
		} else {
			addU16Array(add.getU16Ptr(),add.getNumIndices());
		}
	}
	void destroy() {
		data.clear();
		numIndices = 0;
		unloadFromGPU();
	}
	void unloadFromGPU() {
		// rb pointer (renderer backend)
		// is NULL on dedicated servers
		if(rb) {
			rb->destroyIBO(this);
		}
	}
	void uploadToGPU() {
		// rb pointer (renderer backend)
		// is NULL on dedicated servers
		if(rb) {
			rb->createIBO(this);
		}
	}		
	void reUploadToGPU() {
#if 1
		unloadFromGPU();
		uploadToGPU();
#else
		// TODO
#endif
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
	u32 getNumIndices() const {
		return numIndices;
	}
	u32 getNumTriangles() const {
		return numIndices/3;
	}
	// use this with caution...
	void forceSetIndexCount(u32 newNumIndices) {
		numIndices = newNumIndices;
	}
	u32 getSizeInBytes() const {
		if(type == IBO_U16) {
			return numIndices * sizeof(u16);
		} else if(type == IBO_U32) {
			return numIndices * sizeof(u32);
		}
		return 0;
	}
	void deltaIndices(u32 ofs) {
		if(type == IBO_U16) {
			u16 *p = getU16Ptr();
			for(u32 i = 0; i < numIndices; i++) {
				p[i] += ofs;
			}
		} else if(type == IBO_U32){ 
			u32 *p = getU32Ptr();
			for(u32 i = 0; i < numIndices; i++) {
				p[i] += ofs;
			}
		}
	}
	bool is32Bit() const {
		return (type == IBO_U32);
	}
	bool isNotSet() const {
		return (type == IBO_NOT_SET);
	}
	const void *getArray() const {
		return data.getArray();
	}
	void *getArray() {
		return data.getArray();
	}
	u32 *getU32Ptr() {
		if(type != IBO_U32)
			return 0;
		return (u32*)data.getArray();
	}
	u16 *getU16Ptr() {
		if(type != IBO_U16)
			return 0;
		return (u16*)data.getArray();
	}
	const u32 *getU32Ptr() const {
		if(type != IBO_U32)
			return 0;
		return (const u32*)data.getArray();
	}
	const u16 *getU16Ptr() const {
		if(type != IBO_U16)
			return 0;
		return (const u16*)data.getArray();
	}
	u32 getGLIndexType() const {
		if(type == IBO_U16) {
			return 0x1403; // GL_UNSIGNED_SHORT
		} else if(type == IBO_U32) {
			return 0x1405; // GL_UNSIGNED_INT
		}
		return 0;
	}
	enum _D3DFORMAT getDX9IndexType() const {
		if(type == IBO_U16) {
			return (_D3DFORMAT)101; // D3DFMT_INDEX16
		} else if(type == IBO_U32) {
			return (_D3DFORMAT)102;
		}
		return (_D3DFORMAT)0;
	}
	const void *getVoidPtr() const {
		if(data.size() == 0)
			return 0;
		return (void*)data.getArray();
	}
	void operator=(const rIndexBuffer_c &o) {
		numIndices = o.numIndices;
		data = o.data;
		type = o.type;
		handleU32 = 0;
	}
	void ensureAllocated_bytes(u32 numBytes) {
		if(data.size() >= numBytes)
			return;
		data.resize(numBytes);
	}
	void ensureAllocated_indices(u32 neededIndicesCount) {
		if(type == IBO_NOT_SET)
			type = IBO_U16;
		u32 requiredMemory = getSingleIndexSize() * neededIndicesCount;
		ensureAllocated_bytes(requiredMemory);
	}
	void addIndex(u32 idx) {
		if(type == IBO_NOT_SET) {
			if(idx < U16_MAX) {
				type = IBO_U16;
			} else {
				type = IBO_U32;
			}
		} else if(type == IBO_U16) {
			if(idx >= U16_MAX) {
				this->convertToU32Buffer();
			}
		}
		u32 prevSize = numIndices * this->getSingleIndexSize();
		if(type == IBO_U16) {
			ensureAllocated_bytes(prevSize+2);
			u16 *p = (u16*)&data[prevSize];
			*p = idx;
		} else if(type == IBO_U32) {
			ensureAllocated_bytes(prevSize+4);
			u32 *p = (u32*)&data[prevSize];
			*p = idx;
		}
		this->numIndices++;
	}
	int findTriangle(u32 a, u32 b, u32 c, int skip = -1) const {
		for(u32 i = 0; i < numIndices; i+=3) {
			if(skip >= 0 && i/3 == skip)
				continue;
			u32 i0 = getIndex(i+0);
			u32 i1 = getIndex(i+1);
			u32 i2 = getIndex(i+2);
			if(i0 == a && i1 == b && i2 == c)
				return i/3;
		}
		return -1;
	}
	bool hasTriangle(u32 i0, u32 i1, u32 i2) const {
		if(findTriangle(i0,i1,i2) >= 0)
			return true;
		return false;
	}
	u32 countDuplicatedTriangles() const {
		u32 ret = 0;
		for(u32 i = 0; i < numIndices; i+=3) {
			u32 triNum = i / 3;
			u32 i0 = getIndex(i+0);
			u32 i1 = getIndex(i+1);
			u32 i2 = getIndex(i+2);
			int triIndex = findTriangle(i0,i1,i2,triNum);
			if(triIndex >= 0)
				ret++;
		}
		return ret;
	}
	void addTriangle(u32 i0, u32 i1, u32 i2) {
		addIndex(i0);
		addIndex(i1);
		addIndex(i2);
	}
	void addTriangleINV(u32 i0, u32 i1, u32 i2) {
		addTriangle(i2,i1,i0);
	}
	void addQuad(u32 i0, u32 i1, u32 i2, u32 i3) {
		addIndex(i0);
		addIndex(i1);
		addIndex(i2);
		addIndex(i1);
		addIndex(i3);
		addIndex(i2);
	}
	void swapIndices() {
		bool bWasLoadedToGPU = this->handleU32 != 0;
		if(bWasLoadedToGPU) {
			unloadFromGPU();
		}
		if(type == IBO_U16) {
			u16 *indices16 = (u16*)data.getArray();
			for(u32 i = 0; i < numIndices; i+=3) {
				u16 tmp16 = indices16[i];
				indices16[i] = indices16[i+2];
				indices16[i+2] = tmp16;
			}
		} else if(type == IBO_U32) {
			u32 *indices32 = (u32*)data.getArray();
			for(u32 i = 0; i < numIndices; i+=3) {
				u32 tmp32 = indices32[i];
				indices32[i] = indices32[i+2];
				indices32[i+2] = tmp32;
			}
		}
		if(bWasLoadedToGPU) {
			uploadToGPU();
		}
	}
	u32 getSingleIndexSize() const {
		if(type == IBO_U16) {
			return 2;
		} else if(type == IBO_U32) {
			return 4;
		}
		return 0;
	}
	void setIndex(u32 number, u32 value) {
		if(getSingleIndexSize()*data.size() <= number) {
			data.resize((number+1)*getSingleIndexSize());
		}
		if(type == IBO_U16) {
			u16 *p16 = (u16*)data.getArray();
			p16[number] = value;
		} else if(type == IBO_U32) {
			u32 *p32 = (u32*)data.getArray();
			p32[number] = value;
		}
	}
	u32 getIndex(u32 idx) const {
		if(type == IBO_U16) {
			const u16 *p16 = (const u16*)data.getArray();
			return p16[idx];
		} else if(type == IBO_U32) {
			const u32 *p32 = (const u32*)data.getArray();
			return p32[idx];
		} else {
			return 0;
		}
	}
	inline u32 operator [] (u32 idx) const {
		return getIndex(idx);
	}
};

#endif // __RINDEXBUFFER_H__
