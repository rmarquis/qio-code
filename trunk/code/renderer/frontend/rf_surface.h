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
#include <shared/planeArray.h>
#include <api/staticModelCreatorAPI.h>

class r_surface_c {
	str name;
	str matName;
	class mtrAPI_i *mat;
	class textureAPI_i *lightmap;
	rVertexBuffer_c verts;
	rIndexBuffer_c indices;
	// shared index buffer (if it's used then this->indices can be left empty)
	// Used for animated models, where they vertices data is different
	// but indices remain the same
	const rIndexBuffer_c *refIndices;
	aabb bounds;
	// triangle planes are calculated while recalculating model normals.
	// they are used by shadow volumes creation algorithm
	planeArray_c trianglePlanes; // arraySTD_c of plane_c
	// this is not NULL only for skeletal model instances
	const class skelSurfaceAPI_i *mySkelSF;

	u32 registerVert(const struct simpleVert_s &v);
public:
	r_surface_c();
	~r_surface_c();

	u32 getNumVerts() const {
		return verts.size();
	}
	u32 getNumTris() const {
		return indices.getNumIndices() / 3;
	}
	u32 getNumIndices() const {
		return indices.getNumIndices();
	}
	const char *getName() const {
		return name;
	}
	mtrAPI_i *getMat() const {
		return mat;
	}
	const char *getMatName() const {
		return matName;
	}
	const rVertexBuffer_c &getVerts() const {
		return verts;
	}
	const rIndexBuffer_c &getIndices() const {
		if(refIndices) {
			return *refIndices;
		}
		return indices;
	}
	const rIndexBuffer_c &getIndices2() const {
		if(refIndices) {
			return *refIndices;
		}
		return indices;
	}
	u32 getIndex(u32 i) const {
		return indices.getIndex(i);
	}
	const planeArray_c &getTriPlanes() const {
		return trianglePlanes;
	}
	rVertexBuffer_c &getVerts() {
		return verts;
	}
	rIndexBuffer_c &getIndices() {
		return indices;
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
	void addWinding(const texturedVertex_c *verts, u32 numVerts);
	void addIndex(u32 idx) {
		indices.addIndex(idx);
	}
	void add3Indices(u32 i0, u32 i1, u32 i2) {
		indices.addIndex(i0);
		indices.addIndex(i1);
		indices.addIndex(i2);
	}
	void add3Indices_swapped(u32 i0, u32 i1, u32 i2) {
		indices.addIndex(i2);
		indices.addIndex(i1);
		indices.addIndex(i0);
	}
	void setIndicesU32(u32 newNumIndices, const u32 *newFirstIndex) {
		indices.fromU32Array(newNumIndices,newFirstIndex);
	}
	void addTriangle(const struct simpleVert_s &v0, const struct simpleVert_s &v1, const struct simpleVert_s &v2);
	void getTriangle(u32 triNum, vec3_c &v0, vec3_c &v1, vec3_c &v2) const;
	void addPoly(const struct simplePoly_s &poly);
	void addQuad(const rVert_c &v0, const rVert_c &v1, const rVert_c &v2, const rVert_c &v3);

	bool hasPoint(const vec3_c &p) const;
	u32 hasPoints(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) const;

	virtual void resizeVerts(u32 newNumVerts);
	virtual void setVert(u32 vertexIndex, const struct simpleVert_s &v);
	virtual void setVertexPos(u32 vertexIndex, const vec3_c &newPos);
	virtual void resizeIndices(u32 newNumIndices);
	virtual void setIndex(u32 indexNum, u32 value);
	virtual void transform(const class matrix_c &mat);
	virtual u32 countDuplicatedTriangles() const;
	virtual bool hasTriangle(u32 i0, u32 i1, u32 i2) const;

	const struct extraSurfEdgesData_s *getExtraSurfEdgesData() const;
	bool hasStageWithoutBlendFunc() const;

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
	void drawSurfaceWithSingleTexture(class textureAPI_i *tex);
	void addDrawCall(bool bUseVertexColors = false, const vec3_c *extraRGB = 0);

	void addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out);
	bool createDecalInternal(class decalProjector_c &proj);
	// skeletal models instancing
	void initSkelSurfInstance(const class skelSurfaceAPI_i *skelSF);
	void updateSkelSurfInstance(const class skelSurfaceAPI_i *skelSF, const class boneOrArray_c &bones);
	// keyframed models instancing
	void initKeyframedSurfaceInstance(const class kfSurfAPI_i *sfApi);
	void updateKeyframedSurfInstance(const class kfSurfAPI_i *sfApi, u32 singleFrame);
	// single sprite surface
	void initSprite(class mtrAPI_i *newSpriteMaterial, float newSpriteRadius, u32 subSpriteNumber = 0);
	void updateSprite(const class axis_c &viewAxis, const vec3_c &spritePos, float newSpriteRadius, u32 subSpriteNumber = 0, byte alpha = 255);
	// resets the surface data
	void setSurface(const char *matName, const struct simpleVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices);
	void setSurface(const char *matName, const class rVert_c *verts, u32 numVerts, const u16 *indices, u32 numIndices);

	bool traceRay(class trace_c &tr);

	virtual void scaleXYZ(float scale);
	virtual void swapYZ();
	virtual void swapIndexes();
	virtual void translateY(float ofs);
	virtual void multTexCoordsY(float f);
	virtual void multTexCoordsXY(float f);
	virtual void translateXYZ(const vec3_c &ofs);
	void addPointsToBounds(aabb &out);

	bool parseProcSurface(class parser_c &p);

	// procedural mesh generation
	void createFlatGrid(float size, int rows);
	void createBox(float halfSize);
	void createBox(float halfSize, const vec3_c &center) {
		createBox(halfSize);
		translateXYZ(center);
	}
	void createSphere(f32 radius, u32 polyCountX, u32 polyCountY);
	void createSphere(const vec3_c &center, f32 radius, u32 polyCountX, u32 polyCountY) {
		createSphere(radius,polyCountX,polyCountY);
		translateXYZ(center);
	}

	void scaleTexCoords(float tcScale) {
		rVert_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			v->tc *= tcScale;
		}
	}

	// returns true if surfaces material requires tangent/binormal vectors to be drawn
	bool needsTBN() const;

	void recalcNormals();
#ifdef RVERT_STORE_TANGENTS
	void recalcTBN();
#endif // RVERT_STORE_TANGENTS
	class mtrAPI_i *findSunMaterial() const;

	void calcVertexLighting(const struct pointLightSample_s &sample);
	void setAmbientLightingVec3_255(const vec3_c &color);
	void setAllVertexColors(byte r, byte g, byte b, byte a);

	void getReferencedMatNames(class perStringCallbackListener_i *callback) const;

	const aabb &getBB() const {
		return bounds;
	}

};

// tags (bones) for static models.
class r_tag_c {
	str name;
	vec3_c origin;
	vec3_c angles;
	//matrix_c mat;
public:
	void setupTag(const char *newTagName, const class vec3_c &newPos, const class vec3_c &newAngles) { 
		name = newTagName;
		origin = newPos;
		angles = newAngles;
		//mat.fromAnglesAndOrigin(angles,origin);
	}
	const class vec3_c &getAngles() const {
		return angles;
	}
	const class vec3_c &getOrigin() const {
		return origin;
	}
};

class r_model_c : public staticModelCreatorAPI_i {
	// non-animated render model surfaces
	arraySTD_c<r_surface_c> surfs;
	// extra bezier patches data (for bsp inline models)
	arraySTD_c<class r_bezierPatch_c*> bezierPatches;
	// extra tags data
	arraySTD_c<r_tag_c> tags;
	aabb bounds;
	str name;
	// used to speed up raycasting / decal creation
	struct tsOctTreeHeader_s *extraCollOctTree;
	// used to speed up stencil shadow volumes generation
	class r_stencilShadowCaster_c *ssvCaster;
	// this is not NULL only for skeletal model instances
	const class skelModelAPI_i *mySkel;

	void ensureExtraTrisoupOctTreeIsBuild();
public:
	r_model_c();
	~r_model_c();

	const char *getName() const {
		return name;
	}
	inline void setName(const char *newName) {
		name = newName;
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
	void iterateMaterialNames(class perStringCallbackListener_i *cb) const;
	// staticModelCreatorAPI_i implementation
	virtual void addTriangle(const char *matName, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2);
	virtual void addWinding(class mtrAPI_i *mat, const texturedVertex_c *verts, u32 numVerts);
	// for default, first surface
	virtual void resizeVerts(u32 newNumVerts);
	virtual void setVert(u32 vertexIndex, const struct simpleVert_s &v);
	virtual void setVertexPos(u32 vertexIndex, const vec3_c &newPos);
	virtual void resizeIndices(u32 newNumIndices);
	virtual void setIndex(u32 indexNum, u32 value);
	virtual void clear();	
	virtual void addSprite(const class vec3_c &origin, float radius, class mtrAPI_i *mat, const axis_c &viewerAxis, byte alpha);
	virtual void addSurface(const char *matName, const simpleVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices);
	virtual void addSurface(const char *matName, const rVert_c *verts, u32 numVerts, const u16 *indices, u32 numIndices);
	virtual void setAllVertexColors(byte r, byte g, byte b, byte a);
	// modelPostProcessFuncs_i implementation
	virtual void scaleXYZ(float scale);
	virtual void swapYZ();
	virtual void swapIndexes();
	virtual void translateY(float ofs);
	virtual void multTexCoordsY(float f);
	virtual void multTexCoordsXY(float f);
	virtual void translateXYZ(const class vec3_c &ofs);
	virtual void getCurrentBounds(class aabb &out);
	virtual void setAllSurfsMaterial(const char *newMatName);
	virtual u32 getNumSurfs() const;
	virtual void setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName);
	virtual void addTriangleToSF(u32 surfNum, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2);
	virtual void setNumSurfs(u32 newSurfsCount);
	virtual void resizeSurfaceVerts(u32 surfNum, u32 numVerts);
	virtual void setSurfaceVert(u32 surfNum, u32 vertIndex, const float *xyz, const float *st);
	virtual void setSurfaceIndicesU32(u32 surfNum, u32 numIndices, const u32 *indices);
	virtual void setSurfaceMaterial(u32 surfNum, const char *matName);
	virtual void recalcBoundingBoxes();
	virtual void addAbsTag(const char *newTagName, const class vec3_c &newPos, const class vec3_c &newAngles); 
	virtual bool hasPerSurfaceFunctionsImplemented() const {
		return true;
	}
	virtual void recalcTBNs() { 
		recalcModelTBNs();
	}
	virtual void scaleTexCoords(float f) {
		for(u32 i = 0; i < surfs.size(); i++) {
			surfs[i].scaleTexCoords(f);
		}
	}
	virtual void transform(const class matrix_c &mat);
	virtual u32 countDuplicatedTriangles() const;
	virtual bool hasTriangle(u32 i0, u32 i1, u32 i2) const;

	bool hasStageWithoutBlendFunc() const;
	bool getModelData(class staticModelCreatorAPI_i *out) const;

	bool getTagOrientation(int tagNum, class matrix_c &out) const;

	void addPatch(class r_bezierPatch_c *newPatch);

	void createVBOsAndIBOs();

	void addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out);
	// skeletal models instancing
	void initSkelModelInstance(const class skelModelAPI_i *skel);
	void updateSkelModelInstance(const class skelModelAPI_i *skel, const class boneOrArray_c &bones);
	// keyframed models instacing
	void initKeyframedModelInstance(const class kfModelAPI_i *kf);
	void updateKeyframedModelInstance(const class kfModelAPI_i *kf, u32 frameNum);
	// q3 multipart models (separate .md3's for head, torso, legs...)
	void initQ3PlayerModelInstance(const class q3PlayerModelAPI_i *qp);
	void updateQ3PlayerModelInstance(const class q3PlayerModelAPI_i *qp, u32 legsFrameNum, u32 torsoFrameNum);
	// single sprite models
	void initSprite(class mtrAPI_i *newSpriteMaterial, float newSpriteRadius);
	void updateSprite(const class axis_c &ax, float newSpriteRadius);

	mtrAPI_i *getMaterialForABSTriangleIndex(u32 absTriNum) const;
	int getSurfaceIndexForABSTriangleIndex(u32 absTriNum) const;

	bool traceRay(class trace_c &tr, bool bAllowExtraOctTreeCreation = true);
	bool createDecalInternal(class decalProjector_c &proj);
	bool createDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
								 const class vec3_c &normal, float radius, class mtrAPI_i *material);

	r_surface_c *registerPlanarSurf(const char *matName, const vec3_c &p0, const vec3_c &p1, const vec3_c &p2);
	r_surface_c *registerSurf(const char *matName);
	void addDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags = 0, bool useVertexColors = false, const vec3_c *extraRGB = 0);
	void addDrawCallsCulled(const class frustumExt_c &f);
	void addDrawCallsCulled(const class frustum_c &f);
	void cacheLightStaticModelInteractions(class rLightImpl_c *light);
	void setSurfMaterial(const char *surfName, const char *matName);
	void appendSkinRemap(const class rSkinRemap_c *skin);
	void boxSurfaces(const class aabb &bb, arraySTD_c<const class r_surface_c*> &out) const;

	void calcVertexLightingLocal(const struct pointLightSample_s &sample);
	void calcVertexLightingABS(const class matrix_c &mat, const struct pointLightSample_s &sample);
	void setAmbientLightingVec3_255(const vec3_c &color);

	bool parseProcModel(class parser_c &p);
	
	// model export
	bool writeOBJ(const char *fname) const;

	void getReferencedMatNames(class perStringCallbackListener_i *callback) const {
		for(u32 i = 0; i < surfs.size(); i++) {
			surfs[i].getReferencedMatNames(callback);
		}
	}
	void recalcModelNormals() {
		for(u32 i = 0; i < surfs.size(); i++) {
			surfs[i].recalcNormals();
		}
	}
	void recalcModelTBNs(bool forceTBNRecalculation = true) {
		for(u32 i = 0; i < surfs.size(); i++) {
			if(forceTBNRecalculation == false) {
				// if surface dont needs tangents and binormals, just recalculate normals
				if(surfs[i].needsTBN() == false) {
					surfs[i].recalcNormals();
					continue;
				}
			}
			surfs[i].recalcTBN();
		}
	}
	class mtrAPI_i *findSunMaterial() const {
		for(u32 i = 0; i < surfs.size(); i++) {
			mtrAPI_i *r = surfs[i].findSunMaterial();
			if(r)
				return r;
		}
		return 0;
	}
	void getSunBounds(aabb &bb) const {
		bb.clear();
		for(u32 i = 0; i < surfs.size(); i++) {
			mtrAPI_i *r = surfs[i].findSunMaterial();
			if(r)
				continue;
			bb.addBox(surfs[i].getBB());
		}
	}
	void precalculateStencilShadowCaster();
	const class r_stencilShadowCaster_c *getStencilShadowCaster() const {
		return this->ssvCaster;
	}
	void resizeSurfaces(u32 newNumSurfs) {
		surfs.resize(newNumSurfs);
	}
	const r_surface_c *getSurf(u32 sfNum) const {
		return &surfs[sfNum];
	}
	r_surface_c *getSurf(u32 sfNum) {
		return &surfs[sfNum];
	}

	u32 getNumSurfaces() const {
		return surfs.size();
	}
	const aabb &getBounds() const {
		return bounds;
	}
	inline void setBounds(const aabb &newBB) {
		bounds = newBB;
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

