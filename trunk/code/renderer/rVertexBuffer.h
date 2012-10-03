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

#include <math/vec3.h>
#include <math/vec2.h>
#include <shared/array.h>

class rVert_c {
public:
	vec3_c xyz;
	vec3_c normal;
	vec2_c tc;
	vec2_c lc;
	byte color[4];
	//vec2_c tan;
	//vec2_c bin;
};
class rVertexBuffer_c {
	arraySTD_c<rVert_c> data;
	union {
		u32 handleU32;
		void *handleV;
	};
public:
	void resize(u32 newSize) {
		data.resize(newSize);
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
};

#endif // __RVERTEXBUFFER_H__
