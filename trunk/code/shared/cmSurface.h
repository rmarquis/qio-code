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
#include <math/matrix.h>
#include <api/colMeshBuilderAPI.h>
#include <api/staticModelCreatorAPI.h>

class cmSurface_c : public colMeshBuilderAPI_i, public staticModelCreatorAPI_i {
	arraySTD_c<u32> indices;
	arraySTD_c<vec3_c> verts;
	mutable arraySTD_c<vec3_c> *scaledVerts; // scaled vertices for Bullet
	aabb bb;
public:
	cmSurface_c() {
		scaledVerts = 0;
	}
	~cmSurface_c() {
		if(scaledVerts) {
			delete scaledVerts;
		}
	}
	void prepareScaledVerts(float scaledVertsScale) const {
		if(scaledVerts == 0) {
			scaledVerts = new arraySTD_c<vec3_c>;
		}
		scaledVerts->resize(verts.size());
		for(u32 i = 0; i < verts.size(); i++) {
			(*scaledVerts)[i] = verts[i] * scaledVertsScale;
		}
	}
	void addSurface(const cmSurface_c &other) {
		u32 prevVerts = verts.size();
		u32 prevIndices = indices.size();
		verts.resize(verts.size()+other.verts.size());
		indices.resize(indices.size()+other.indices.size());
		memcpy(verts.getArray()+prevVerts,other.verts.getArray(),other.verts.getSizeInBytes());
		u32 *newIndices = indices.getArray() + prevIndices;
		for(u32 i = 0; i < other.indices.size(); i++) {
			newIndices[i] = prevVerts + other.indices[i]; 
		}
	}
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
	virtual void addXYZTri(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) {
		indices.push_back(verts.size()+0);
		indices.push_back(verts.size()+1);
		indices.push_back(verts.size()+2);
		addVert(p0);
		addVert(p1);
		addVert(p2);
	}
	virtual void addMesh(const float *pVerts, u32 vertStride, u32 numVerts, const void *pIndices, bool indices32Bit, u32 numIndices) {
		u32 firstVert = verts.size();
		verts.resize(firstVert + numVerts);
		const float *v = pVerts;
		vec3_c *nv = &verts[firstVert];
		for(u32 i = 0; i < numVerts; i++, nv++) {
			*nv = v;
			bb.addPoint(v);
			v = (const float*)(((byte*)v)+vertStride);
		}
		u32 firstIndex = indices.size();
		indices.resize(firstIndex + numIndices);
		u32 *newIndices = &indices[firstIndex];
		if(indices32Bit) {
			const u32 *indices32 = (const u32*)pIndices;
			for(u32 i = 0; i < numIndices; i++) {
				newIndices[i] = firstVert+indices32[i];
			}
		} else {
			const u16 *indices16 = (const u16*)pIndices;
			for(u32 i = 0; i < numIndices; i++) {
				newIndices[i] = firstVert+indices16[i];
			}
		}
	}
	const vec3_c *getVerts() const {
		return verts.getArray();
	}
	const u32 *getIndices() const  {
		return indices.getArray();
	}
	void setNumVerts(u32 newNumVerts) {
		verts.resize(newNumVerts);
	}
	void setVertex(u32 vertexIndex, const vec3_c &xyz) {
		verts[vertexIndex] = xyz;
	}
	void setNumIndices(u32 newNumIndices) {
		indices.resize(newNumIndices);
	}
	//void setIndex(u32 idx, u32 indexValue) {
	//	indices[idx] = indexValue;
	//}
	const byte *getVerticesBase() const {
		return (const byte*)verts.getArray();
	}
	const byte *getIndicesBase() const {
		return (const byte*)indices.getArray();
	}
	const vec3_c *getScaledVerts() const {
		if(scaledVerts == 0)
			return 0;
		return scaledVerts->getArray();
	}
	const byte *getScaledVerticesBase() const {
		if(scaledVerts == 0)
			return 0;
		return (const byte*)scaledVerts->getArray();
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
	// "surfNum" is ignored here
	virtual void addTriangleToSF(u32 surfNum, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
		// add a single triangle
		indices.push_back(verts.size());
		indices.push_back(verts.size()+1);
		indices.push_back(verts.size()+2);
		this->addVert(v0.xyz);
		this->addVert(v1.xyz);
		this->addVert(v2.xyz);
	}
	virtual void resizeVerts(u32 newNumVerts) {
		verts.resize(newNumVerts);
	}
	virtual void setVert(u32 vertexIndex, const struct simpleVert_s &v) {
		verts[vertexIndex] = v.xyz;
		bb.addPoint(v.xyz);
	}
	virtual void setVertexPos(u32 vertexIndex, const vec3_c &newPos) {
		verts[vertexIndex] = newPos;
	}
	virtual void resizeIndices(u32 newNumIndices) {
		indices.resize(newNumIndices);
	}
	virtual void setIndex(u32 indexNum, u32 value) {
		indices[indexNum] = value;
	}
	virtual void recalcBoundingBoxes() {
		bb.clear();
		for(u32 i = 0; i < verts.size(); i++) {
			bb.addPoint(verts[i]);
		}
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
	virtual void swapIndexes()  {
		for(u32 i = 0; i < indices.size(); i+= 3) {
			u32 tmp = indices[i];
			indices[i] = indices[i+2];
			indices[i+2] = tmp;
		}
	}
	virtual void translateY(float ofs) {
		vec3_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			v->y += ofs;
		}
		bb.mins.y += ofs;
		bb.maxs.y += ofs;
	}
	virtual void multTexCoordsY(float f) { 
		// ignore. Collision models dont need texcoords.
	}
	virtual void multTexCoordsXY(float f) { 
		// ignore. Collision models dont need texcoords.
	}
	virtual void translateXYZ(const class vec3_c &ofs) {
		vec3_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			(*v) += ofs;
		}
		bb.mins += ofs;
		bb.maxs += ofs;
	}
	virtual void transform(const class matrix_c &mat) {
		vec3_c *v = verts.getArray();
		for(u32 i = 0; i < verts.size(); i++, v++) {
			mat.transformPoint(*v);
		}
	}
	virtual void getCurrentBounds(class aabb &out) {
		out = bb;
	}
	virtual void setAllSurfsMaterial(const char *newMatName) {

	}
	virtual u32 getNumSurfs() const {
		return 0;
	}
	virtual void setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName) {
		
	}
	virtual void clear() {
		verts.clear();
		indices.clear();
		if(scaledVerts) {
			delete scaledVerts;
			scaledVerts = 0;
		}
		bb.clear();
	}

	// quake polygon generation
	void addPolyEdge(const vec3_c &v0, const vec3_c &v1, u32 localNum) {
		if(localNum == 0) {
			verts.push_back(v0);
		}
		verts.push_back(v1);
	}
	void calcPolygonIndexes() {
		for(u32 i = 2; i < verts.size(); i++) {
			indices.push_back(0);
			indices.push_back(i-1);
			indices.push_back(i);
		}
	}

	void addTriPointsToAABB(u32 triNum, aabb &out) const {
		u32 i0 = indices[triNum*3+0];
		u32 i1 = indices[triNum*3+1];
		u32 i2 = indices[triNum*3+2];
		out.addPoint(verts[i0]);
		out.addPoint(verts[i1]);
		out.addPoint(verts[i2]);
	}
	void calcTriListBounds(const arraySTD_c<u32> &triNums, class aabb &out) const {
		for(u32 i = 0; i < triNums.size(); i++) {
			u32 tri = triNums[i];
			addTriPointsToAABB(tri,out);
		}
	}

	void addToColMeshBuilder(colMeshBuilderAPI_i *out) const {
		for(u32 i = 0; i < indices.size(); i+= 3) {
			u32 i0 = indices[i+0];
			u32 i1 = indices[i+1];
			u32 i2 = indices[i+2];
			const vec3_c &v0 = verts[i0];
			const vec3_c &v1 = verts[i1];
			const vec3_c &v2 = verts[i2];
			out->addXYZTri(v0,v1,v2);
		}
	}
	// load vertices/triangles data directly from Doom3 .proc file
	bool loadDoom3ProcFileWorldModel(const char *fname);

	const vec3_c &getVert(u32 vertNum) const {
		return verts[vertNum];
	}
	u32 getIndex(u32 idx) const {
		return indices[idx];
	}

	const aabb &getAABB() const {
		return bb;
	}
	const arraySTD_c<vec3_c> &getVertsArray() const {
		return verts;
	}
};

#endif // __CMSURFACE_H__
