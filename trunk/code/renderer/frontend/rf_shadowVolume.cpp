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
// rf_shadowVolume.cpp - shadow volume class for stencil shadows (Doom3-style)
#include "rf_shadowVolume.h"
#include "rf_drawCall.h"
#include "rf_local.h"
#include "rf_entities.h"
#include "rf_surface.h"
#include <math/plane.h>

void rIndexedShadowVolume_c::addTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2, const vec3_c &light) {
	plane_c triPlane;
	triPlane.fromThreePoints(p0,p1,p2);
	float d = triPlane.distance(light);
	if(d < 0)
		return;
const float shadowVolumeInf = 4096.f;
	vec3_c p0Projected = p0 - light;
	p0Projected.normalize();
	p0Projected *= shadowVolumeInf;
	p0Projected += p0;
	vec3_c p1Projected = p1 - light;
	p1Projected.normalize();
	p1Projected *= shadowVolumeInf;
	p1Projected += p1;
	vec3_c p2Projected = p2 - light;
	p2Projected.normalize();
	p2Projected *= shadowVolumeInf;
	p2Projected += p2;

	u32 i0 = points.registerVec3(p0);
	u32 i1 = points.registerVec3(p1);
	u32 i2 = points.registerVec3(p2);
	u32 pi0 = points.registerVec3(p0Projected);
	u32 pi1 = points.registerVec3(p1Projected);
	u32 pi2 = points.registerVec3(p2Projected);

	indices.addTriangle(i2,i1,i0);
	indices.addTriangle(pi0,pi1,pi2);
	indices.addQuad(i0,i1,pi0,pi1);
	indices.addQuad(i1,i2,pi1,pi2);
	indices.addQuad(i2,i0,pi2,pi0);
}
void rIndexedShadowVolume_c::addDrawCall() {
	RF_AddShadowVolumeDrawCall(&this->points,&this->indices);
}
void rIndexedShadowVolume_c::createShadowVolumeForEntity(class rEntityImpl_c *ent, const vec3_c &light) {
	const r_model_c *m = ent->getCurrentRModelInstance();
	if(m) {
		fromRModel(m,light);
	} else {
		
	}
}
void rIndexedShadowVolume_c::addRSurface(const class r_surface_c *sf, const vec3_c &light) {
	const rVertexBuffer_c &verts = sf->getVerts();
	const rIndexBuffer_c &indices = sf->getIndices();
	for(u32 i = 0; i < indices.getNumIndices(); i+=3){
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		const vec3_c &v0 = verts[i0].xyz;
		const vec3_c &v1 = verts[i1].xyz;
		const vec3_c &v2 = verts[i2].xyz;
		addTriangle(v0,v1,v2,light);
	}
}
void rIndexedShadowVolume_c::fromRModel(const class r_model_c *m, const vec3_c &light) {
	clear();
	for(u32 i = 0; i < m->getNumSurfs(); i++) {
		const r_surface_c *sf = m->getSurf(i);
		addRSurface(sf,light);
	}
}
void rEntityShadowVolume_c::addDrawCall() {
	rf_currentEntity = this->ent;
	this->data.addDrawCall();
}
