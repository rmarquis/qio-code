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
// rf_surface.h
#ifndef __RF_SURFACE_H__
#define __RF_SURFACE_H__

#include "../rIndexBuffer.h"
#include "../rVertexBuffer.h"
#include <shared/str.h>
#include <math/aabb.h>
#include <api/staticModelCreatorAPI.h>

class r_surface_c {
	str name;
	str matName;
	class mtrAPI_i *mat;
	class textureAPI_i *lightmap;
	rVertexBuffer_c verts;
	rIndexBuffer_c indices;
	aabb bounds;
public:
	r_surface_c();

	u32 getNumVerts() const {
		return verts.size();
	}
	const char *getMatName() const {
		return matName;
	}
	void addVert(const rVert_c &v) {
		verts.push_back(v);
	}
	void addIndex(u32 idx) {
		indices.addIndex(idx);
	}
	void addTriangle(const struct simpleVert_s &v0, const struct simpleVert_s &v1, const struct simpleVert_s &v2);

	void resizeVerts(u32 newNumVerts);
	void setVert(u32 vertexIndex, const struct simpleVert_s &v);
	void resizeIndices(u32 newNumIndices);
	void setIndex(u32 indexNum, u32 value);

	void clear() {
		indices.destroy();
		verts.destroy();
	}
	void createVBO() {
		verts.uploadToGPU();
	}
	void createIBO() {
		indices.uploadToGPU();
	}
	void setMaterial(mtrAPI_i *newMat);
	void setMaterial(const char *newMatName);
	void setLightmap(textureAPI_i *newLM) {
		lightmap = newLM;
	}

	void recalcBB() {
		bounds.clear();
		for(u32 i = 0; i < verts.size(); i++) {
			bounds.addPoint(verts[i].xyz);
		}
	}
	void drawSurface();
	void addDrawCall();

	bool traceRay(class trace_c &tr);

	void scaleXYZ(float scale);
	void swapYZ();
	void translateY(float ofs);
	void multTexCoordsY(float f);
	void translateXYZ(const vec3_c &ofs);
	void addPointsToBounds(aabb &out);

	const aabb &getBB() const {
		return bounds;
	}

};

class r_model_c : public staticModelCreatorAPI_i {
	arraySTD_c<r_surface_c> surfs;
	aabb bounds;
	str name;
public:
	// staticModelCreatorAPI_i implementation
	virtual void addTriangle(const char *matName, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2);
	// for default, first surface
	virtual void resizeVerts(u32 newNumVerts);
	virtual void setVert(u32 vertexIndex, const struct simpleVert_s &v);
	virtual void resizeIndices(u32 newNumIndices);
	virtual void setIndex(u32 indexNum, u32 value);
	// modelPostProcessFuncs_i implementation
	virtual void scaleXYZ(float scale);
	virtual void swapYZ();
	virtual void translateY(float ofs);
	virtual void multTexCoordsY(float f);
	virtual void translateXYZ(const class vec3_c &ofs);
	virtual void getCurrentBounds(class aabb &out);
	virtual void setAllSurfsMaterial(const char *newMatName);

	bool traceRay(class trace_c &tr);

	r_surface_c *registerSurf(const char *matName);
	void addDrawCalls();
};

#endif // __RF_SURFACE_H__

