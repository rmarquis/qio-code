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
// rf_surface.cpp - static surface class
#include "rf_local.h"
#include "rf_surface.h"
#include "rf_drawCall.h"
#include "rf_skin.h"
#include "rf_bezier.h"
#include <api/rbAPI.h>
#include <api/materialSystemAPI.h>
#include <api/mtrAPI.h>
#include <api/skelModelAPI.h>
#include <api/kfModelAPI.h>
#include <api/coreAPI.h>
#include <shared/trace.h>
#include <shared/parser.h> // for Doom3 .proc surfaces parsing
#include <shared/autoCvar.h>
#include "rf_decalProjector.h"
#include <shared/simpleTexturedPoly.h>
#include <api/colMeshBuilderAPI.h>
#include "../pointLightSample.h"
#include <shared/perStringCallback.h>

static aCvar_c rf_dontRecalcNormals("rf_dontRecalcNormals","0");
static aCvar_c rf_dontRecalcTB("rf_dontRecalcTB","0");
static aCvar_c rf_fastCalcTBN("rf_fastCalcTBN","1");

//
//	r_surface_c class
//
r_surface_c::r_surface_c() {
	if(g_ms) {
		mat = g_ms->registerMaterial("defaultMaterial");
	} else {
		mat = 0;
	}
	lightmap = 0;
	mySkelSF = 0;
	bounds.clear();
	refIndices = 0;
}
r_surface_c::~r_surface_c() {
	this->clear();
}
void r_surface_c::addTriangle(const struct simpleVert_s &v0, const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
#if 0
	// add a new triangle
	indices.addIndex(verts.size());
	indices.addIndex(verts.size()+1);
	indices.addIndex(verts.size()+2);
	// use plane_c class to calculate triangle normal
	plane_c pl;
	pl.fromThreePoints(v2.xyz,v1.xyz,v0.xyz);
	rVert_c nv;
	nv.xyz = v0.xyz;
	nv.tc = v0.tc;
	nv.normal = pl.norm;
	verts.push_back(nv);
	nv.xyz = v1.xyz;
	nv.tc = v1.tc;
	verts.push_back(nv);
	nv.xyz = v2.xyz;
	nv.tc = v2.tc;
	verts.push_back(nv);
	// update bounds
	bounds.addPoint(v0.xyz);
	bounds.addPoint(v1.xyz);
	bounds.addPoint(v2.xyz);
#else
	// see if we can reuse some vertices
	u32 i0 = registerVert(v0);
	u32 i1 = registerVert(v1);
	u32 i2 = registerVert(v2);
	// add a new triangle
	indices.addIndex(i0);
	indices.addIndex(i1);
	indices.addIndex(i2);
	// update bounds
	bounds.addPoint(v0.xyz);
	bounds.addPoint(v1.xyz);
	bounds.addPoint(v2.xyz);
#endif
}	
u32 r_surface_c::registerVert(const struct simpleVert_s &in) {
	for(u32 i = 0; i < verts.size(); i++) {
		const rVert_c &v = verts[i];
		if(v.tc == in.tc && v.xyz == in.xyz) {
			return i;
		}
	}
	verts.push_back(rVert_c(in.tc,in.xyz));
	return verts.size()-1;
}
void r_surface_c::getTriangle(u32 triNum, vec3_c &v0, vec3_c &v1, vec3_c &v2) const {
	u32 i = triNum * 3;
	u32 i0 = indices[i+0];
	u32 i1 = indices[i+1];
	u32 i2 = indices[i+2];
	const rVert_c &vert0 = verts[i0];
	const rVert_c &vert1 = verts[i1];
	const rVert_c &vert2 = verts[i2];
	v0 = vert0.xyz;
	v1 = vert1.xyz;
	v2 = vert2.xyz;
}
void r_surface_c::addPoly(const simplePoly_s &poly) {
	for(u32 i = 2; i < poly.verts.size(); i++) {
		addTriangle(poly.verts[0],poly.verts[i-1],poly.verts[i]);
	}
}
void r_surface_c::resizeVerts(u32 newNumVerts) {
	verts.resize(newNumVerts);
}
void r_surface_c::setVert(u32 vertexIndex, const struct simpleVert_s &v) {
	rVert_c &rv = verts[vertexIndex];
	rv.xyz = v.xyz;
	rv.tc = v.tc;
}
void r_surface_c::setVertexPos(u32 vertexIndex, const vec3_c &newPos) {
	verts[vertexIndex].xyz = newPos;
}
void r_surface_c::resizeIndices(u32 newNumIndices) {
	// TODO: see if we can use u16 buffer here
	indices.initU32(newNumIndices);
}
void r_surface_c::setIndex(u32 indexNum, u32 value) {
	indices.setIndex(indexNum,value);
}
void r_surface_c::transform(const class matrix_c &mat) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		mat.transformPoint(v->xyz);
		mat.transformNormal(v->normal);
	}
}
u32 r_surface_c::countDuplicatedTriangles() const {
	return indices.countDuplicatedTriangles();
}
bool r_surface_c::hasTriangle(u32 i0, u32 i1, u32 i2) const {
	return indices.hasTriangle(i0,i1,i2);
}
const struct extraSurfEdgesData_s *r_surface_c::getExtraSurfEdgesData() const {
	if(mySkelSF == 0)
		return 0;
	return mySkelSF->getEdgesData();
}
void r_surface_c::setMaterial(mtrAPI_i *newMat) {
#if 1
	if(newMat == 0) {
		newMat = g_ms->registerMaterial("noMaterial");
	}
#endif
	mat = newMat;
	if(newMat == 0) {
		matName = "noMaterial";
	} else {
		matName = newMat->getName();
	}
}
void r_surface_c::setMaterial(const char *newMatName) {
	if(newMatName == 0 || newMatName[0] == 0) {
		matName = "noMaterial";
		mat = g_ms->registerMaterial("noMaterial");
		return;
	}
	matName = newMatName;
	mat = g_ms->registerMaterial(newMatName);
}
void r_surface_c::drawSurfaceWithSingleTexture(class textureAPI_i *tex) {
	rb->setBindVertexColors(false);
	rb->drawElementsWithSingleTexture(this->verts,this->indices,tex);
}

void r_surface_c::addDrawCall(bool bUseVertexColors, const vec3_c *extraRGB) {
	if(this->mat == 0)
		return;
	if(this->mat->getNumStages() == 0)
		return;
	if(refIndices) {
		RF_AddDrawCall(&this->verts,refIndices,this->mat,this->lightmap,this->mat->getSort(),bUseVertexColors,0,extraRGB);
	} else {
		RF_AddDrawCall(&this->verts,&this->indices,this->mat,this->lightmap,this->mat->getSort(),bUseVertexColors,0,extraRGB);
	}
}
void r_surface_c::addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out) {
#if 0
	for(u32 i = 0; i < indices.getNumIndices(); i+=3) {
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		const rVert_c &v0 = verts[i0];
		const rVert_c &v1 = verts[i1];
		const rVert_c &v2 = verts[i2];
		out->addXYZTri(v0.xyz,v1.xyz,v2.xyz);
	}
#else
	out->addMesh(verts.getArray()->xyz,sizeof(rVert_c),verts.size(),
		indices.getArray(),indices.is32Bit(),indices.getNumIndices());
#endif
}
bool r_surface_c::traceRay(class trace_c &tr) {
	if(tr.getTraceBounds().intersect(this->bounds) == false)
		return false;
	bool hasHit = false;
	for(u32 i = 0; i < indices.getNumIndices(); i+=3) {
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		const rVert_c &v0 = verts[i0];
		const rVert_c &v1 = verts[i1];
		const rVert_c &v2 = verts[i2];
#if 1
		aabb tmpBB;
		tmpBB.fromTwoPoints(v0.xyz,v1.xyz);
		tmpBB.addPoint(v2.xyz);
		if(tmpBB.intersect(tr.getTraceBounds()) == false) {
			continue;
		}
#endif
		if(tr.clipByTriangle(v0.xyz,v1.xyz,v2.xyz,true)) {
			hasHit = true;
		}
	}
	if(hasHit) {
		tr.setHitRMaterial(this->mat);
	}
	return hasHit;
}	
bool r_surface_c::createDecalInternal(class decalProjector_c &proj) {
	u32 newPoints = 0;
	for(u32 i = 0; i < indices.getNumIndices(); i+=3) {
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		const rVert_c &v0 = verts[i0];
		const rVert_c &v1 = verts[i1];
		const rVert_c &v2 = verts[i2];
		newPoints += proj.clipTriangle(v0.xyz,v1.xyz,v2.xyz);
	}
	return newPoints;
}
void r_surface_c::initSkelSurfInstance(const skelSurfaceAPI_i *skelSF) {
	clear();
	this->mySkelSF = skelSF;
	//this->name = skelSF->getSurfName();
	setMaterial(skelSF->getMatName());
	verts.resize(skelSF->getNumVerts());
	rVert_c *v = verts.getArray();
	const skelVert_s *inV = skelSF->getVerts();
	for(u32 i = 0; i < verts.size(); i++, v++, inV++) {
		v->tc = inV->tc;
	}
	indices.addU16Array(skelSF->getIndices(),skelSF->getNumIndices());
}
void r_surface_c::updateSkelSurfInstance(const class skelSurfaceAPI_i *skelSF, const class boneOrArray_c &bones) {
	rVert_c *v = verts.getArray();
	const skelVert_s *inV = skelSF->getVerts();
	const skelWeight_s *inWeights = skelSF->getWeights();
	bounds.clear();
	for(u32 i = 0; i < verts.size(); i++, v++, inV++) {
		const skelWeight_s *w = inWeights + inV->firstWeight;
		v->xyz.clear();
		for(u32 j = 0; j < inV->numWeights; j++, w++) {
			vec3_c p;
			if(w->boneIndex >= bones.size()) {
				g_core->RedWarning("r_surface_c::updateSkelSurfInstance: bone index %i out of range <0,%i)\n",w->boneIndex,bones.size());
			} else {
				bones[w->boneIndex].mat.transformPoint(w->ofs,p);
				v->xyz += p * w->weight;
			}
		}
		bounds.addPoint(v->xyz);
	}	
}
void r_surface_c::initKeyframedSurfaceInstance(const kfSurfAPI_i *sfApi) {
	u32 numVerts = sfApi->getNumVertices();
	verts.resize(numVerts);
	sfApi->copyTexCoords(verts[0].tc,sizeof(rVert_c));
	this->name = sfApi->getSurfName();
	const char *sfMatName = sfApi->getMatName();
	this->setMaterial(sfMatName);
	this->refIndices = sfApi->getIBO();
	updateKeyframedSurfInstance(sfApi,0);
}
void r_surface_c::updateKeyframedSurfInstance(const class kfSurfAPI_i *sfApi, u32 singleFrame) {
	sfApi->instanceSingleFrame(verts[0].xyz,sizeof(rVert_c),singleFrame);
}
void r_surface_c::initSprite(class mtrAPI_i *newSpriteMaterial, float newSpriteRadius, u32 subSpriteNumber) {
	// set material
	this->setMaterial(newSpriteMaterial);

	u32 firstVertex = subSpriteNumber * 4;
	u32 numRequiredVertices = firstVertex + 4;
	u32 firstIndex = subSpriteNumber * 6;
	u32 numRequiredIndices = firstIndex + 6;

	// init indices
	if(indices.isNotSet()) {
		indices.initU16(numRequiredIndices);
	}
	indices.ensureAllocated_indices(numRequiredIndices);
	indices.forceSetIndexCount(numRequiredIndices);

	indices.setIndex(firstIndex+0,firstVertex+0);
	indices.setIndex(firstIndex+1,firstVertex+1);
	indices.setIndex(firstIndex+2,firstVertex+2);
	indices.setIndex(firstIndex+3,firstVertex+2);
	indices.setIndex(firstIndex+4,firstVertex+3);
	indices.setIndex(firstIndex+5,firstVertex+0);

	// init vertices
	verts.resize(numRequiredVertices);

	verts[firstVertex+0].tc[0] = 0.f;
	verts[firstVertex+0].tc[1] = 0.f;

	verts[firstVertex+1].tc[0] = 1.f;
	verts[firstVertex+1].tc[1] = 0.f;

	verts[firstVertex+2].tc[0] = 1.f;
	verts[firstVertex+2].tc[1] = 1.f;

	verts[firstVertex+3].tc[0] = 0.f;
	verts[firstVertex+3].tc[1] = 1.f;

	// just to be safe - force a single update with identity camera axis
	axis_c axisIdentity;
	axisIdentity.identity();
	updateSprite(axisIdentity,vec3_c(0,0,0),newSpriteRadius,subSpriteNumber);
}
void r_surface_c::updateSprite(const class axis_c &viewAxis, const vec3_c &spritePos, float newSpriteRadius, u32 subSpriteNumber, byte alpha) {
	vec3_c up = viewAxis.getUp() * newSpriteRadius;
	vec3_c left = viewAxis.getLeft() * newSpriteRadius;

	u32 firstVertex = subSpriteNumber * 4;
	u32 numRequiredVertices = firstVertex + 4;

	verts[firstVertex+0].xyz = spritePos + left + up;
	verts[firstVertex+1].xyz = spritePos - left + up;
	verts[firstVertex+2].xyz = spritePos - left - up;
	verts[firstVertex+3].xyz = spritePos + left - up;
	verts[firstVertex+0].normal = viewAxis.getForward();
	verts[firstVertex+1].normal = viewAxis.getForward();
	verts[firstVertex+2].normal = viewAxis.getForward();
	verts[firstVertex+3].normal = viewAxis.getForward();
	verts[firstVertex+0].color[3] = alpha;
	verts[firstVertex+1].color[3] = alpha;
	verts[firstVertex+2].color[3] = alpha;
	verts[firstVertex+3].color[3] = alpha;
}	
void r_surface_c::setSurface(const char *matName, const struct simpleVert_s *pVerts, u32 numVerts, const u16 *pIndices, u32 numIndices) {
	setMaterial(matName);
	verts.destroy();
	indices.destroy();
	indices.addU16Array(pIndices,numIndices);
	verts.resize(numVerts);
	for(u32 i = 0; i < numVerts; i++) {
		verts[i].xyz = pVerts[i].xyz;
		verts[i].tc = pVerts[i].tc;
	}
}
void r_surface_c::setSurface(const char *matName, const class rVert_c *pVerts, u32 numVerts, const u16 *pIndices, u32 numIndices) {
	setMaterial(matName);
	verts.destroy();
	indices.destroy();
	indices.addU16Array(pIndices,numIndices);
	verts.setVertexArray(pVerts,numVerts);
}
void r_surface_c::scaleXYZ(float scale) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz *= scale;
	}
	bounds.scaleBB(scale);
}
void r_surface_c::swapYZ() {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		float tmp = v->xyz.y;
		v->xyz.y = v->xyz.z;
		v->xyz.z = tmp;
	}
	bounds.swapYZ();
}
void r_surface_c::swapIndexes() {
	indices.swapIndices();
}
void r_surface_c::translateY(float ofs) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz.y += ofs;
	}
	bounds.translate(vec3_c(0,ofs,0));
}
void r_surface_c::multTexCoordsY(float f) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->tc.y *= f;
	}
}
void r_surface_c::multTexCoordsXY(float f) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->tc *= f;
	}
}
void r_surface_c::translateXYZ(const vec3_c &ofs) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz += ofs;
	}
	bounds.translate(ofs);
}
void r_surface_c::addPointsToBounds(aabb &out) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		out.addPoint(v->xyz);
	}
}
bool r_surface_c::needsTBN() const {
	if(mat == 0)
		return false;
	if(rf_dontRecalcTB.getInt())
		return false;
	return rb->areTangentsNeededForMaterial(mat);
}
void r_surface_c::recalcNormals() {
	if(rf_dontRecalcNormals.getInt())
		return;
	verts.nullNormals();
	const rIndexBuffer_c &pIndices = getIndices2();
	trianglePlanes.resize(pIndices.getNumTriangles());
	plane_c *op = trianglePlanes.getArray();
	for(u32 i = 0; i < pIndices.getNumIndices(); i+=3, op++) {
		u32 i0 = pIndices[i+0];
		u32 i1 = pIndices[i+1];
		u32 i2 = pIndices[i+2];
		rVert_c &v0 = verts[i0];
		rVert_c &v1 = verts[i1];
		rVert_c &v2 = verts[i2];
#if 0
		vec3_c e0 = v2.xyz - v0.xyz;
		vec3_c e1 = v1.xyz - v0.xyz;
		vec3_c newNorm;
		newNorm.crossProduct(e0,e1);
		newNorm.normalize();
		v0.normal += newNorm;
		v1.normal += newNorm;
		v2.normal += newNorm;
#else
		op->fromThreePoints(v2.xyz,v1.xyz,v0.xyz);
		v0.normal += op->norm;
		v1.normal += op->norm;
		v2.normal += op->norm;
#endif
	}
	verts.normalizeNormals();
}

void R_FastCalcTBNArray( plane_c *planes, rVert_c *verts, const int numVerts, const u16 *indexes, const int numIndexes ) {
	typedef byte counter_t;
	counter_t *counts = (counter_t *)alloca( numVerts * sizeof( counts[0] ) );
	memset( counts, 0, numVerts * sizeof( counts[0] ) );

	plane_c *planesPtr = planes;
	for (u32 i = 0; i < numIndexes; i+=3) {
		rVert_c *a, *b, *c;
		unsigned long signBit;
		float d0[5], d1[5], f, area;
		vec3_c n, t0, t1;

		int v0 = indexes[i + 0];
		int v1 = indexes[i + 1];
		int v2 = indexes[i + 2];

		a = verts + v0;
		b = verts + v1;
		c = verts + v2;

		d0[0] = b->xyz[0] - a->xyz[0];
		d0[1] = b->xyz[1] - a->xyz[1];
		d0[2] = b->xyz[2] - a->xyz[2];
		d0[3] = b->tc[0] - a->tc[0];
		d0[4] = b->tc[1] - a->tc[1];

		d1[0] = c->xyz[0] - a->xyz[0];
		d1[1] = c->xyz[1] - a->xyz[1];
		d1[2] = c->xyz[2] - a->xyz[2];
		d1[3] = c->tc[0] - a->tc[0];
		d1[4] = c->tc[1] - a->tc[1];

		// normal
		n[0] = d1[1] * d0[2] - d1[2] * d0[1];
		n[1] = d1[2] * d0[0] - d1[0] * d0[2];
		n[2] = d1[0] * d0[1] - d1[1] * d0[0];

		f = G_rsqrt( n.x * n.x + n.y * n.y + n.z * n.z );

		n.x *= f;
		n.y *= f;
		n.z *= f;

		planesPtr->fromPointAndNormal(a->xyz,n);
		planesPtr++;

		// area sign bit
		area = d0[3] * d1[4] - d0[4] * d1[3];
		signBit = ( *(unsigned long *)&area ) & ( 1 << 31 );

		// first tangent
		t0[0] = d0[0] * d1[4] - d0[4] * d1[0];
		t0[1] = d0[1] * d1[4] - d0[4] * d1[1];
		t0[2] = d0[2] * d1[4] - d0[4] * d1[2];

		f = G_rsqrt( t0.x * t0.x + t0.y * t0.y + t0.z * t0.z );
		*(unsigned long *)&f ^= signBit;

		t0.x *= f;
		t0.y *= f;
		t0.z *= f;

		// second tangent
		t1[0] = d0[3] * d1[0] - d0[0] * d1[3];
		t1[1] = d0[3] * d1[1] - d0[1] * d1[3];
		t1[2] = d0[3] * d1[2] - d0[2] * d1[3];

		f = G_rsqrt( t1.x * t1.x + t1.y * t1.y + t1.z * t1.z );
		*(unsigned long *)&f ^= signBit;

		t1.x *= f;
		t1.y *= f;
		t1.z *= f;

		if ( counts[v0] ) {
			a->normal += n;
			a->tan += t0;
			a->bin += t1;
			counts[v0]++;
		} else {
			a->normal = n;
			a->tan = t0;
			a->bin = t1;
			counts[v0] = 1;
		}

		if ( counts[v1] ) {
			b->normal += n;
			b->tan += t0;
			b->bin += t1;
			counts[v1]++;
		} else {
			b->normal = n;
			b->tan = t0;
			b->bin = t1;
			counts[v1] = 1;
		}

		if ( counts[v2] ) {
			c->normal += n;
			c->tan += t0;
			c->bin += t1;
			counts[v2]++;
		} else {
			c->normal = n;
			c->tan = t0;
			c->bin = t1;
			counts[v2] = 1;
		}
	}
	// V: normalization pass
	// this is not a perfect solution but gives acceptable results
	for(u32 i = 0; i < numVerts; i++) {
		if(counts[i] >= 2) {
			rVert_c *v = verts + i;
			float inv = 1.f / float(counts[i]);
			v->normal *= inv;
			v->bin *= inv;
			v->tan *= inv;
		}
	}
}

#ifdef RVERT_STORE_TANGENTS
void r_surface_c::recalcTBN() {
#if 1
	const rIndexBuffer_c &pIndices = getIndices2();
	if(rf_fastCalcTBN.getInt()) {
		if(trianglePlanes.size() != getNumTris())
			trianglePlanes.resize(getNumTris());
		R_FastCalcTBNArray(trianglePlanes.getArray(),verts.getArray(),verts.size(),pIndices.getU16Ptr(),indices.getNumIndices());
	} else {
		verts.nullTBN();
		verts.calcTBNForIndices(pIndices,&this->trianglePlanes);
		verts.normalizeTBN();
	}
#else
	const rIndexBuffer_c &pIndices = getIndices2();
	trianglePlanes.resize(pIndices.getNumTriangles());
	plane_c *op = trianglePlanes.getArray();
	verts.nullTBN();
	for(u32 i = 0; i < pIndices.getNumIndices(); i+=3) {
		u32 i0 = pIndices[i];
		u32 i1 = pIndices[i+1];
		u32 i2 = pIndices[i+2];

		rVert_c &v0 = verts[i0];
		rVert_c &v1 = verts[i1];
		rVert_c &v2 = verts[i2];

		// calc normal
		op->fromThreePoints(v2.xyz,v1.xyz,v0.xyz);
		v0.normal += op->norm;
		v1.normal += op->norm;
		v2.normal += op->norm;

		// calc tangent
		const vec3_c &p0 = v0.xyz;
		const vec3_c &p1 = v1.xyz;
		const vec3_c &p2 = v2.xyz;

		const vec2_c &tc0 = v0.tc;
		const vec2_c &tc1 = v1.tc;
		const vec2_c &tc2 = v2.tc;

		float x1 = p1.x - p0.x;
		float x2 = p2.x - p0.x;
		float y1 = p1.y - p0.y;
		float y2 = p2.y - p0.y;
		float z1 = p1.z - p0.z;
		float z2 = p2.z - p0.z;

		float s1 = tc1.x - tc0.x;
		float s2 = tc2.x - tc0.x;
		float t1 = tc1.y - tc0.y;
		float t2 = tc2.y - tc0.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		vec3_c sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		vec3_c tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		v0.tan += sdir;
		v1.tan += sdir;
		v2.tan += sdir;

		v0.bin += tdir;
		v1.bin += tdir;
		v2.bin += tdir;
	}
	for (u32 i = 0; i < verts.size(); i++) {
		rVert_c &v = verts[i];

		v.normal.normalize();

		const vec3_c &n = v.normal;
		vec3_c t = v.tan;

		// Gram-Schmidt orthogonalize
		v.tan = (t - n * n.dotProduct(t));
		v.tan.normalize();

		v.bin.normalize();

		// calculate handedness
		float w = (n.crossProduct(t).dotProduct(v.bin) < 0.0F) ? -1.0F : 1.0F;

		//v.bin = v.tan.crossProduct(v.normal);
		//v.bin *= w;
	}
#endif
}
#endif // RVERT_STORE_TANGENTS

void CalcVertexGridLighting(rVertexBuffer_c &verts, const struct pointLightSample_s &in);


class mtrAPI_i *r_surface_c::findSunMaterial() const {
	if(mat == 0)
		return 0;
	if(mat->getSunParms())
		return mat;
	return 0;
}
void r_surface_c::calcVertexLighting(const struct pointLightSample_s &sample) {
	CalcVertexGridLighting(verts,sample);
}
void r_surface_c::setAmbientLightingVec3_255(const vec3_c &color) {
	verts.setVertexAlphaToConstValue(255);
	verts.setVertexColorsToConstValuesVec3255(color);
}
void r_surface_c::setAllVertexColors(byte r, byte g, byte b, byte a) {
	verts.setVertexAlphaToConstValue(a);
	verts.setVertexColorsToConstValuesVec3255(vec3_c(r,g,b));
}	
void r_surface_c::getReferencedMatNames(class perStringCallbackListener_i *callback) const {
	callback->perStringCallback(getMatName());
}
	
bool r_surface_c::parseProcSurface(class parser_c &p) {
	int sky = -1;
	if(p.atWord("{")==false) {
		// check for extra 'sky' parameter used in Q4 proc files
		str token = p.getToken();
		if(token.isNumerical() && p.atWord("{")) {
			sky = atoi(token);
		} else {
			g_core->RedWarning("r_surface_c::parseProcSurface: expected '{' to follow \"model\"'s surface in file %s at line %i, found %s\n",
				p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
			return true; // error
		}
	}
	this->matName = p.getToken();
	this->mat = g_ms->registerMaterial(this->matName);
	u32 numVerts = p.getInteger();
	u32 numIndices = p.getInteger();
	// read verts
	verts.resize(numVerts);
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < numVerts; i++, v++) {
		if(p.atWord("(")==false) {
			g_core->RedWarning("r_surface_c::parseProcSurface: expected '(' to follow vertex %i in file %s at line %i, found %s\n",
				i,p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
			return true; // error
		}
		p.getFloatMat(v->xyz,3);
		p.getFloatMat(v->tc,2);
		p.getFloatMat(v->normal,3);
		if(p.atWord(")")==false) {
			g_core->RedWarning("r_surface_c::parseProcSurface: expected '(' after vertex %i in file %s at line %i, found %s\n",
				i,p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
			return true; // error
		}
	}
	// read triangles
	u16 *indicesu16 = indices.initU16(numIndices);
	for(u32 i = 0; i < numIndices; i++) {
		indicesu16[i] = p.getInteger();
	}
	if(p.atWord("}")==false) {
		g_core->RedWarning("r_surface_c::parseProcSurface: expected closing '}' for \"model\"'s surface block in file %s at line %i, found %s\n",
			p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	return false; // OK
}
u32 getQTIndex(u32 factor, u32 x, u32 y) {
	u32 out = x * (factor + 1) + y;
	return out;
}
void r_surface_c::createFlatGrid(float size, int rows) {
	float halfSize = size * 0.5f;

	rVert_c controls[4];
	controls[0].xyz.set(-halfSize,-halfSize,0);
	controls[0].tc.set(0,0);
	controls[0].normal.set(0,0,1);
	controls[1].xyz.set(halfSize,-halfSize,0);
	controls[1].tc.set(0,1);
	controls[1].normal.set(0,0,1);
	controls[2].xyz.set(halfSize,halfSize,0);
	controls[2].tc.set(1,1);
	controls[2].normal.set(0,0,1);
	controls[3].xyz.set(-halfSize,halfSize,0);
	controls[3].tc.set(1,0);
	controls[3].normal.set(0,0,1);

	int factor = rows;
	
	this->verts.resize(Square(factor+1));
	u32 vertIndex = 0;

	// calc first (factor+1)*2 vertices
	for(int i = 0; i <= factor; i++) {
		float frac = float(i) / float(factor);
		rVert_c first, second;
		first.lerpAll(controls[0],controls[1],frac);
		this->verts[vertIndex] = first;
		vertIndex++;
		second.lerpAll(controls[3],controls[2],frac);
		for(int j = 1; j < factor; j++) {
			float frac = float(j) / float(factor);
			rVert_c nv;
			nv.lerpAll(first,second,frac);
			this->verts[vertIndex] = nv;
			vertIndex++;
		}
		this->verts[vertIndex] = second;
		vertIndex++;
	}
	// create indices
	// quads
	//u16 *indices16 = this->indices.initU16(Square(factor) * 4);
	// triangle pairs
	u16 *indices16 = this->indices.initU16(Square(factor) * 6);
	u32 i = 0;
	for(int x = 0; x < factor; x++) {
		for(int y = 0; y < factor; y++) {
			u32 i0 = getQTIndex(factor,x,y);;
			u32 i1 = getQTIndex(factor,x,y+1);
			u32 i2 = getQTIndex(factor,x+1,y+1);
			u32 i3 = getQTIndex(factor,x+1,y);
#if 0
			// quads (GL_QUADS)
			indices16[i] = i0;
			i++;
			indices16[i] = i1;
			i++;
			indices16[i] = i2;
			i++;
			indices16[i] = i3;
			i++;
#else
			// two triangles (GL_TRIANGLES)
			indices16[i] = i0;
			i++;
			indices16[i] = i1;
			i++;
			indices16[i] = i2;
			i++;

			indices16[i] = i3;
			i++;
			indices16[i] = i0;
			i++;
			indices16[i] = i2;
			i++;
#endif
		}
	}
}
void r_surface_c::addQuad(const rVert_c &v0, const rVert_c &v1, const rVert_c &v2, const rVert_c &v3) {
	u32 i0 = verts.size();
	verts.push_back(v0);
	u32 i1 = verts.size();
	verts.push_back(v1);
	u32 i2 = verts.size();
	verts.push_back(v2);
	u32 i3 = verts.size();
	verts.push_back(v3);

	bounds.addPoint(v0.xyz);
	bounds.addPoint(v1.xyz);
	bounds.addPoint(v2.xyz);
	bounds.addPoint(v3.xyz);
	
	indices.addTriangleINV(i0,i1,i2);
	indices.addTriangleINV(i2,i3,i0);
}
void r_surface_c::createBox(float halfSize) {
    // front face
	addQuad(
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c(-halfSize, -halfSize,  halfSize)),
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c( halfSize, -halfSize,  halfSize)),
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c( halfSize,  halfSize,  halfSize)),
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c(-halfSize,  halfSize,  halfSize))
	);
    // back face
	addQuad(
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c(-halfSize, -halfSize, -halfSize)), 
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c(-halfSize,  halfSize, -halfSize)),
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c( halfSize,  halfSize, -halfSize)),
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c( halfSize, -halfSize, -halfSize))
	);
    // top face
	addQuad(
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c(-halfSize,  halfSize, -halfSize)),
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c(-halfSize,  halfSize,  halfSize)),
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c( halfSize,  halfSize,  halfSize)),
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c( halfSize,  halfSize, -halfSize))
	);
    // bottom face
	addQuad(
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c(-halfSize, -halfSize, -halfSize)),
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c( halfSize, -halfSize, -halfSize)), 
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c( halfSize, -halfSize,  halfSize)), 
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c(-halfSize, -halfSize,  halfSize))
	);
    // right face
	addQuad(
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c( halfSize, -halfSize, -halfSize)),
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c( halfSize,  halfSize, -halfSize)),
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c( halfSize,  halfSize,  halfSize)),
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c( halfSize, -halfSize,  halfSize))
	); 
    // left face
	addQuad(
		rVert_c(vec2_c(0.0f, 0.0f), vec3_c(-halfSize, -halfSize, -halfSize)),
		rVert_c(vec2_c(1.0f, 0.0f), vec3_c(-halfSize, -halfSize,  halfSize)),
		rVert_c(vec2_c(1.0f, 1.0f), vec3_c(-halfSize,  halfSize,  halfSize)), 
		rVert_c(vec2_c(0.0f, 1.0f), vec3_c(-halfSize,  halfSize, -halfSize))
	);
	this->recalcTBN();
}
void r_surface_c::createSphere(f32 radius, u32 polyCountX, u32 polyCountY) {
	if (polyCountX < 2)
		polyCountX = 2;
	if (polyCountY < 2)
		polyCountY = 2;
	while (polyCountX * polyCountY > 32767) // prevent u16 overflow
	{
		polyCountX /= 2;
		polyCountY /= 2;
	}

	const u32 polyCountXPitch = polyCountX+1; // get to same vertex on next level

	indices.setTypeU16();
	indices.reserve((polyCountX * polyCountY) * 6);

	u32 level = 0;

	for (u32 p1 = 0; p1 < polyCountY-1; ++p1)
	{
		//main quads, top to bottom
		for (u32 p2 = 0; p2 < polyCountX - 1; ++p2)
		{
			const u32 curr = level + p2;
			this->indices.push_back(curr + 1);
			this->indices.push_back(curr);
			this->indices.push_back(curr + polyCountXPitch);
			this->indices.push_back(curr + 1 + polyCountXPitch);
			this->indices.push_back(curr+1);
			this->indices.push_back(curr + polyCountXPitch);
		}

		// the connectors from front to end
		this->indices.push_back(level + polyCountX);
		this->indices.push_back(level + polyCountX - 1);
		this->indices.push_back(level + polyCountX - 1 + polyCountXPitch);

		this->indices.push_back(level + polyCountX + polyCountXPitch);
		this->indices.push_back(level + polyCountX);
		this->indices.push_back(level + polyCountX - 1 + polyCountXPitch);
		level += polyCountXPitch;
	}

	const u32 polyCountSq = polyCountXPitch * polyCountY; // top point
	const u32 polyCountSq1 = polyCountSq + 1; // bottom point
	const u32 polyCountSqM1 = (polyCountY - 1) * polyCountXPitch; // last row's first vertex

	for (u32 p2 = 0; p2 < polyCountX - 1; ++p2)
	{
		// create triangles which are at the top of the sphere
		this->indices.push_back(p2);
		this->indices.push_back(p2 + 1);
		this->indices.push_back(polyCountSq);


		// create triangles which are at the bottom of the sphere

		this->indices.push_back(polyCountSq1);
		this->indices.push_back(polyCountSqM1 + p2 + 1);
		this->indices.push_back(polyCountSqM1 + p2);
	}

	// create final triangle which is at the top of the sphere

	this->indices.push_back(polyCountX-1);
	this->indices.push_back(polyCountX);
	this->indices.push_back(polyCountSq);

	// create final triangle which is at the bottom of the sphere

	this->indices.push_back(polyCountSq1);
	this->indices.push_back(polyCountSqM1);
	this->indices.push_back(polyCountSqM1 + polyCountX - 1);

	// calculate the angle which separates all points in a circle
	const f64 angleX = 2 * M_PI / polyCountX;
	const f64 angleY = M_PI / polyCountY;

	u32 i=0;
	f64 axz;

	// we don't start at 0.

	f64 ay = 0;//angleY / 2;

	this->verts.resize((polyCountXPitch * polyCountY) + 2);
	for (u32 y = 0; y < polyCountY; ++y)
	{
		ay += angleY;
		const f64 sinay = sin(ay);
		axz = 0;

		// calculate the necessary vertices without the doubled one
		for (u32 xz = 0;xz < polyCountX; ++xz)
		{
			// calculate points position

			const vec3_c pos(static_cast<f32>(radius * cos(axz) * sinay),
						static_cast<f32>(radius * cos(ay)),
						static_cast<f32>(radius * sin(axz) * sinay));
			// for spheres the normal is the position
			vec3_c normal(pos);
			normal.normalize();

			// calculate texture coordinates via sphere mapping
			// tu is the same on each level, so only calculate once
			f32 tu = 0.5f;
			if (y==0)
			{
				if (normal.y != -1.0f && normal.y != 1.0f)
					tu = static_cast<f32>(acos(Q_clamp(normal.x/sinay, -1.0, 1.0)) * 0.5 *M_ONEDIVPI64);
				if (normal.z < 0.0f)
					tu=1-tu;
			}
			else
				tu = this->verts[i-polyCountXPitch].tc.x;
			this->verts[i] = rVert_c(pos.x, pos.y, pos.z,
						normal.x, normal.y, normal.z,
						tu,	static_cast<f32>(ay*M_ONEDIVPI64));
			++i;
			axz += angleX;
		}
		// This is the doubled vertex on the initial position
		this->verts[i] = this->verts[i-polyCountX];
		this->verts[i].tc.x=1.0f;
		++i;
	}

	// the vertex at the top of the sphere
	this->verts[i] = rVert_c(0.0f,radius,0.0f, 0.0f,1.0f,0.0f, 0.5f, 0.0f);

	// the vertex at the bottom of the sphere
	++i;
	this->verts[i] = rVert_c(0.0f,-radius,0.0f, 0.0f,-1.0f,0.0f, 0.5f, 1.0f);

	// recalculate bounding box

	this->bounds.reset(this->verts[i].xyz);
	this->bounds.addPoint(this->verts[i-1].xyz);
	this->bounds.addPoint(radius,0.0f,0.0f);
	this->bounds.addPoint(-radius,0.0f,0.0f);
	this->bounds.addPoint(0.0f,0.0f,radius);
	this->bounds.addPoint(0.0f,0.0f,-radius);
}
//
//	r_model_c class
//
#include <shared/cmTriSoupOctTree.h>
r_model_c::r_model_c() {
	extraCollOctTree = 0;
	bounds.clear();
	ssvCaster = 0;
	mySkel = 0;
}
#include "rf_stencilShadowCaster.h"
r_model_c::~r_model_c() {
	clear();
}
void r_model_c::precalculateStencilShadowCaster() {
	if(ssvCaster) {
		delete ssvCaster;
	}
	ssvCaster = new r_stencilShadowCaster_c;
	ssvCaster->addRModel(this);
	ssvCaster->calcEdges();
}
void r_model_c::addTriangle(const char *matName, const struct simpleVert_s &v0,
							const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
	// HACK: ignore collision surfaces from Prey LWO models!
	if(!stricmp(matName,"textures/common/collision") || !stricmp(matName,"textures/common/collision_metal")) {
		return;
	}
	r_surface_c *sf = registerSurf(matName);
	sf->addTriangle(v0,v1,v2);
	this->bounds.addPoint(v0.xyz);
	this->bounds.addPoint(v1.xyz);
	this->bounds.addPoint(v2.xyz);
}
void r_model_c::resizeVerts(u32 newNumVerts) {
	if(surfs.size() == 0)
		surfs.resize(1);
	surfs[0].resizeVerts(newNumVerts);
}
void r_model_c::setVert(u32 vertexIndex, const struct simpleVert_s &v) {
	if(surfs.size() == 0)
		surfs.resize(1);
	surfs[0].setVert(vertexIndex,v);
}
void r_model_c::setVertexPos(u32 vertexIndex, const vec3_c &newPos) {
	if(surfs.size() == 0)
		surfs.resize(1);
	surfs[0].setVertexPos(vertexIndex,newPos);
}
void r_model_c::resizeIndices(u32 newNumIndices) {
	if(surfs.size() == 0)
		surfs.resize(1);
	surfs[0].resizeIndices(newNumIndices);
}
void r_model_c::setIndex(u32 indexNum, u32 value) {
	if(surfs.size() == 0)
		surfs.resize(1);
	surfs[0].setIndex(indexNum,value);
}
void r_model_c::addSprite(const class vec3_c &origin, float radius, class mtrAPI_i *mat, const axis_c &viewerAxis, byte alpha) {
	if(surfs.size() == 0)
		surfs.resize(1);
	u32 spriteNum = surfs[0].getNumTris()/2;
	surfs[0].initSprite(mat,radius,spriteNum);
	surfs[0].updateSprite(viewerAxis,origin,radius,spriteNum, alpha);
}
void r_model_c::addSurface(const char *matName, const simpleVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {	
	if(indices == 0) {
		g_core->RedWarning("r_model_c::addSurface: NULL indices16 pointer passed\n");
		return;
	}
	r_surface_c &newSF = surfs.pushBack();
	newSF.setSurface(matName,verts,numVerts,indices,numIndices);
}
void r_model_c::addSurface(const char *matName, const rVert_c *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
	if(indices == 0) {
		g_core->RedWarning("r_model_c::addSurface: NULL indices16 pointer passed\n");
		return;
	}
	r_surface_c &newSF = surfs.pushBack();
	newSF.setSurface(matName,verts,numVerts,indices,numIndices);
}
void r_model_c::setAllVertexColors(byte r, byte g, byte b, byte a) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->setAllVertexColors(r,g,b,a);
	}
}
void r_model_c::clear() {
	if(extraCollOctTree) {
		CMU_FreeTriSoupOctTree(extraCollOctTree);
		extraCollOctTree = 0;
	}
	if(ssvCaster) {
		delete ssvCaster;
		ssvCaster = 0;
	}
	for(u32 i = 0; i < bezierPatches.size(); i++) {
		delete bezierPatches[i];
	}
	surfs.clear();
}
void r_model_c::scaleXYZ(float scale) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->scaleXYZ(scale);
	}
	bounds.scaleBB(scale);
}
void r_model_c::swapYZ() {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->swapYZ();
	}
	bounds.swapYZ();
}
void r_model_c::swapIndexes() {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->swapIndexes();
	}
}
void r_model_c::translateY(float ofs) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->translateY(ofs);
	}
	bounds.translate(vec3_c(0,ofs,0));
}
void r_model_c::multTexCoordsY(float f) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->multTexCoordsY(f);
	}
}
void r_model_c::multTexCoordsXY(float f) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->multTexCoordsXY(f);
	}
}
void r_model_c::translateXYZ(const class vec3_c &ofs) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->translateXYZ(ofs);
	}
	bounds.translate(ofs);
}
void r_model_c::getCurrentBounds(class aabb &out) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->addPointsToBounds(out);
	}
}
void r_model_c::setAllSurfsMaterial(const char *newMatName) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->setMaterial(newMatName);
	}
}
u32 r_model_c::getNumSurfs() const {
	return surfs.size();
}
void r_model_c::setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName) {
	for(u32 i = 0; i < numSurfIndexes; i++) {
		u32 sfNum = surfIndexes[i];
		surfs[sfNum].setMaterial(newMatName);
	}
}
void r_model_c::addTriangleToSF(u32 surfNum, const struct simpleVert_s &v0,
	const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
	if(surfNum >= surfs.size()) {
		surfs.resize(surfNum+1);
	}
	surfs[surfNum].addTriangle(v0,v1,v2);
	bounds.addPoint(v0.xyz);
	bounds.addPoint(v1.xyz);
	bounds.addPoint(v2.xyz);
}
void r_model_c::setNumSurfs(u32 newSurfsCount) {
	surfs.resize(newSurfsCount);
}
void r_model_c::resizeSurfaceVerts(u32 surfNum, u32 numVerts) {
	if(surfNum >= surfs.size()) {
		g_core->RedWarning("r_model_c::resizeSurfaceVerts: bad surface index %i\n",surfNum);
		return;
	}
	surfs[surfNum].resizeVerts(numVerts);
}
void r_model_c::setSurfaceVert(u32 surfNum, u32 vertIndex, const float *xyz, const float *st) {
	if(surfNum >= surfs.size()) {
		g_core->RedWarning("r_model_c::setSurfaceVert: bad surface index %i\n",surfNum);
		return;
	}
	surfs[surfNum].setVertXYZTC(vertIndex,xyz,st[0],st[1]);
}
void r_model_c::setSurfaceIndicesU32(u32 surfNum, u32 numIndices, const u32 *indices) {
	if(surfNum >= surfs.size()) {
		g_core->RedWarning("r_model_c::setSurfaceIndicesU32: bad surface index %i\n",surfNum);
		return;
	}
	surfs[surfNum].setIndicesU32(numIndices,indices);
}
void r_model_c::setSurfaceMaterial(u32 surfNum, const char *matName) {
	if(surfNum >= surfs.size()) {
		g_core->RedWarning("r_model_c::setSurfaceMaterial: bad surface index %i\n",surfNum);
		return;
	}
	surfs[surfNum].setMaterial(matName);
}	
void r_model_c::recalcBoundingBoxes() {
	bounds.clear();
	for(u32 i = 0; i < surfs.size(); i++) {
		surfs[i].recalcBB();
		bounds.addBox(surfs[i].getBB());
	}
}
void r_model_c::addAbsTag(const char *newTagName, const class vec3_c &newPos, const class vec3_c &newAngles) { 
	tags.pushBack().setupTag(newTagName,newPos,newAngles);
}
void r_model_c::transform(const class matrix_c &mat) {
	for(u32 i = 0; i < surfs.size(); i++) {
		surfs[i].transform(mat);
	}
}
u32 r_model_c::countDuplicatedTriangles() const {
	u32 ret = 0;
	for(u32 i = 0; i < surfs.size(); i++) {
		ret += surfs[i].countDuplicatedTriangles();
	}
	return ret;
}
bool r_model_c::hasTriangle(u32 i0, u32 i1, u32 i2) const {
	if(surfs.size() == 0)
		return false;
	return surfs[0].hasTriangle(i0,i1,i2);
}
bool r_model_c::getTagOrientation(int tagNum, class matrix_c &out) const {
	if(tagNum < 0 || tagNum >= tags.size())
		return true;
	const r_tag_c &tag = tags[tagNum];
	out.fromAnglesAndOrigin(tag.getAngles(),tag.getOrigin());
	return false; // ok
}
void r_model_c::addPatch(class r_bezierPatch_c *newPatch) {
	// pretesselate the patch so we can get the valid bounding box
	newPatch->tesselate(3);
	bounds.addBox(newPatch->getBB());
	bezierPatches.push_back(newPatch);
}
void r_model_c::createVBOsAndIBOs() {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->createIBO();
		sf->createVBO();
	}
}
void r_model_c::addGeometryToColMeshBuilder(class colMeshBuilderAPI_i *out) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->addGeometryToColMeshBuilder(out);
	}
}
void r_model_c::initSkelModelInstance(const class skelModelAPI_i *skel) {
	surfs.resize(skel->getNumSurfs());
	r_surface_c *sf = surfs.getArray();
	this->mySkel = skel;
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		const skelSurfaceAPI_i *inSF = skel->getSurface(i);
		sf->initSkelSurfInstance(inSF);
	}
}
void r_model_c::updateSkelModelInstance(const class skelModelAPI_i *skel, const class boneOrArray_c &bones) {
	r_surface_c *sf = surfs.getArray();
	this->bounds.clear();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		const skelSurfaceAPI_i *inSF = skel->getSurface(i);
		sf->updateSkelSurfInstance(inSF,bones);
		this->bounds.addBox(sf->getBB());
	}
}
void r_model_c::initKeyframedModelInstance(const class kfModelAPI_i *kf) {
	u32 numSurfs = kf->getNumSurfaces();
	surfs.resize(numSurfs);
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < numSurfs; i++, sf++) {
		const kfSurfAPI_i *sfApi = kf->getSurfAPI(i);
		sf->initKeyframedSurfaceInstance(sfApi);
	}
}
void r_model_c::updateKeyframedModelInstance(const class kfModelAPI_i *kf, u32 frameNum) {
	u32 numSurfs = surfs.size();
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < numSurfs; i++, sf++) {
		const kfSurfAPI_i *sfApi = kf->getSurfAPI(i);
		sf->updateKeyframedSurfInstance(sfApi,frameNum);
	}
}
#include <api/q3PlayerModelDeclAPI.h>
#include <shared/tagOr.h>
void r_model_c::initQ3PlayerModelInstance(const q3PlayerModelAPI_i *qp) {
	u32 totalSurfs = qp->getNumTotalSurfaces();
	surfs.resize(totalSurfs);
	r_surface_c *sf = surfs.getArray();
	const kfModelAPI_i *legs = qp->getLegsModel();
	if(legs) {
		for(u32 i = 0; i < legs->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = legs->getSurfAPI(i);
			sf->initKeyframedSurfaceInstance(sfApi);
		}
	}
	const kfModelAPI_i *torso = qp->getTorsoModel();
	if(torso) {
		for(u32 i = 0; i < torso->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = torso->getSurfAPI(i);
			sf->initKeyframedSurfaceInstance(sfApi);
		}
	}
	const kfModelAPI_i *head = qp->getHeadModel();
	if(head) {
		for(u32 i = 0; i < head->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = head->getSurfAPI(i);
			sf->initKeyframedSurfaceInstance(sfApi);
		}
	}
}
void r_model_c::updateQ3PlayerModelInstance(const q3PlayerModelAPI_i *qp, u32 legsFrameNum, u32 torsoFrameNum) {
	u32 totalSurfs = qp->getNumTotalSurfaces();
	r_surface_c *sf = surfs.getArray();
	const kfModelAPI_i *legs = qp->getLegsModel();
	u32 fixedLegsFrameNum;
	u32 fixedTorsoFrameNum;
	if(legs) {
		fixedLegsFrameNum = legs->fixFrameNum(legsFrameNum);
		for(u32 i = 0; i < legs->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = legs->getSurfAPI(i);
			sf->updateKeyframedSurfInstance(sfApi,fixedLegsFrameNum);
		}
	}
	const kfModelAPI_i *torso = qp->getTorsoModel();
	matrix_c torsoTagMat;
	if(torso && legs) {
		// attach torso model to legs model tag
		const tagOr_c *torsoTag = legs->getTagOrientation("tag_torso",fixedLegsFrameNum);
		torsoTag->toMatrix(torsoTagMat);
		fixedTorsoFrameNum = torso->fixFrameNum(torsoFrameNum);
		for(u32 i = 0; i < torso->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = torso->getSurfAPI(i);
			sf->updateKeyframedSurfInstance(sfApi,fixedTorsoFrameNum);
			sf->transform(torsoTagMat);
		}
	}
	const kfModelAPI_i *head = qp->getHeadModel();
	if(head && torso && legs) {
		// attach head model to torso model tah
		const tagOr_c *headTagInTorsoSpace = torso->getTagOrientation("tag_head",fixedTorsoFrameNum);
		matrix_c headTagInTorsoSpaceMat;
		headTagInTorsoSpace->toMatrix(headTagInTorsoSpaceMat);
		matrix_c headInLegsSpace = torsoTagMat * headTagInTorsoSpaceMat;
		//u32 fixedFrameNum = head->fixFrameNum(frameNum);
		for(u32 i = 0; i < head->getNumSurfaces(); i++, sf++) {
			const kfSurfAPI_i *sfApi = head->getSurfAPI(i);
			sf->updateKeyframedSurfInstance(sfApi,0);
			sf->transform(headInLegsSpace);
		}
	}
}
void r_model_c::initSprite(class mtrAPI_i *newSpriteMaterial, float newSpriteRadius) {
	if(surfs.size() != 1)
		surfs.resize(1);
	surfs[0].initSprite(newSpriteMaterial, newSpriteRadius);
}
void r_model_c::updateSprite(const class axis_c &ax, float newSpriteRadius) {
	surfs[0].updateSprite(ax,vec3_c(0,0,0),newSpriteRadius);
}
mtrAPI_i *r_model_c::getMaterialForABSTriangleIndex(u32 absTriNum) const {
	u32 firstTri = 0;
	const r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		u32 lastTri = firstTri + sf->getNumTris();
		if(firstTri <= absTriNum && absTriNum < lastTri)
			return sf->getMat();
		firstTri += sf->getNumTris();
	}
	return 0;
}
int r_model_c::getSurfaceIndexForABSTriangleIndex(u32 absTriNum) const {
	u32 firstTri = 0;
	const r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		u32 lastTri = firstTri + sf->getNumTris();
		if(firstTri <= absTriNum && absTriNum < lastTri)
			return i;
		firstTri += sf->getNumTris();
	}
	return -1;
}
#include <shared/cmSurface.h>
#include <shared/autoCvar.h>

static aCvar_c r_useTriSoupOctTreesForRayTracing("r_useTriSoupOctTreesForRayTracing","1");
static aCvar_c r_useTriSoupOctTreesForDecalCreation("r_useTriSoupOctTreesForDecalCreation","1");
static aCvar_c r_minOctTreeTrisCount("r_minOctTreeTrisCount","2048");

void r_model_c::ensureExtraTrisoupOctTreeIsBuild() {
	if(extraCollOctTree)
		return;
	if(getTotalTriangleCount() > r_minOctTreeTrisCount.getInt()) {
		cmSurface_c sf;
		addGeometryToColMeshBuilder(&sf);
		extraCollOctTree = CMU_BuildTriSoupOctTree(sf);
	}
}

bool r_model_c::traceRay(class trace_c &tr, bool bAllowExtraOctTreeCreation) {
	if(tr.getTraceBounds().intersect(this->bounds) == false)
		return false;
	// bAllowExtraOctTreeCreation is set to false for dynamic, skeletal models
	if(r_useTriSoupOctTreesForRayTracing.getInt() && bAllowExtraOctTreeCreation) {
		ensureExtraTrisoupOctTreeIsBuild();
		if(extraCollOctTree) {
			if(extraCollOctTree->traceRay(tr)) {
				u32 absTriNum = tr.getHitTriangleIndex();
				mtrAPI_i *hitMat = this->getMaterialForABSTriangleIndex(absTriNum);
				int hitSurface = this->getSurfaceIndexForABSTriangleIndex(absTriNum);
				tr.setHitRMaterial(hitMat);
				tr.setHitSurfaceNum(hitSurface);
				return true;
			}
			return false;
		}
	}
	bool hit = false;
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(tr.getTraceBounds().intersect(sf->getBB()) == false) {
			continue;
		}
		if(sf->traceRay(tr)) {
			hit = true;
			tr.setHitSurfaceNum(i);
		}
	}
	return hit;
}
bool r_model_c::createDecalInternal(class decalProjector_c &proj) {
	// see if we can use extra octree structure to speed up
	// decal creation
	if(r_useTriSoupOctTreesForDecalCreation.getInt()) {
		ensureExtraTrisoupOctTreeIsBuild();
		if(extraCollOctTree) {
			u32 prev = proj.getNumCreatedWindings();
			extraCollOctTree->boxTriangles(proj.getBounds(),&proj);
			return proj.getNumCreatedWindings() != prev;
		}
	}

	bool hit = false;
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		//if(proj.getBounds().intersect(sf->getBB()) == false) {
		//	continue;
		//}
		if(sf->createDecalInternal(proj)) {
			hit = true;
		}
	}	
	return hit;
}
bool r_model_c::createDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
								   const class vec3_c &normal, float radius, class mtrAPI_i *material) {
	decalProjector_c proj;
	proj.init(pos,normal,radius);
	proj.setMaterial(material);
	
	bool hit = createDecalInternal(proj);

	// get results
	proj.addResultsToDecalBatcher(out);
	return hit;
}
r_surface_c *r_model_c::registerSurf(const char *matName) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(!Q_stricmp(sf->getMatName(),matName)) {
			return sf;
		}
	}	
	sf = &surfs.pushBack();
	sf->setMaterial(matName);
	return sf;
}
#include <renderer/rfSurfsFlagsArray.h>
#include <renderer/rfSurfFlags.h>
void r_model_c::addDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags, bool useVertexColors, const vec3_c *extraRGB) {
	if(extraSfFlags) {
		r_surface_c *sf = surfs.getArray();
		for(u32 i = 0; i < surfs.size(); i++, sf++) {
			if(rf_camera.isThirdPerson()) {
				if(extraSfFlags->getFlags(i).isFlag(RSF_HIDDEN_3RDPERSON)) {
					continue;
				}
			} else {
				if(extraSfFlags->getFlags(i).isFlag(RSF_HIDDEN_1STPERSON)) {
					continue;
				}
			}
			sf->addDrawCall(useVertexColors,extraRGB);
		}
	} else {
		r_surface_c *sf = surfs.getArray();
		for(u32 i = 0; i < surfs.size(); i++, sf++) {
			sf->addDrawCall(useVertexColors,extraRGB);
		}
	}
	for(u32 i = 0; i < bezierPatches.size(); i++) {
		bezierPatches[i]->addDrawCall();
	}
}
void r_model_c::setSurfMaterial(const char *surfName, const char *matName) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(!stricmp(sf->getName(),surfName)) {
		//if(!Q_stricmpn(sf->getName(),surfName,strlen(surfName))) {
			sf->setMaterial(matName);
		}
	}
}
void r_model_c::appendSkinRemap(const class rSkinRemap_c *skin) {
	if(skin == 0)
		return;
	for(u32 i = 0; i < skin->size(); i++) {
		const rSkinSurface_s &ss = skin->getSkinSurf(i);
		this->setSurfMaterial(ss.surfName,ss.matName);
	}
}
void r_model_c::boxSurfaces(const class aabb &bb, arraySTD_c<const r_surface_c*> &out) const {
	const r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(sf->getBB().intersect(bb)) {
			out.push_back(sf);
		}
	}
}
void r_model_c::calcVertexLightingLocal(const struct pointLightSample_s &sample) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->calcVertexLighting(sample);
	}
}
void r_model_c::calcVertexLightingABS(const class matrix_c &mat, const struct pointLightSample_s &sample) {
	pointLightSample_s localSample = sample;
	matrix_c inv = mat.getInversed();
	inv.transformNormal(localSample.lightDir);
	calcVertexLightingLocal(sample);
}	
void r_model_c::setAmbientLightingVec3_255(const vec3_c &color) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->setAmbientLightingVec3_255(color);
	}
}

#include "rf_lights.h"
void r_model_c::cacheLightStaticModelInteractions(class rLightImpl_c *light) {
	// TODO: handle models with non-identity orientations
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(sf->getBB().intersect(light->getABSBounds())) {
			if(sf->getMat()->isNeededForLightPass()==false)
				continue;
			light->addStaticModelSurfaceInteraction(/*this,*/sf);
		}
	}
	for(u32 i = 0; i < bezierPatches.size(); i++) {
		class r_bezierPatch_c *bp = bezierPatches[i];
		if(bp->getBB().intersect(light->getABSBounds())) {
			if(bp->getMat()->isNeededForLightPass()==false)
				continue;
			//light->addBezierPatchInteraction(/*this,*/bp);
			light->addStaticModelSurfaceInteraction((r_surface_c*)bp->getInstancePtr());
		}
	}
}
bool r_model_c::parseProcModel(class parser_c &p) {
	if(p.atWord("{")==false) {
		g_core->RedWarning("r_model_c::parseProcModel: expected '{' to follow \"model\" in file %s at line %i, found %s\n",
			p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	this->name = p.getToken();
	u32 numSurfs = p.getInteger();
	if(numSurfs) {
		this->bounds.clear();
		this->surfs.resize(numSurfs);
		for(u32 i = 0; i < numSurfs; i++) {
			r_surface_c &sf = surfs[i];
			if(sf.parseProcSurface(p))
				return true; // error occured while parsing the surface
			sf.recalcBB();
			this->bounds.addBox(sf.getBB());
		}
	}
	if(p.atWord("}")==false) {
		g_core->RedWarning("r_model_c::parseProcModel: expected closing '}' for \"model\" block in file %s at line %i, found %s\n",
			p.getDebugFileName(),p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	return false;; // OK
}
