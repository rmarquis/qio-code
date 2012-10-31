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
// cmSurface.h - simple trimesh surface for collision detection
#ifndef __CMSURFACE_H__
#define __CMSURFACE_H__

#include "array.h"
#include <math/vec3.h>
#include <math/aabb.h>
#include <api/colMeshBuilderAPI.h>
#include <api/staticModelCreatorAPI.h>

class cmSurface_c : public colMeshBuilderAPI_i, public staticModelCreatorAPI_i {
	arraySTD_c<u32> indices;
	arraySTD_c<vec3_c> verts;
	aabb bb;
public:
	// colMeshBuilderAPI_i api
	virtual void addVert(const class vec3_c &nv) {
		bb.addPoint(nv);
		verts.push_back(nv);
	}
	virtual void addIndex(const u32 idx) {
		indices.push_back(idx);
	}
	virtual u32 getNumVerts() const {
		return verts.size();
	}
	virtual u32 getNumIndices() const {
		return indices.size();
	}
	virtual u32 getNumTris() const {
		return indices.size()/3;
	}
	const byte *getVerticesBase() const {
		return (const byte*)verts.getArray();
	}
	const byte *getIndicesBase() const {
		return (const byte*)indices.getArray();
	}
	// staticModelCreatorAPI_i api
	// NOTE: material name is ignored here
	virtual void addTriangle(const char *matName, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
		// add a single triangle
		indices.push_back(verts.size());
		indices.push_back(verts.size()+1);
		indices.push_back(verts.size()+2);
		this->addVert(v0.xyz);
		this->addVert(v1.xyz);
		this->addVert(v2.xyz);
	}
	// modelPostProcessFuncs_i api
	virtual void scaleXYZ(float scale) {
		for(u32 i = 0; i < verts.size(); i++) {
			verts[i] *= scale;
		}
		bb.scaleBB(scale);
	}
	virtual void swapYZ()  {
		vec3_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			v->swapYZ();
		}
		bb.swapYZ();
	}
	virtual void translateY(float ofs) {
		vec3_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			v->y += ofs;
		}
		bb.mins.y += ofs;
		bb.maxs.y += ofs;
	}

	const aabb &getAABB() const {
		return bb;
	}
	const arraySTD_c<vec3_c> &getVertsArray() const {
		return verts;
	}
};

#endif // __CMSURFACE_H__
