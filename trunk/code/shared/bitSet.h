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
// bitSet.h - bitSet array used for areabytes / pvs
#ifndef __BITSET_H__
#define __BITSET_H__

#include <stdlib.h> // memset

class bitSet_c {
	u32 numBits;
	u32 numBytes;
	byte *data;

	void ensureAllocatedBytes(u32 numNeededBytes) {
		if(numBytes >= numNeededBytes)
			return;
		data = (byte*)realloc(data,numNeededBytes);
		// ensure that new bytes (just alloced) are zeroed
		memset(data+numBytes,0,numNeededBytes-numBytes);
		numBytes = numNeededBytes;
	}
	void ensureAllocatedBits(u32 numNeededBits) {
		u32 byteNum = 0;
		while (numNeededBits > 7) {
			byteNum++;
			numNeededBits -= 8;
		}
		ensureAllocatedBytes(byteNum+1);
	}
public:
	bitSet_c() {
		numBits = 0;
		numBytes = 0;
		data = 0;
	}
	~bitSet_c() {
		if(data) {
			free(data);
		}
	}
	void init(u32 initialBitCount, bool defaultTrue) {
		ensureAllocatedBits(initialBitCount+1);
		numBits = initialBitCount;
		if(defaultTrue) {
			memset(data,0xff,numBytes);
		} else {
			memset(data,0,numBytes);
		}
	}
	void operator =(const bitSet_c &other) {
		ensureAllocatedBytes(other.numBytes);
		numBits = other.numBits;
		memcpy(data,other.data,numBytes);
	}
	void set(u32 bitNum, bool bValue) {
		if(bitNum >= numBits) {
			numBits = bitNum + 1;
			ensureAllocatedBits(numBits);
		}
		u32 byteNum = 0;
		while (bitNum > 7) {
			byteNum++;
			bitNum -= 8;
		}
		if(bValue) {
			data[byteNum] |= (1 << bitNum);
		} else {
			data[byteNum] &= ~(1 << bitNum);
		}
	}
	void setAll(bool b) {
		if(b) {
			memset(data,0xff,numBytes);
		} else {
			memset(data,0,numBytes);
		}
	}
	bool get(u32 bitNum) const {
		if(bitNum >= numBits) {
			return 0; // bit index out of range - should never happen
		}
		u32 byteNum = 0;
		while(bitNum > 7) {
			byteNum++;
			bitNum -= 8;
		}
		return ((data[byteNum] & (1 << bitNum) ) != 0);
	}
	void fromBytes(const byte *np, u32 numNewBytes) {
		ensureAllocatedBytes(numNewBytes);
		numBits = numNewBytes * 8;
		memcpy(data,np,numNewBytes);
	}
	bool compare(const bitSet_c &other) const {
		if(numBits != other.numBits)
			return false;
		for(u32 i = 0; i < numBits; i++) {
			if(get(i) != other.get(i))
				return false;
		}
		return true;
	}
	const byte *getArray() const {
		return data;
	}
	u32 getSizeInBytes() const {
		return numBytes;
	}
};

#endif // __BITSET_H__
