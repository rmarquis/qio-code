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
// readStreamAPI.h - reading stream interface
#ifndef __READSTREAMAPI_H_
#define __READSTREAMAPI_H_

#include <shared/typedefs.h>
#include <string.h>

class readStreamAPI_i {
public:
	virtual u32 readData(void *out, u32 numBytes) = 0;
	virtual bool isAtData(const void *out, u32 numBytes) = 0;
	virtual u32 skipBytes(u32 numBytesToSkip) = 0;
	virtual const void *getCurDataPtr() const = 0;
	virtual u32 getPos() const = 0;
	virtual void setPos(u32 newPosABS) = 0;
	virtual bool isAtEOF() const = 0;
	virtual u32 getTotalLen() const = 0;

	int readInt() {
		int ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	s32 readS32() {
		s32 ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	u32 readU32() {
		u32 ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	s16 readS16() {
		s16 ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	u16 readU16() {
		u16 ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	u8 readU8() {
		u8 ret;
		this->readData(&ret,sizeof(ret));
		return ret;
	}
	s8 readS8() {
		s8 ret;
		this->readData(&ret,sizeof(ret));
		return ret;
	}
	float readFloat() {
		float ret;
		this->readData(&ret,sizeof(ret));
		return ret;
	}
	byte readByte() {
		byte ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	char readChar() {
		char ret;
		readData(&ret,sizeof(ret));
		return ret;
	}
	int readVec2(float *out) {
		return readData(out,sizeof(float)*2);
	}
	int readVec3(float *out) {
		return readData(out,sizeof(float)*3);
	}

	// read functions for data with swapped byte order
	u32 swReadData(void *p, u32 numBytesToRead) {
		byte *pAsByte = (byte*)p;
		u32 read = 0;
		for(int i = numBytesToRead-1; i >= 0; i--) {
			read += readData(&pAsByte[i],1);
		}
		return read;
	}
	float swReadFloat() {
		float ret;
		this->swReadData(&ret,sizeof(ret));
		return ret;
	}
	s32 swReadS32() {
		s32 ret;
		swReadData(&ret,sizeof(ret));
		return ret;
	}
	u32 swReadU32() {
		u32 ret;
		swReadData(&ret,sizeof(ret));
		return ret;
	}
	s16 swReadS16() {
		s16 ret;
		swReadData(&ret,sizeof(ret));
		return ret;
	}
	u16 swReadU16() {
		u16 ret;
		swReadData(&ret,sizeof(ret));
		return ret;
	}
	int swReadVec2(float *out) {
		out[0] = swReadFloat();
		out[1] = swReadFloat();
	}
	void swReadVec3(float *out) {
		out[0] = swReadFloat();
		out[1] = swReadFloat();
		out[2] = swReadFloat();
	}

	u32 readCharArray(char *out, u32 numCharacters) {
		return readData(out,numCharacters);
	}
	bool isAtCharArray(const char *str, u32 numCharacters) {
		return isAtData(str,numCharacters);
	}
	bool isAtStr(const char *str) {
		return isAtData(str,strlen(str));
	}
};

#endif // __READSTREAMAPI_H_
