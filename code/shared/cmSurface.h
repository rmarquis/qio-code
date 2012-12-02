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
	}
	virtual void resizeIndices(u32 newNumIndices) {
		indices.resize(newNumIndices);
	}
	virtual void setIndex(u32 indexNum, u32 value) {
		indices[indexNum] = value;
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
	virtual void multTexCoordsY(float f) { 
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