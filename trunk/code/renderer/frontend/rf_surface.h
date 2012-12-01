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
	~r_surface_c();

	u32 getNumVerts() const {
		return verts.size();
	}
	u32 getNumTris() const {
		return indices.getNumIndices() / 3;
	}
	mtrAPI_i *getMat() const {
		return mat;
	}
	const char *getMatName() const {
		return matName;
	}
	void addVert(const rVert_c &v) {
		verts.push_back(v);
	}
	void addVertXYZTC(const vec3_c &xyz, float tX, float tY) {
		rVert_c nv;
		nv.xyz = xyz;
		nv.tc.set(tX,tY);
		verts.push_back(nv);
	}
	void setVertXYZTC(u32 vertNum, const vec3_c &xyz, float tX, float tY) {
		rVert_c &nv = verts[vertNum];
		nv.xyz = xyz;
		nv.tc.set(tX,tY);
	}
	void addIndex(u32 idx) {
		indices.addIndex(idx);
	}
	void add3Indices(u32 i0, u32 i1, u32 i2) {
		indices.addIndex(i0);
		indices.addIndex(i1);
		indices.addIndex(i2);
	}
	void addTriangle(const struct simpleVert_s &v0, const struct simpleVert_s &v1, const struct simpleVert_s &v2);
	void addPoly(const struct simplePoly_s &poly);

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
	void createVBOandIBO() {
		createVBO();
		createIBO();
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
	void drawSurfaceWithSingleTexture(class textureAPI_i *tex);
	void addDrawCall();

	void addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out);
	bool createDecalInternal(class decalProjector_c &proj);
	void initSkelSurfInstance(const class skelSurfaceAPI_i *skelSF);
	void updateSkelSurfInstance(const class skelSurfaceAPI_i *skelSF, const class boneOrArray_c &bones);

	bool traceRay(class trace_c &tr);

	void scaleXYZ(float scale);
	void swapYZ();
	void translateY(float ofs);
	void multTexCoordsY(float f);
	void translateXYZ(const vec3_c &ofs);
	void addPointsToBounds(aabb &out);

	bool parseProcSurface(class parser_c &p);

	const aabb &getBB() const {
		return bounds;
	}

};

class r_model_c : public staticModelCreatorAPI_i {
	arraySTD_c<r_surface_c> surfs;
	aabb bounds;
	str name;
	// used to speed up raycasting / decal creation
	struct tsOctTreeHeader_s *extraCollOctTree;

	void ensureExtraTrisoupOctTreeIsBuild();
public:
	r_model_c();
	~r_model_c();

	const char *getName() const {
		return name;
	}

	bool isAreaModel() const {
		if(!Q_stricmpn(name,"_area",5))
			return true;
		return false;
	}
	int getAreaNumber() const {
		if(isAreaModel() == false)
			return -1;
		const char *num = name.c_str() + 5;
		return atoi(num);
	}

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
	virtual u32 getNumSurfs() const;
	virtual void setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName);
	virtual void addTriangleToSF(u32 surfNum, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2);

	void createVBOsAndIBOs();

	void addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out);
	void initSkelModelInstance(const class skelModelAPI_i *skel);
	void updateSkelModelInstance(const class skelModelAPI_i *skel, const class boneOrArray_c &bones);

	bool traceRay(class trace_c &tr, bool bAllowExtraOctTreeCreation = true);
	bool createDecalInternal(class decalProjector_c &proj);
	bool createDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
								 const class vec3_c &normal, float radius, class mtrAPI_i *material);

	r_surface_c *registerSurf(const char *matName);
	void addDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags = 0);

	bool parseProcModel(class parser_c &p);

	const aabb &getBounds() const {
		return bounds;
	}
	u32 getTotalTriangleCount() const {
		u32 ret = 0;
		for(u32 i = 0; i < surfs.size(); i++) {
			ret += surfs[i].getNumTris();
		}
		return ret;
	}
};

#endif // __RF_SURFACE_H__

