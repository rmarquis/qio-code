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
#include "rf_surface.h"
#include "rf_drawCall.h"
#include <api/rbAPI.h>
#include <api/materialSystemAPI.h>
#include <api/mtrAPI.h>
#include <shared/trace.h>

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
}
void r_surface_c::addTriangle(const struct simpleVert_s &v0, const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
	indices.addIndex(verts.size());
	indices.addIndex(verts.size()+1);
	indices.addIndex(verts.size()+2);
	rVert_c nv;
	nv.xyz = v0.xyz;
	nv.tc = v0.tc;
	verts.push_back(nv);
	nv.xyz = v1.xyz;
	nv.tc = v1.tc;
	verts.push_back(nv);
	nv.xyz = v2.xyz;
	nv.tc = v2.tc;
	verts.push_back(nv);
	bounds.addPoint(v0.xyz);
	bounds.addPoint(v1.xyz);
	bounds.addPoint(v2.xyz);
}
void r_surface_c::resizeVerts(u32 newNumVerts) {
	verts.resize(newNumVerts);
}
void r_surface_c::setVert(u32 vertexIndex, const struct simpleVert_s &v) {
	rVert_c &rv = verts[vertexIndex];
	rv.xyz = v.xyz;
	rv.tc = v.tc;
}
void r_surface_c::resizeIndices(u32 newNumIndices) {
	// TODO: see if we can use u16 buffer here
	indices.initU32(newNumIndices);
}
void r_surface_c::setIndex(u32 indexNum, u32 value) {
	indices.setIndex(indexNum,value);
}
void r_surface_c::setMaterial(mtrAPI_i *newMat) {
	mat = newMat;
	matName = newMat->getName();
}
void r_surface_c::setMaterial(const char *newMatName) {
	matName = newMatName;
	mat = g_ms->registerMaterial(newMatName);
}
void r_surface_c::drawSurface() {
	rb->setBindVertexColors(true);
	rb->setMaterial(this->mat,this->lightmap);
	rb->drawElements(this->verts,this->indices);
	rb->setBindVertexColors(false);
}
void r_surface_c::drawSurfaceWithSingleTexture(class textureAPI_i *tex) {
	rb->setBindVertexColors(false);
	rb->drawElementsWithSingleTexture(this->verts,this->indices,tex);
}

void r_surface_c::addDrawCall() {
	RF_AddDrawCall(&this->verts,&this->indices,this->mat,this->lightmap,this->mat->getSort(),true);
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
		if(tr.clipByTriangle(v0.xyz,v1.xyz,v2.xyz,true)) {
			hasHit = true;
		}
	}
	return hasHit;
}	
void r_surface_c::scaleXYZ(float scale) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz *= scale;
	}
}
void r_surface_c::swapYZ() {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		float tmp = v->xyz.y;
		v->xyz.y = v->xyz.z;
		v->xyz.z = tmp;
	}
}
void r_surface_c::translateY(float ofs) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz.y += ofs;
	}
}
void r_surface_c::multTexCoordsY(float f) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->tc.y *= f;
	}
}
void r_surface_c::translateXYZ(const vec3_c &ofs) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		v->xyz += ofs;
	}
}
void r_surface_c::addPointsToBounds(aabb &out) {
	rVert_c *v = verts.getArray();
	for(u32 i = 0; i < verts.size(); i++, v++) {
		out.addPoint(v->xyz);
	}
}

//
//	r_model_c class
//
void r_model_c::addTriangle(const char *matName, const struct simpleVert_s &v0,
							const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
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
void r_model_c::scaleXYZ(float scale) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->scaleXYZ(scale);
	}
}
void r_model_c::swapYZ() {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->swapYZ();
	}
}
void r_model_c::translateY(float ofs) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->translateY(ofs);
	}
}
void r_model_c::multTexCoordsY(float f) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->multTexCoordsY(f);
	}
}
void r_model_c::translateXYZ(const class vec3_c &ofs) {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->translateXYZ(ofs);
	}
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
bool r_model_c::traceRay(class trace_c &tr) {
	if(tr.getTraceBounds().intersect(this->bounds) == false)
		return false;
	bool hit = false;
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		if(tr.getTraceBounds().intersect(sf->getBB()) == false) {
			continue;
		}
		if(sf->traceRay(tr)) {
			hit = true;
		}
	}
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
void r_model_c::addDrawCalls() {
	r_surface_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		sf->addDrawCall();
	}
}