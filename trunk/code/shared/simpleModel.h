/*
============================================================================
Copyright (C) 2016 V.

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
// simpleModel.h - simple model for use outside the renderer
#ifndef __SHARED_SIMPLEMODEL_H__
#define __SHARED_SIMPLEMODEL_H__

//#include "simpleVert.h"
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>

class simpleSurface_c {
	str matName;
	// I think it's faster to use renderer internal vertexBuffer/indexBuffer classes here
	///arraySTD_c<u32> indices;
	rIndexBuffer_c indices;
	//arraySTD_c<simpleVert_s> verts;
	rVertexBuffer_c verts;

public:
	void addTriangle(const vec3_c &v0, const vec2_c &t0, const vec3_c &v1, const vec2_c &t1, 
		const vec3_c &v2, const vec2_c &t2) {
		u32 first = verts.size();
		indices.push_back(first);
		indices.push_back(first+1);
		indices.push_back(first+2);
		verts.resize(first+3);
		verts[first].setXYZST(v0,t0);
		verts[first+1].setXYZST(v1,t1);
		verts[first+2].setXYZST(v2,t2);
	}
	void addQuad(const vec3_c &v0, const vec2_c &t0, const vec3_c &v1, const vec2_c &t1, 
		const vec3_c &v2, const vec2_c &t2, const vec3_c &v3, const vec2_c &t3) {
		addTriangle(v0,t0,v1,t1,v2,t2);
		addTriangle(v2,t2,v3,t3,v0,t0);
	}
	u32 getNumIndices() const {
		return indices.getNumIndices();
	}
	const vec3_c &getXYZ(u32 i) const {
		return verts[i].xyz;
	}
	u32 getIndex(u32 i) const {
		return indices[i];
	}
	const rIndexBuffer_c &getIndices() const {
		return indices;
	}
	const rVertexBuffer_c &getVerts() const {
		return verts;
	}
};

class simpleMode_c {
	arraySTD_c<simpleSurface_c> surfs;

public:
};

#endif // __SHARED_SIMPLEMODEL_H__

