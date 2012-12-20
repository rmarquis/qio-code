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
// byteRGB.h - red green blue color class
#ifndef __SHARED_BYTERGB_H__
#define __SHARED_BYTERGB_H__

#include "typedefs.h"

struct byteRGB_s {
	byte r;
	byte g;
	byte b;
	byte padding; // padding to 4 bytes

	byteRGB_s() {

	}
	byteRGB_s(byte nr, byte ng, byte nb) {
		r = nr;
		g = ng;
		b = nb;
	}
	byteRGB_s(byte br) {
		r = br;
		g = br;
		b = br;
	}
	// simple access operators
	inline byte	operator [] (const int index) const {
		return (((byte*)this)[index]);
	}
	inline byte&	operator [] (const int index) {
		return (((byte*)this)[index]);
	}
	inline operator const byte *() const {
		return (byte*)&r;
	}
	inline operator byte *() {
		return (byte*)&r;
	}

	void operator = ( int i ) {
		r = g = b = (byte)i;
	}

	// predefined colors
	void setWhite() {
		b = g = r = 255;
	}
	void setBlack() {
		b = g = r = 0;
	}
	void setBlue() {
		g = r = 0;
		b = 255;
	}
	void setRed() {
		g = b = 0;
		r = 255;
	}
	void setGreen() {
		b = r = 0;
		g = 255;
	}
	void setAll(byte nv) {
		r = g = b = nv;
	}
	void clear() {
		r = g = b = 0;
	}
	void fromFloats(const float *rgb) {
		r = rgb[0] * 255.f;
		g = rgb[1] * 255.f;
		b = rgb[2] * 255.f;
	}
	float getFloatGreen() const {
		return float(g) / 255.f;
	}
	float getFloatRed() const {
		return float(r) / 255.f;
	}
	float getFloatBlue() const {
		return float(b) / 255.f;
	}
};

#endif // __SHARED_BYTERGB_H__

